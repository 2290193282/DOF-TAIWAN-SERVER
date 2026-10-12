#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_PVP_MISSION_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_PVP_MISSION_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD376.  The old handler is a vptr-only derivative of the concrete
// state-3 PvP-mission template specialization owned by the C quest-skill
// domain.  It is intentionally not installed into PacketDispatcher here.
class Dispatcher_PvpMissionHPPercent
    : public IPacketDispatcher<MSG_PVP_MISSION_HP_PERCENT,
                               stSendClearedMission,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859768E
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentC2Ev
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::Dispatcher_PvpMissionHPPercent(void)
    Dispatcher_PvpMissionHPPercent();

    // IDA_EA: 0x8244F86
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentD2Ev
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::~Dispatcher_PvpMissionHPPercent()
    // IDA_EA: 0x8244FB6
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercentD0Ev
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::~Dispatcher_PvpMissionHPPercent()
    virtual ~Dispatcher_PvpMissionHPPercent();

    // IDA_EA: 0x81D574E
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D5864
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D58D6
    // IDA_NAME: _ZN30Dispatcher_PvpMissionHPPercent4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_PvpMissionHPPercent::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PvpMissionHPPercent) == 0x04u,
              "CMD376 must retain its old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_PVP_MISSION_H_
