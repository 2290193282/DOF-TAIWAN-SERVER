#include "infra/security/policy/check_conn_dispatcher.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"


// IDA_EA: 0x85959C4
// IDA_NAME: _ZN20DisPatcher_CheckConnC2Ev
// DEMANGLED: DisPatcher_CheckConn::DisPatcher_CheckConn(void)
DisPatcher_CheckConn::DisPatcher_CheckConn() {}

// IDA_EA: 0x82438FA
// IDA_NAME: _ZN20DisPatcher_CheckConnD2Ev
// DEMANGLED: DisPatcher_CheckConn::~DisPatcher_CheckConn()
// IDA_EA: 0x824392A
// IDA_NAME: _ZN20DisPatcher_CheckConnD0Ev
// DEMANGLED: DisPatcher_CheckConn::~DisPatcher_CheckConn()
DisPatcher_CheckConn::~DisPatcher_CheckConn() {}

// IDA_EA: 0x81EA900
// IDA_NAME: _ZN20DisPatcher_CheckConn12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_CheckConn::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_CheckConn::dispatch_sig(CUser* user, PacketBuf& packet) {
    user->decre_check_count();

    int inventory_checksum = 0;
    if (packet.get_int(inventory_checksum) != 1) {
        return LineFunc(
            27640,
            "virtual int DisPatcher_CheckConn::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->getCurCharacInvenCheckSum() != inventory_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x25Bu),
            1u,
            0u,
            0u);
    }

    int skill_checksum = 0;
    if (packet.get_int(skill_checksum) != 1) {
        return LineFunc(
            27647,
            "virtual int DisPatcher_CheckConn::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->getCurCharacSkillCheckSum() != skill_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x25Cu),
            1u,
            0u,
            0u);
    }

    user->getHades()->Send_ChangeGold();
    return 0;
}

// IDA_EA: 0x8232A7E
// IDA_NAME: _ZN20DisPatcher_CheckConn4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_CheckConn::read(PacketBuf &,MSG_BASE &)
int DisPatcher_CheckConn::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232A88
// IDA_NAME: _ZN20DisPatcher_CheckConn7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_CheckConn::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_CheckConn::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
