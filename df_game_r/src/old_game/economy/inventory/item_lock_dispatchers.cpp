#include "old_game/economy/inventory/item_lock_dispatchers.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x8596E70
// IDA_NAME: _ZN26Dispatcher_RequestItemLockC2Ev
// DEMANGLED: Dispatcher_RequestItemLock::Dispatcher_RequestItemLock(void)
Dispatcher_RequestItemLock::Dispatcher_RequestItemLock()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240D1A
// IDA_NAME: _ZN26Dispatcher_RequestItemLockD2Ev
// DEMANGLED: Dispatcher_RequestItemLock::~Dispatcher_RequestItemLock()
// IDA_EA: 0x8240D4A
// IDA_NAME: _ZN26Dispatcher_RequestItemLockD0Ev
// DEMANGLED: Dispatcher_RequestItemLock::~Dispatcher_RequestItemLock()
Dispatcher_RequestItemLock::~Dispatcher_RequestItemLock() {}

// IDA_EA: 0x8222E00
// IDA_NAME: _ZN26Dispatcher_RequestItemLock12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestItemLock::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestItemLock::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    char item_space = 0;
    short slot = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(
            56614,
            "virtual int Dispatcher_RequestItemLock::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            56615,
            "virtual int Dispatcher_RequestItemLock::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(23));
    if (security_result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_ITEM_LOCK,
                                 static_cast<unsigned char>(security_result));
        return 0;
    }

    const int result = user->IsAbleItemLock(item_space, slot);
    if (result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_ITEM_LOCK,
                                 static_cast<unsigned char>(result));
    } else {
        user->DoItemLock(item_space, slot);
    }
    return 0;
}

// IDA_EA: 0x8233FF6
// IDA_NAME: _ZN26Dispatcher_RequestItemLock4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestItemLock::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestItemLock::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234000
// IDA_NAME: _ZN26Dispatcher_RequestItemLock7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestItemLock::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestItemLock::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596E8C
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlockC2Ev
// DEMANGLED: Dispatcher_RequestItemUnlock::Dispatcher_RequestItemUnlock(void)
Dispatcher_RequestItemUnlock::Dispatcher_RequestItemUnlock()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240CCC
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlockD2Ev
// DEMANGLED: Dispatcher_RequestItemUnlock::~Dispatcher_RequestItemUnlock()
// IDA_EA: 0x8240CFC
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlockD0Ev
// DEMANGLED: Dispatcher_RequestItemUnlock::~Dispatcher_RequestItemUnlock()
Dispatcher_RequestItemUnlock::~Dispatcher_RequestItemUnlock() {}

// IDA_EA: 0x8222F82
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlock12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestItemUnlock::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestItemUnlock::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    char item_space = 0;
    short slot = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(
            56652,
            "virtual int Dispatcher_RequestItemUnlock::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            56653,
            "virtual int Dispatcher_RequestItemUnlock::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(24));
    if (security_result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK,
                                 static_cast<unsigned char>(security_result));
        return 0;
    }

    int remain_time = 0;
    const int result = user->IsAbleItemUnlock(item_space, slot, remain_time);
    if (!result) {
        user->DoItemUnlock(item_space, slot);
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK);
    response.put_byte(0);
    response.put_byte(result);
    response.put_int(remain_time);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x823400A
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlock4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestItemUnlock::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestItemUnlock::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234014
// IDA_NAME: _ZN28Dispatcher_RequestItemUnlock7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestItemUnlock::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestItemUnlock::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596EA8
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancelC2Ev
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::Dispatcher_RequestItemUnlockCancel(void)
Dispatcher_RequestItemUnlockCancel::Dispatcher_RequestItemUnlockCancel()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240C7E
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancelD2Ev
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::~Dispatcher_RequestItemUnlockCancel()
// IDA_EA: 0x8240CAE
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancelD0Ev
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::~Dispatcher_RequestItemUnlockCancel()
Dispatcher_RequestItemUnlockCancel::~Dispatcher_RequestItemUnlockCancel() {}

// IDA_EA: 0x82231AE
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestItemUnlockCancel::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    char item_space = 0;
    short slot = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(
            56699,
            "virtual int Dispatcher_RequestItemUnlockCancel::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            56700,
            "virtual int Dispatcher_RequestItemUnlockCancel::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(25));
    if (security_result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK_CANCEL,
                                 static_cast<unsigned char>(security_result));
        return 0;
    }

    const int result = user->IsAbleItemUnlockCancel(item_space, slot);
    if (result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK_CANCEL,
                                 static_cast<unsigned char>(result));
    } else {
        user->DoItemUnlockCancel(item_space, slot);
    }
    return 0;
}

// IDA_EA: 0x823401E
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestItemUnlockCancel::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234028
// IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestItemUnlockCancel::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestItemUnlockCancel::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
