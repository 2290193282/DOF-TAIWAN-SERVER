#include "old_data/db/account_cargo/packet_dispatcher_account_cargo.h"

#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x859712C
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargoC2Ev
// DEMANGLED: Dispatcher_CreateAccountCargo::Dispatcher_CreateAccountCargo(void)
Dispatcher_CreateAccountCargo::Dispatcher_CreateAccountCargo() {}

// IDA_EA: 0x82407EC
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargoD2Ev
// DEMANGLED: Dispatcher_CreateAccountCargo::~Dispatcher_CreateAccountCargo()
// IDA_EA: 0x824081C
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargoD0Ev
// DEMANGLED: Dispatcher_CreateAccountCargo::~Dispatcher_CreateAccountCargo()
Dispatcher_CreateAccountCargo::~Dispatcher_CreateAccountCargo() {}

// IDA_EA: 0x8224F7C
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CreateAccountCargo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CreateAccountCargo::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 2) {
        CAccountCargo::CreateAccountCargo(user);
    }
    return 0;
}

// IDA_EA: 0x82341AE
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CreateAccountCargo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CreateAccountCargo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82341B8
// IDA_NAME: _ZN29Dispatcher_CreateAccountCargo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreateAccountCargo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CreateAccountCargo::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597148
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargoC2Ev
// DEMANGLED: Dispatcher_UpgradeAccountCargo::Dispatcher_UpgradeAccountCargo(void)
Dispatcher_UpgradeAccountCargo::Dispatcher_UpgradeAccountCargo() {}

// IDA_EA: 0x824079E
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargoD2Ev
// DEMANGLED: Dispatcher_UpgradeAccountCargo::~Dispatcher_UpgradeAccountCargo()
// IDA_EA: 0x82407CE
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargoD0Ev
// DEMANGLED: Dispatcher_UpgradeAccountCargo::~Dispatcher_UpgradeAccountCargo()
Dispatcher_UpgradeAccountCargo::~Dispatcher_UpgradeAccountCargo() {}

// IDA_EA: 0x8224FB0
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeAccountCargo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeAccountCargo::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 2) {
        CAccountCargo::UpgradeAccountCargo(user);
    }
    return 0;
}

// IDA_EA: 0x82341C2
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeAccountCargo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeAccountCargo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82341CC
// IDA_NAME: _ZN30Dispatcher_UpgradeAccountCargo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeAccountCargo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeAccountCargo::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
