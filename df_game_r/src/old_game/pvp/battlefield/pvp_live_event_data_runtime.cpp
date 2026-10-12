#include "old_game/pvp/battlefield/pvp_live_event_data_runtime.h"
#include "old_game/pvp/battlefield/inter_pvp_live_event_data_runtime.h"

#include <cstring>
#include <new>
#include <utility>
#include <vector>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_parameter_script_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

// CDataManager::CDataManager @0x8355B30 constructs ServerParameterScript at
// +0x68; CPvPLiveEventData::IncreasePlayCount @0x826925F reaches that exact
// embedded object on every script query.
ServerParameterScript* ServerParameterAt(CDataManager* data_manager) {
    return data_manager->get_server_parameter_script();
}

// CPvPLiveEventData::IncreasePlayCount @0x82692F9 loads the canonical
// CItemList pointer from CDataManager+0x0c before calling create_item.
CItemList* ItemListAt(CDataManager* data_manager) {
    return data_manager->get_item_list();
}

}  // namespace

// IDA_EA: 0x8269054
// IDA_NAME: _ZN17CPvPLiveEventDataC2Ev
// DEMANGLED: CPvPLiveEventData::CPvPLiveEventData(void)
CPvPLiveEventData::CPvPLiveEventData() {}

// IDA_EA: 0x8269070
// IDA_NAME: _ZN17CPvPLiveEventDataD2Ev
// DEMANGLED: CPvPLiveEventData::~CPvPLiveEventData()
// IDA_EA: 0x82690A0
// IDA_NAME: _ZN17CPvPLiveEventDataD0Ev
// DEMANGLED: CPvPLiveEventData::~CPvPLiveEventData()
CPvPLiveEventData::~CPvPLiveEventData() {}

// IDA_EA: 0x82690BE
// IDA_NAME: _ZN17CPvPLiveEventData8loadDataEP5CUserPc
// DEMANGLED: CPvPLiveEventData::loadData(CUser *,char *)
bool CPvPLiveEventData::loadData(CUser* user, char* data) {
    (void)user;
    play_count_ = *reinterpret_cast<const int*>(data);
    return 1;
}

// IDA_EA: 0x82690D4
// IDA_NAME: _ZN17CPvPLiveEventData9_saveDataEP5CUser
// DEMANGLED: CPvPLiveEventData::_saveData(CUser *)
int CPvPLiveEventData::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "PvPLiveEventData.cpp", 30);
    CStreamGuard guard(stream, true);
    *guard << 0x1E1;
    *guard << user->GetUID();
    *guard << static_cast<char>(user->GetServerGroup());
    *guard << static_cast<unsigned int>(user->getCurCharacNo());
    *guard << play_count_;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x8269206
// IDA_NAME: _ZNK17CPvPLiveEventData7getDataEPc
// DEMANGLED: CPvPLiveEventData::getData(char *)const
void CPvPLiveEventData::getData(char* data) const {
    (void)data;
}

// IDA_EA: 0x826920C
// IDA_NAME: _ZN17CPvPLiveEventData6_resetEv
// DEMANGLED: CPvPLiveEventData::_reset(void)
int CPvPLiveEventData::_reset() {
    play_count_ = 0;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x826921C
// IDA_NAME: _ZN17CPvPLiveEventData17IncreasePlayCountEP5CUser
// DEMANGLED: CPvPLiveEventData::IncreasePlayCount(CUser *)
bool CPvPLiveEventData::IncreasePlayCount(CUser* user) {
    if (!user->getCurCharacR()) {
        return true;
    }

    alter();
    ++play_count_;
    if (play_count_ !=
        ServerParameterAt(G_CDataManager())->GetPvPLiveEventPlayCount()) {
        return false;
    }

    std::vector<std::pair<int, int> >* rewards =
        ServerParameterAt(G_CDataManager())->GetPvPLiveEventReward();
    for (std::vector<std::pair<int, int> >::iterator it = rewards->begin();
         it != ServerParameterAt(G_CDataManager())
                   ->GetPvPLiveEventReward()
                   ->end();
         ++it) {
        const int item_id = it->first;
        const int add_info = it->second;
        if (!G_CDataManager()->find_item(item_id)) {
            continue;
        }

        // The old body constructs the 0x3d-byte item at 0x82692EB and never
        // calls Inven_Item::~Inven_Item.  Constructor-only storage preserves
        // that exact lifetime topology.
        unsigned char item_storage[sizeof(Inven_Item)];
        Inven_Item* item = new (item_storage) Inven_Item;
        item->item_id_ = item_id;
        ItemListAt(G_CDataManager())
            ->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT, *item, add_info);

        if (item->item_id_ || item->get_add_info()) {
            char title[21];
            char text[256];
            std::strncpy(
                title,
                g_scriptStringManager_.findString(
                    4, "game_server_msg_140", 0),
                0x14u);
            std::strncpy(
                text,
                g_scriptStringManager_.findString(
                    4, "game_server_msg_141", 0),
                0xFFu);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                *item,
                0,
                static_cast<unsigned int>(user->getCurCharacNo()),
                text,
                static_cast<int>(std::strlen(text)),
                0,
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
                false,
                false);
            item->reset();
        }
    }
    return false;
}

// IDA_EA: 0x826948E
// IDA_NAME: _ZN17CPvPLiveEventData18ResetDailyMidnightEv
// DEMANGLED: CPvPLiveEventData::ResetDailyMidnight(void)
void CPvPLiveEventData::ResetDailyMidnight() {
    play_count_ = 0;
}

// IDA_EA: 0x84EE730
// IDA_NAME: _ZN26Inter_LoadPvPLiveEventDataC2Ev
Inter_LoadPvPLiveEventData::Inter_LoadPvPLiveEventData()
    : ISigDispatcher() {}

// IDA_EA: 0x84F6C96 / 0x84F6CC6
Inter_LoadPvPLiveEventData::~Inter_LoadPvPLiveEventData() {}

// IDA_EA: 0x84E25CC
// IDA_NAME: _ZN26Inter_LoadPvPLiveEventData12dispatch_sigEP5CUserPci
int Inter_LoadPvPLiveEventData::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2) {
        return 0;
    }
    if (user->getCurCharacR()) {
        // CCharacExpandCDataMgr constructs CPvPLiveEventData in its verified
        // expand slot 3.  Keep the original virtual CData::loadData call so
        // this receiver does not project or duplicate that owner state.
        user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(3))
            ->loadData(user, payload);
    }
    return 0;
}
