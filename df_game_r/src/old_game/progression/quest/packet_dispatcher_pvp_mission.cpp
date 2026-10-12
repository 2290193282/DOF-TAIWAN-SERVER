#include "old_game/progression/quest/packet_dispatcher_pvp_mission.h"

#include <stddef.h>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kPvpMissionHPPercentReadSignature[] =
    "virtual int Dispatcher_PvpMissionHPPercent::read(PacketBuf&, MSG_BASE&)";
const char kPvpMissionHPPercentProcessSignature[] =
    "virtual int Dispatcher_PvpMissionHPPercent::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// This C TU is the sole owner of the old concrete state-3 template/vtable.
// It carries no PacketDispatcher registration or manifest change.
// IDA_EA: 0x8250DEC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8250F6A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8250F74
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C388
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C3B8
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D740
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598BC6
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PVP_MISSION_HP_PERCENT20stSendClearedMissionL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,stSendClearedMission,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,
                                 stSendClearedMission,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x859768E
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentC2Ev
// DEMANGLED: Dispatcher_PvpMissionHPPercent::Dispatcher_PvpMissionHPPercent(void)
Dispatcher_PvpMissionHPPercent::Dispatcher_PvpMissionHPPercent()
    : IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,
                        stSendClearedMission,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8244F86
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentD2Ev
// DEMANGLED: Dispatcher_PvpMissionHPPercent::~Dispatcher_PvpMissionHPPercent()
// IDA_EA: 0x8244FB6
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentD0Ev
// DEMANGLED: Dispatcher_PvpMissionHPPercent::~Dispatcher_PvpMissionHPPercent()
Dispatcher_PvpMissionHPPercent::~Dispatcher_PvpMissionHPPercent() {}

// IDA_EA: 0x81D574E
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PvpMissionHPPercent::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PvpMissionHPPercent::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    MSG_PVP_MISSION_HP_PERCENT& request =
        static_cast<MSG_PVP_MISSION_HP_PERCENT&>(message);

    // The original does not branch on either PacketBuf read return.  It
    // validates only the resulting count and preserves that exact behavior.
    packet.get_byte(request.byte_0d);
    if (request.byte_0d != 0 && request.byte_0d <= 4u) {
        request.percent_0e.resize(request.byte_0d, 0);
        for (int index = 0; index < request.byte_0d; ++index) {
            packet.get_short(request.percent_0e[static_cast<size_t>(index)]);
        }
        return 0;
    }

    cMyTrace trace(kPvpMissionHPPercentReadSignature, 14628, 5);
    trace("[@missionSystem] HackUser: <clearMissionCount : %d>",
          static_cast<int>(request.byte_0d));
    return LineFunc(14629, kPvpMissionHPPercentReadSignature, 0, 0);
}

// IDA_EA: 0x81D5864
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PvpMissionHPPercent::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PvpMissionHPPercent::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    if (!user->getCurCharacR()) {
        return LineFunc(14640, kPvpMissionHPPercentProcessSignature, 0, 0);
    }

    MSG_PVP_MISSION_HP_PERCENT& request =
        static_cast<MSG_PVP_MISSION_HP_PERCENT&>(message);
    stSendClearedMission& result = static_cast<stSendClearedMission&>(param);
    user->UpdateMissionCondition(request.percent_0e,
                                 result.cleared_missions);
    return 0;
}

// IDA_EA: 0x81D58D6
// IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PvpMissionHPPercent::send(CUser *,ParamBase &)
void Dispatcher_PvpMissionHPPercent::send(CUser* user, ParamBase& param) {
    const stSendClearedMission& result =
        static_cast<const stSendClearedMission&>(param);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_MISSION_CLEAR);
    packet.put_byte(static_cast<int>(result.cleared_missions.size()));
    for (std::vector<int>::const_iterator it =
             result.cleared_missions.begin();
         it != result.cleared_missions.end();
         ++it) {
        packet.put_short(*it);
    }
    packet.finalize(true);
    user->Send(packet);
}
