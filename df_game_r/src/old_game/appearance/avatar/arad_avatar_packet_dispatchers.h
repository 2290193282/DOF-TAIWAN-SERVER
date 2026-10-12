#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_DISPATCHERS_H_

#include <stdint.h>

#include "old_game/appearance/avatar/arad_avatar_packet_signals.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

namespace arad {

// This three-byte record is the CMD586 ingress payload.  It is deliberately
// separate from the four service-result records in
// arad_avatar_packet_signals.h: the original constructor initializes all
// three bytes before the dispatcher reads only the first two from PacketBuf.
#pragma pack(push, 1)
struct SigAvatarRoulette {
    uint8_t roulette_type;
    uint8_t select_no;
    // The old C2 clears this byte and forwards it, but the receiver does not
    // consume it.  Do not assign it a protocol meaning without further
    // evidence.
    uint8_t zeroed_unused_byte;

    // IDA_EA: 0x81A092E
    // IDA_NAME: _ZN4arad17SigAvatarRouletteC2Ev
    // DEMANGLED: arad::SigAvatarRoulette::SigAvatarRoulette(void)
    SigAvatarRoulette();

    // IDA_EA: 0x81A0942
    // IDA_NAME: _ZN4arad17SigAvatarRoulette4initEv
    // DEMANGLED: arad::SigAvatarRoulette::init(void)
    SigAvatarRoulette* init();
};
#pragma pack(pop)

static_assert(sizeof(SigAvatarRoulette) == 0x03u,
              "SigAvatarRoulette must retain the three-byte ARAD queue ABI");

}  // namespace arad

namespace ARAD {
namespace DISPATCHER {

// CMD586: ENUM_CMDPACKET_USE_AVATAR_ROULETTE.
class Dispatcher_AvartarRoulette
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597BEA
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteC2Ev
    Dispatcher_AvartarRoulette();

    // IDA_EA: 0x81A0E3E
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteD2Ev
    // IDA_EA: 0x81A0E6E
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteD0Ev
    virtual ~Dispatcher_AvartarRoulette();

    // IDA_EA: 0x819EF54
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819EE6C
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette14checkWorkStateEP5CUserRN4arad17SigAvatarRouletteE
    int checkWorkState(CUser* user, arad::SigAvatarRoulette& payload);

    // IDA_EA: 0x81A0AE6
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0AF0
    // IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD587: ENUM_CMDPACKET_AVATAR_COIN_COUNT.
class Dispatcher_AvartarCoinCount
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597C06
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountC2Ev
    Dispatcher_AvartarCoinCount();

    // IDA_EA: 0x81A0DF0
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountD2Ev
    // IDA_EA: 0x81A0E20
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountD0Ev
    virtual ~Dispatcher_AvartarCoinCount();

    // IDA_EA: 0x819F1BC
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819F1E2
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount14checkWorkStateEP5CUser
    int checkWorkState(CUser* user);

    // IDA_EA: 0x81A0AFA
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B04
    // IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD588: ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE.
class Dispatcher_AvartarHiddenOptionChange
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597C22
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeC2Ev
    Dispatcher_AvartarHiddenOptionChange();

    // IDA_EA: 0x81A0D54
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeD2Ev
    // IDA_EA: 0x81A0D84
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeD0Ev
    virtual ~Dispatcher_AvartarHiddenOptionChange();

    // IDA_EA: 0x819F6F8
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819F956
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange14checkWorkStateEP5CUserRN4arad27SigAvatarHiddenOptionChangeE
    int checkWorkState(CUser* user,
                       arad::SigAvatarHiddenOptionChange& payload);

    // IDA_EA: 0x81A0B0E
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B18
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD589: ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN.
class Dispatcher_AradUseAvatarRecharge
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597C3E
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeC2Ev
    Dispatcher_AradUseAvatarRecharge();

    // IDA_EA: 0x81A0D06
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeD2Ev
    // IDA_EA: 0x81A0D36
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeD0Ev
    virtual ~Dispatcher_AradUseAvatarRecharge();

    // IDA_EA: 0x819FA56
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819F9D6
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge14checkWorkStateEP5CUserRN4arad24SigAradUseAvatarRechargeE
    int checkWorkState(CUser* user, arad::SigAradUseAvatarRecharge& payload);

    // IDA_EA: 0x81A0B36
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B40
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD590: ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN.
class Dispatcher_AradEmblemCompound
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597C76
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundC2Ev
    Dispatcher_AradEmblemCompound();

    // IDA_EA: 0x81A0CB8
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundD2Ev
    // IDA_EA: 0x81A0CE8
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundD0Ev
    virtual ~Dispatcher_AradEmblemCompound();

    // IDA_EA: 0x819FD7E
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819FCB4
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound14checkWorkStateEP5CUserRN4arad21SigAradEmblemCompoundE
    int checkWorkState(CUser* user, arad::SigAradEmblemCompound& payload);

    // IDA_EA: 0x81A0B4A
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B54
    // IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD591: ENUM_CMDPACKET_AVATAR_CONVERT_JPN.
class Dispatcher_AradAvatarConvert
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597C5A
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertC2Ev
    Dispatcher_AradAvatarConvert();

    // IDA_EA: 0x81A0C6A
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertD2Ev
    // IDA_EA: 0x81A0C9A
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertD0Ev
    virtual ~Dispatcher_AradAvatarConvert();

    // IDA_EA: 0x81A00BE
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x81A003E
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert14checkWorkStateEP5CUserRN4arad20SigAradAvatarConvertE
    int checkWorkState(CUser* user, arad::SigAradAvatarConvert& payload);

    // IDA_EA: 0x81A0B5E
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B68
    // IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AvartarRoulette) == 0x04u,
              "CMD586 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AvartarCoinCount) == 0x04u,
              "CMD587 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AvartarHiddenOptionChange) == 0x04u,
              "CMD588 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AradUseAvatarRecharge) == 0x04u,
              "CMD589 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AradEmblemCompound) == 0x04u,
              "CMD590 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AradAvatarConvert) == 0x04u,
              "CMD591 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_DISPATCHERS_H_
