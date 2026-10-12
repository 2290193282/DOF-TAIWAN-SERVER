#include "old_game/activity/schedule/collect_box_dispatcher.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/localization/collect_box_script.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/event/db_collect_box.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/economy/inventory/item_timeout_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <stdint.h>
#include <vector>


namespace {

using collect_box_db::Message;

const uint8_t kErrorOperationFailed = 1u;
const uint8_t kErrorInventoryFull = 4u;
const uint8_t kErrorAlreadyRegisteredOrCannotExtend = 18u;
const uint8_t kErrorItemNotFound = 21u;
const uint8_t kErrorInvalidForOperation = 23u;
const uint8_t kErrorBoxNotFound = 95u;

struct Session {
    Session()
        : user_idx(-1), account_id(0u), charac_no(0u), server_group(-1),
          session_token(0u), next_request_token(1u), pending(false),
          recovery(false), cancel_error(kErrorOperationFailed), request(),
          client_request() {
        std::memset(&request, 0, sizeof(request));
        std::memset(&client_request, 0, sizeof(client_request));
    }

    int32_t user_idx;
    uint32_t account_id;
    uint32_t charac_no;
    int32_t server_group;
    uint32_t session_token;
    uint32_t next_request_token;
    bool pending;
    bool recovery;
    uint8_t cancel_error;
    Message request;
    // Preserve the actual client intent while `request` advances through
    // PREPARE -> FINALIZE/CANCEL or is replaced by a recovered old saga.
    Message client_request;
};

std::map<CUser*, Session> g_sessions;
uint32_t g_next_session_token = 1u;

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

bool IsBead(uint32_t item_id) {
    return collect_box_script::IsBead(item_id);
}

bool IsExtension(uint32_t item_id) {
    return collect_box_script::IsExtension(item_id, 0, 0);
}

uint32_t CharacterNumber(CUser* user) {
    const int value = user ? user->get_charac_no(-1) : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

bool IsReady(CUser* user) {
    private_store::CPrivateStoreMgr* const private_store =
        private_store::GetInstancePrivateStoreMgr();
    return collect_box_script::IsReady() && user &&
           user->get_state() == 3 && user->getCurCharacR() &&
           user->getCurCharacInvenR() && user->getCurCharacInvenW() &&
           !user->CheckInTrade() &&
           (!private_store || !private_store->IsBusyPrivateStore(user));
}

bool HasIdentity(CUser* user, const Session& session) {
    return IsReady(user) && session.session_token != 0u &&
           session.user_idx == user->GetUID() &&
           session.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           session.charac_no == CharacterNumber(user) &&
           session.server_group == user->GetServerGroup();
}

Session* CurrentSession(CUser* user, bool create) {
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() && HasIdentity(user, found->second)) {
        return &found->second;
    }
    if (found != g_sessions.end()) g_sessions.erase(found);
    if (!create || !IsReady(user) || user->GetUID() == -1 ||
        user->get_acc_id() <= 0 || CharacterNumber(user) == 0u) {
        return 0;
    }
    Session session;
    session.user_idx = user->GetUID();
    session.account_id = static_cast<uint32_t>(user->get_acc_id());
    session.charac_no = CharacterNumber(user);
    session.server_group = user->GetServerGroup();
    session.session_token = NextNonZero(g_next_session_token);
    return &g_sessions.insert(std::make_pair(user, session)).first->second;
}

void SendError(CUser* user, ENUM_CMDPACKET command, uint8_t error) {
    if (user) user->SendCmdErrorPacket(command, error);
}

void SendSelectSuccess(CUser* user, uint8_t box_id) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SELECT_COLLECTBOX);
    packet.put_byte(1u);
    packet.put_byte(box_id);
    packet.finalize(true);
    user->Send(packet);
}

void SendAddSuccess(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM);
    packet.put_byte(1u);
    packet.finalize(true);
    user->Send(packet);
}

void SendRemoveSuccess(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM);
    packet.put_byte(1u);
    packet.finalize(true);
    user->Send(packet);
}

void SendExtendSuccess(CUser* user, uint8_t box_id, uint32_t item_id) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE);
    packet.put_byte(1u);
    packet.put_byte(box_id);
    packet.put_int(static_cast<int>(item_id));
    packet.finalize(true);
    user->Send(packet);
}

uint32_t RemainingSeconds(const Message& state) {
    if (state.unlimited != 0u) return 0u;
    const long current = GlobalData::s_systemTime_.getCurSec();
    const uint32_t now = current > 0 ? static_cast<uint32_t>(current) : 0u;
    return collect_box_script::RemainingSeconds(state.expiry_at, now);
}

void SendState(CUser* user, const Message& state, bool refresh) {
    if (!user || !collect_box_script::IsReady() ||
        state.box_id != collect_box_script::BoxId() ||
        state.bead_count > collect_box_db::kMaximumBeadCount ||
        !collect_box_script::IsRawExpiry(state.expiry_at) ||
        state.unlimited > 1u) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_COLLECT_BOX);
    packet.put_byte(state.box_id);
    packet.put_byte(refresh ? 1u : 0u);
    packet.put_int(static_cast<int>(RemainingSeconds(state)));
    packet.put_byte(state.unlimited);
    packet.put_byte(state.bead_count);
    for (unsigned int index = 0u; index < state.bead_count; ++index) {
        packet.put_int(static_cast<int>(state.bead_items[index]));
    }
    packet.finalize(true);
    user->Send(packet);
}

bool IsLocked(CUser* user, const Inven_Item& item) {
    const int lock = item.amplify_option_.GetLock();
    if (!lock) return false;
    const item_lock::CItemLock* const data =
        static_cast<const item_lock::CItemLock*>(
            user->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return !data || data->CheckItemLock(static_cast<unsigned char>(lock));
}

bool ValidateSource(CUser* user,
                    uint16_t slot,
                    uint32_t item_id) {
    const CInventory* const inventory =
        user ? user->getCurCharacInvenR() : 0;
    const Inven_Item* const item =
        inventory && inventory->CheckValidSlot(1, slot) == 1
            ? inventory->GetInvenRef(1, slot)
            : 0;
    CItem* const prototype =
        item ? G_CDataManager()->find_item(item->item_id_) : 0;
    return item && static_cast<uint32_t>(item->item_id_) == item_id &&
           item->get_add_info() > 0 && prototype &&
           prototype->is_stackable() && !_checkTimeoutItem(*item) &&
           !IsLocked(user, *item);
}

bool MakeReturnedBead(uint32_t item_id, Inven_Item& item) {
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype || !prototype->is_stackable()) return false;
    item.item_id_ = static_cast<int>(item_id);
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int>(item_id);
    item.set_add_info(1);
    return !item.isEquipableItemType();
}

bool ReadSlotAndTotal(const CInventory* inventory,
                      uint16_t slot,
                      uint32_t item_id,
                      uint32_t& slot_count,
                      uint32_t& total_count) {
    if (!inventory || slot >= oldsrv::config::kMainInventoryItemCount ||
        item_id == 0u || item_id > 0x7fffffffu) {
        return false;
    }
    slot_count = 0u;
    total_count = 0u;
    bool slot_seen = false;
    for (int index = 0; index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        const Inven_Item* const item = inventory->GetInvenRef(1, index);
        if (!item) return false;
        const int count = item->get_add_info();
        if (item->item_id_ == static_cast<int32_t>(item_id)) {
            if (count <= 0 ||
                total_count > 0x7fffffffu - static_cast<uint32_t>(count)) {
                return false;
            }
            total_count += static_cast<uint32_t>(count);
        }
        if (index != static_cast<int>(slot)) continue;
        slot_seen = true;
        if (item->item_id_ == 0 && count == 0) {
            slot_count = 0u;
        } else if (item->item_id_ == static_cast<int32_t>(item_id) &&
                   count > 0) {
            slot_count = static_cast<uint32_t>(count);
        } else {
            return false;
        }
    }
    return slot_seen && total_count >= slot_count;
}

bool BuildDeleteProof(CUser* user,
                      uint16_t slot,
                      uint32_t item_id,
                      Message& request) {
    const CInventory* const inventory =
        user ? user->getCurCharacInvenR() : 0;
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    return ValidateSource(user, slot, item_id) &&
           ReadSlotAndTotal(inventory, slot, item_id,
                            request.slot_count_before,
                            request.total_count_before) &&
           request.slot_count_before != 0u && shadow.setCopy(*inventory) &&
           shadow.delete_item(static_cast<INVEN_TYPE>(1), slot, 1,
                              static_cast<eItemDelReason>(3), false) == 1 &&
           ReadSlotAndTotal(&shadow, slot, item_id,
                            request.slot_count_after,
                            request.total_count_after) &&
           request.slot_count_after + 1u == request.slot_count_before &&
           request.total_count_after + 1u == request.total_count_before;
}

bool BuildInsertProof(CUser* user,
                      uint32_t item_id,
                      Message& request) {
    const CInventory* const inventory =
        user ? user->getCurCharacInvenR() : 0;
    Inven_Item returned;
    if (!inventory || !MakeReturnedBead(item_id, returned)) return false;
    const int slot = inventory->tryInsertItemIntoInventory(returned);
    if (slot < 0 || slot >= oldsrv::config::kMainInventoryItemCount) {
        return false;
    }
    request.source_slot = static_cast<uint16_t>(slot);
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!ReadSlotAndTotal(inventory, request.source_slot, item_id,
                          request.slot_count_before,
                          request.total_count_before) ||
        !shadow.setCopy(*inventory)) {
        return false;
    }
    const int inserted = shadow.insertItemIntoInventory(
        returned, static_cast<eItemAddReason>(8), false, false);
    return inserted == slot &&
           ReadSlotAndTotal(&shadow, request.source_slot, item_id,
                            request.slot_count_after,
                            request.total_count_after) &&
           request.slot_count_after == request.slot_count_before + 1u &&
           request.total_count_after == request.total_count_before + 1u;
}

bool HasProofImage(CUser* user, const Message& message, bool after) {
    uint32_t slot_count = 0u;
    uint32_t total_count = 0u;
    return user && ReadSlotAndTotal(user->getCurCharacInvenR(),
                                   message.source_slot, message.item_id,
                                   slot_count, total_count) &&
           slot_count == (after ? message.slot_count_after
                                : message.slot_count_before) &&
           total_count == (after ? message.total_count_after
                                 : message.total_count_before);
}

bool ApplyMutation(CUser* user,
                   const Message& message,
                   bool& definitely_pre_image) {
    definitely_pre_image = false;
    if (!HasProofImage(user, message, false)) return false;
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) return false;
    if (message.saga_operation == collect_box_db::kOperationPrepareAdd ||
        message.saga_operation ==
            collect_box_db::kOperationPrepareExtend) {
        const Inven_Item* const source =
            inventory->GetInvenRef(1, message.source_slot);
        Inven_Item snapshot;
        if (!source || !ValidateSource(user, message.source_slot,
                                       message.item_id)) {
            definitely_pre_image = HasProofImage(user, message, false);
            return false;
        }
        snapshot.setCopy(*source);
        if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                   message.source_slot, 1,
                                   static_cast<eItemDelReason>(3), true) != 1) {
            definitely_pre_image = HasProofImage(user, message, false);
            return false;
        }
        if (!HasProofImage(user, message, true)) {
            (void)inventory->update_item(static_cast<INVEN_TYPE>(1),
                                         message.source_slot, snapshot);
            definitely_pre_image = HasProofImage(user, message, false);
            return false;
        }
    } else if (message.saga_operation ==
               collect_box_db::kOperationPrepareRemove) {
        Inven_Item returned;
        const Inven_Item* const target =
            inventory->GetInvenRef(1, message.source_slot);
        Inven_Item snapshot;
        if (!target || !MakeReturnedBead(message.item_id, returned)) {
            definitely_pre_image = HasProofImage(user, message, false);
            return false;
        }
        snapshot.setCopy(*target);
        const int inserted = inventory->insertItemIntoInventory(
            returned, static_cast<eItemAddReason>(8), true, true);
        if (inserted != static_cast<int>(message.source_slot) ||
            !HasProofImage(user, message, true)) {
            if (inserted >= 0 &&
                inserted < oldsrv::config::kMainInventoryItemCount) {
                (void)inventory->delete_item(
                    static_cast<INVEN_TYPE>(1), inserted, 1,
                    static_cast<eItemDelReason>(3), false);
            }
            (void)inventory->update_item(static_cast<INVEN_TYPE>(1),
                                         message.source_slot, snapshot);
            definitely_pre_image = HasProofImage(user, message, false);
            return false;
        }
    } else {
        return false;
    }
    return HasProofImage(user, message, true);
}

void ClearProjection(Message& request) {
    request.expiry_at = 0u;
    request.unlimited = 0u;
    request.bead_count = 0u;
    std::memset(request.bead_items, 0, sizeof(request.bead_items));
}

bool QueueDirect(CUser* user, Session& session, Message& request) {
    if (session.pending ||
        !collect_box_db::DB_CollectBox::Queue(user->GetUID(),
                                                       request)) {
        return false;
    }
    session.request = request;
    if (request.operation >= collect_box_db::kOperationSelect &&
        request.operation <= collect_box_db::kOperationPrepareExtend &&
        request.client_reply_expected != 0u) {
        session.client_request = request;
    }
    session.pending = true;
    return true;
}

bool AttachClientSelectToBackgroundLoad(Session& session) {
    if (!session.pending ||
        session.request.operation != collect_box_db::kOperationSelect ||
        session.request.client_reply_expected != 0u) {
        return false;
    }
    // The post-NOTI124 preload and an immediately opened CollectBox share the
    // same authoritative SELECT.  Do not queue a second DB request or reject
    // CMD690 merely because the asynchronous preload reply is still in flight.
    session.client_request = session.request;
    session.client_request.client_reply_expected = 1u;
    return true;
}

bool QueueFinalizeFence(CUser* user, Session& session, const Message& claim) {
    StreamPool* const pool = GlobalData::stream_pool();
    if (!pool || session.pending) return false;
    Message request = claim;
    request.operation = collect_box_db::kOperationFinalize;
    request.result = collect_box_db::kResultRequest;
    request.phase = collect_box_db::kPhaseNone;
    ClearProjection(request);

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(collect_box_db::kDbSignal);
    *guard << user->GetUID();
    Message* const output = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!output) return false;
    *output = request;
    if (!legacy_feature_fence::QueueAfterLegacy(
            legacy_feature_fence::kFenceCollectBoxFinalize, guard)) {
        return false;
    }
    session.request = request;
    session.pending = true;
    return true;
}

bool QueueCancel(CUser* user,
                 Session& session,
                 const Message& claim,
                 uint8_t error) {
    Message request = claim;
    request.operation = collect_box_db::kOperationCancel;
    request.result = collect_box_db::kResultRequest;
    request.phase = collect_box_db::kPhaseNone;
    ClearProjection(request);
    session.cancel_error = error;
    return QueueDirect(user, session, request);
}

void InitializeRequest(CUser* user,
                       Session& session,
                       uint8_t operation,
                       ENUM_CMDPACKET command,
                       Message& request) {
    std::memset(&request, 0, sizeof(request));
    request.user_idx = user->GetUID();
    request.account_id = static_cast<uint32_t>(user->get_acc_id());
    request.charac_no = CharacterNumber(user);
    request.server_group = user->GetServerGroup();
    request.session_token = session.session_token;
    request.request_token = NextNonZero(session.next_request_token);
    request.operation = operation;
    request.saga_operation = operation;
    request.result = collect_box_db::kResultRequest;
    request.phase = collect_box_db::kPhaseNone;
    request.box_id = collect_box_script::BoxId();
    request.client_command = static_cast<uint16_t>(command);
    request.client_reply_expected = 1u;
}

bool ValidStateProjection(const Message& response) {
    if (response.bead_count > collect_box_db::kMaximumBeadCount ||
        response.unlimited > 1u ||
        (response.expiry_at != 0u &&
         !collect_box_script::IsRawExpiry(response.expiry_at))) {
        return false;
    }
    std::vector<uint32_t> items;
    for (unsigned int index = 0u; index < response.bead_count; ++index) {
        if (!IsBead(response.bead_items[index]) ||
            std::find(items.begin(), items.end(),
                      response.bead_items[index]) != items.end()) {
            return false;
        }
        items.push_back(response.bead_items[index]);
    }
    for (unsigned int index = response.bead_count;
         index < collect_box_db::kMaximumBeadCount; ++index) {
        if (response.bead_items[index] != 0u) return false;
    }
    return true;
}

bool MatchResponse(CUser* user,
                   const Message& response,
                   Session*& session) {
    session = CurrentSession(user, false);
    return session && session->pending &&
           response.user_idx == user->GetUID() &&
           response.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           response.charac_no == CharacterNumber(user) &&
           response.server_group == user->GetServerGroup() &&
           response.session_token == session->session_token &&
           response.request_token == session->request.request_token &&
           response.operation == session->request.operation &&
           response.box_id == collect_box_script::BoxId() &&
           response.reserved[0] == 0u &&
           response.reserved[1] == 0u &&
           response.result <= collect_box_db::kResultInventoryNotDurable &&
           response.phase <= collect_box_db::kPhaseCancelled &&
           ValidStateProjection(response);
}

ENUM_CMDPACKET SagaCommand(uint8_t operation) {
    if (operation == collect_box_db::kOperationPrepareAdd) {
        return ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM;
    }
    if (operation == collect_box_db::kOperationPrepareRemove) {
        return ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM;
    }
    return ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE;
}

uint8_t InvalidStateError(const Message& response) {
    if (response.operation == collect_box_db::kOperationPrepareAdd) {
        for (unsigned int index = 0u; index < response.bead_count; ++index) {
            if (response.bead_items[index] == response.item_id) {
                return kErrorAlreadyRegisteredOrCannotExtend;
            }
        }
        return kErrorInvalidForOperation;
    }
    if (response.operation == collect_box_db::kOperationPrepareRemove) {
        return kErrorItemNotFound;
    }
    return kErrorAlreadyRegisteredOrCannotExtend;
}

void FailCurrent(CUser* user, Session& session, uint8_t error) {
    if (session.client_request.client_reply_expected != 0u) {
        SendError(user,
                  static_cast<ENUM_CMDPACKET>(
                      session.client_request.client_command),
                  error);
    }
    session.pending = false;
    session.recovery = false;
}

bool PersistAndFence(CUser* user, Session& session, const Message& claim) {
    // signal35 is queued first.  signal823 is then queued to the same DB_Q;
    // its strict wrapper forwards FINALIZE to queue6 only after signal35 has
    // executed.  Queue6 still verifies the decompressed authoritative image,
    // so an SQL failure cannot clear the pending journal.
    if (user->SaveInventory() != 1) return false;
    session.pending = false;
    return QueueFinalizeFence(user, session, claim);
}

void HandleReserved(CUser* user,
                    Session& session,
                    const Message& response,
                    bool recovery) {
    session.pending = false;
    session.recovery = recovery;

    bool post_image = HasProofImage(user, response, true);
    if (!post_image) {
        if (!HasProofImage(user, response, false)) {
            // This process has not applied the reserved mutation.  A cancel
            // is accepted only if queue6 independently observes the exact
            // durable pre-image; otherwise the journal remains pending.
            if (!QueueCancel(user, session, response,
                             kErrorOperationFailed)) {
                FailCurrent(user, session, kErrorOperationFailed);
            }
            return;
        }
        bool definitely_pre_image = false;
        if (!ApplyMutation(user, response, definitely_pre_image)) {
            if (!definitely_pre_image ||
                !QueueCancel(user, session, response,
                             response.saga_operation ==
                                     collect_box_db::kOperationPrepareRemove
                                 ? kErrorInventoryFull
                                 : kErrorItemNotFound)) {
                FailCurrent(user, session, kErrorOperationFailed);
            }
            return;
        }
        post_image = true;
    }

    if (!post_image || !PersistAndFence(user, session, response)) {
        FailCurrent(user, session, kErrorOperationFailed);
    }
}

void HandleCommitted(CUser* user,
                     Session& session,
                     const Message& response) {
    const ENUM_CMDPACKET current =
        static_cast<ENUM_CMDPACKET>(
            session.client_request.client_command);
    const ENUM_CMDPACKET saga = SagaCommand(response.saga_operation);
    const bool same_client_mutation =
        session.client_request.operation == response.saga_operation &&
        session.client_request.item_id == response.item_id &&
        (response.saga_operation ==
                 collect_box_db::kOperationPrepareRemove ||
         session.client_request.source_slot == response.source_slot);
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0),
                         response.source_slot);

    if (session.client_request.client_reply_expected != 0u) {
        if (session.recovery &&
            current != ENUM_CMDPACKET_SELECT_COLLECTBOX &&
            (!same_client_mutation || current != saga)) {
            SendError(user, current, kErrorOperationFailed);
        } else if (current == ENUM_CMDPACKET_SELECT_COLLECTBOX) {
            SendSelectSuccess(user, response.box_id);
        } else if (saga == ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM) {
            SendAddSuccess(user);
        } else if (saga == ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM) {
            SendRemoveSuccess(user);
        } else {
            SendExtendSuccess(user, response.box_id, response.item_id);
        }
    }
    SendState(user, response, true);
    session.pending = false;
    session.recovery = false;
}

}  // namespace


namespace collect_box {

void BeginSession(CUser* user) {
    Session* const session = CurrentSession(user, true);
    if (!session || session->pending) return;
    Message request;
    InitializeRequest(user, *session,
                      collect_box_db::kOperationSelect,
                      ENUM_CMDPACKET_SELECT_COLLECTBOX, request);
    request.client_reply_expected = 0u;
    if (!QueueDirect(user, *session, request)) g_sessions.erase(user);
}

void ResetSession(CUser* user) {
    g_sessions.erase(user);
}

void ApplyDatabaseResult(CUser* user,
                         const collect_box_db::Message& response) {
    Session* session = 0;
    if (!MatchResponse(user, response, session)) return;

    if (response.result == collect_box_db::kResultDatabaseFailure ||
        response.result ==
            collect_box_db::kResultInventoryNotDurable) {
        FailCurrent(user, *session, kErrorOperationFailed);
        return;
    }

    if (response.result == collect_box_db::kResultPendingRecovery &&
        response.phase == collect_box_db::kPhaseReserved &&
        response.saga_id != 0u) {
        HandleReserved(user, *session, response, true);
        return;
    }

    if (response.operation == collect_box_db::kOperationSelect) {
        if (response.result == collect_box_db::kResultSuccess &&
            response.phase == collect_box_db::kPhaseNone) {
            if (session->client_request.client_reply_expected != 0u) {
                SendSelectSuccess(user, response.box_id);
            }
            SendState(user, response, true);
            session->pending = false;
        } else {
            FailCurrent(user, *session, kErrorOperationFailed);
        }
        return;
    }

    if (response.operation >= collect_box_db::kOperationPrepareAdd &&
        response.operation <=
            collect_box_db::kOperationPrepareExtend) {
        if (response.result == collect_box_db::kResultInvalidState) {
            FailCurrent(user, *session, InvalidStateError(response));
        } else if (response.result == collect_box_db::kResultSuccess &&
                   response.phase == collect_box_db::kPhaseReserved &&
                   response.saga_id != 0u &&
                   response.saga_operation == response.operation) {
            HandleReserved(user, *session, response, false);
        } else {
            FailCurrent(user, *session, kErrorOperationFailed);
        }
        return;
    }

    if (response.operation == collect_box_db::kOperationFinalize) {
        if (response.result == collect_box_db::kResultSuccess &&
            response.phase == collect_box_db::kPhaseCommitted) {
            HandleCommitted(user, *session, response);
        } else {
            FailCurrent(user, *session, kErrorOperationFailed);
        }
        return;
    }

    if (response.operation == collect_box_db::kOperationCancel) {
        const uint8_t error = session->cancel_error;
        if (response.result == collect_box_db::kResultSuccess &&
            response.phase == collect_box_db::kPhaseCancelled) {
            FailCurrent(user, *session, error);
        } else {
            FailCurrent(user, *session, kErrorOperationFailed);
        }
    }
}

}  // namespace collect_box_0828


Dispatcher_SelectCollectBox::Dispatcher_SelectCollectBox()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_SelectCollectBox::~Dispatcher_SelectCollectBox() {}

int Dispatcher_SelectCollectBox::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint8_t box_id = 0u;
    if (packet.get_byte(box_id) != 1) return 0;
    if (!collect_box_script::IsReady() ||
        box_id != collect_box_script::BoxId()) {
        SendError(user, ENUM_CMDPACKET_SELECT_COLLECTBOX, kErrorBoxNotFound);
        return 0;
    }
    Session* const session = CurrentSession(user, true);
    if (!session) {
        SendError(user, ENUM_CMDPACKET_SELECT_COLLECTBOX,
                  kErrorOperationFailed);
        return 0;
    }
    if (session->pending) {
        if (!AttachClientSelectToBackgroundLoad(*session)) {
            SendError(user, ENUM_CMDPACKET_SELECT_COLLECTBOX,
                      kErrorOperationFailed);
        }
        return 0;
    }
    Message request;
    InitializeRequest(user, *session,
                      collect_box_db::kOperationSelect,
                      ENUM_CMDPACKET_SELECT_COLLECTBOX, request);
    if (!QueueDirect(user, *session, request)) {
        SendError(user, ENUM_CMDPACKET_SELECT_COLLECTBOX,
                  kErrorOperationFailed);
    }
    return 0;
}

int Dispatcher_SelectCollectBox::read(PacketBuf&, MSG_BASE&) { return 0; }
int Dispatcher_SelectCollectBox::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}


Dispatcher_AddCollectBoxItem::Dispatcher_AddCollectBoxItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_AddCollectBoxItem::~Dispatcher_AddCollectBoxItem() {}

int Dispatcher_AddCollectBoxItem::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    uint8_t box_id = 0u;
    uint8_t item_space = 0u;
    uint16_t slot = 0u;
    uint32_t item_id = 0u;
    if (packet.get_byte(box_id) != 1 || packet.get_byte(item_space) != 1 ||
        packet.get_short(slot) != 1 || packet.get_int(item_id) != 1) {
        return 0;
    }
    if (!collect_box_script::IsReady() ||
        box_id != collect_box_script::BoxId()) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorBoxNotFound);
        return 0;
    }
    if (item_space != 0u) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorItemNotFound);
        return 0;
    }
    if (!IsBead(item_id)) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorInvalidForOperation);
        return 0;
    }
    Session* const session = CurrentSession(user, true);
    if (!session || session->pending) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorOperationFailed);
        return 0;
    }
    Message request;
    InitializeRequest(user, *session,
                      collect_box_db::kOperationPrepareAdd,
                      ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM, request);
    request.item_id = item_id;
    request.source_slot = slot;
    if (!BuildDeleteProof(user, slot, item_id, request)) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorItemNotFound);
        return 0;
    }
    if (!QueueDirect(user, *session, request)) {
        SendError(user, ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM,
                  kErrorOperationFailed);
    }
    return 0;
}

int Dispatcher_AddCollectBoxItem::read(PacketBuf&, MSG_BASE&) { return 0; }
int Dispatcher_AddCollectBoxItem::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}


Dispatcher_RemoveCollectBoxItem::Dispatcher_RemoveCollectBoxItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_RemoveCollectBoxItem::~Dispatcher_RemoveCollectBoxItem() {}

int Dispatcher_RemoveCollectBoxItem::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    uint8_t box_id = 0u;
    uint32_t item_id = 0u;
    if (packet.get_byte(box_id) != 1 || packet.get_int(item_id) != 1) {
        return 0;
    }
    if (!collect_box_script::IsReady() ||
        box_id != collect_box_script::BoxId()) {
        SendError(user, ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM,
                  kErrorBoxNotFound);
        return 0;
    }
    if (!IsBead(item_id)) {
        SendError(user, ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM,
                  kErrorItemNotFound);
        return 0;
    }
    Session* const session = CurrentSession(user, true);
    if (!session || session->pending) {
        SendError(user, ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM,
                  kErrorOperationFailed);
        return 0;
    }
    Message request;
    InitializeRequest(user, *session,
                      collect_box_db::kOperationPrepareRemove,
                      ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM, request);
    request.item_id = item_id;
    if (!BuildInsertProof(user, item_id, request)) {
        SendError(user, ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM,
                  kErrorInventoryFull);
        return 0;
    }
    if (!QueueDirect(user, *session, request)) {
        SendError(user, ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM,
                  kErrorOperationFailed);
    }
    return 0;
}

int Dispatcher_RemoveCollectBoxItem::read(PacketBuf&, MSG_BASE&) { return 0; }
int Dispatcher_RemoveCollectBoxItem::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}


Dispatcher_ExtendCollectBoxExpiryDate::Dispatcher_ExtendCollectBoxExpiryDate()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_ExtendCollectBoxExpiryDate::~Dispatcher_ExtendCollectBoxExpiryDate() {}

int Dispatcher_ExtendCollectBoxExpiryDate::dispatch_sig(CUser* user,
                                                         PacketBuf& packet) {
    uint8_t box_id = 0u;
    uint8_t item_space = 0u;
    uint16_t slot = 0u;
    uint32_t item_id = 0u;
    if (packet.get_byte(box_id) != 1 || packet.get_byte(item_space) != 1 ||
        packet.get_short(slot) != 1 || packet.get_int(item_id) != 1) {
        return 0;
    }
    if (!collect_box_script::IsReady() ||
        box_id != collect_box_script::BoxId()) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorBoxNotFound);
        return 0;
    }
    if (item_space != 0u) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorItemNotFound);
        return 0;
    }
    if (!IsExtension(item_id)) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorInvalidForOperation);
        return 0;
    }
    Session* const session = CurrentSession(user, true);
    if (!session || session->pending) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorOperationFailed);
        return 0;
    }
    Message request;
    InitializeRequest(user, *session,
                      collect_box_db::kOperationPrepareExtend,
                      ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE, request);
    request.item_id = item_id;
    request.source_slot = slot;
    if (!BuildDeleteProof(user, slot, item_id, request)) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorItemNotFound);
        return 0;
    }
    if (!QueueDirect(user, *session, request)) {
        SendError(user, ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE,
                  kErrorOperationFailed);
    }
    return 0;
}

int Dispatcher_ExtendCollectBoxExpiryDate::read(PacketBuf&, MSG_BASE&) {
    return 0;
}
int Dispatcher_ExtendCollectBoxExpiryDate::process(CUser*, MSG_BASE&,
                                                    ParamBase&) {
    return 0;
}
