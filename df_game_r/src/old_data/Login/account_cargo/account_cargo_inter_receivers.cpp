#include "old_data/Login/account_cargo/account_cargo_inter_receivers.h"

#include <stddef.h>
#include <stdint.h>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/inventory/account_cargo_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x84E23A2
// IDA_NAME: _ZN24Inter_CreateAccountCargo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CreateAccountCargo::dispatch_sig(CUser *,char *,int)
int Inter_CreateAccountCargo::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_CREATE_ACCOUNT_CARGO_RESULT* const result =
        reinterpret_cast<const SIG_CREATE_ACCOUNT_CARGO_RESULT*>(payload);
    if (result->result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                 result->result);
    } else {
        user->SetAccountCargoDBData(0, 0, result->capacity);
        user->SendCmdOkPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO);
    }
    return 0;
}

// IDA_EA: 0x84EE82C
// IDA_NAME: _ZN24Inter_CreateAccountCargoC2Ev
// DEMANGLED: Inter_CreateAccountCargo::Inter_CreateAccountCargo(void)
Inter_CreateAccountCargo::Inter_CreateAccountCargo() {}

// IDA_EA: 0x84F6D80
// IDA_NAME: _ZN24Inter_CreateAccountCargoD2Ev
// DEMANGLED: Inter_CreateAccountCargo::~Inter_CreateAccountCargo()
// IDA_EA: 0x84F6DB0
// IDA_NAME: _ZN24Inter_CreateAccountCargoD0Ev
// DEMANGLED: Inter_CreateAccountCargo::~Inter_CreateAccountCargo()
Inter_CreateAccountCargo::~Inter_CreateAccountCargo() {}

// IDA_EA: 0x84E243A
// IDA_NAME: _ZN25Inter_UpgradeAccountCargo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpgradeAccountCargo::dispatch_sig(CUser *,char *,int)
int Inter_UpgradeAccountCargo::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_UPGRADE_ACCOUNT_CARGO_RESULT* const result =
        reinterpret_cast<const SIG_UPGRADE_ACCOUNT_CARGO_RESULT*>(payload);
    if (result->result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                 result->result);
        return 0;
    }

    if (user->IsExistAccountCargo()) {
        CAccountCargo* const account_cargo =
            static_cast<CAccountCargo*>(user->GetAccountCargo());
        account_cargo->SetCapacity(result->capacity);
        user->SendCmdOkPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO);
    }
    return 0;
}

// IDA_EA: 0x84EE848
// IDA_NAME: _ZN25Inter_UpgradeAccountCargoC2Ev
// DEMANGLED: Inter_UpgradeAccountCargo::Inter_UpgradeAccountCargo(void)
Inter_UpgradeAccountCargo::Inter_UpgradeAccountCargo() {}

// IDA_EA: 0x84F6D32
// IDA_NAME: _ZN25Inter_UpgradeAccountCargoD2Ev
// DEMANGLED: Inter_UpgradeAccountCargo::~Inter_UpgradeAccountCargo()
// IDA_EA: 0x84F6D62
// IDA_NAME: _ZN25Inter_UpgradeAccountCargoD0Ev
// DEMANGLED: Inter_UpgradeAccountCargo::~Inter_UpgradeAccountCargo()
Inter_UpgradeAccountCargo::~Inter_UpgradeAccountCargo() {}

// IDA_EA: 0x84E24EC
// IDA_NAME: _ZN22Inter_LoadAccountCargo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadAccountCargo::dispatch_sig(CUser *,char *,int)
int Inter_LoadAccountCargo::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)payload_size;

    SIG_ACCOUNT_CARGO_DATA* const result =
        reinterpret_cast<SIG_ACCOUNT_CARGO_DATA*>(payload);
    if (user->get_state() <= 1) {
        return 0;
    }

    if (user->IsExistAccountCargo()) {
        const char* const account_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
        const char* const account_name = user->get_acc_name();
        cMyTrace trace(
            "virtual int Inter_LoadAccountCargo::dispatch_sig(CUser*, char*, int)",
            20969,
            5);
        trace("AccountCargo duplicated!(%s(%s))",
              account_name,
              account_id_text);
    } else {
        user->SetAccountCargoDBData(result->items, result->money,
                                    result->capacity);
    }
    return 0;
}

// IDA_EA: 0x84EE864
// IDA_NAME: _ZN22Inter_LoadAccountCargoC2Ev
// DEMANGLED: Inter_LoadAccountCargo::Inter_LoadAccountCargo(void)
Inter_LoadAccountCargo::Inter_LoadAccountCargo() {}

// IDA_EA: 0x84F6CE4
// IDA_NAME: _ZN22Inter_LoadAccountCargoD2Ev
// DEMANGLED: Inter_LoadAccountCargo::~Inter_LoadAccountCargo()
// IDA_EA: 0x84F6D14
// IDA_NAME: _ZN22Inter_LoadAccountCargoD0Ev
// DEMANGLED: Inter_LoadAccountCargo::~Inter_LoadAccountCargo()
Inter_LoadAccountCargo::~Inter_LoadAccountCargo() {}
