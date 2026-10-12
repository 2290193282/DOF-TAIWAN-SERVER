#include "old_game/progression/quest/packet_dispatcher_request_seria_buff.h"

#include <stdint.h>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/seria_blessing_script.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x8597912
// IDA_NAME: _ZN29Dispatcher_Request_Seria_BuffC2Ev
// DEMANGLED: Dispatcher_Request_Seria_Buff::Dispatcher_Request_Seria_Buff(void)
Dispatcher_Request_Seria_Buff::Dispatcher_Request_Seria_Buff()
    : IPacketDispatcher<MSG_BASE,
                        stGrowthPowerParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244920
// IDA_NAME: _ZN29Dispatcher_Request_Seria_BuffD2Ev
// DEMANGLED: Dispatcher_Request_Seria_Buff::~Dispatcher_Request_Seria_Buff()
// IDA_EA: 0x8244950
// IDA_NAME: _ZN29Dispatcher_Request_Seria_BuffD0Ev
// DEMANGLED: Dispatcher_Request_Seria_Buff::~Dispatcher_Request_Seria_Buff()
Dispatcher_Request_Seria_Buff::~Dispatcher_Request_Seria_Buff() {}

// IDA_EA: 0x81DBCCE
// IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Request_Seria_Buff::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Request_Seria_Buff::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81DBCD8
// IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Request_Seria_Buff::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Request_Seria_Buff::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)message;
    (void)param;
    if (!user->getCurCharacR()) {
        return -1;
    }

    const int charac_level = user->get_charac_level();
    CDataManager* const data_manager = G_CDataManager();
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(93);
    if (repeat_event->IsEventing(0)) {
        return data_manager->get_seria_blessing_script()->is_possible_lv(
                   charac_level) == 1
                   ? 0
                   : -1;
    }

    return data_manager->get_growth_power_data()
                   ->get_idx_break_away_section(charac_level) >= 0
               ? 0
               : -1;
}

// IDA_EA: 0x81DBDA0
// IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Request_Seria_Buff::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Request_Seria_Buff::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    const int error = check_error(user, message, param);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }
    user->set_growth_power_exp_reward_right(false);
    return 0;
}

// IDA_EA: 0x81DBE08
// IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Request_Seria_Buff::send(CUser *,ParamBase &)
void Dispatcher_Request_Seria_Buff::send(CUser* user, ParamBase& param) {
    const stGrowthPowerParam& response =
        static_cast<const stGrowthPowerParam&>(param);
    if (response.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_SERIA_BUFF,
            static_cast<uint8_t>(response.result));
        return;
    }
    user->set_growth_power_exp_reward(user->get_charac_level());
    user->SendCmdOkPacket(ENUM_CMDPACKET_REQUEST_SERIA_BUFF);
}
