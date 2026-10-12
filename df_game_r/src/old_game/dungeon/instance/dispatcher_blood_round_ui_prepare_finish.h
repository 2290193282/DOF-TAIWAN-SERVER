#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_BLOOD_ROUND_UI_PREPARE_FINISH_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_BLOOD_ROUND_UI_PREPARE_FINISH_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_BloodRoundUiPrepareFinish.
// The object contains only the state-0 IPacketDispatcher vptr; the dispatch
// hook advances the existing typed CParty/CBattle_Field blood-round state.
class Dispatcher_BloodRoundUiPrepareFinish
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596DC8
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishC2Ev
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::Dispatcher_BloodRoundUiPrepareFinish(void)
    Dispatcher_BloodRoundUiPrepareFinish();

    // IDA_EA: 0x8240EA0
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishD2Ev
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::~Dispatcher_BloodRoundUiPrepareFinish()
    // IDA_EA: 0x8240ED0
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishD0Ev
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::~Dispatcher_BloodRoundUiPrepareFinish()
    virtual ~Dispatcher_BloodRoundUiPrepareFinish();

    // IDA_EA: 0x8222270
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233F92
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233F9C
    // IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BloodRoundUiPrepareFinish) == 0x04u,
              "CMD262 dispatcher must retain its old vptr-only object ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DISPATCHER_BLOOD_ROUND_UI_PREPARE_FINISH_H_
