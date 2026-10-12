#include "old_data/Login/packet_dispatch/break_away_quest_check_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const char kBreakAwayReadSignature[] =
    "virtual int Dispatcher_Break_Away_Quest_Check::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x82521A6
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BREAK_AWAY,stBreakAwayParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8252302
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BREAK_AWAY,stBreakAwayParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x825230C
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BREAK_AWAY,stBreakAwayParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C694 / 0x825C6C4
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EED0Ev
// IDA_EA: 0x825D752
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x85989CE
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_BREAK_AWAY16stBreakAwayParamL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_BREAK_AWAY,
                                 stBreakAwayParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x822DC4C
// IDA_NAME: _ZN14MSG_BREAK_AWAYC2Ev
// DEMANGLED: MSG_BREAK_AWAY::MSG_BREAK_AWAY(void)
MSG_BREAK_AWAY::MSG_BREAK_AWAY() : MSG_BASE(), accept_reward(0) {}

// IDA_EA: 0x8231572
// IDA_NAME: _ZN16stBreakAwayParamC2Ev
// DEMANGLED: stBreakAwayParam::stBreakAwayParam(void)
stBreakAwayParam::stBreakAwayParam() : ParamBase(), result(0) {}

// IDA_EA: 0x824708A
// IDA_NAME: _ZN16stBreakAwayParamD2Ev
// DEMANGLED: stBreakAwayParam::~stBreakAwayParam()
// IDA_EA: 0x82470BA
// IDA_NAME: _ZN16stBreakAwayParamD0Ev
// DEMANGLED: stBreakAwayParam::~stBreakAwayParam()
stBreakAwayParam::~stBreakAwayParam() {}

// IDA_EA: 0x8597228
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_CheckC2Ev
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::Dispatcher_Break_Away_Quest_Check(void)
Dispatcher_Break_Away_Quest_Check::Dispatcher_Break_Away_Quest_Check()
    : IPacketDispatcher<MSG_BREAK_AWAY,
                        stBreakAwayParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x824537C
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_CheckD2Ev
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::~Dispatcher_Break_Away_Quest_Check()
// IDA_EA: 0x82453AC
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_CheckD0Ev
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::~Dispatcher_Break_Away_Quest_Check()
Dispatcher_Break_Away_Quest_Check::~Dispatcher_Break_Away_Quest_Check() {}

// IDA_EA: 0x81D38BA
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_Check4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Break_Away_Quest_Check::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    MSG_BREAK_AWAY& request = static_cast<MSG_BREAK_AWAY&>(message);
    if (packet.get_byte(request.accept_reward) == 1) {
        return 0;
    }
    return LineFunc(12695, kBreakAwayReadSignature, 0, 0);
}

// IDA_EA: 0x81D390A
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_Check7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Break_Away_Quest_Check::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    const MSG_BREAK_AWAY& request =
        static_cast<const MSG_BREAK_AWAY&>(message);
    stBreakAwayParam& result = static_cast<stBreakAwayParam&>(param);

    if (request.accept_reward != 0) {
        if (user->getBreakAwayDungeonClearCnt() != 0) {
            result.result = 1;
        } else {
            GlobalData::s_BreakAwaySys->giveBreakAwayDungeonClearReward(user);
        }
        return 0;
    }

    user->setBreakAwayDungeonClearCnt(-1);
    user->setBreakAwayQuestTime(0);
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("PacketDispatcher_Impl_1.cpp",
                                           12761),
        true);
    (*guard) << 324;
    (*guard) << user->GetUID();
    (*guard) << user->get_acc_id();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81D3A72
// IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_Check4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Break_Away_Quest_Check::send(CUser *,ParamBase &)
void Dispatcher_Break_Away_Quest_Check::send(CUser* user, ParamBase& param) {
    const stBreakAwayParam& result = static_cast<const stBreakAwayParam&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_BREAK_AWAY_QUEST_CHECK);
    if (result.result != 0) {
        packet.put_byte(0);
        packet.put_short(result.result);
    } else {
        packet.put_byte(1);
    }
    packet.finalize(true);
    user->Send(packet);
}
