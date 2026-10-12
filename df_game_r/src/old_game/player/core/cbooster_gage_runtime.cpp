#include "old_game/player/core/cbooster_gage_runtime.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x832AF06
// IDA_NAME: _ZN12CBoosterGageC2Ev
// DEMANGLED: CBoosterGage::CBoosterGage(void)
CBoosterGage::CBoosterGage() : total_gage_(0) {
    _reset();
}

// IDA_EA: 0x80DE1DC
// IDA_NAME: _ZN12CBoosterGageD2Ev
// DEMANGLED: CBoosterGage::~CBoosterGage()
// IDA_EA: 0x80DE20C
// IDA_NAME: _ZN12CBoosterGageD0Ev
// DEMANGLED: CBoosterGage::~CBoosterGage()
CBoosterGage::~CBoosterGage() {}

// IDA_EA: 0x80DC640
// IDA_NAME: _ZN12CBoosterGage14inc_total_gageEi
// DEMANGLED: CBoosterGage::inc_total_gage(int)
void CBoosterGage::inc_total_gage(int point) {
    alter();
    total_gage_ += point;
}

// IDA_EA: 0x80DC664
// IDA_NAME: _ZN12CBoosterGage14sub_total_gageEi
// DEMANGLED: CBoosterGage::sub_total_gage(int)
void CBoosterGage::sub_total_gage(int point) {
    alter();
    total_gage_ -= point;
    if (total_gage_ < 0) {
        total_gage_ = 0;
    }
}

// IDA_EA: 0x80DC69C
// IDA_NAME: _ZN12CBoosterGage17get_booster_pointEm
// DEMANGLED: CBoosterGage::get_booster_point(unsigned long)
int CBoosterGage::get_booster_point(unsigned long item_index) {
    BoosterGageData& script_data =
        *G_CDataManager()->get_booster_gage_data();
    return script_data.getBoosterPoint(item_index);
}

// IDA_EA: 0x80DD0DA
// IDA_NAME: _ZN12CBoosterGage22check_max_booster_gageEm
// DEMANGLED: CBoosterGage::check_max_booster_gage(unsigned long)
bool CBoosterGage::check_max_booster_gage(unsigned long item_index) {
    if (!get_booster_point(item_index)) {
        return false;
    }
    const BoosterGageData& script_data =
        *G_CDataManager()->get_booster_gage_data();
    return total_gage_ >= script_data.max_gage;
}

// IDA_EA: 0x80DD134
// IDA_NAME: _ZN12CBoosterGage16cal_booster_gageEmP5CUser
// DEMANGLED: CBoosterGage::cal_booster_gage(unsigned long,CUser *)
int CBoosterGage::cal_booster_gage(unsigned long item_index, CUser* user) {
    // The specified old IDB is exactly six instructions at 0x80DD134 and
    // returns zero without reading either argument.
    (void)item_index;
    (void)user;
    return 0;
}

// IDA_EA: 0x80DD142
// IDA_NAME: _ZN12CBoosterGage23send_error_state_packetEP5CUserm
// DEMANGLED: CBoosterGage::send_error_state_packet(CUser *,unsigned long)
void CBoosterGage::send_error_state_packet(CUser* user,
                                            unsigned long item_index) {
    CBoosterGage* booster = static_cast<CBoosterGage*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(17)));
    const bool max_gage = booster->check_max_booster_gage(item_index);
    int reward_count = 2;
    if (max_gage) {
        const BoosterGageData& script_data =
            *G_CDataManager()->get_booster_gage_data();
        reward_count *= script_data.reward_multiple;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_RANDOMBOX_ITEM);
    packet.put_byte(0);
    packet.put_byte(19);
    packet.put_byte(reward_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x80DD246
// IDA_NAME: _ZN12CBoosterGage9send_dataEP5CUseri
// DEMANGLED: CBoosterGage::send_data(CUser *,int)
void CBoosterGage::send_data(CUser* user, int state) {
    // The specified old IDB is exactly five instructions at 0x80DD246 and
    // returns without reading either argument.
    (void)user;
    (void)state;
}

// IDA_EA: 0x80DD24E
// IDA_NAME: _ZNK12CBoosterGage7getDataEPc
// DEMANGLED: CBoosterGage::getData(char *)const
void CBoosterGage::getData(char* data) const {
    SIG_BOOSTER_GAGE_DATA* payload =
        reinterpret_cast<SIG_BOOSTER_GAGE_DATA*>(data);
    std::memset(payload, 0, sizeof(*payload));
    payload->gage = total_gage_;
}

// IDA_EA: 0x80DD284
// IDA_NAME: _ZN12CBoosterGage8loadDataEP5CUserPc
// DEMANGLED: CBoosterGage::loadData(CUser *,char *)
bool CBoosterGage::loadData(CUser* user, char* data) {
    _reset();
    const SIG_BOOSTER_GAGE_DATA* payload =
        reinterpret_cast<const SIG_BOOSTER_GAGE_DATA*>(data);
    total_gage_ = payload->gage;

    const int current_time = GlobalData::s_systemTime_.getCurSec();
    const int last_play_time = user->GetAccountLastPlayTime();
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               last_play_time,
                               current_time)) {
        ResetDaily();
        send_data(user, 0);
    }
    return true;
}

// IDA_EA: 0x80DD322
// IDA_NAME: _ZN12CBoosterGage10ResetDailyEv
// DEMANGLED: CBoosterGage::ResetDaily(void)
void CBoosterGage::ResetDaily() {
    alter();
    total_gage_ = 0;
}

// IDA_EA: 0x80DD340
// IDA_NAME: _ZN12CBoosterGage9_saveDataEP5CUser
// DEMANGLED: CBoosterGage::_saveData(CUser *)
int CBoosterGage::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "BoosterGage.cpp", 403);
    CStreamGuard guard(stream, true);
    *guard << 0x2D8;
    *guard << user->GetUID();

    SIG_BOOSTER_GAGE_DATA* payload =
        guard->GetInBuffer<SIG_BOOSTER_GAGE_DATA>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->gage = total_gage_;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x80DD468
// IDA_NAME: _ZN12CBoosterGage6_resetEv
// DEMANGLED: CBoosterGage::_reset(void)
int CBoosterGage::_reset() {
    total_gage_ = 0;
    return reinterpret_cast<int>(this);
}
