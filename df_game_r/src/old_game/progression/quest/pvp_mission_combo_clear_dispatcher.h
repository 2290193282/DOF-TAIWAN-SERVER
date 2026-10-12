#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PVP_MISSION_COMBO_CLEAR_DISPATCHER_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PVP_MISSION_COMBO_CLEAR_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#pragma pack(push, 1)

// The raw CMD411 template reserves exactly 15 unconstructed bytes.  The two
// values below are established by Dispatcher_PvpMissionComboClear::read.
struct MSG_PVP_MISSION_COMBO_CLEAR : public MSG_BASE {
    uint8_t mission_index;  // IDA +0x0d; accepted range 0..8.
    uint8_t combo_value;    // IDA +0x0e; forwarded as Update_ComboMission arg 1.
};

#pragma pack(pop)

static_assert(sizeof(MSG_PVP_MISSION_COMBO_CLEAR) == 0x0fu,
              "CMD411 must preserve its observed 15-byte request ABI");

class CUser;
class PacketBuf;

class Dispatcher_PvpMissionComboClear
    : public IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85976AA
    // IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearC2Ev
    // DEMANGLED: Dispatcher_PvpMissionComboClear::Dispatcher_PvpMissionComboClear(void)
    Dispatcher_PvpMissionComboClear();

    // IDA_EA: 0x8244F38
    // IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearD2Ev
    // DEMANGLED: Dispatcher_PvpMissionComboClear::~Dispatcher_PvpMissionComboClear()
    // IDA_EA: 0x8244F68
    // IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearD0Ev
    // DEMANGLED: Dispatcher_PvpMissionComboClear::~Dispatcher_PvpMissionComboClear()
    virtual ~Dispatcher_PvpMissionComboClear();

    // IDA_EA: 0x81D59C6
    // IDA_NAME: _ZN31Dispatcher_PvpMissionComboClear4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_PvpMissionComboClear::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D5A5E
    // IDA_NAME: _ZN31Dispatcher_PvpMissionComboClear7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_PvpMissionComboClear::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PvpMissionComboClear) == 0x04u,
              "CMD411 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PVP_MISSION_COMBO_CLEAR_DISPATCHER_H_
