#include "old_game/economy/inventory/inventory.h"

#include <cstdint>
#include <cstring>

#include "old_data/db/account/db_delete_account_cargo.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8289794
// IDA_NAME: _ZN13CAccountCargoC2Ev
// DEMANGLED: CAccountCargo::CAccountCargo(void)
CAccountCargo::CAccountCargo()
    : user_(0),
      money_(0),
      capacity_(0),
      altered_(0) {
    // IDA first constructs all 56 Inven_Item children, then clears their
    // exact 0xd58-byte storage range.
    std::memset(items_, 0, sizeof(items_));
}

// IDA_EA: 0x828B4EC
// IDA_NAME: _ZN13CAccountCargo18DeleteAccountCargoEP5CUser
// DEMANGLED: CAccountCargo::DeleteAccountCargo(CUser *)
int CAccountCargo::DeleteAccountCargo(CUser* user) {
    if (!user->IsExistAccountCargo()) {
        return 1;
    }

    user->GetAccountCargo();
    DB_DeleteAccountCargo::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()));
    return static_cast<int>(reinterpret_cast<intptr_t>(user->DeleteCargo()));
}
