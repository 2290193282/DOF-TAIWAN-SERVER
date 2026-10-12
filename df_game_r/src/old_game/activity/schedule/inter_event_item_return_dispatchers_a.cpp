#include "old_game/activity/schedule/inter_event_item_return_dispatchers_a.h"

#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/activity/schedule/event_login_item_runtime.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

const uint32_t kEventItemWebPayloadSize = 0x22u;
const uint32_t kEventItemLoadHeaderSize = 0x0cu;
const uint32_t kEventItemLoadRowSize = 0x10u;
const uint32_t kMaximumEventItemRows = 5u;

uint32_t ReadU32(const char* data, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

// The legacy SIG_SAVE_EVENTITEM specialization reserves precisely 25 bytes.
// Keep the old first dword opaque: DB_SaveEventItem only forwards its scalar
// companions and never treats it as a C++ object.
#pragma pack(push, 1)
struct EventItemSaveRequest {
    uint32_t event_context;  // +0x00
    uint32_t charac_no;      // +0x04
    uint32_t event_code;     // +0x08
    uint32_t item_id;        // +0x0c
    uint32_t stack_count;    // +0x10
    uint32_t event_source;   // +0x14
    uint8_t no_persist;      // +0x18
};
#pragma pack(pop)

static_assert(sizeof(EventItemSaveRequest) == 0x19u,
              "SIG_SAVE_EVENTITEM wire body must remain 25 bytes");

// IDA data 0x93ACE60: eleven fixed 18-byte event strings.
const char kEventNames[11][18] = {
    "",
    "Yegam",
    "ATI Coupon",
    "DNF Guide book",
    "Everland",
    "Muz",
    "King of Alttle",
    "Free Charge Combo",
    "",
    "",
    "",
};

// IDA data 0x93ACF20: ten fixed 11-byte IPG program codes.
const char kEventIPGCodes[10][11] = {
    "",
    "",
    "IP00000001",
    "IP00000002",
    "",
    "",
    "",
    "",
    "",
    "",
};

}  // namespace

// IDA_EA: 0x84CEFD2
// IDA_NAME: _ZN28Inter_MonitorEventItemUpdate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorEventItemUpdate::dispatch_sig(CUser *,char *,int)
int Inter_MonitorEventItemUpdate::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)user;

    // The legacy sender reads six unaligned dwords from +0x0a through +0x1e.
    if (!payload || payload_size < static_cast<int>(kEventItemWebPayloadSize)) {
        return 0;
    }

    WongWork::CWebEvent::RecvInfoFromWeb(
        ReadU32(payload, 0x0a),  // opaque event context
        ReadU32(payload, 0x0e),  // character number
        ReadU32(payload, 0x16),  // item id
        ReadU32(payload, 0x1a),  // stack count
        ReadU32(payload, 0x12),  // event code
        ReadU32(payload, 0x1e)); // source
    return 0;
}

// IDA_EA: 0x84CF0C4
// IDA_NAME: _ZN19Inter_LoadEventItem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadEventItem::dispatch_sig(CUser *,char *,int)
int Inter_LoadEventItem::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    (void)user;

    if (!payload || payload_size < static_cast<int>(kEventItemLoadHeaderSize)) {
        return 0;
    }

    const unsigned int account_id = ReadU32(payload, 0);
    CUser* const owner = G_CGameManager()->GetUserByAccId(account_id);
    if (!owner) {
        return 0;
    }

    const unsigned int charac_no = ReadU32(payload, 4);
    const unsigned int row_count = ReadU32(payload, 8);
    // EventCommon::LoadCharacEventItem uses SQL LIMIT 5 and emits one
    // SIG_LOAD_EVENTITEM (0x5c) record.  Refuse malformed data before the
    // old six-dword completion array could be overrun.
    if (row_count > kMaximumEventItemRows ||
        payload_size < static_cast<int>(kEventItemLoadHeaderSize +
                                        row_count * kEventItemLoadRowSize)) {
        return 0;
    }

    uint32_t completed[6] = {};
    for (uint32_t index = 0; index < row_count; ++index) {
        const int offset = static_cast<int>(kEventItemLoadHeaderSize +
                                            index * kEventItemLoadRowSize);
        const uint32_t event_item_id = ReadU32(payload, offset + 0);
        const uint32_t item_id = ReadU32(payload, offset + 4);
        const uint32_t stack_count = ReadU32(payload, offset + 8);
        const uint32_t event_code = ReadU32(payload, offset + 12);

        bool skip = false;
        if (item_id != 999u &&
            (owner->get_state() <= 2 ||
             owner->getCurCharacNo() != static_cast<int>(charac_no))) {
            skip = true;
        }

        if (!skip && WongWork::CWebEvent::RecvInfoFromDB(
                         owner, item_id, stack_count, event_code,
                         event_item_id)) {
            completed[++completed[0]] = event_item_id;
        }
    }

    if (completed[0] != 0) {
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("InterDispatcher.cpp", 11295),
            true);
        guard << 77;
        guard << -1;
        guard.put_binary(completed, sizeof(completed));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }
    return 0;
}

// IDA_EA: 0x86C11D0
// IDA_NAME: _ZN8WongWork9CWebEvent15RecvInfoFromWebEjjjjjj
// DEMANGLED: WongWork::CWebEvent::RecvInfoFromWeb(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int)
int WongWork::CWebEvent::RecvInfoFromWeb(unsigned int event_context,
                                         unsigned int charac_no,
                                         unsigned int item_id,
                                         unsigned int stack_count,
                                         unsigned int event_code,
                                         unsigned int event_source) {
    if (item_id == 999u) {
        return 0;
    }
    return ReqSaveEventItem(event_context,
                            charac_no,
                            item_id,
                            stack_count,
                            event_code,
                            event_source,
                            false);
}

// IDA_EA: 0x86C121E
// IDA_NAME: _ZN8WongWork9CWebEvent14RecvInfoFromDBEP5CUserjjjj
// DEMANGLED: WongWork::CWebEvent::RecvInfoFromDB(CUser *,unsigned int,unsigned int,unsigned int,unsigned int)
int WongWork::CWebEvent::RecvInfoFromDB(CUser* user,
                                        unsigned int item_id,
                                        unsigned int stack_count,
                                        unsigned int event_code,
                                        unsigned int event_item_id) {
    if (item_id == 1u) {
        const int current_coin = user->getCurCharacInvenR()->GetEventCoin();
        user->getCurCharacInvenW()->SetEventCoin(
            static_cast<unsigned int>(current_coin +
                                      static_cast<int>(stack_count)));
        user->history_log().EventCoinAdd(
            user->getCurCharacInvenR()->GetEventCoin(),
            static_cast<int>(stack_count),
            static_cast<eCoinAddReason>(2));
        user->SaveMoneyCoin();
        user->SendCoin();
        return 1;
    }

    if (item_id == 999u) {
        char price[12] = {};
        std::snprintf(price, sizeof(price), "%d",
                      static_cast<int>(stack_count));

        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("WebEvent.cpp", 71), false);
        guard << static_cast<int>(event_code);
        guard << static_cast<int>(event_item_id);

        const char* const ipg_code = _GetEventIPGCode(
            static_cast<int>(event_code));
        if (!ipg_code) {
            GlobalData::s_stream_pool->Free(guard);
            return 1;
        }

        char unique_no[20] = {};
        CGenUniqueNo::GenUniqueNo(unique_no);
        // The old call is CIPGHelper vtable slot 0 (IPGInput), which takes
        // ownership of the callback stream on the asynchronous success path.
        GlobalData::s_pIPGHelper->IPGInput(
            user,
            ipg_code,
            price,
            unique_no,
            _GetEventString(static_cast<int>(event_code)),
            user->getCurCharacName(),
            guard.Get());
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!item) {
        return 0;
    }

    Inven_Item inventory_item;
    item->vtable_->make_item(item, inventory_item);
    if (inventory_item.item_category_ == 1u) {
        if (stack_count > 1u) {
            inventory_item.add_info_ = static_cast<int>(stack_count);
        }
    } else {
        inventory_item.add_info_ = static_cast<int>(stack_count);
    }
    inventory_item.item_id_ = static_cast<int32_t>(item_id);

    eItemAddReason add_reason = static_cast<eItemAddReason>(13);
    if (event_code == 100001u) {
        add_reason = static_cast<eItemAddReason>(14);
    } else if (event_code == 200006u) {
        add_reason = static_cast<eItemAddReason>(20);
    }

    if (item->vtable_->is_avatar_item(item)) {
        char ipg_no[20] = {};
        std::snprintf(ipg_no, sizeof(ipg_no), "E_%s",
                      _GetEventString(static_cast<int>(event_code)));
        const int slot = user->getCurCharacInvenW()->AddAvatarItem(
            static_cast<int>(item_id),
            0,
            false,
            static_cast<char>(_GetATIAvatarAbility(static_cast<int>(item_id))),
            static_cast<char>(-1),
            ipg_no,
            static_cast<eAvatarItemAddReason>(2),
            false,
            0);
        if (slot == -1) {
            return 0;
        }
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(1),
                                 slot);
        return 1;
    }

    int special_type = 0;
    int special_value = 0;
    bool unique = false;
    if (GlobalData::s_pCeraShop->IsSpecialItem(
            inventory_item.item_id_, special_type, special_value, unique)) {
        int processing_result = 0;
        const int restrict_code =
            G_CDataManager()->get_item_list()->GetRestrictCode(
                inventory_item.item_id_);
        const long current_seconds = GlobalData::s_systemTime_.getCurSec();
        std::vector<Inven_Item> output_items;
        CCeraShop::stCeraShopItemParam_t item_param;
        if (GlobalData::s_pCeraShop->ProcessSpecialItem(
                user,
                inventory_item.item_id_,
                restrict_code,
                processing_result,
                current_seconds,
                true,
                output_items,
                item_param,
                0,
                0,
                false,
                1)) {
            if (processing_result != 0) {
                return 0;
            }
            if (restrict_code != 0 && !user->IsRestrictedGoods(restrict_code)) {
                user->SetRestrictedGoods(restrict_code);
                GlobalData::s_pCeraShop->SaveFeaturedIdx(
                    user, restrict_code, current_seconds, 0);
            }
        }
        return 1;
    }

    const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        inventory_item, add_reason, true, false);
    if (slot == -1) {
        return 0;
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             slot);
    return 1;
}

// IDA_EA: 0x86C1910
// IDA_NAME: _ZN8WongWork9CWebEvent16ReqSaveEventItemEjjjjjjb
// DEMANGLED: WongWork::CWebEvent::ReqSaveEventItem(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,bool)
int WongWork::CWebEvent::ReqSaveEventItem(unsigned int event_context,
                                          unsigned int charac_no,
                                          unsigned int item_id,
                                          unsigned int stack_count,
                                          unsigned int event_code,
                                          unsigned int event_source,
                                          bool no_persist) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("WebEvent.cpp", 160), true);
    guard << 76;
    guard << -1;

    EventItemSaveRequest* const request =
        static_cast<EventItemSaveRequest*>(
            guard.GetInBufferUntyped(sizeof(EventItemSaveRequest)));
    request->event_context = event_context;
    request->charac_no = charac_no;
    request->event_code = event_code;
    request->item_id = item_id;
    request->stack_count = stack_count;
    request->event_source = event_source;
    request->no_persist = no_persist ? 1u : 0u;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x86C1A2E
// IDA_NAME: _ZN8WongWork9CWebEvent20_GetATIAvatarAbilityEi
// DEMANGLED: WongWork::CWebEvent::_GetATIAvatarAbility(int)
int WongWork::CWebEvent::_GetATIAvatarAbility(int item_id) {
    switch (item_id) {
    case 40656:
        return 1;
    case 44648:
        return 6;
    case 48652:
        return 4;
    default:
        return 0;
    }
}

// IDA_EA: 0x86C1BA8
// IDA_NAME: _ZN8WongWork9CWebEvent15_GetEventStringEi
// DEMANGLED: WongWork::CWebEvent::_GetEventString(int)
const char* WongWork::CWebEvent::_GetEventString(int event_code) {
    if (event_code < 0 || event_code > 10) {
        return 0;
    }
    return kEventNames[event_code];
}

// IDA_EA: 0x86C1BD4
// IDA_NAME: _ZN8WongWork9CWebEvent16_GetEventIPGCodeEi
// DEMANGLED: WongWork::CWebEvent::_GetEventIPGCode(int)
const char* WongWork::CWebEvent::_GetEventIPGCode(int event_code) {
    if (event_code < 0 || event_code > 9 ||
        kEventIPGCodes[event_code][0] == '\0') {
        return 0;
    }
    return kEventIPGCodes[event_code];
}

// IDA_EA: 0x84EB396 / 0x84EB3B2 / 0x84EB3E2
Inter_MonitorEventItemUpdate::Inter_MonitorEventItemUpdate()
    : ISigDispatcher() {}
Inter_MonitorEventItemUpdate::~Inter_MonitorEventItemUpdate() {}

// IDA_EA: 0x84EB400 / 0x84EB41C / 0x84EB44C
Inter_LoadEventItem::Inter_LoadEventItem() : ISigDispatcher() {}
Inter_LoadEventItem::~Inter_LoadEventItem() {}
