#include "old_game/activity/schedule/bingo_fatigue_packet_dispatchers.h"

#include <stdint.h>

#include "common/manager/bit_utils.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/bingo_runtime.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const int kBingoEventId = 106;
const int kAttendanceEventType = 0x56;

const char kBingoRewardProcessSignature[] =
    "virtual int Dispatcher_BingoReward::process(CUser*, MSG_BASE&, ParamBase&)";
const char kBingoQuizReadSignature[] =
    "virtual int Dispatcher_BingoQuiz::read(PacketBuf&, MSG_BASE&)";
const char kBingoQuizProcessSignature[] =
    "virtual int Dispatcher_BingoQuiz::process(CUser*, MSG_BASE&, ParamBase&)";
const char kFatigueReadSignature[] =
    "virtual int Dispatcher_FatigueAttendance::read(PacketBuf&, MSG_BASE&)";
const char kFatigueProcessSignature[] =
    "virtual int Dispatcher_FatigueAttendance::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// The three exact old template specializations are emitted only here after
// their concrete message/response types are complete.
// IDA_EA: 0x824E786
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824E8E2
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824E8EC
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825BE0C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BE3C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D666
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x859841E
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_FATIGUE_ATTENDANCE19stFatigueAttendanceL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,stFatigueAttendance,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,
                                 stFatigueAttendance,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8261A2A
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8261B86
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8261B90
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82624D0
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8262500
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x826262A
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598322
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BINGO_QUIZ11stBingoQuizL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BINGO_QUIZ,stBingoQuiz,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BINGO_QUIZ,
                                 stBingoQuiz,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8261C8E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8261DDE
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8261DE8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x826251E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x826254E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8262630
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598306
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE13stBingoRewardL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stBingoReward,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 stBingoReward,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8595858
// IDA_NAME: _ZN22Dispatcher_BingoRewardC2Ev
// DEMANGLED: Dispatcher_BingoReward::Dispatcher_BingoReward(void)
Dispatcher_BingoReward::Dispatcher_BingoReward() {}

// IDA_EA: 0x8260B2A
// IDA_NAME: _ZN22Dispatcher_BingoRewardD2Ev
// DEMANGLED: Dispatcher_BingoReward::~Dispatcher_BingoReward()
// IDA_EA: 0x8260B5A
// IDA_NAME: _ZN22Dispatcher_BingoRewardD0Ev
// DEMANGLED: Dispatcher_BingoReward::~Dispatcher_BingoReward()
Dispatcher_BingoReward::~Dispatcher_BingoReward() {}

// IDA_EA: 0x825E682
// IDA_NAME: _ZN22Dispatcher_BingoReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BingoReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BingoReward::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x825E7B2
// IDA_NAME: _ZN22Dispatcher_BingoReward11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_BingoReward::check_error(CUser *,MSG_BASE &)
int Dispatcher_BingoReward::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 30;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(kBingoEventId);
    if (repeat_event->IsEventing(0) != 1) {
        return 19;
    }

    BingoData* const bingo_data = user->getBingoData();
    if (bingo_data->selected_count != 0u) {
        return 18;
    }
    return static_cast<int8_t>(bingo_data->line_count) > 2 ? 0 : 22;
}

// IDA_EA: 0x825E68C
// IDA_NAME: _ZN22Dispatcher_BingoReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BingoReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BingoReward::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    stBingoReward& response = static_cast<stBingoReward&>(param);
    response.error_code = check_error(user, message);
    if (user) {
        response.bingo_data_dword =
            static_cast<int32_t>(user->getBingoData()->bingo_count);
    }
    if (response.error_code > 0) {
        return 0;
    }
    if (response.error_code >= 0) {
        BingoData* const bingo_data = user->getBingoData();
        bingo_data->reward_state = 1;
        bingo_data->selected_count = 1;

        BingoEvent* const repeat_event = static_cast<BingoEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(kBingoEventId));
        if (repeat_event) {
            repeat_event->sendMaxMatchLineReward(*user);
            user->sendDecorationObjectCnt(1);
        }
        return 0;
    }

    LogManager::logFormat(1,
                          "PacketDispatcher_Impl_2.cpp",
                          kBingoRewardProcessSignature,
                          0x19A,
                          "Dispatcher Check_Error No(%d)",
                          response.error_code);
    return LineFunc(411, kBingoRewardProcessSignature, 0, 0);
}

// IDA_EA: 0x825E87A
// IDA_NAME: _ZN22Dispatcher_BingoReward4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_BingoReward::send(CUser *,ParamBase &)
void Dispatcher_BingoReward::send(CUser* user, ParamBase& param) {
    stBingoReward& response = static_cast<stBingoReward&>(param);
    if (response.error_code != 0) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_BINGO_REWARD);
        packet.put_byte(0);
        packet.put_byte(response.error_code);
        packet.put_int(response.bingo_data_dword);
        packet.finalize(true);
        user->Send(packet);
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_BINGO_REWARD);
}

// IDA_EA: 0x8595874
// IDA_NAME: _ZN20Dispatcher_BingoQuizC2Ev
// DEMANGLED: Dispatcher_BingoQuiz::Dispatcher_BingoQuiz(void)
Dispatcher_BingoQuiz::Dispatcher_BingoQuiz() {}

// IDA_EA: 0x8260ADC
// IDA_NAME: _ZN20Dispatcher_BingoQuizD2Ev
// DEMANGLED: Dispatcher_BingoQuiz::~Dispatcher_BingoQuiz()
// IDA_EA: 0x8260B0C
// IDA_NAME: _ZN20Dispatcher_BingoQuizD0Ev
// DEMANGLED: Dispatcher_BingoQuiz::~Dispatcher_BingoQuiz()
Dispatcher_BingoQuiz::~Dispatcher_BingoQuiz() {}

// IDA_EA: 0x825E962
// IDA_NAME: _ZN20Dispatcher_BingoQuiz4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BingoQuiz::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BingoQuiz::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_BINGO_QUIZ& input = static_cast<MSG_BINGO_QUIZ&>(message);
    if (packet.get_int(input.quiz_value) == 1) {
        return 0;
    }
    return LineFunc(478, kBingoQuizReadSignature, 0, 0);
}

// IDA_EA: 0x825EACA
// IDA_NAME: _ZN20Dispatcher_BingoQuiz11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_BingoQuiz::check_error(CUser *,MSG_BASE &)
int Dispatcher_BingoQuiz::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 30;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(kBingoEventId);
    if (repeat_event->IsEventing(0) != 1) {
        return 19;
    }

    BingoData* const bingo_data = user->getBingoData();
    if (bingo_data->selected_count != 0u) {
        return 18;
    }
    return static_cast<int8_t>(bingo_data->line_count) <= 2 ? 0 : 22;
}

// IDA_EA: 0x825E9B8
// IDA_NAME: _ZN20Dispatcher_BingoQuiz7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BingoQuiz::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BingoQuiz::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    const MSG_BINGO_QUIZ& input = static_cast<const MSG_BINGO_QUIZ&>(message);
    stBingoQuiz& response = static_cast<stBingoQuiz&>(param);
    response.error_code = check_error(user, message);
    if (response.error_code > 0) {
        return 0;
    }
    if (response.error_code >= 0) {
        BingoEvent* const repeat_event = static_cast<BingoEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(kBingoEventId));
        if (repeat_event) {
            response.error_code =
                repeat_event->reqBingoQuizMark(*user, input.quiz_value);
        } else {
            response.error_code = 19;
        }
        response.bingo_data_dword =
            static_cast<int32_t>(user->getBingoData()->bingo_count);
        return 0;
    }

    LogManager::logFormat(1,
                          "PacketDispatcher_Impl_2.cpp",
                          kBingoQuizProcessSignature,
                          0x1F0,
                          "Dispatcher Check_Error No(%d)",
                          response.error_code);
    return LineFunc(497, kBingoQuizProcessSignature, 0, 0);
}

// IDA_EA: 0x825EB92
// IDA_NAME: _ZN20Dispatcher_BingoQuiz4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_BingoQuiz::send(CUser *,ParamBase &)
void Dispatcher_BingoQuiz::send(CUser* user, ParamBase& param) {
    stBingoQuiz& response = static_cast<stBingoQuiz&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_BINGO_QUIZ);
    if (response.error_code != 0) {
        packet.put_byte(0);
        packet.put_byte(response.error_code);
        packet.put_int(response.bingo_data_dword);
    } else {
        packet.put_byte(1);
        packet.put_int(response.bingo_data_dword);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8595970
// IDA_NAME: _ZN28Dispatcher_FatigueAttendanceC2Ev
// DEMANGLED: Dispatcher_FatigueAttendance::Dispatcher_FatigueAttendance(void)
Dispatcher_FatigueAttendance::Dispatcher_FatigueAttendance() {}

// IDA_EA: 0x82449BC
// IDA_NAME: _ZN28Dispatcher_FatigueAttendanceD2Ev
// DEMANGLED: Dispatcher_FatigueAttendance::~Dispatcher_FatigueAttendance()
// IDA_EA: 0x82449EC
// IDA_NAME: _ZN28Dispatcher_FatigueAttendanceD0Ev
// DEMANGLED: Dispatcher_FatigueAttendance::~Dispatcher_FatigueAttendance()
Dispatcher_FatigueAttendance::~Dispatcher_FatigueAttendance() {}

// IDA_EA: 0x81DAFA4
// IDA_NAME: _ZN28Dispatcher_FatigueAttendance4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_FatigueAttendance::read(PacketBuf &,MSG_BASE &)
int Dispatcher_FatigueAttendance::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_FATIGUE_ATTENDANCE& input =
        static_cast<MSG_FATIGUE_ATTENDANCE&>(message);
    if (packet.get_short(input.request_value) == 1) {
        return 0;
    }
    return LineFunc(18867, kFatigueReadSignature, 0, 0);
}

// IDA_EA: 0x81DB310
// IDA_NAME: _ZN28Dispatcher_FatigueAttendance11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_FatigueAttendance::check_error(CUser *,MSG_BASE &)
int Dispatcher_FatigueAttendance::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 99;
    }
    return G_CDataManager()->get_event_script_mng()->is_eventing(
               kAttendanceEventType) == 1
               ? 0
               : 7;
}

// IDA_EA: 0x81DAFFA
// IDA_NAME: _ZN28Dispatcher_FatigueAttendance7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_FatigueAttendance::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_FatigueAttendance::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    const MSG_FATIGUE_ATTENDANCE& input =
        static_cast<const MSG_FATIGUE_ATTENDANCE&>(message);
    stFatigueAttendance& response = static_cast<stFatigueAttendance&>(param);
    response.request_value = input.request_value;
    response.error_code = check_error(user, message);
    if (response.error_code > 0) {
        return 0;
    }
    if (response.error_code < 0) {
        return LineFunc(18885,
                        kFatigueProcessSignature,
                        response.error_code,
                        0);
    }

    const unsigned char today_day = static_cast<unsigned char>(
        G_CDataManager()->get_event_script_mng()->get_today_time_day());
    const int record_type = G_CDataManager()->get_event_script_mng()
                                ->get_record_type(kAttendanceEventType);
    std::pair<int, int> history;
    user->get_ingame_event_history(record_type, history);
    unsigned int attendance_bits = static_cast<unsigned int>(history.first);
    if (isSetBit_Uint(attendance_bits, static_cast<int>(today_day) - 1)) {
        response.error_code = 18;
        return 0;
    }

    const int fatigue_count =
        static_cast<int16_t>(user->get_server_fatigue_count(today_day)) +
        user->get_fatigue_count_db(today_day);
    EventClassify::CEventEntity* const event_entity =
        G_CDataManager()->get_event_script_mng()->get_event_entity(
            kAttendanceEventType);
    if (!event_entity) {
        return 0;
    }

    const int today_time =
        G_CDataManager()->get_event_script_mng()->get_today_time();
    if (event_entity->get_event_start_time() > today_time ||
        event_entity->get_event_end_time() < today_time) {
        response.error_code = 7;
        return 0;
    }

    EventClassify::CAttendance* const attendance =
        static_cast<EventClassify::CAttendance*>(event_entity);
    if (attendance->get_fatigue_condition() > fatigue_count) {
        response.error_code = 22;
        return 0;
    }

    setBit_Uint(attendance_bits, true, static_cast<int>(today_day) - 1);
    user->set_ingame_event_history(record_type,
                                   static_cast<int>(attendance_bits),
                                   0);
    response.history_bits = attendance_bits;
    response.today_time =
        G_CDataManager()->get_event_script_mng()->get_today_time();
    const int pass_day = G_CDataManager()->get_event_script_mng()->get_pass_day(
        event_entity->get_event_start_time(), response.today_time);
    attendance->send_attend_reward_item(user, pass_day);
    G_CDataManager()->get_event_script_mng()->process_update_ingame_history(
        user);
    user->reset_ingame_event_history_update();
    attendance->send_attend_final_rward_item(
        user,
        static_cast<int>(attendance_bits),
        attendance->get_final_day_condition());
    return 0;
}

// IDA_EA: 0x81DB392
// IDA_NAME: _ZN28Dispatcher_FatigueAttendance4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_FatigueAttendance::send(CUser *,ParamBase &)
void Dispatcher_FatigueAttendance::send(CUser* user, ParamBase& param) {
    stFatigueAttendance& response = static_cast<stFatigueAttendance&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_ATTENDANCE_CHECK);
    if (response.error_code != 0) {
        packet.put_byte(0);
        packet.put_byte(response.error_code);
        packet.put_short(response.request_value);
    } else {
        packet.put_byte(1);
        packet.put_short(response.request_value);
        packet.put_int(static_cast<int>(response.history_bits));
        packet.put_int(response.today_time);
    }
    packet.finalize(true);
    user->Send(packet);
}
