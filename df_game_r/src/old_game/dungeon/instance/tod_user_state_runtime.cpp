#include "old_game/dungeon/instance/tod_user_state_runtime.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8643460
// IDA_NAME: _ZN13TOD_UserStateC2Ev
// DEMANGLED: TOD_UserState::TOD_UserState(void)
TOD_UserState::TOD_UserState() {}

// IDA_EA: 0x86434AE
// IDA_NAME: _ZN13TOD_UserStateD2Ev
// DEMANGLED: TOD_UserState::~TOD_UserState()
// IDA_EA: 0x86434DE
// IDA_NAME: _ZN13TOD_UserStateD0Ev
// DEMANGLED: TOD_UserState::~TOD_UserState()
TOD_UserState::~TOD_UserState() {}

// IDA_EA: 0x86434FC
// IDA_NAME: _ZN13TOD_UserState8loadDataEP5CUserPc
// DEMANGLED: TOD_UserState::loadData(CUser *,char *)
bool TOD_UserState::loadData(CUser* user, char* data) {
    _reset();
    const SIG_REQUEST_TOD_USER_STATE* payload =
        reinterpret_cast<const SIG_REQUEST_TOD_USER_STATE*>(data);
    state_.charac_no = payload->charac_no;
    state_.first_layer_start_time = payload->first_layer_start_time;
    state_.today_enter_count = payload->today_enter_count;
    state_.last_clear_layer = payload->last_clear_layer;
    state_.enter_count_by_week = payload->enter_count_by_week;
    state_.modified_time = payload->modified_time;
    state_.last_clear_time = payload->last_clear_time;

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    const int last_play_tick = user->getCurCharacLastPlayTick();
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               last_play_tick,
                               current_second)) {
        ResetDaily();
    }
    if (state_.enter_count_by_week) {
        if (state_.modified_time <= 1311202800) {
            state_.enter_count_by_week = 0;
        }
        alter();
    }
    return 1;
}

// IDA_EA: 0x86435CA
// IDA_NAME: _ZNK13TOD_UserState7getDataEPc
// DEMANGLED: TOD_UserState::getData(char *)const
void TOD_UserState::getData(char* data) const {
    SIG_REQUEST_TOD_USER_STATE* payload =
        reinterpret_cast<SIG_REQUEST_TOD_USER_STATE*>(data);
    payload->enter_count_by_week = state_.enter_count_by_week;
    payload->last_clear_layer = state_.last_clear_layer;
    payload->first_layer_start_time = state_.first_layer_start_time;
    payload->today_enter_count = state_.today_enter_count;
    payload->modified_time = state_.modified_time;
    payload->last_clear_time = state_.last_clear_time;
}

// IDA_EA: 0x8643624
// IDA_NAME: _ZN13TOD_UserState10ResetDailyEv
// DEMANGLED: TOD_UserState::ResetDaily(void)
void TOD_UserState::ResetDaily() {
    state_.today_enter_count = 0;
    alter();
}

// IDA_EA: 0x8643642
// IDA_NAME: _ZN13TOD_UserState9_saveDataEP5CUser
// DEMANGLED: TOD_UserState::_saveData(CUser *)
int TOD_UserState::_saveData(CUser* user) {
    if (user->get_charac_level() <
        static_cast<uint16_t>(
            GlobalData::s_pTowerOfDespairMgr->GetMinLv())) {
        return 1;
    }

    Stream* stream =
        GlobalData::s_stream_pool->Acquire("TOD_UserState.cpp", 88);
    CStreamGuard guard(stream, true);
    *guard << 0x278;
    *guard << user->GetUID();
    SIG_REQUEST_TOD_USER_STATE* payload =
        guard->GetInBuffer<SIG_REQUEST_TOD_USER_STATE>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    payload->enter_count_by_week = state_.enter_count_by_week;
    payload->last_clear_layer = state_.last_clear_layer;
    payload->first_layer_start_time = state_.first_layer_start_time;
    payload->today_enter_count = state_.today_enter_count;
    payload->modified_time = state_.modified_time;
    payload->last_clear_time = state_.last_clear_time;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x86437EC
// IDA_NAME: _ZN13TOD_UserState6_resetEv
// DEMANGLED: TOD_UserState::_reset(void)
int TOD_UserState::_reset() {
    state_.charac_no = 0;
    state_.first_layer_start_time = 0;
    state_.today_enter_count = 0;
    state_.last_clear_layer.resetLayer();
    state_.enter_count_by_week = 0;
    state_.modified_time = 0;
    state_.last_clear_time = 0;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x864383E
// IDA_NAME: _ZNK13TOD_UserState18getTodayEnterLayerEv
// DEMANGLED: TOD_UserState::getTodayEnterLayer(void)const
TOD_Layer TOD_UserState::getTodayEnterLayer() const {
    return state_.last_clear_layer + 1;
}

// IDA_EA: 0x8643872
// IDA_NAME: _ZNK13TOD_UserState13getEnterCountEv
// DEMANGLED: TOD_UserState::getEnterCount(void)const
int TOD_UserState::getEnterCount() const {
    return state_.today_enter_count;
}

// IDA_EA: 0x864387E
// IDA_NAME: _ZNK13TOD_UserState16getLastClearTimeEv
// DEMANGLED: TOD_UserState::getLastClearTime(void)const
int TOD_UserState::getLastClearTime() const {
    return state_.last_clear_time;
}

// IDA_EA: 0x864388A
// IDA_NAME: _ZN13TOD_UserState18IncreaseEnterCountEv
// DEMANGLED: TOD_UserState::IncreaseEnterCount(void)
charac_expand::CData* TOD_UserState::IncreaseEnterCount() {
    ++state_.today_enter_count;
    ++state_.enter_count_by_week;
    state_.modified_time = GlobalData::s_systemTime_.getCurSec();
    if (state_.last_clear_layer.GetLayer() == 0u) {
        state_.first_layer_start_time =
            GlobalData::s_systemTime_.getCurSec();
    }
    return alter();
}

// IDA_EA: 0x86438FC
// IDA_NAME: _ZN13TOD_UserState13setEnterLayerERK9TOD_Layer
// DEMANGLED: TOD_UserState::setEnterLayer(TOD_Layer const&)
TOD_UserState* TOD_UserState::setEnterLayer(const TOD_Layer& layer) {
    state_.last_clear_layer = layer;
    return this;
}

// IDA_EA: 0x864390E
// IDA_NAME: _ZN13TOD_UserState10ClearLayerEv
// DEMANGLED: TOD_UserState::ClearLayer(void)
int TOD_UserState::ClearLayer() {
    alter();
    state_.last_clear_layer.increaseLayer();
    state_.last_clear_time = GlobalData::s_systemTime_.getCurSec();
    return state_.last_clear_time;
}

// IDA_EA: 0x8643942
// IDA_NAME: _ZNK13TOD_UserState17getLastEnterLayerEv
// DEMANGLED: TOD_UserState::getLastEnterLayer(void)const
TOD_Layer TOD_UserState::getLastEnterLayer() const {
    return TOD_Layer(state_.last_clear_layer);
}
