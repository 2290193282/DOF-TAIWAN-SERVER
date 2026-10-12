#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/battlefield/packet_dispatcher_die_assault_player.h"

#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x85962A0
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayerC2Ev
// DEMANGLED: Dispatcher_DieAssaultPlayer::Dispatcher_DieAssaultPlayer(void)
Dispatcher_DieAssaultPlayer::Dispatcher_DieAssaultPlayer()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82426B2
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayerD2Ev
// DEMANGLED: Dispatcher_DieAssaultPlayer::~Dispatcher_DieAssaultPlayer()
// IDA_EA: 0x82426E2
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayerD0Ev
// DEMANGLED: Dispatcher_DieAssaultPlayer::~Dispatcher_DieAssaultPlayer()
Dispatcher_DieAssaultPlayer::~Dispatcher_DieAssaultPlayer() {}

// IDA_EA: 0x8207208
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayer12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DieAssaultPlayer::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DieAssaultPlayer::dispatch_sig(CUser* user, PacketBuf& packet) {
    static const char kFunctionName[] =
        "virtual int Dispatcher_DieAssaultPlayer::dispatch_sig(CUser*, "
        "PacketBuf&)";
    if (user->get_state() <= 1) {
        return LineFunc(41426, kFunctionName, 0, 0);
    }

    unsigned char killer_index = static_cast<unsigned char>(-1);
    MSG_DIE_ASSAULT_PLAYER message = {};
        if (packet.get_byte(killer_index) != 1) {
        return LineFunc(41430, kFunctionName, 0, 0);
    }
        if (packet.get_int(message.damage_value) != 1) {
        return LineFunc(41436, kFunctionName, 0, 0);
    }
        if (packet.get_int(message.damage_count) != 1) {
        return LineFunc(41439, kFunctionName, 0, 0);
    }
        if (packet.get_short(message.attacker_stat0) != 1) {
        return LineFunc(41442, kFunctionName, 0, 0);
    }
        if (packet.get_short(message.attacker_stat1) != 1) {
        return LineFunc(41445, kFunctionName, 0, 0);
    }
        if (packet.get_short(message.attacker_stat2) != 1) {
        return LineFunc(41448, kFunctionName, 0, 0);
    }
        if (packet.get_short(message.attacker_stat3) != 1) {
        return LineFunc(41451, kFunctionName, 0, 0);
    }
    if (pvp_assault::GetInstanceAssaultMgr()->OnDieUser(
            user, killer_index, message) == 1) {
        return 0;
    }
    return LineFunc(41457, kFunctionName, 0, 0);
}

// IDA_EA: 0x823393E
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayer4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DieAssaultPlayer::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DieAssaultPlayer::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233948
// IDA_NAME: _ZN27Dispatcher_DieAssaultPlayer7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DieAssaultPlayer::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DieAssaultPlayer::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}
