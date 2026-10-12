#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_FAIR_PVP_SCORE_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_FAIR_PVP_SCORE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// IDA unit: 09_dungeon_pvp_party/Dispatcher_FairPvPScore.
// The class has no data after the IPacketDispatcher base vptr.
class Dispatcher_FairPvPScore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597672
    // IDA_NAME: _ZN23Dispatcher_FairPvPScoreC2Ev
    // DEMANGLED: Dispatcher_FairPvPScore::Dispatcher_FairPvPScore(void)
    Dispatcher_FairPvPScore();

    // IDA_EA: 0x8240186
    // IDA_NAME: _ZN23Dispatcher_FairPvPScoreD2Ev
    // DEMANGLED: Dispatcher_FairPvPScore::~Dispatcher_FairPvPScore()
    // IDA_EA: 0x82401B6
    // IDA_NAME: _ZN23Dispatcher_FairPvPScoreD0Ev
    // DEMANGLED: Dispatcher_FairPvPScore::~Dispatcher_FairPvPScore()
    virtual ~Dispatcher_FairPvPScore();

    // IDA_EA: 0x822793C
    // IDA_NAME: _ZN23Dispatcher_FairPvPScore12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_FairPvPScore::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234316
    // IDA_NAME: _ZN23Dispatcher_FairPvPScore4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_FairPvPScore::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234320
    // IDA_NAME: _ZN23Dispatcher_FairPvPScore7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_FairPvPScore::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_FairPvPScore) == 0x04u,
              "Dispatcher_FairPvPScore must retain the old vptr-only ABI");
#endif


#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_FAIR_PVP_SCORE_H_
