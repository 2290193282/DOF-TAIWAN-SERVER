#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RECOVER_STAMINA_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RECOVER_STAMINA_H_

#include "old_game/player/core/userinfo_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_RecoverStamina.  The class owns
// no fields beyond the exact state-3 dispatcher base on i386.
class DisPatcher_RecoverStamina
    : public IPacketDispatcher<MSG_USERINFO_REQ,
                               stOnRecoverStaminaParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85951C0
    // IDA_NAME: _ZN25DisPatcher_RecoverStaminaC2Ev
    // DEMANGLED: DisPatcher_RecoverStamina::DisPatcher_RecoverStamina(void)
    DisPatcher_RecoverStamina();

    // IDA_EA: 0x823237A / 0x82323AA
    // IDA_NAME: _ZN25DisPatcher_RecoverStaminaD2Ev /
    //           _ZN25DisPatcher_RecoverStaminaD0Ev
    // DEMANGLED: DisPatcher_RecoverStamina::~DisPatcher_RecoverStamina()
    virtual ~DisPatcher_RecoverStamina();

    // IDA_EA: 0x81C3EA4
    // IDA_NAME: _ZN25DisPatcher_RecoverStamina4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_RecoverStamina::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81C3EAE
    // IDA_NAME: _ZN25DisPatcher_RecoverStamina7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_RecoverStamina::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C3FB8
    // IDA_NAME: _ZN25DisPatcher_RecoverStamina4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_RecoverStamina::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81C3F4A
    // IDA_NAME: _ZN25DisPatcher_RecoverStamina11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_RecoverStamina::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_RecoverStamina) == 0x04u,
              "CMD9 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RECOVER_STAMINA_H_
