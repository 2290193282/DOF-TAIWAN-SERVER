#include "common/manager/shared_server_message_abi.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8602146
// IDA_NAME: _ZN16stSERVER_MESSAGED2Ev
// DEMANGLED: stSERVER_MESSAGE::~stSERVER_MESSAGE()
stSERVER_MESSAGE::~stSERVER_MESSAGE() {}

namespace {

stSERVER_MESSAGE MakeRuntimeServerMessage(
    const EventServerMessagePayload& source) {
    stSERVER_MESSAGE result = {};
    result.kind = static_cast<uint8_t>(source.kind);
    result.message_index = static_cast<uint8_t>(source.message_index);
    result.reserved_02[0] = source.reserved_02[0];
    result.reserved_02[1] = source.reserved_02[1];
    result.message_len = source.message_length;
    std::memcpy(result.message, source.message, sizeof(result.message));
    result.charac_name_len = source.charac_name_length;
    std::memcpy(result.charac_name,
                source.charac_name,
                sizeof(result.charac_name));
    result.update_time = static_cast<uint32_t>(source.update_time);
    return result;
}

}  // namespace

// IDA_EA: 0x836FA92
// IDA_NAME: _ZN25stSharedServerMessageInfoC2Ev
// DEMANGLED: stSharedServerMessageInfo::stSharedServerMessageInfo(void)
stSharedServerMessageInfo::stSharedServerMessageInfo() {}

// IDA_EA: 0x836FAFE
// IDA_NAME: _ZN25stSharedServerMessageInfoD2Ev
// DEMANGLED: stSharedServerMessageInfo::~stSharedServerMessageInfo()
stSharedServerMessageInfo::~stSharedServerMessageInfo() {}

// IDA_EA: 0x8601640
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapter16GetMappedEventIDEi
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::GetMappedEventID(int)
int CSharedServerMessageManager::CSharedServerMessageAdapter::GetMappedEventID(
    int message_index) {
    for (std::vector<std::pair<int, int> >::iterator it =
             info->index_kind.begin();
         it != info->index_kind.end(); ++it) {
        if (it->first == message_index) {
            return it->second;
        }
    }
    return -1;
}

// IDA_EA: 0x8601700
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapter9IsOnIndexEi
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::IsOnIndex(int)
int CSharedServerMessageManager::CSharedServerMessageAdapter::IsOnIndex(
    int message_index) {
    for (std::vector<int>::iterator it = info->onoff.begin();
         it != info->onoff.end(); ++it) {
        if (*it == message_index) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8601794
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapter21GetShareKindWithIndexEi
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::GetShareKindWithIndex(int)
int CSharedServerMessageManager::CSharedServerMessageAdapter::
    GetShareKindWithIndex(int message_index) {
    const int mapped_event_id = GetMappedEventID(message_index);
    if (mapped_event_id == -1) {
        return -1;
    }

    for (std::vector<std::pair<int, int> >::iterator it =
             info->event_kind.begin();
         it != info->event_kind.end(); ++it) {
        if (it->first == mapped_event_id) {
            return it->second;
        }
    }
    return -1;
}

// IDA_EA: 0x86019EC
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapterC2Ev
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::CSharedServerMessageAdapter(void)
CSharedServerMessageManager::CSharedServerMessageAdapter::
    CSharedServerMessageAdapter()
    : info(0) {}

// IDA_EA: 0x86019FA
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapterD2Ev
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::~CSharedServerMessageAdapter()
CSharedServerMessageManager::CSharedServerMessageAdapter::
    ~CSharedServerMessageAdapter() {}

// IDA_EA: 0x8601A00
// IDA_NAME: _ZN27CSharedServerMessageManager27CSharedServerMessageAdapter23SetServerMessageAddInfoEP25stSharedServerMessageInfo
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageAdapter::SetServerMessageAddInfo(stSharedServerMessageInfo *)
CSharedServerMessageManager::CSharedServerMessageAdapter*
CSharedServerMessageManager::CSharedServerMessageAdapter::
    SetServerMessageAddInfo(stSharedServerMessageInfo* message_info) {
    info = message_info;
    return this;
}

// IDA_EA: 0x84ED182
// IDA_NAME: _ZN27CSharedServerMessageManager18IsSetServerMessageEv
// DEMANGLED: CSharedServerMessageManager::IsSetServerMessage(void)
int CSharedServerMessageManager::IsSetServerMessage() {
    return loading_complete;
}

// IDA_EA: 0x84ED18E
// IDA_NAME: _ZN27CSharedServerMessageManager19GetCurServerMessageEv
// DEMANGLED: CSharedServerMessageManager::GetCurServerMessage(void)
EVENT_SERVER_MESSAGE* CSharedServerMessageManager::GetCurServerMessage() {
    return &current;
}

// IDA_EA: 0x860079C
// IDA_NAME: _ZN27CSharedServerMessageManagerC2Ev
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageManager(void)
CSharedServerMessageManager::CSharedServerMessageManager()
    : adapter(),
      loading_complete(0),
      reserved_05(),
      current() {
    current.clear();
}

// IDA_EA: 0x860080E
// IDA_NAME: _ZN27CSharedServerMessageManagerC2EP25stSharedServerMessageInfo
// DEMANGLED: CSharedServerMessageManager::CSharedServerMessageManager(stSharedServerMessageInfo *)
CSharedServerMessageManager::CSharedServerMessageManager(
    stSharedServerMessageInfo* info)
    : adapter(),
      loading_complete(0),
      reserved_05(),
      current() {
    adapter.SetServerMessageAddInfo(info);
    current.clear();
}

// IDA_EA: 0x8600892
// IDA_NAME: _ZN27CSharedServerMessageManagerD2Ev
// DEMANGLED: CSharedServerMessageManager::~CSharedServerMessageManager()
CSharedServerMessageManager::~CSharedServerMessageManager() {}

// IDA_EA: 0x86008D8
// IDA_NAME: _ZN27CSharedServerMessageManager21SendServerMessageInfoEP5CUserP24SIG_EVENT_SERVER_MESSAGEc
// DEMANGLED: CSharedServerMessageManager::SendServerMessageInfo(CUser *,SIG_EVENT_SERVER_MESSAGE *,char)
void CSharedServerMessageManager::SendServerMessageInfo(
    CUser* user,
    SIG_EVENT_SERVER_MESSAGE* message,
    char flag) {
    // The original constructs the packet before filtering, so a source list
    // with more than 50 selected entries still takes the no-send path only
    // after all adapter checks and copies have completed.
    PacketGuard packet;
    packet.put_header(0, 192);
    packet.put_byte(flag);

    EVENT_SERVER_MESSAGE selected;
    selected.count = 0;
    for (int index = 0; index < static_cast<int>(message->count); ++index) {
        const EventServerMessagePayload& source = message->messages[index];
        if (static_cast<unsigned char>(adapter.IsOnIndex(
                static_cast<int>(source.message_index))) != 0) {
            ++selected.count;
            selected.messages.push_back(MakeRuntimeServerMessage(source));
        }
    }

    if (selected.count <= 0x32u) {
        packet.put_byte(selected.count);
        for (std::vector<stSERVER_MESSAGE>::const_iterator it =
                 selected.messages.begin();
             it != selected.messages.end(); ++it) {
            packet.put_byte(it->message_index);
            packet.put_int(static_cast<int>(it->message_len));
            packet.put_str(it->message,
                           static_cast<int>(it->message_len));
            packet.put_int(static_cast<int>(it->charac_name_len));
            packet.put_str(it->charac_name,
                           static_cast<int>(it->charac_name_len));
        }
        packet.finalize(true);
        user->Send(packet);
    }
}

// IDA_EA: 0x8600B7A
// IDA_NAME: _ZN27CSharedServerMessageManager16SetServerMessageEP24SIG_EVENT_SERVER_MESSAGE
// DEMANGLED: CSharedServerMessageManager::SetServerMessage(SIG_EVENT_SERVER_MESSAGE *)
int CSharedServerMessageManager::SetServerMessage(
    SIG_EVENT_SERVER_MESSAGE* message) {
    if (loading_complete != 1) {
        const int source_count = message->count;
        if (!source_count) {
            return source_count;
        }

        current.count = message->count;
        for (int index = 0; source_count > index; ++index) {
            if (adapter.IsOnIndex(message->messages[index].message_index)) {
                current.messages.push_back(
                    MakeRuntimeServerMessage(message->messages[index]));
            }
        }
        current.count = static_cast<uint8_t>(current.messages.size());
    }
    return static_cast<int>(reinterpret_cast<intptr_t>(setLoadingComplete()));
}

// IDA_EA: 0x8600C34
// IDA_NAME: _ZN27CSharedServerMessageManager13getCurMessageEi
// DEMANGLED: CSharedServerMessageManager::getCurMessage(int)
stSERVER_MESSAGE* CSharedServerMessageManager::getCurMessage(
    int message_index) {
    for (std::vector<stSERVER_MESSAGE>::iterator it = current.messages.begin();
         it != current.messages.end(); ++it) {
        if (it->message_index == message_index) {
            return &*it;
        }
    }
    return 0;
}

// IDA_EA: 0x8600CC8
// IDA_NAME: _ZN27CSharedServerMessageManager18IsChangableMessageEii
// DEMANGLED: CSharedServerMessageManager::IsChangableMessage(int,int)
int CSharedServerMessageManager::IsChangableMessage(int message_index,
                                                    int cur_sec) {
    stSERVER_MESSAGE* message = getCurMessage(message_index);
    return !message || static_cast<int>(message->update_time) + 120 < cur_sec;
}

// IDA_EA: 0x8600D0C
// IDA_NAME: _ZN27CSharedServerMessageManager21DeleteSpecificMessageEi
// DEMANGLED: CSharedServerMessageManager::DeleteSpecificMessage(int)
CSharedServerMessageManager*
CSharedServerMessageManager::DeleteSpecificMessage(int message_index) {
    for (std::vector<stSERVER_MESSAGE>::iterator it = current.messages.begin();
         it != current.messages.end(); ++it) {
        if (it->message_index == message_index) {
            current.messages.erase(it);
            --current.count;
            return this;
        }
    }
    return 0;
}

// IDA_EA: 0x8601A0E
// IDA_NAME: _ZN27CSharedServerMessageManager18setLoadingCompleteEv
// DEMANGLED: CSharedServerMessageManager::setLoadingComplete(void)
CSharedServerMessageManager*
CSharedServerMessageManager::setLoadingComplete() {
    loading_complete = 1;
    return this;
}

// IDA_EA: 0x8600DCC
// IDA_NAME: _ZN27CSharedServerMessageManager18CheckUserConditionEP5CUseriRi
// DEMANGLED: CSharedServerMessageManager::CheckUserCondition(CUser *,int,int &)
int CSharedServerMessageManager::CheckUserCondition(CUser* user,
                                                    int message_index,
                                                    int& result) {
    (void)result;
    const int mapped_event_id = adapter.GetMappedEventID(message_index);
    if (mapped_event_id == -1) {
        return 0;
    }
    if (mapped_event_id == 1) {
        return 1;
    }

    CInventory* const inventory = user->getCurCharacInvenR();
    if (mapped_event_id == 2) {
        return inventory->check_item_exist(0x1D9D) == -1 ? 0 : 1;
    }
    if (mapped_event_id == 3 || mapped_event_id == 0) {
        return inventory->check_item_exist(0x28D3AA) == -1 ? 0 : 1;
    }
    return 0;
}

// IDA_EA: 0x8600EC0
// IDA_NAME: _ZN27CSharedServerMessageManager19UpdateServerMessageEP5CUseriPcS2_i
// DEMANGLED: CSharedServerMessageManager::UpdateServerMessage(CUser *,int,char *,char *,int)
void CSharedServerMessageManager::UpdateServerMessage(CUser* user,
                                                      int message_index,
                                                      char* message,
                                                      char* charac_name,
                                                      int cur_second) {
    stSERVER_MESSAGE* current_message = getCurMessage(message_index);
    if (current_message) {
        current_message->message_len = std::strlen(message);
        std::strncpy(current_message->message,
                     message,
                     current_message->message_len + 1);
        current_message->charac_name_len = std::strlen(charac_name);
        std::strncpy(current_message->charac_name,
                     charac_name,
                     current_message->charac_name_len + 1);
        current_message->update_time = cur_second;
    } else {
        if (static_cast<unsigned char>(adapter.IsOnIndex(message_index)) == 0) {
            cMyTrace trace(
                "void CSharedServerMessageManager::UpdateServerMessage(CUser*, int, char*, char*, int)",
                294,
                0);
            trace("SERVER_MESSAGE : NOT EXIST INDEX and NOT EXIST DATA");
            return;
        }

        const int share_kind = adapter.GetShareKindWithIndex(message_index);
        if (share_kind == -1) {
            return;
        }

        stSERVER_MESSAGE new_message;
        new_message.kind = static_cast<uint8_t>(share_kind);
        new_message.message_index = static_cast<uint8_t>(message_index);
        new_message.charac_name_len = std::strlen(charac_name);
        std::strncpy(new_message.charac_name,
                     charac_name,
                     new_message.charac_name_len + 1);
        new_message.message_len = std::strlen(message);
        std::strncpy(new_message.message,
                     message,
                     new_message.message_len + 1);
        new_message.update_time = cur_second;
        current.messages.push_back(new_message);
        ++current.count;
        current_message = &new_message;
    }

    if (static_cast<unsigned char>(IsSetServerMessage()) != 1) {
        setLoadingComplete();
    }

    const int mapped_event_id =
        adapter.GetMappedEventID(current_message->message_index);
    int item_slot = 0;
    switch (mapped_event_id) {
        case 2: {
            CInventory* const inventory = user->getCurCharacInvenR();
            item_slot = inventory->check_item_exist(0x1D9D);
            if (item_slot == -1) {
                return;
            }
            CInventory* const writable_inventory = user->getCurCharacInvenW();
            if (static_cast<unsigned char>(writable_inventory->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    item_slot,
                    1,
                    static_cast<eItemDelReason>(3),
                    true)) != 1 ||
                static_cast<unsigned char>(user->SendUpdateItem(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    item_slot)) != 1) {
                return;
            }
            break;
        }
        case 3: {
            CInventory* const inventory = user->getCurCharacInvenR();
            item_slot = inventory->check_item_exist(0x28D3AA);
            if (item_slot == -1) {
                return;
            }
            CInventory* const writable_inventory = user->getCurCharacInvenW();
            if (static_cast<unsigned char>(writable_inventory->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    item_slot,
                    1,
                    static_cast<eItemDelReason>(3),
                    true)) != 1 ||
                static_cast<unsigned char>(user->SendUpdateItem(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    item_slot)) != 1) {
                return;
            }
            break;
        }
        case 0: {
            CInventory* const inventory = user->getCurCharacInvenR();
            item_slot = inventory->check_item_exist(0x28D3AA);
            if (item_slot == -1) {
                return;
            }
            CInventory* const writable_inventory = user->getCurCharacInvenW();
            if (static_cast<unsigned char>(writable_inventory->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    item_slot,
                    1,
                    static_cast<eItemDelReason>(29),
                    true)) != 1 ||
                static_cast<unsigned char>(user->SendUpdateItem(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    item_slot)) != 1) {
                return;
            }
            break;
        }
        default:
            break;
    }

    Stream* const stream = GlobalData::stream_pool()->Acquire(
        "SharedServerMessageManager.cpp", 364);
    CStreamGuard guard(stream, true);
    guard << 364;
    guard << user->GetUID();
    SIG_UPDATE_SERVER_MESSAGE* const payload =
        guard.GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>();
    payload->message_index = current_message->message_index;
    payload->server_group = G_CEnvironment()->get_server_group();
    if (current_message->kind == 0) {
        payload->channel_no = G_CEnvironment()->get_channel_no();
    } else if (current_message->kind == 1) {
        payload->channel_no = 0;
    }
    payload->kind = current_message->kind;
    payload->charac_name_len = current_message->charac_name_len;
    payload->message_len = current_message->message_len;
    std::strncpy(payload->message,
                 current_message->message,
                 payload->message_len + 1);
    std::strncpy(payload->charac_name,
                 current_message->charac_name,
                 payload->charac_name_len + 1);
    payload->update_time = current_message->update_time;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8601476
// IDA_NAME: _ZN27CSharedServerMessageManager30UpdateServerMessageByBroadcastEiPcS0_i
// DEMANGLED: CSharedServerMessageManager::UpdateServerMessageByBroadcast(int,char *,char *,int)
int CSharedServerMessageManager::UpdateServerMessageByBroadcast(
    int message_index,
    char* message,
    char* charac_name,
    int cur_second) {
    stSERVER_MESSAGE* current_message = getCurMessage(message_index);
    if (current_message) {
        current_message->message_len = std::strlen(message);
        std::strncpy(current_message->message,
                     message,
                     current_message->message_len + 1);
        current_message->charac_name_len = std::strlen(charac_name);
        std::strncpy(current_message->charac_name,
                     charac_name,
                     current_message->charac_name_len + 1);
        current_message->update_time = cur_second;
    } else {
        if (static_cast<unsigned char>(adapter.IsOnIndex(message_index)) ==
            0) {
            cMyTrace trace(
                "void CSharedServerMessageManager::UpdateServerMessageByBroadcast(int, char*, char*, int)",
                454,
                0);
            return trace(
                "SERVER_MESSAGE : NOT EXIST INDEX and NOT EXIST DATA");
        }

        const int share_kind = adapter.GetShareKindWithIndex(message_index);
        if (share_kind == -1) {
            return share_kind;
        }

        stSERVER_MESSAGE new_message;
        new_message.kind = static_cast<uint8_t>(share_kind);
        new_message.message_index = static_cast<uint8_t>(message_index);
        new_message.charac_name_len = std::strlen(charac_name);
        std::strncpy(new_message.charac_name,
                     charac_name,
                     new_message.charac_name_len + 1);
        new_message.message_len = std::strlen(message);
        std::strncpy(new_message.message,
                     message,
                     new_message.message_len + 1);
        new_message.update_time = cur_second;
        current.messages.push_back(new_message);
        ++current.count;
    }

    const int result = IsSetServerMessage() ^ 1;
    if (static_cast<unsigned char>(result) != 0) {
        return static_cast<int>(reinterpret_cast<intptr_t>(
            setLoadingComplete()));
    }
    return result;
}
