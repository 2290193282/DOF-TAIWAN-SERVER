#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>
#include <utility>
#include <vector>

#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/social/party/hell_party_abi.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_guard.h"

class UserQuest;
namespace {

CInventory* CurrentInventoryR(CUser* user) {
    return static_cast<CInventory*>(user->getCurCharacInvenR());
}

CInventory* CurrentInventoryW(CUser* user) {
    return static_cast<CInventory*>(user->getCurCharacInvenW());
}

void SnapshotInventoryItem(Inven_Item& snapshot, CUser* user, int slot) {
    snapshot = CurrentInventoryR(user)->GetInvenSlot(1, slot);
}

int ItemCount(const Inven_Item& item) {
    return item.add_info_;
}

int ItemKey(const Inven_Item& item) {
    return item.item_id_;
}

short ItemDuration(const Inven_Item& item) {
    return static_cast<int16_t>(item.avatar_select_);
}

int ItemAddInfo(const Inven_Item& item) {
    return item.extend_info_;
}

unsigned char ItemAbilityType(const Inven_Item& item) {
    const stAmplifyOption_t amplify_option = item.amplify_option_;
    return static_cast<unsigned char>(amplify_option.getAbilityType());
}

short ItemAbilityValue(const Inven_Item& item) {
    const stAmplifyOption_t amplify_option = item.amplify_option_;
    return static_cast<short>(amplify_option.getAbilityValue());
}

}  // namespace

// IDA_EA: 0x859E528
// IDA_NAME: _ZN6CParty24CheckEnterAncientDungeonEPK8CDungeonRhP10Inven_ItemPi
// DEMANGLED: CParty::CheckEnterAncientDungeon(CDungeon const*,unsigned char &,Inven_Item *,int *)
bool CParty::CheckEnterAncientDungeon(const CDungeon* dungeon,
                                      unsigned char& failed_member,
                                      Inven_Item* inventory_snapshots,
                                      int* inventory_slots) {
    if (!dungeon || dungeon->isTowerOfDespairDungeon()) {
        return true;
    }
    if (dungeon->event_dungeon_) {
        return CheckEnterEventDungeon(
                   dungeon, failed_member, inventory_snapshots, inventory_slots) ==
               1;
    }
    if (dungeon->required_item_.item_id == -1 ||
        dungeon->required_item_.delete_after_start != 1 ||
        battle_field_.instance_dungeon_mode_ == 1) {
        return true;
    }

    const int required_item = dungeon->required_item_.item_id;
    const int required_count = dungeon->required_item_.item_count;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        int slot = 0;
        if (dungeon->get_index() == 1500) {
            slot = CurrentInventoryR(user)->check_vilmark_dungeon_item_exist(
                required_item);
        } else {
            slot = CurrentInventoryR(user)->check_item_exist(required_item);
        }
        inventory_slots[seat] = slot;
        if (slot == -1) {
            failed_member = static_cast<unsigned char>(seat);
            return false;
        }

        SnapshotInventoryItem(inventory_snapshots[seat], user, slot);
        if (ItemCount(inventory_snapshots[seat]) < required_count) {
            failed_member = static_cast<unsigned char>(seat);
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x859E7FE
// IDA_NAME: _ZN6CParty24CheckEnterAncientDungeonEPK8CDungeon
// DEMANGLED: CParty::CheckEnterAncientDungeon(CDungeon const*)
bool CParty::CheckEnterAncientDungeon(const CDungeon* dungeon) {
    Inven_Item inventory_snapshots[4];
    int inventory_slots[4];
    std::memset(inventory_snapshots, 0, sizeof(inventory_snapshots));
    std::memset(inventory_slots, 0, sizeof(inventory_slots));

    if (dungeon->event_dungeon_) {
        unsigned char failed_member = 0;
        return CheckEnterEventDungeon(
                   dungeon, failed_member, inventory_snapshots, inventory_slots) ==
               1;
    }
    if (dungeon->required_item_.item_id == -1 ||
        dungeon->required_item_.delete_after_start != 1 ||
        battle_field_.instance_dungeon_mode_ == 1) {
        return true;
    }

    const int required_item = dungeon->required_item_.item_id;
    const int required_count = dungeon->required_item_.item_count;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        const int slot = CurrentInventoryR(members_[seat].user)
                             ->check_item_exist(required_item);
        inventory_slots[seat] = slot;
        if (slot == -1) {
            return false;
        }
        SnapshotInventoryItem(inventory_snapshots[seat], members_[seat].user, slot);
        if (ItemCount(inventory_snapshots[seat]) < required_count) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x859EAC2
// IDA_NAME: _ZN6CParty22UseAncientDungeonItemsEPK8CDungeonP10Inven_ItemPi
// DEMANGLED: CParty::UseAncientDungeonItems(CDungeon const*,Inven_Item *,int *)
int CParty::UseAncientDungeonItems(const CDungeon* dungeon,
                                   Inven_Item* inventory_snapshots,
                                   int* inventory_slots) {
    if (battle_field_.instance_dungeon_mode_ == 1 ||
        !dungeon->required_item_.delete_after_start) {
        return 1;
    }

    const int required_count = dungeon->required_item_.item_count;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }

        CUser* const user = members_[seat].user;
        if (static_cast<unsigned char>(CurrentInventoryW(user)->delete_item(
                static_cast<INVEN_TYPE>(1),
                inventory_slots[seat],
                required_count,
                static_cast<eItemDelReason>(14),
                true)) != 1u) {
            cMyTrace trace(
                "bool CParty::UseAncientDungeonItems(const CDungeon*, Inven_Item*, int*)",
                2656,
                5);
            trace("CParty::UseAncientDungeonItems, delete_item failed , User ch=%d , %d %d",
                  user->getCurCharacNo());
            return 0;
        }

        const Inven_Item& item = inventory_snapshots[seat];
        const int remaining_count = ItemCount(item) - required_count;
        PacketGuard packet;
        packet.put_header(0, 14);
        packet.put_byte(0);
        packet.put_short(1);
        packet.put_short(static_cast<short>(inventory_slots[seat]));
        packet.put_int(remaining_count ? ItemKey(item) : -1);
        packet.put_int(remaining_count);
        packet.put_byte(static_cast<unsigned char>(item.GetItemAttr()));
        packet.put_short(ItemDuration(item));
        const uint8_t seal_or_state = item.seal_or_state_;
        packet.put_byte(seal_or_state);
        packet.put_int(ItemAddInfo(item));
        packet.put_byte(ItemAbilityType(item));
        packet.put_short(ItemAbilityValue(item));
        packet.put_byte(0);
        packet.put_packet(item);
        packet.finalize(true);
        user->Send(packet);
        user->SaveInventory();
    }
    return 1;
}

// IDA_EA: 0x859EE66
// IDA_NAME: _ZN6CParty22CheckEnterEventDungeonEPK8CDungeonRhP10Inven_ItemPi
// DEMANGLED: CParty::CheckEnterEventDungeon(CDungeon const*,unsigned char &,Inven_Item *,int *)
int CParty::CheckEnterEventDungeon(const CDungeon* dungeon,
                                   unsigned char& failed_member,
                                   Inven_Item* inventory_snapshots,
                                   int* inventory_slots) {
    const int event_item = dungeon->additional_required_item_.item_id;
    const int ordinary_item = dungeon->required_item_.item_id;
    if (event_item == -1 && ordinary_item == -1) {
        return 1;
    }
    if (event_item != -1 &&
        dungeon->additional_required_item_.delete_after_start != 1 &&
        ordinary_item != -1 &&
        dungeon->required_item_.delete_after_start != 1) {
        return 1;
    }

    const int event_count = dungeon->additional_required_item_.item_count;
    const int ordinary_count = dungeon->required_item_.item_count;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        bool usable = true;

        int slot = CurrentInventoryR(user)->check_item_exist(event_item);
        inventory_slots[seat] = slot;
        if (slot == -1) {
            failed_member = static_cast<unsigned char>(seat);
            usable = false;
        } else {
            SnapshotInventoryItem(inventory_snapshots[seat], user, slot);
            if (ItemCount(inventory_snapshots[seat]) < event_count) {
                failed_member = static_cast<unsigned char>(seat);
                usable = false;
            }
        }

        if (!usable) {
            usable = true;
            slot = CurrentInventoryR(user)->check_item_exist(ordinary_item);
            inventory_slots[seat] = slot;
            if (slot == -1) {
                failed_member = static_cast<unsigned char>(seat);
                usable = false;
            } else {
                SnapshotInventoryItem(inventory_snapshots[seat], user, slot);
                if (ItemCount(inventory_snapshots[seat]) < ordinary_count) {
                    failed_member = static_cast<unsigned char>(seat);
                    usable = false;
                }
            }
        }
        if (!usable) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x859F1CE
// IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeonRhb
// DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*,unsigned char &,bool)
int CParty::CheckEnterDimensionDungeon(const CDungeon* dungeon,
                                       unsigned char& failed_member,
                                       bool check_party_count) {
    const int member_count = this->get_member_count();
    const int dimension_index = dungeon->get_dimension_possible() - 1;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        if (check_party_count &&
            dungeon->get_dimension_min_partymem() > member_count &&
            static_cast<unsigned char>(user->isGMUser()) != 1u) {
            return 171;
        }
        if (user->get_charac_level() < dungeon->get_min_level()) {
            failed_member = static_cast<unsigned char>(seat);
            return 172;
        }
        const _Charac_info* const character = user->getCurCharacR();
        if (static_cast<signed char>(
                character->fields_.dimension_inout_[dimension_index]) <= 0 &&
            battle_field_.instance_dungeon_mode_ != 1 &&
            static_cast<unsigned char>(user->isGMUser()) != 1u) {
            failed_member = static_cast<unsigned char>(seat);
            return 173;
        }
    }
    return 1;
}

// IDA_EA: 0x859F36A
// IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeonRhc
// DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*,unsigned char &,char)
int CParty::CheckEnterDimensionDungeon(const CDungeon* dungeon,
                                       unsigned char& failed_member,
                                       char region) {
    bool check_party_count = true;
    DimensionPartyCount count(static_cast<char>(
                                  battle_field_.instance_dungeon_mode_),
                              region,
                              -1);
    if (static_cast<unsigned char>(
            dungeon->get_dimension_member_count(count)) != 0u &&
        count.third_count <= this->get_member_count()) {
        check_party_count = false;
    }
    return CheckEnterDimensionDungeon(dungeon, failed_member, check_party_count);
}

// IDA_EA: 0x859F3FC
// IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeon
// DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*)
int CParty::CheckEnterDimensionDungeon(const CDungeon* dungeon) {
    if (static_cast<signed char>(
            battle_field_.dungeon_->get_dimension_possible()) <= 0) {
        return 0;
    }

    const int member_count = this->get_member_count();
    const int dimension_index = dungeon->get_dimension_possible() - 1;
    DimensionPartyCount count(static_cast<char>(
                                  battle_field_.instance_dungeon_mode_),
                              static_cast<char>(battle_field_.dungeon_difficulty_),
                              -1);
    if (static_cast<unsigned char>(
            dungeon->get_dimension_member_count(count)) != 0u) {
        if (count.third_count > member_count) {
            return 171;
        }
    } else if (dungeon->get_dimension_min_partymem() > member_count) {
        return 171;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        const _Charac_info* const character = user->getCurCharacR();
        if (static_cast<signed char>(
                character->fields_.dimension_inout_[dimension_index]) <= 0 &&
            battle_field_.instance_dungeon_mode_ != 1 &&
            static_cast<unsigned char>(user->isGMUser()) != 1u) {
            return 173;
        }
    }
    return 0;
}

// IDA_EA: 0x859F582
// IDA_NAME: _ZN6CParty22getPartyMemberAvgLevelEv
// DEMANGLED: CParty::getPartyMemberAvgLevel(void)
int CParty::getPartyMemberAvgLevel() {
    int level_sum = 0;
    int member_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
            level_sum += members_[seat].user->get_charac_level();
            ++member_count;
        }
    }
    return member_count ? level_sum / member_count : 0;
}

// IDA_EA: 0x859F612
// IDA_NAME: _ZN6CParty25setStandardDimensionLevelEPK8CDungeon
// DEMANGLED: CParty::setStandardDimensionLevel(CDungeon const*)
int CParty::setStandardDimensionLevel(const CDungeon* /*dungeon*/) {
    int level_sum = 0;
    int member_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
            CUser* const user = members_[seat].user;
            user->isGMUser();
            level_sum += user->get_charac_level();
            ++member_count;
        }
    }
    battle_field_.dimension_level_ = level_sum / member_count;
    return 1;
}

// IDA_EA: 0x859F6D4
// IDA_NAME: _ZN6CParty25getStandardDimensionLevelEv
// DEMANGLED: CParty::getStandardDimensionLevel(void)
int CParty::getStandardDimensionLevel() {
    return battle_field_.dimension_level_;
}

// IDA_EA: 0x859F6E2
// IDA_NAME: _ZN6CParty20checkDugeonInoutTimeEi
// DEMANGLED: CParty::checkDugeonInoutTime(int)
int CParty::checkDugeonInoutTime(int dungeon_index) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u &&
            static_cast<unsigned char>(members_[seat].user->isGMUser()) != 0u) {
            return 1;
        }
    }
    ServerParameterScript* const server_parameter =
        G_CDataManager()->get_server_parameter_script();
    return server_parameter->isDungeonOpen(dungeon_index);
}

// IDA_EA: 0x859F7FA
// IDA_NAME: _ZN6CParty19check_dungeon_startEPK8CDungeonRi
// DEMANGLED: CParty::check_dungeon_start(CDungeon const*,int &)
int CParty::check_dungeon_start(const CDungeon* dungeon, int& failed_member) {
    if (dungeon_runtime::IsLimitedInCountDungeon(dungeon->get_index())) {
        if (battle_field_.instance_dungeon_mode_ != 1) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                    continue;
                }
                CUser* const user = members_[seat].user;
                int maximum = 0;
                int remaining = 0;
                if (!user || !dungeon_runtime::IsInCountLoaded(user) ||
                    !dungeon_runtime::GetRemainingInCount(
                        user, dungeon->get_index(), &maximum, &remaining) ||
                    remaining <= 0) {
                    failed_member = seat;
                    return 245;
                }
            }
        }
    }
    if (dungeon->blood_dungeon_type_ == 1u) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
                continue;
            }
            CUser* const user = members_[seat].user;
            const int admission_fee = get_blood_dungeon_admission_fee(user);
            if (CurrentInventoryR(user)->get_money() < admission_fee) {
                failed_member = seat;
                return 10;
            }
            if (user->get_charac_level() < dungeon->get_min_level()) {
                failed_member = seat;
                return 95;
            }
            CCharacBloodDungeon* const blood =
                static_cast<CCharacBloodDungeon*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
            if (dungeon->get_limit_Inout_count() <= blood->GetEnterCount()) {
                return 245;
            }
        }
    } else if (dungeon->blood_dungeon_type_ == 2u) {
        const int dungeon_index = dungeon->get_index();
        if (static_cast<unsigned char>(
                checkDugeonInoutTime(dungeon_index)) != 1u) {
            return 244;
        }
        if (CPowerManager::IsPowerWarEventOn()) {
            return 246;
        }
        if (GlobalData::s_villageMonsterMgr->OnEvent()) {
            return 247;
        }
        if (dungeon->get_limit_party_count() < this->get_member_count()) {
            return 243;
        }
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
                continue;
            }
            CUser* const user = members_[seat].user;
            if (user->get_charac_level() < dungeon->get_min_level()) {
                failed_member = seat;
                return 248;
            }
            const _Charac_info* const character = user->getCurCharacR();
            if (static_cast<signed char>(
                    character->fields_.blood_inout_count_) <= 0 &&
                battle_field_.instance_dungeon_mode_ != 1) {
                failed_member = seat;
                return 245;
            }
        }
    } else if (dungeon->isTournamentDungeon() &&
               dungeon->get_index() == 122) {
        if (dungeon->get_limit_party_count() < this->get_member_count()) {
            return 243;
        }
        if (battle_field_.instance_dungeon_mode_ != 1) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                    continue;
                }
                CUser* const user = members_[seat].user;
                if (user && user->isGMUser() != 1 &&
                    static_cast<signed char>(
                        user->getUltimateInoutValue(0)) <= 0) {
                    failed_member = seat;
                    return 245;
                }
            }
        }
    } else if (dungeon->get_limit_party_count() < this->get_member_count()) {
        return 243;
    }
    return 0;
}

// IDA_EA: 0x859FB16
// IDA_NAME: _ZN6CParty19check_dungeon_startEPK8CDungeon
// DEMANGLED: CParty::check_dungeon_start(CDungeon const*)
int CParty::check_dungeon_start(const CDungeon* dungeon) {
    if (dungeon_runtime::IsLimitedInCountDungeon(dungeon->get_index())) {
        if (battle_field_.instance_dungeon_mode_ != 1) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                    continue;
                }
                CUser* const user = members_[seat].user;
                int maximum = 0;
                int remaining = 0;
                if (!user || !dungeon_runtime::IsInCountLoaded(user) ||
                    !dungeon_runtime::GetRemainingInCount(
                        user, dungeon->get_index(), &maximum, &remaining) ||
                    remaining <= 0) {
                    return 245;
                }
            }
        }
    }
    if (dungeon->blood_dungeon_type_ == 1u) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
                CUser* const user = members_[seat].user;
                if (CurrentInventoryR(user)->get_money() <
                    get_blood_dungeon_admission_fee(user)) {
                    return 10;
                }
            }
        }
    } else if (dungeon->blood_dungeon_type_ == 2u) {
        const int dungeon_index = dungeon->get_index();
        if (static_cast<unsigned char>(
                checkDugeonInoutTime(dungeon_index)) != 1u) {
            return 244;
        }
        if (CPowerManager::IsPowerWarEventOn()) {
            return 246;
        }
        if (GlobalData::s_villageMonsterMgr->OnEvent()) {
            return 247;
        }
        if (dungeon->get_limit_party_count() < this->get_member_count()) {
            return 243;
        }
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
                CUser* const user = members_[seat].user;
                const _Charac_info* const character = user->getCurCharacR();
                if (static_cast<signed char>(
                        character->fields_.blood_inout_count_) <= 0 &&
                    battle_field_.instance_dungeon_mode_ != 1) {
                    return 245;
                }
            }
        }
    } else if (dungeon->isTournamentDungeon() &&
               dungeon->get_index() == 122) {
        if (dungeon->get_limit_party_count() < this->get_member_count()) {
            return 243;
        }
        if (battle_field_.instance_dungeon_mode_ != 1) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                    continue;
                }
                CUser* const user = members_[seat].user;
                if (user && user->isGMUser() != 1 &&
                    static_cast<signed char>(
                        user->getUltimateInoutValue(0)) <= 0) {
                    return 245;
                }
            }
        }
    }
    return 0;
}

// IDA_EA: 0x85A7D86
// IDA_NAME: _ZN6CParty29CheckDestroyConditionSealDoorEPK8CDungeonRSt6vectorIiSaIiEES6_
// DEMANGLED: CParty::CheckDestroyConditionSealDoor(CDungeon const*,std::vector<int,std::allocator<int>> &,std::vector<int,std::allocator<int>> &)
int CParty::CheckDestroyConditionSealDoor(const CDungeon* dungeon,
                                          std::vector<int>& failed_members,
                                          std::vector<int>& seal_item_ids) {
    int entry_eligible_member_count = 0;
    int member_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        ++member_count;
        CUser* const user = members_[seat].user;
        if (CheckHellDungeonFreepassItemHaveAndDel(user, false)) {
            ++entry_eligible_member_count;
            continue;
        }
        if (dungeon->get_min_level() <= 44) {
            return 0;
        }

        const int required_count =
            getHellPartyEntranceItemNeedCount(dungeon->get_min_level());
        int matching_item_kinds = 0;
        for (std::vector<int>::const_iterator it = seal_item_ids.begin();
             it != seal_item_ids.end();
             ++it) {
            const int slot = CurrentInventoryR(user)->check_item_exist(*it);
            if (slot == -1) {
                continue;
            }
            Inven_Item item;
            SnapshotInventoryItem(item, user, slot);
            if (ItemCount(item) >= required_count) {
                ++matching_item_kinds;
            }
        }
        if (matching_item_kinds == static_cast<int>(seal_item_ids.size())) {
            ++entry_eligible_member_count;
        } else {
            failed_members.push_back(seat);
        }
    }
    return member_count <= entry_eligible_member_count;
}

// IDA_EA: 0x85A8038
// IDA_NAME: _ZN6CParty16UseSealDoorItemsERSt6vectorIiSaIiEE
// DEMANGLED: CParty::UseSealDoorItems(std::vector<int,std::allocator<int>> &)
int CParty::UseSealDoorItems(std::vector<int>& seal_item_ids) {
    const CDungeon* const dungeon = battle_field_.dungeon_;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        if (CheckHellDungeonFreepassItemHaveAndDel(user, true)) {
            continue;
        }
        for (std::vector<int>::const_iterator it = seal_item_ids.begin();
             it != seal_item_ids.end();
             ++it) {
            if (dungeon->get_min_level() <= 44) {
                hell_dungeon_seal_door_ready_ = 0;
                return 0;
            }
            const int required_count =
                getHellPartyEntranceItemNeedCount(dungeon->get_min_level());
            const int slot = CurrentInventoryR(user)->check_item_exist(*it);
            if (slot == -1) {
                hell_dungeon_seal_door_ready_ = 0;
                return 0;
            }
            if (static_cast<unsigned char>(CurrentInventoryW(user)->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    slot,
                    required_count,
                    static_cast<eItemDelReason>(23),
                    true)) == 1u) {
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     slot);
            } else {
                cMyTrace trace("bool CParty::UseSealDoorItems(IntVector&)", 7458, 5);
                trace("CParty::UseSealDoorItems, delete_item failed , User ch =%d , %d %d",
                      user->getCurCharacNo());
            }
        }
    }
    hell_dungeon_seal_door_ready_ = 1;
    return 1;
}

// IDA_EA: 0x85A82D2
// IDA_NAME: _ZN6CParty14IsClearedQuestEi
// DEMANGLED: CParty::IsClearedQuest(int)
int CParty::IsClearedQuest(int quest_index) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u &&
            static_cast<unsigned char>(
                reinterpret_cast<UserQuest*>(
                    members_[seat].user->getCurCharacQuestR())
                    ->isClearQuest(quest_index)) != 1u) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85A834A
// IDA_NAME: _ZN6CParty19CheckHellCoinLimintEi
// DEMANGLED: CParty::CheckHellCoinLimint(int)
bool CParty::CheckHellCoinLimint(int limit) {
    return !static_cast<unsigned char>(CBattle_Field::IsEnableHellDungeon()) ||
           battle_field_.dungeon_type_value_ != 1 ||
           static_cast<unsigned char>(
               battle_field_.IsKilledAllHellGruoups()) == 1u ||
           GetUsedCoinCount() < limit;
}

// IDA_EA: 0x822D8CE
// IDA_NAME: _ZN6CParty21IncreaseUsedCoinCountEv
// DEMANGLED: CParty::IncreaseUsedCoinCount(void)
CParty* CParty::IncreaseUsedCoinCount() {
    ++used_coin_count_;
    return this;
}

// IDA_EA: 0x822D8E8
// IDA_NAME: _ZN6CParty16GetUsedCoinCountEv
// DEMANGLED: CParty::GetUsedCoinCount(void)
int CParty::GetUsedCoinCount() {
    return used_coin_count_;
}

// IDA_EA: 0x85A8C88
// IDA_NAME: _ZN6CParty12canFatigueUpEh
// DEMANGLED: CParty::canFatigueUp(unsigned char)
bool CParty::canFatigueUp(unsigned char /*state*/) {
    const CDungeon* const dungeon = battle_field_.dungeon_;
    if (battle_field_.instance_dungeon_mode_ == 1 || !dungeon) {
        return false;
    }
    return !dungeon->blood_dungeon_type_ &&
           dungeon->village_attack_revenge_dungeon_ != 1u &&
           static_cast<unsigned char>(dungeon->isNoFatigueDungeon()) != 1u;
}

// IDA_EA: 0x85BA804
// IDA_NAME: _ZN6CParty30CheckEnterVillageAttackRevengeEPK8CDungeon
// DEMANGLED: CParty::CheckEnterVillageAttackRevenge(CDungeon const*)
int CParty::CheckEnterVillageAttackRevenge(const CDungeon* dungeon) {
    if (!dungeon->village_attack_revenge_dungeon_) {
        return 0;
    }
    const int member_count = this->get_member_count();
    if (static_cast<unsigned char>(
            GlobalData::s_revengeDungeonMgr->IsOpenRevengeDungeon()) != 1u) {
        return 21;
    }
    if (dungeon->revenge_dungeon_max_party_members_ < member_count ||
        dungeon->revenge_dungeon_min_party_members_ > member_count) {
        return 171;
    }
    const unsigned char required_count = dungeon->revenge_dungeon_entry_limit_;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = members_[seat].user;
        const unsigned char current_count =
            static_cast<unsigned char>(user->GetCurRevengeDungeonCount());
        if (!current_count || required_count < current_count) {
            user->SetCurRevengeDungeonCount(0);
            return 173;
        }
    }
    return 0;
}

// IDA_EA: 0x85BC86E
// IDA_NAME: _ZN6CParty38CheckHellDungeonFreepassItemHaveAndDelEP5CUserb
// DEMANGLED: CParty::CheckHellDungeonFreepassItemHaveAndDel(CUser *,bool)
bool CParty::CheckHellDungeonFreepassItemHaveAndDel(CUser* user, bool consume) {
    CWorldMap*& world_map = world_map_;
    if (!world_map) {
        const int world_map_index = G_GameWorld()->GetWorldMapIndex(user);
        if (world_map_index > 0) {
            const CWorldMapList* const world_map_list =
                CDataManagerWorldMapList(G_CDataManager());
            world_map = world_map_list
                            ? world_map_list->find_world_map(world_map_index)
                            : 0;
        }
        if (!world_map) {
            return false;
        }
    }

    const std::vector<std::pair<int, int> >& freepass_items =
        world_map->hell_quests_;
    int slot = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             freepass_items.begin();
         it != freepass_items.end();
         ++it) {
        slot = CurrentInventoryR(user)->check_item_exist(it->first);
        if (slot == -1) {
            continue;
        }
        if (!consume) {
            return slot > 0;
        }
        if (static_cast<unsigned char>(CurrentInventoryW(user)->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                it->second,
                static_cast<eItemDelReason>(23),
                true)) == 1u) {
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 slot);
            return slot > 0;
        }
        cMyTrace trace(
            "bool CParty::CheckHellDungeonFreepassItemHaveAndDel(CUser*, bool)",
            17659,
            5);
        trace("CParty::UseSealDoorItems, freepass delete_item failed , User ch=%d , %d %d",
              user->getCurCharacNo());
    }
    return slot > 0;
}
