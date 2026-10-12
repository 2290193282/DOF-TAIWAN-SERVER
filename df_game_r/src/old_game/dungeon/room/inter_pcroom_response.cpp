#include "old_data/db/taiwan/inter_slots_357_382.h"

#include <ctime>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/nexon_msg_table_runtime.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x84DB40E
// IDA_NAME: _ZN20Inter_PcroomResponse22IsShutdonTimeOverLoginEv
// DEMANGLED: Inter_PcroomResponse::IsShutdonTimeOverLogin(void)
bool Inter_PcroomResponse::IsShutdonTimeOverLogin() {
    const time_t current_time =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    struct tm local_time;
    return localtime_r(&current_time, &local_time)->tm_hour <= 5;
}

// IDA_EA: 0x84DB452
// IDA_NAME: _ZN20Inter_PcroomResponse12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PcroomResponse::dispatch_sig(CUser *,char *,int)
int Inter_PcroomResponse::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;

    const SIG_PCROOM_RESPONSE& input =
        *reinterpret_cast<const SIG_PCROOM_RESPONSE*>(payload);

    if (!user->get_state()) {
        return 17412;
    }
    if (user->getCurCharacR()) {
        user->setPcRoomAuth(true);
    }
    user->SetPCRoomNo(input.pc_room_no);

    if (input.security_result == 7 && input.ip_restriction_type != 3 &&
        input.ban_ip_login == 1) {
        user->DisConnSig(static_cast<DISCONN_SIG>(60), true, 0);
        cMyTrace trace(
            "virtual int Inter_PcroomResponse::dispatch_sig(CUser*, char*, int)",
            17529, 0);
        trace("SecuritySystem user ban ip login(%s,%u)", user->get_acc_name(),
              user->get_acc_id());
    }

    bool forbidden = false;
    bool packet_flag = true;
    if (input.ip_restriction_type != 1) {
        if (input.ip_restriction_type > 1) {
            if (input.ip_restriction_type == 3) {
                forbidden = true;
                packet_flag = input.restriction_message != 1;
            }
        } else if (input.ip_restriction_type == 0) {
            forbidden = input.restriction_message == 2;
        }
    }

    if (forbidden) {
        cMyTrace trace(
            "virtual int Inter_PcroomResponse::dispatch_sig(CUser*, char*, int)",
            17644, 0);
        trace("Inter_PcroomResponse::dispatch_sig() forbidden ip (%d), %d)",
              input.ip_restriction_type);

        PacketGuard packet;
        packet.put_header(0, 233);
        packet.put_byte(1);
        packet.put_byte(5);

        const char* const message_key =
            g_nexonMsgTable.GetMessageString(input.restriction_message);
        char localized_message[1025] = {};
        const char* const translated =
            g_scriptStringManager_.findString(4, message_key, 0);
        std::strncpy(localized_message, translated, 0x400u);
        const int message_length = std::strlen(localized_message);
        packet.put_int(message_length);
        packet.put_str(localized_message, message_length);
        packet.put_byte(packet_flag);
        packet.finalize(true);
        user->Send(packet);

        G_TimerQueue()->InsertTimer(0, user->GetUID(), 132, 120,
                                    user->getCurCharacNo(), 0);
    }

    if (input.pc_room_no) {
        WongWork::CHandlePremium::handleSetUserPCRoom(
            user, GlobalData::s_systemTime_.getCurSec(),
            static_cast<long>(input.premium_end_time));
    }
    G_CDataManager()->get_event_script_mng()->send_pc_room_play_init_time(user);
    return 0;
}


// IDA_EA: 0x84EE20C
// IDA_NAME: _ZN20Inter_PcroomResponseC2Ev
// DEMANGLED: Inter_PcroomResponse::Inter_PcroomResponse(void)
Inter_PcroomResponse::Inter_PcroomResponse() {}

// IDA_EA: 0x84F7CBC
// IDA_NAME: _ZN20Inter_PcroomResponseD2Ev
// IDA_EA: 0x84F7CEC
// IDA_NAME: _ZN20Inter_PcroomResponseD0Ev
// DEMANGLED: Inter_PcroomResponse::~Inter_PcroomResponse()
Inter_PcroomResponse::~Inter_PcroomResponse() {}
