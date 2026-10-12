#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/config.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

#include <vector>

// IDA_EA: 0x8190416
// IDA_NAME: _ZN20AvatarRechargeServer14SendDurabilityEP5CUsercsci
// DEMANGLED: AvatarRechargeServer::SendDurability(CUser *,char,short,char,int)
char AvatarRechargeServer::SendDurability(CUser* user,
                                          char source_space,
                                          short,
                                          char destination_space,
                                          int destination_slot) {
    if (!user) return 0;
    if (user->get_state() != 5) return 1;
    if (source_space != 1 || destination_space != 3) return 0;

    const int target = user->GetParty() ? 2 : 1;
    UpdateAvatarEndurance(target,
                          user,
                          static_cast<short>(destination_slot));
    return 0;
}

// IDA_EA: 0x81905B4
// IDA_NAME: _ZN20AvatarRechargeServer25CalcDurabilityChangeEquipEP5CUseri
// DEMANGLED: AvatarRechargeServer::CalcDurabilityChangeEquip(CUser *,int)
void AvatarRechargeServer::CalcDurabilityChangeEquip(CUser* user,
                                                      int equipment_slot) {
    if (!user || user->get_state() != 5) return;

    CParty* const party = user->GetParty();
    if (!party || party->IsUseFatigueDungeon() != 1 ||
        party->CheckMapClear() != 0 ||
        equipment_slot > oldsrv::config::kAvatarEquipmentTypeLast) {
        return;
    }

    // The original obtains a temporary vector solely so CalcDurabilityDetail
    // can perform its normal endurance mutation; it does not notify this path.
    std::vector<int> changed_slots;
    CalcDurabilityDetail(user, equipment_slot, changed_slots);
}
