#include "common/manager/global_tools.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/game_manager.h"
#include "infra/security/policy/hades_public_abi.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/concurrent/timer/time_gate.h"
#include "common/manager/user_history_log_abi.h"
#include "common/manager/system_time.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/invalid_area_move.h"
#include "old_game/world/region/world_area_allocators.h"
#include "old_game/world/region/world_area_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "core/config/env/environment.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/economy/mall/crane_minigame_runtime.h"

#include "common/manager/statistics_shusia_service_abi.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_data/PVF/town/town_script.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <time.h>
#include <vector>


namespace {

}  // namespace

// IDA_EA: 0x80F131E
// IDA_NAME: _ZNK9GameWorld26IsIntegratedPvPBaseChannelEv
// DEMANGLED: GameWorld::IsIntegratedPvPBaseChannel(void)const
bool GameWorld::IsIntegratedPvPBaseChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel <= 0x10u && ((1u << channel) & 0x1E700u) != 0;
}

// IDA_EA: 0x8116B20
// IDA_NAME: _ZNK9GameWorld14GetChannelTypeEv
// DEMANGLED: GameWorld::GetChannelType(void)const
int GameWorld::GetChannelType() const {
    return channel_type_;
}

// IDA_EA: 0x86CBC85
// IDA_NAME: _Z19deleteSpecificItemsP5CUserPKSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: deleteSpecificItems(CUser *,std::vector<std::pair<int,int>> const*)
int deleteSpecificItems(
    CUser* user,
    const std::vector<std::pair<int, int> >* matched_items) {
    if (user->get_state() > 2) {
        std::vector<std::pair<int, int> > changed_slots;
        user->deleteSpecificItem(*matched_items, changed_slots);

        PacketGuard packets[29];
        int packet_indices[29] = {};
        int packet_counts[29] = {};
        for (int item_space = 0; item_space <= 28; ++item_space) {
            PacketGuard& packet = packets[item_space];
            packet.put_header(0, 14);
            packet.put_byte(item_space);
            packet_indices[item_space] = packet.get_index();
            packet.put_short(0);
        }

        for (std::vector<std::pair<int, int> >::const_iterator it =
                 changed_slots.begin();
             it != changed_slots.end();
             ++it) {
            const int item_space = it->first;
            const int slot = it->second;
            PacketGuard& packet = packets[item_space];

            if (item_space == 2) {
                user->getCurCharacCargoR()->MakeItemPacket(&packet, slot);
                ++packet_counts[item_space];
            } else if (item_space == 12) {
                if (user->IsExistAccountCargo()) {
                    user->GetAccountCargo()->MakeItemPacket(&packet, slot);
                    ++packet_counts[item_space];
                }
            } else {
                int inventory_type = 1;
                switch (item_space) {
                    case 3:
                        inventory_type = 0;
                        break;
                    case 7:
                        inventory_type = 3;
                        break;
                    case 1:
                        inventory_type = 2;
                        break;
                }

                CInventory* const inventory = user->getCurCharacInvenR();
                const Inven_Item item =
                    inventory->GetInvenSlot(inventory_type, slot);
                if (item.item_id_) {
                    inventory->MakeItemPacket(
                        static_cast<INVEN_TYPE>(inventory_type), slot, packet);
                } else {
                    packet.put_short(slot);
                    packet.put_int(-1);
                    packet.put_int(0);
                    packet.put_byte(0);
                    packet.put_short(0);
                    packet.put_byte(0);
                    packet.put_int(0);
                    packet.put_byte(0);
                    packet.put_short(0);
                    packet.put_byte(0);
                    packet.put_packet(g_emptySlot);
                }
                ++packet_counts[item_space];
            }
        }

        for (int item_space = 0; item_space <= 28; ++item_space) {
            if (packet_counts[item_space]) {
                PacketGuard& packet = packets[item_space];
                packet.put_short(packet_indices[item_space],
                                 packet_counts[item_space]);
                packet.finalize(true);
                user->Send(packet);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x86CCBF4
// IDA_NAME: _ZN9GameWorld19deleteSpecificItemsEPKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: GameWorld::deleteSpecificItems(std::vector<std::pair<int,int>> const*)
bool GameWorld::deleteSpecificItems(
    const std::vector<std::pair<int, int> >* matched_items) {
    GameWorldUserMap::iterator it = users_in_world_.begin();
    while (it != users_in_world_.end()) {
        ::deleteSpecificItems(it->second, matched_items);
        ++it;
    }
    return false;
}

// IDA_EA: 0x813495C
// IDA_NAME: _ZN9GameWorld8send_allER11PacketGuardP5CUser
// DEMANGLED: GameWorld::send_all(PacketGuard &,CUser *)
int GameWorld::send_all(PacketGuard& packet, CUser* user) {
    GameWorld* const world = this;
    if (static_cast<unsigned char>(user->IsCurCharacVisible())) {
        return world->send_all(packet);
    }
    return user->Send(packet);
}

// IDA_EA: 0x81424E8
// IDA_NAME: _ZNK9GameWorld12IsPVPChannelEv
// DEMANGLED: GameWorld::IsPVPChannel(void)const
bool GameWorld::IsPVPChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    // 0828 keeps the legacy PvP set and adds channel type 20.  Zombie
    // (type 19) deliberately remains outside the PvP predicate.
    return (channel <= 0x10u && ((1u << channel) & 0x1E7F0u) != 0) ||
           channel == 20u;
}

// IDA_EA: 0x81A0AB2
// IDA_NAME: _ZNK9GameWorld16IsFreePvPChannelEv
// DEMANGLED: GameWorld::IsFreePvPChannel(void)const
bool GameWorld::IsFreePvPChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel <= 0x10u && ((1u << channel) & 0x166A0u) != 0;
}

// IDA_EA: 0x82343BA
// IDA_NAME: _ZN9GameWorld20GetStatisticsNatTypeEv
// DEMANGLED: GameWorld::GetStatisticsNatType(void)
char* GameWorld::GetStatisticsNatType() {
    return reinterpret_cast<char*>(&nat_type_statistics_);
}

// IDA_EA: 0x82343C8
// IDA_NAME: _ZN9GameWorld21IsSameTeamChannelJoinEv
// DEMANGLED: GameWorld::IsSameTeamChannelJoin(void)
bool GameWorld::IsSameTeamChannelJoin() {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel <= 0x0Fu && ((1u << channel) & 0xC200u) != 0;
}

// IDA_EA: 0x82343FC
// IDA_NAME: _ZN9GameWorld24IsEnchantRevisionChannelEv
// DEMANGLED: GameWorld::IsEnchantRevisionChannel(void)
bool GameWorld::IsEnchantRevisionChannel() {
    const int channel = channel_type_;
    return channel == 7 || channel == 14;
}

// IDA_EA: 0x823441E
// IDA_NAME: _ZNK9GameWorld20IsPvPSkilTreeChannelEv
// DEMANGLED: GameWorld::IsPvPSkilTreeChannel(void)const
bool GameWorld::IsPvPSkilTreeChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 14u <= 2u;
}

// IDA_EA: 0x823443E
// IDA_NAME: _ZNK9GameWorld24IsEquipSlotSwitchChannelEv
// DEMANGLED: GameWorld::IsEquipSlotSwitchChannel(void)const
bool GameWorld::IsEquipSlotSwitchChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 14u <= 2u;
}

// IDA_EA: 0x844DC26
// IDA_NAME: _ZN9GameWorld27getStatisticsToObserveCrashEv
// DEMANGLED: GameWorld::getStatisticsToObserveCrash(void)
char* GameWorld::getStatisticsToObserveCrash() {
    return reinterpret_cast<char*>(&crash_observation_count_);
}

// IDA_EA: 0x844DC32
// IDA_NAME: _ZN9GameWorld26getStatisticsShusiaServiceEv
// DEMANGLED: GameWorld::getStatisticsShusiaService(void)
StatisticsShusiaService* GameWorld::getStatisticsShusiaService() {
    return &shusia_statistics_;
}

// IDA_EA: 0x844DC40
// IDA_NAME: _ZN9GameWorld16getStatisticsPvPEv
// DEMANGLED: GameWorld::getStatisticsPvP(void)
StatisticsPvP* GameWorld::getStatisticsPvP() {
    return &pvp_statistics_;
}

// IDA_EA: 0x844DC4E
// IDA_NAME: _ZNK9GameWorld18IsSchoolPvPChannelEv
// DEMANGLED: GameWorld::IsSchoolPvPChannel(void)const
bool GameWorld::IsSchoolPvPChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 9u <= 1u;
}

// IDA_EA: 0x84ED128
// IDA_NAME: _ZNK9GameWorld31IsCharacterLevelRevisionChannelEv
// DEMANGLED: GameWorld::IsCharacterLevelRevisionChannel(void)const
bool GameWorld::IsCharacterLevelRevisionChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 14u <= 2u;
}

// IDA_EA: 0x85DFA76
// IDA_NAME: _ZNK9GameWorld32IsCharacterPvPExpRevisionChannelEv
// DEMANGLED: GameWorld::IsCharacterPvPExpRevisionChannel(void)const
bool GameWorld::IsCharacterPvPExpRevisionChannel() const {
    return channel_type_ == 15;
}

// IDA_EA: 0x85DFA92
// IDA_NAME: _ZNK9GameWorld20IsWinPointPvPChannelEv
// DEMANGLED: GameWorld::IsWinPointPvPChannel(void)const
bool GameWorld::IsWinPointPvPChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel <= 0x0Fu && ((1u << channel) & 0x8110u) != 0;
}

// IDA_EA: 0x869779E
// IDA_NAME: _ZN9GameWorld31IsCheckUnusableEquipmentChannelEv
// DEMANGLED: GameWorld::IsCheckUnusableEquipmentChannel(void)
bool GameWorld::IsCheckUnusableEquipmentChannel() {
    return channel_type_ == 14;
}

// IDA_EA: 0x86977BA
// IDA_NAME: _ZNK9GameWorld22IsPvPVillageMapChannelEv
// DEMANGLED: GameWorld::IsPvPVillageMapChannel(void)const
bool GameWorld::IsPvPVillageMapChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 14u <= 2u;
}

// IDA_EA: 0x86977DA
// IDA_NAME: _ZNK9GameWorld30IsCharacterStatRevisionChannelEv
// DEMANGLED: GameWorld::IsCharacterStatRevisionChannel(void)const
bool GameWorld::IsCharacterStatRevisionChannel() const {
    const uint32_t channel = static_cast<uint32_t>(channel_type_);
    return channel - 14u <= 2u;
}

// IDA_EA: 0x86C4550
// IDA_NAME: _ZN9GameWorld21get_UserCount_InWorldEv
// DEMANGLED: GameWorld::get_UserCount_InWorld(void)
int GameWorld::get_UserCount_InWorld() {
    return users_in_world_.size();
}

// IDA_EA: 0x86CD568
// IDA_NAME: _ZN9GameWorld17InsertUserToIDMapEPKcj
// DEMANGLED: GameWorld::InsertUserToIDMap(char const*,unsigned int)
bool GameWorld::InsertUserToIDMap(const char* charac_name,
                                  unsigned int account_id) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return false;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return false;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end() || it->second.account_id == 0) {
        (*id_map)[key].account_id = account_id;
    }
    return true;
}

// IDA_EA: 0x86CD6B8
// IDA_NAME: _ZN9GameWorld17DeleteUserInIDMapEPKc
// DEMANGLED: GameWorld::DeleteUserInIDMap(char const*)
bool GameWorld::DeleteUserInIDMap(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return false;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return false;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return false;
    }
    id_map->erase(it);
    return true;
}

// IDA_EA: 0x86CD7E2
// IDA_NAME: _ZN9GameWorld16GetIDUserInIDMapEPKc
// DEMANGLED: GameWorld::GetIDUserInIDMap(char const*)
unsigned int GameWorld::GetIDUserInIDMap(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return 0;
    }
    return (*id_map)[key].account_id;
}

// IDA_EA: 0x86CD908
// IDA_NAME: _ZN9GameWorld15IsForbiddenChatEPKc
// DEMANGLED: GameWorld::IsForbiddenChat(char const*)
int GameWorld::IsForbiddenChat(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end() || !it->second.forbid_chat) {
        return 0;
    }
    if (world->GetLeftTimeOfForbiddenChat(charac_name)) {
        return 1;
    }
    world->EnableChat(charac_name);
    return 0;
}

// IDA_EA: 0x86CDC60
// IDA_NAME: _ZN9GameWorld10EnableChatEPKc
// DEMANGLED: GameWorld::EnableChat(char const*)
int GameWorld::EnableChat(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return 0;
    }
    if (it->second.forbid_chat) {
        it->second.forbid_chat = 0;
        it->second.forbid_chat_until = 0;
    }
    return 1;
}

// IDA_EA: 0x86CE054
// IDA_NAME: _ZN9GameWorld11DisableChatEPKci
// DEMANGLED: GameWorld::DisableChat(char const*,int)
int GameWorld::DisableChat(const char* charac_name, int seconds) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return 0;
    }
    if (!it->second.forbid_chat) {
        const int cur_ms = GlobalData::system_time()->getCurSec() * 1000;
        it->second.forbid_chat = 1;
        it->second.forbid_chat_until =
            static_cast<uint32_t>(cur_ms + seconds * 1000);
    }
    return 1;
}

// IDA_EA: 0x86CE4A0
// IDA_NAME: _ZN9GameWorld27WhenIsTheEndOfForbiddenChatEPKc
// DEMANGLED: GameWorld::WhenIsTheEndOfForbiddenChat(char const*)
unsigned int GameWorld::WhenIsTheEndOfForbiddenChat(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end() || !it->second.forbid_chat) {
        return 0;
    }
    return it->second.forbid_chat_until;
}

// IDA_EA: 0x86CE73C
// IDA_NAME: _ZN9GameWorld26GetLeftTimeOfForbiddenChatEPKc
// DEMANGLED: GameWorld::GetLeftTimeOfForbiddenChat(char const*)
unsigned int GameWorld::GetLeftTimeOfForbiddenChat(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    if (id_map->find(key) == id_map->end()) {
        return 0;
    }

    const unsigned int end_ms =
        world->WhenIsTheEndOfForbiddenChat(charac_name);
    if (!end_ms) {
        return 0;
    }
    const long long remaining = static_cast<long long>(end_ms) -
        static_cast<long long>(GlobalData::system_time()->getCurSec() * 1000);
    if (remaining <= 0) {
        return 0;
    }
    if (remaining > 59999) {
        return static_cast<unsigned int>(remaining);
    }
    return 60000;
}

// IDA_EA: 0x86CDAB4
// IDA_NAME: _ZN9GameWorld15IsForbiddenMoveEPKc
// DEMANGLED: GameWorld::IsForbiddenMove(char const*)
int GameWorld::IsForbiddenMove(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end() || !it->second.forbid_move) {
        return 0;
    }
    if (world->GetLeftTimeOfForbiddenMove(charac_name)) {
        return 1;
    }
    world->EnableMove(charac_name);
    return 0;
}

// IDA_EA: 0x86CDE5A
// IDA_NAME: _ZN9GameWorld10EnableMoveEPKc
// DEMANGLED: GameWorld::EnableMove(char const*)
int GameWorld::EnableMove(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return 0;
    }
    if (it->second.forbid_move) {
        (*id_map)[key].forbid_move = 0;
        (*id_map)[key].forbid_move_until = 0;
    }
    return 1;
}

// IDA_EA: 0x86CE27A
// IDA_NAME: _ZN9GameWorld11DisableMoveEPKci
// DEMANGLED: GameWorld::DisableMove(char const*,int)
int GameWorld::DisableMove(const char* charac_name, int seconds) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end()) {
        return 0;
    }
    if (!it->second.forbid_move) {
        const int cur_ms = GlobalData::system_time()->getCurSec() * 1000;
        (*id_map)[key].forbid_move = 1;
        (*id_map)[key].forbid_move_until =
            static_cast<uint32_t>(cur_ms + seconds * 1000);
    }
    return 1;
}

// IDA_EA: 0x86CE5EE
// IDA_NAME: _ZN9GameWorld27WhenIsTheEndOfForbiddenMoveEPKc
// DEMANGLED: GameWorld::WhenIsTheEndOfForbiddenMove(char const*)
unsigned int GameWorld::WhenIsTheEndOfForbiddenMove(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    std::map<std::string, SimpleInfo>::iterator it = id_map->find(key);
    if (it == id_map->end() || !it->second.forbid_move) {
        return 0;
    }
    return (*id_map)[key].forbid_move_until;
}

// IDA_EA: 0x86CE958
// IDA_NAME: _ZN9GameWorld26GetLeftTimeOfForbiddenMoveEPKc
// DEMANGLED: GameWorld::GetLeftTimeOfForbiddenMove(char const*)
unsigned int GameWorld::GetLeftTimeOfForbiddenMove(const char* charac_name) {
    GameWorld* const world = this;
    if (!world || !charac_name) {
        return 0;
    }
    if (strlen(charac_name) > 0x1Eu) {
        return 0;
    }

    GameWorldSimpleInfoMap* id_map = &simple_info_by_name_;
    const std::string key(charac_name);
    if (id_map->find(key) == id_map->end()) {
        return 0;
    }

    const unsigned int end_ms =
        world->WhenIsTheEndOfForbiddenMove(charac_name);
    if (!end_ms) {
        return 0;
    }
    const long long remaining = static_cast<long long>(end_ms) -
        static_cast<long long>(GlobalData::system_time()->getCurSec() * 1000);
    if (remaining <= 0) {
        return 0;
    }
    if (remaining > 59999) {
        return static_cast<unsigned int>(remaining);
    }
    return 60000;
}
// IDA_EA: 0x86C8C14
// IDA_NAME: _ZN9GameWorld8send_allER11PacketGuard
// DEMANGLED: GameWorld::send_all(PacketGuard &)
bool GameWorld::send_all(PacketGuard& packet) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    bool result = false;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        result = true;
        CUser* user = it->second;
        if (user->get_state() > 2) {
            user->Send(packet);
        }
    }
    return result;
}

// IDA_EA: 0x86C7F10
// IDA_NAME: _ZN9GameWorld12send_to_userEiR11PacketGuard
// DEMANGLED: GameWorld::send_to_user(int,PacketGuard &)
void GameWorld::send_to_user(int user_uid, PacketGuard& packet) {
    CUser* user = find_from_world(static_cast<uint16_t>(user_uid));
    if (user) {
        user->Send(packet);
    }
}

// IDA_EA: 0x86C9234
// IDA_NAME: _ZN9GameWorld10send_groupER11PacketGuardP7CBelong
// DEMANGLED: GameWorld::send_group(PacketGuard &,CBelong *)
bool GameWorld::send_group(PacketGuard& packet, CBelong* group) {
    GameWorldUserMap* users = &users_in_world_;

    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2 && group->IsRight(user)) {
            user->Send(packet);
        }
    }

    // IDA returns the terminal iterator comparison after the traversal.
    return false;
}

// IDA_EA: 0x86C90AE
// IDA_NAME: _ZN9GameWorld17send_server_groupER11PacketGuardc
// DEMANGLED: GameWorld::send_server_group(PacketGuard &,char)
bool GameWorld::send_server_group(PacketGuard& packet, char server_group) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    bool result = false;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        result = true;
        CUser* user = it->second;
        if (user->get_state() > 2 &&
            user->GetServerGroup() == server_group) {
            user->Send(packet);
        }
    }
    return result;
}

// IDA_EA: 0x86C9184
// IDA_NAME: _ZN9GameWorld19send_all_with_stateER11PacketGuardi
// DEMANGLED: GameWorld::send_all_with_state(PacketGuard &,int)
bool GameWorld::send_all_with_state(PacketGuard& packet, int state) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    bool result = false;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        result = true;
        CUser* user = it->second;
        if (user->get_state() >= state) {
            user->Send(packet);
        }
    }
    return result;
}

// IDA_EA: 0x86D13BA
// IDA_NAME: _ZN9GameWorld26ResetCraneItemNeedMaterialEi
// DEMANGLED: GameWorld::ResetCraneItemNeedMaterial(int)
bool GameWorld::ResetCraneItemNeedMaterial(int rotation_seed) {
    CGameManager* const game_manager = G_CGameManager();
    CraneMinigameManager* const crane_manager =
        game_manager->GetCraneMinigameManager();
    crane_manager->updateCraneItemNeedMaterial(rotation_seed);

    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        CUser* const user = it->second;
        if (user->get_state() > 2) {
            game_manager->GetCraneMinigameManager()
                ->SendCraneItemNeedMaterial(user);
        }
    }

    // The original decompiled loop returns the final iterator comparison,
    // which is false after completing the directory scan.
    return false;
}

// IDA_EA: 0x86D14D4
// IDA_NAME: _ZN9GameWorld20UpdateServerSnapShotEv
// DEMANGLED: GameWorld::UpdateServerSnapShot(void)
bool GameWorld::UpdateServerSnapShot() {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;

    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        CUser* const user = it->second;
        if (!user || !user->getCurCharacR()) {
            continue;
        }

        CParty* const party = user->GetParty();
        const unsigned int dungeon_index =
            party ? static_cast<unsigned int>(party->getDungIndex()) : 0u;
        time_t now = time(0);
        struct tm local_time;
        localtime_r(&now, &local_time);
        char time_text[16];
        char date_text[16];
        strftime(time_text, sizeof(time_text), "%H:%M:%S", &local_time);
        strftime(date_text, sizeof(date_text), "%Y-%m-%d", &local_time);

        cMyTrace trace("void GameWorld::UpdateServerSnapShot()", 7009, 8);
        trace("%u,%d,%u,%d,%d,%d,%u,%d,%s %s",
              static_cast<unsigned int>(user->get_acc_id()),
              user->getCurCharacNo(),
              static_cast<unsigned int>(user->getCurCharacMoney()),
              user->getCurCharacExp(),
              user->get_area(false),
              user->getCurCharacVill(),
              dungeon_index,
              user->GetServerGroup(),
              time_text,
              date_text);
    }

    // Like the original, this return is the final loop-comparison result.
    return false;
}

// IDA_EA: 0x86C93A2
// IDA_NAME: _ZN9GameWorld22send_all_excepted_userER11PacketGuardP5CUser
// DEMANGLED: GameWorld::send_all_excepted_user(PacketGuard &,CUser *)
bool GameWorld::send_all_excepted_user(PacketGuard& packet,
                                       CUser* except_user) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    bool result = false;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        result = true;
        CUser* user = it->second;
        if (user->get_state() > 2 && user != except_user) {
            user->Send(packet);
        }
    }
    return result;
}

// IDA_EA: 0x86C9464
// IDA_NAME: _ZN9GameWorld24find_user_by_charac_nameEPKc
// DEMANGLED: GameWorld::find_user_by_charac_name(char const*)
CUser* GameWorld::find_user_by_charac_name(const char* charac_name) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2 &&
            user->getCurCharacR()) {
            const char* current_name = user->getCurCharacName();
            if (strcmp(current_name, charac_name) == 0) {
                return user;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86C954C
// IDA_NAME: _ZN9GameWorld20make_packet_chat_msgER11PacketGuardP5CUserPcic15ENUM_NOTIPACKET
// DEMANGLED: GameWorld::make_packet_chat_msg(PacketGuard &,CUser *,char *,int,char,ENUM_NOTIPACKET)
int GameWorld::make_packet_chat_msg(PacketGuard& packet,
                                    CUser* user,
                                    char* message,
                                    int message_len,
                                    char chat_type,
                                    ENUM_NOTIPACKET noti_packet) {
    GameWorld* const world = this;
    (void)world;
    packet.clear();
    packet.put_header(0, noti_packet);
    packet.put_byte(chat_type);
    if (user) {
        packet.put_short(user->get_unique_id());
        packet.put_byte(user->GetServerGroup());
    } else {
        packet.put_short(0);
        packet.put_byte(0);
    }
    packet.put_int(message_len);
    packet.put_str(message, message_len);
    return packet.finalize(true);
}

// IDA_EA: 0x86C9638
// IDA_NAME: _ZN9GameWorld15WriteLogChatMsgEP5CUsercPc
// DEMANGLED: GameWorld::WriteLogChatMsg(CUser *,char,char *)
char GameWorld::WriteLogChatMsg(CUser* user,
                                char chat_type,
                                char* message) {
    GameWorld* const world = this;
    const char result = world->GetChannelType() == 6;
    if (result && user) {
        char path[256];
        memset(path, 0, sizeof(path));
        CEnvironment* environment = G_CEnvironment();
        const char* file_name = environment->get_file_name();
        sprintf(path, "./log/%s/GuildWarChat", file_name);
        const int state = user->get_state();
        const int pvp_index = static_cast<int16_t>(user->GetPvpIndex());
        const int guild_key = user->get_charac_guildkey();
        const char* charac_name = user->getCurCharacName();
        CMyFileLog log("void GameWorld::WriteLogChatMsg(CUser*, char, char*)",
                       3437);
        return static_cast<char>(
            log(path,
                "T:%d\tMsg:%s\tC_Name:%s\tG_Key:%d\tR_Idx:%d\tState:%d",
                chat_type,
                message,
                charac_name,
                guild_key,
                pvp_index,
                state));
    }
    return result;
}

// IDA_EA: 0x86D2BA4
// IDA_NAME: _ZN9GameWorld30send_peerTypemsg_sameWorldUserEP5CUserS1_ciiPKci
// DEMANGLED: GameWorld::send_peerTypemsg_sameWorldUser(CUser *,CUser *,char,int,int,char const*,int)
void GameWorld::send_peerTypemsg_sameWorldUser(CUser* from_user,
                                               CUser* to_user,
                                               char chat_type,
                                               int result_code,
                                               int peer_type,
                                               const char* message,
                                               int message_len) {
    GameWorld* const world = this;
    (void)world;
    (void)peer_type;

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_MESSAGE_OTHER_CHANNEL);
    packet.put_byte(chat_type);
    packet.put_byte(result_code);
    const char* charac_name = from_user->getCurCharacName();
    const int charac_name_len = strlen(charac_name);
    packet.put_int(charac_name_len);
    packet.put_str(charac_name, charac_name_len);
    packet.put_byte(0);
    packet.put_int(message_len);
    packet.put_str(message, message_len);
    packet.finalize(true);
    to_user->Send(packet);
}

// IDA_EA: 0x816270C
// IDA_NAME: _ZN9GameWorld15ProcessAllUsersIN29InterSelectCreateDnfEventInfo6RewardEEEvT_
// DEMANGLED: void GameWorld::ProcessAllUsers<InterSelectCreateDnfEventInfo::Reward>(InterSelectCreateDnfEventInfo::Reward)
template <>
void GameWorld::ProcessAllUsers<InterSelectCreateDnfEventInfo::Reward>(
    InterSelectCreateDnfEventInfo::Reward reward) {
    GameWorld* const world = this;
    GameWorldUserMap* users = &users_in_world_;
    for (std::map<uint16_t, CUser*>::iterator it = users->begin();
         it != users->end();
         ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2) {
            reward(user);
        }
    }
}

// IDA_EA: 0x86C471E
// IDA_NAME: _ZNK9GameWorld21get_area_center_pointEiiRiS0_
// DEMANGLED: GameWorld::get_area_center_point(int,int,int &,int &)const
bool GameWorld::get_area_center_point(int village_index,
                                      int area_index,
                                      int& out_x,
                                      int& out_y) const {
    if (village_index < 0 || village_index >= ::MAX_VILLAGE_NUM) {
        return false;
    }

    Area* area = villages_[village_index].getArea(area_index);
    if (!area) {
        return false;
    }

    if (area->map_areas.empty()) {
        return false;
    }

    const MapArea& map_area = area->map_areas.at(0);
    out_x = map_area.width / 2 + map_area.x;
    out_y = map_area.height / 2 + map_area.y;
    return true;
}

// IDA_EA: 0x86C4814
// IDA_NAME: _ZNK9GameWorld18is_available_pointEiiii
// DEMANGLED: GameWorld::is_available_point(int,int,int,int)const
bool GameWorld::is_available_point(int village_index,
                                   int area_index,
                                   int pos_x,
                                   int pos_y) const {
    if (village_index < 0 || village_index >= ::MAX_VILLAGE_NUM) {
        return false;
    }

    Area* area = villages_[village_index].getArea(area_index);
    if (!area) {
        return false;
    }

    if (area->map_areas.empty()) {
        return false;
    }

    for (std::vector<MapArea>::iterator it = area->map_areas.begin();
         it != area->map_areas.end();
         ++it) {
        const MapArea& map_area = *it;
        if (map_area.x <= pos_x && map_area.x + map_area.width >= pos_x &&
            map_area.y <= pos_y && map_area.y + map_area.height >= pos_y) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x86C497C
// IDA_NAME: _ZNK9GameWorld13get_gate_areaEi
// DEMANGLED: GameWorld::get_gate_area(int)const
int GameWorld::get_gate_area(int village_index) const {
    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        return villages_[village_index].get_gate_area();
    }
    return -1;
}

// IDA_EA: 0x86C49B2
// IDA_NAME: _ZNK9GameWorld12GetUserCountEii
// DEMANGLED: GameWorld::GetUserCount(int,int)const
int GameWorld::GetUserCount(int village_index, int area_index) const {
    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        return villages_[village_index].GetUserCount(area_index);
    }
    return 0;
}

// IDA_EA: 0x86C49EE
// IDA_NAME: _ZN9GameWorld16check_valid_areaEii
// DEMANGLED: GameWorld::check_valid_area(int,int)
bool GameWorld::check_valid_area(int village_index, int area_index) {
    if (village_index <= 0 || village_count_ < village_index) {
        LogManager::logFormat(
            1,
            "world.cpp",
            "bool GameWorld::check_valid_area(int, int)",
            0x4A2,
            "[GameWorld::check_valid_area] invalid village : %d, m_iVillCount : %d",
            village_index,
            village_count_);
        return false;
    }

    Village* const village = &villages_[village_index];
    const int area_count = village->area_count;
    if (!area_count) {
        LogManager::logFormat(
            1,
            "world.cpp",
            "bool GameWorld::check_valid_area(int, int)",
            0x4AA,
            "[GameWorld::check_valid_area] invalid area count : %d",
            area_count);
        return false;
    }

    if (area_index >= 0 && area_count > area_index) {
        return true;
    }

    LogManager::logFormat(
        1,
        "world.cpp",
        "bool GameWorld::check_valid_area(int, int)",
        0x4B2,
        "[GameWorld::check_valid_area] invalid area : %d",
        area_index);
    return false;
}

// IDA_EA: 0x86C592C
// IDA_NAME: _ZN9GameWorld18IsPowerWarSafeZoneEii20ENUM_POWER_SIDE_TYPE
// DEMANGLED: GameWorld::IsPowerWarSafeZone(int,int,ENUM_POWER_SIDE_TYPE)
bool GameWorld::IsPowerWarSafeZone(int village_index,
                                   int area_index,
                                   ENUM_POWER_SIDE_TYPE power_side) {
    if (village_index != 7 || !CPowerManager::IsPowerWarEventOn()) {
        return false;
    }
    if (village_index < 0 || village_index >= ::MAX_VILLAGE_NUM) {
        return true;
    }

    Area* area = villages_[village_index].getArea(area_index);
    if (!area) {
        return true;
    }

    const int area_power_type = static_cast<int>(area->town_area_type);
    return (power_side == 1 && area_power_type == 5) ||
           (power_side == 2 && area_power_type == 6);
}

// IDA_EA: 0x86C59DE
// IDA_NAME: _ZN9GameWorld18IsPowerWarSafeZoneEii
// DEMANGLED: GameWorld::IsPowerWarSafeZone(int,int)
bool GameWorld::IsPowerWarSafeZone(int village_index, int area_index) {
    if (village_index != 7 || !CPowerManager::IsPowerWarEventOn()) {
        return false;
    }
    if (village_index < 0 || village_index >= ::MAX_VILLAGE_NUM) {
        return true;
    }

    Area* area = villages_[village_index].getArea(area_index);
    if (!area) {
        return true;
    }

    const int area_power_type = static_cast<int>(area->town_area_type);
    return area_power_type == 5 || area_power_type == 6;
}

// IDA_EA: 0x86CD084
// IDA_NAME: _ZN9GameWorld14getUserVillageEi
// DEMANGLED: GameWorld::getUserVillage(int)
Village* GameWorld::getUserVillage(int village_index) {
    if (village_index > 0 && village_index < ::MAX_VILLAGE_NUM) {
        return &villages_[village_index];
    }
    return 0;
}

// IDA_EA: 0x86CD038
// IDA_NAME: _ZN9GameWorld14getUserVillageEP5CUser
// DEMANGLED: GameWorld::getUserVillage(CUser *)
Village* GameWorld::getUserVillage(CUser* user) {
    if (!user) {
        return 0;
    }

    const int village_index = user->getCurCharacVill();
    if (village_index > 0 && village_index < ::MAX_VILLAGE_NUM) {
        return &villages_[village_index];
    }
    return 0;
}

// IDA_EA: 0x86D1002
// IDA_NAME: _ZN9GameWorld15check_move_areaEP5CUseriiiiibi
// DEMANGLED: GameWorld::check_move_area(CUser *,int,int,int,int,int,bool,int)
int GameWorld::check_move_area(CUser* user,
                               int village_index,
                               int before_area,
                               int teleport_pos_x,
                               int teleport_pos_y,
                               int teleport_direction,
                               bool move_by_user,
                               int move_type) {
    GameWorld* const world = this;
    (void)teleport_pos_x;
    (void)teleport_pos_y;
    (void)teleport_direction;
    (void)move_by_user;

    if (user->get_state() != 3) {
        return 19;
    }
    if (static_cast<unsigned char>(user->CheckInTrade())) {
        return 218;
    }
    if (user->IsBusyDisjointStore()) {
        return 191;
    }

    private_store::CPrivateStoreMgr* private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (private_store_manager->IsBusyPrivateStore(user)) {
        return 60;
    }

    expert_job::CExpertJob* const current_expert_job =
        user->GetCurCharacExpertJob();
    if (current_expert_job && current_expert_job->GetType() == 1) {
        return 56;
    }
    if (user->GetAssaultPlace()) {
        return 156;
    }
    if (static_cast<unsigned char>(user->IsCurCharacGhost()) &&
        !CPowerManager::IsPowerWarEventOn() &&
        static_cast<unsigned char>(user->getCurCharacVill()) !=
            7u) {
        return 159;
    }

    if (static_cast<unsigned char>(user->isGMUser()) != 1u) {
        const int power_side = user->getPowerSide();
        if (IsPowerWarSafeZone(
                village_index,
                before_area,
                static_cast<ENUM_POWER_SIDE_TYPE>(power_side))) {
            return 4;
        }
    }

    const int current_village =
        static_cast<signed char>(user->getCurCharacVill());
    const int current_area = user->get_area(false);
    void* party = user->GetParty();
    if (party && reinterpret_cast<CParty*>(party)->getManager() == user &&
        village_index == current_village && before_area == current_area &&
        move_type != 2) {
        return 19;
    }

    if (!reinterpret_cast<GameWorld*>(world)->check_valid_area(
            village_index, before_area) ||
        !reinterpret_cast<GameWorld*>(world)->check_valid_area(
            current_village, current_area)) {
        return 19;
    }

    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        if (&villages_[village_index]) {
            const int move_town_result =
                user->CheckMoveTown(village_index);
            if (move_town_result) {
                return move_town_result;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86C5A84
// IDA_NAME: _ZN9GameWorld9move_areaEP5CUseriiiiibiii
// DEMANGLED: GameWorld::move_area(CUser *,int,int,int,int,int,bool,int,int,int)
int GameWorld::move_area(CUser* user,
                         int village_index,
                         int area_index,
                         int pos_x,
                         int pos_y,
                         int direction,
                         bool move_by_user,
                         int move_type,
                         int gate_village,
                         int gate_area) {
    GameWorld* const world = this;
    if (!world || !user) {
        return 0;
    }

    const bool time_gate_open =
        ::GlobalData::s_timeGate_ && ::GlobalData::s_timeGate_->isOpen();
    if (!time_gate_open && village_index == 11 && area_index == 5) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_SET_USER_AREA);
        packet.put_byte(0);
        packet.put_byte(19);
        packet.put_byte(village_index);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int current_village =
        static_cast<signed char>(user->getCurCharacVill());
    const int current_area = user->get_area(false);
    bool teleported = false;
    void* party = user->GetParty();
    if (party) {
        CPartyTelePort* party_teleport =
            reinterpret_cast<CParty*>(party)->GetPartyTelePort();
        if (party_teleport &&
            static_cast<unsigned char>(
                party_teleport->get_teleport_state()) == 1u) {
            teleported = true;
        }
    }
    if (move_type == 2) {
        teleported = true;
    }
    if (!teleported && village_index == current_village &&
        area_index == current_area) {
        return 0;
    }

    if (!reinterpret_cast<GameWorld*>(world)->check_valid_area(
            village_index, area_index) ||
        !reinterpret_cast<GameWorld*>(world)->check_valid_area(
            current_village, current_area)) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_SET_USER_AREA);
        packet.put_byte(0);
        packet.put_byte(21);
        packet.put_byte(0);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int move_area_result =
        check_move_area(user,
                        village_index,
                        area_index,
                        pos_x,
                        pos_y,
                        direction,
                        move_by_user,
                        move_type);
    if (move_area_result) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_SET_USER_AREA);
        packet.put_byte(0);
        packet.put_byte(move_area_result);
        packet.put_byte(village_index);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    Village* target_village = 0;
    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        target_village = &villages_[village_index];
    }
    if (!target_village) {
        return 0;
    }
    Village* current_village_ptr = 0;
    if (current_village >= 0 && current_village < ::MAX_VILLAGE_NUM) {
        current_village_ptr = &villages_[current_village];
    }
    const int previous_village =
        static_cast<signed char>(user->getCurCharacVillPrev());
    const int previous_area = user->get_before_area();

    user->set_position(static_cast<unsigned short>(pos_x),
                       static_cast<unsigned short>(pos_y),
                       static_cast<char>(direction));
    user->setCurCharacVill(static_cast<char>(village_index));
    user->set_area(area_index);

    Area* old_area = 0;
    if (current_village_ptr) {
        old_area = current_village_ptr->getArea(current_area);
        if (old_area && move_by_user &&
            !reinterpret_cast<GameWorld*>(world)->IsPVPChannel()) {
            const int cur_sec = GlobalData::system_time()->getCurSec();
            const int before_area_time = user->getBeforeAreaTime();
            CHackLog_InvalidAreaMove* invalid_current =
                reinterpret_cast<CHackLog_InvalidAreaMove*>(current_village);
            if (before_area_time && cur_sec - before_area_time <= 2 &&
                invalid_current->IsInvalidMovableArea(
                    current_area,
                    village_index,
                    area_index,
                    0) &&
                reinterpret_cast<CHackLog_InvalidAreaMove*>(previous_village)
                        ->isMovableRoute(previous_area,
                                         current_village,
                                         current_area,
                                         village_index,
                                         area_index,
                                         0) != 1 &&
                previous_village != village_index &&
                previous_area != area_index) {
                reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(508),
                    1,
                    0,
                    0);
            }
            user->setBeforeAreaTime(cur_sec);

            if (!move_type &&
                old_area->IsMovalbeArea(village_index, area_index) != 1 &&
                invalid_current->IsInvalidMovableArea(
                    current_area,
                    village_index,
                    area_index,
                    0) &&
                current_village == gate_village &&
                current_area == gate_area) {
                CHackLog_InvalidAreaMove invalid_area_move;
                reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(
                        invalid_area_move.GetHackType()),
                    1,
                    static_cast<unsigned int>(gate_village * 10 + gate_area),
                    static_cast<unsigned int>(village_index * 10 +
                                              area_index));
            }
        }
        if (old_area) {
            old_area->delete_user(user);
        }
    }

    int real_area_index = area_index;
    if (G_GameWorld()->IsSchoolPvPChannel() && village_index == 2 &&
        area_index == 4) {
        const int school_area =
            g_schoolMgr.GetSchoolArea(
                static_cast<unsigned int>(user->GetSchoolNo()));
        if (school_area >= 0) {
            real_area_index = school_area;
            user->set_area(real_area_index);
            user->set_trick_area(area_index);
        } else {
            user->set_trick_area(-1);
        }
    } else if (village_index == 8 && area_index < 4) {
        const unsigned int guild_key =
            static_cast<unsigned int>(user->get_charac_guildkey());
        const int guild_area =
            g_guildAgitMgr.GetGuildAgitAreaIndex(guild_key, area_index);
        if (guild_area >= 0) {
            real_area_index = guild_area;
            user->set_area(real_area_index);
            user->set_trick_area(area_index);
        } else {
            user->set_trick_area(-1);
        }
        cMyTrace trace(
            "bool GameWorld::move_area(CUser*, int, int, int, int, int, "
            "bool, int, int, int)",
            2190,
            0);
        trace("Move Guild Agit Area User(%d) Guild(%d) dest area(%d), "
              "virtual area(%d) ",
              user->getCurCharacNo(),
              guild_key,
              real_area_index,
              area_index);
    } else {
        user->set_trick_area(-1);
    }

    Area* target_area = target_village->getArea(real_area_index);
    if (target_area) {
        target_area->insert_user(user);
    }

    // Upgrade Room membership is scoped to the authoritative town area.
    // Revalidate only after village/real-area state and area membership have
    // committed; the hook is idempotent for same-scope moves.
    upgrade_room::OnUserAreaChanged(user);

    CUser_history_log(user)->MoveArea(current_village,
                                      current_area,
                                      village_index,
                                      area_index,
                                      move_type);
    return 1;
}

// IDA_EA: 0x86C67A0
// IDA_NAME: _ZN9GameWorld12goto_dungeonEP5CUser
// DEMANGLED: GameWorld::goto_dungeon(CUser *)
void GameWorld::goto_dungeon(CUser* user) {
    GameWorld* const world = this;
    if (!user) {
        return;
    }

    if (user->getMoveSpace() != 1) {
        Village* village = reinterpret_cast<GameWorld*>(world)->getUserVillage(
            user);
        if (!village) {
            cMyTrace trace("void GameWorld::goto_dungeon(CUser*)", 2257, 5);
            trace("%d", user->getCurCharacVill());
            return;
        }

        const int area_index = user->get_area(false);
        Area* area = reinterpret_cast<Village*>(village)->getArea(area_index);
        if (!area) {
            cMyTrace trace("void GameWorld::goto_dungeon(CUser*)", 2264, 5);
            trace("%d", user->get_area(false));
            return;
        }

        area->erase_user(user);

        PacketGuard packet;
        packet.put_header(0, 23);
        packet.put_short(user->get_unique_id());
        packet.put_byte(user->getCurCharacVill());
        packet.put_byte(255);
        packet.put_short(user->get_posX());
        packet.put_short(user->get_posY());
        packet.put_byte(user->get_direction());
        packet.put_byte(user->get_charac_visible_values());
        packet.finalize(true);
        user->Send(packet);
    }

    user->set_state(5);
}

// IDA_EA: 0x86C6A16
// IDA_NAME: _ZN9GameWorld16out_from_dungeonEP5CUser
// DEMANGLED: GameWorld::out_from_dungeon(CUser *)
void GameWorld::out_from_dungeon(CUser* user) {
    GameWorld* const world = this;
    user->SetRevivalTime(0);
    user->SetUseAPCPotionTime(0);

    if (user->getMoveSpace() == 1) {
        user->SendFatigue();
        user->set_state(11);

        PacketGuard packet;
        packet.put_header(0, 3);
        packet.put_byte(1);
        packet.put_short(user->get_unique_id());
        packet.put_byte(7);
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    const int area_index = user->get_area(false);
    const int village_index =
        static_cast<signed char>(user->getCurCharacVill());
    if (world->check_valid_area(village_index, area_index)) {
        Village* village = world->getUserVillage(user);
        if (village) {
            Area* area = village->getArea(user->get_area(false));
            if (area) {
                area->insert_user(user);
            }
        }

        PacketGuard packet;
        packet.put_header(0, 3);
        packet.put_byte(1);
        packet.put_short(user->get_unique_id());
        packet.put_byte(0);
        packet.finalize(true);
        world->send_all(packet, user);
        user->SendFatigue();
        fatigue_acceleration::SendTownState(user);
        user->getHades()->Send_ReturnToVillage();
        return;
    }

    LogManager::logFormat(
        1,
        "world.cpp",
        "void GameWorld::out_from_dungeon(CUser*)",
        0x926,
        "user %s - check_valid_area() fail",
        user->getCurCharacName());
}

// IDA_EA: 0x86C7452
// IDA_NAME: _ZN9GameWorld16out_from_warroomEP5CUser
// DEMANGLED: GameWorld::out_from_warroom(CUser *)
void GameWorld::out_from_warroom(CUser* user) {
    GameWorld* const world = this;
    const int area_index = user->get_area(false);
    const int village_index =
        static_cast<signed char>(user->getCurCharacVill());
    if (world->check_valid_area(village_index, area_index)) {
        user->SetWarRoomIndex(-1);
        Village* const village = world->getUserVillage(user);
        if (village) {
            Area* const area = village->getArea(user->get_area(false));
            if (area) {
                area->insert_user(user);
            }
        }

        PacketGuard packet;
        packet.put_header(0, 3);
        packet.put_byte(1);
        packet.put_short(user->get_unique_id());
        packet.put_byte(0);
        packet.finalize(true);
        world->send_all(packet, user);
        user->getCurCharacInvenW()->RemoveKCItem();
        user->InitFinishPoint();
        return;
    }

    cMyTrace trace("void GameWorld::out_from_warroom(CUser*)", 2601, 0);
    trace("user %s - check_valid_area() fail", user->getCurCharacName());
    user->DisConnSig(static_cast<DISCONN_SIG>(3), true, 0);
}

// IDA_EA: 0x86D1764
// IDA_NAME: _ZN9GameWorld10GetVillageEi
// DEMANGLED: GameWorld::GetVillage(int)
Village* GameWorld::GetVillage(int village_index) {
    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        return &villages_[village_index];
    }
    return 0;
}

// IDA_EA: 0x86C7F48
// IDA_NAME: _ZN9GameWorld12send_to_villEiR11PacketGuard
// DEMANGLED: GameWorld::send_to_vill(int,PacketGuard &)
void GameWorld::send_to_vill(int village_index, PacketGuard& packet) {
    if (village_index <= 0 || village_index >= ::MAX_VILLAGE_NUM ||
        g_townScriptFileList.size() <
            static_cast<std::size_t>(village_index)) {
        return;
    }

    Village* village = GetVillage(village_index);
    if (!village) {
        return;
    }

    for (int area_index = 0; area_index < village->area_count; ++area_index) {
        Area* area = village->getArea(area_index);
        if (area && area->get_area_index() != -1) {
            area->send_to_all(packet);
        }
    }
}

// IDA_EA: 0x86C8168
// IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuard
// DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &)
void GameWorld::send_to_area(int village_index,
                             int area_index,
                             PacketGuard& packet) {
    if ((village_index || area_index) &&
        village_index > 0 &&
        village_index < ::MAX_VILLAGE_NUM) {
        villages_[village_index].send_to_area(area_index, packet);
    }
}

// IDA_EA: 0x86C81F8
// IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuardP7CBelong
// DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &,CBelong *)
void GameWorld::send_to_area(int village_index,
                             int area_index,
                             PacketGuard& packet,
                             CBelong* group) {
    if (village_index <= 0 || village_index >= ::MAX_VILLAGE_NUM ||
        g_townScriptFileList.size() <
            static_cast<std::size_t>(village_index)) {
        return;
    }

    Village* village = GetVillage(village_index);
    if (!village) {
        return;
    }

    if (area_index >= 0 && area_index < village->area_count) {
        Area* area = village->getArea(area_index);
        if (area) {
            area->send_group(packet, group);
        }
    }
}

// IDA_EA: 0x86C828E
// IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuardRSt6vectorIP5CUserSaIS4_EE
// DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &,
//                                   std::vector<CUser *> &)
bool GameWorld::send_to_area(int village_index,
                             int area_index,
                             PacketGuard& packet,
                             std::vector<CUser*>& touched_users) {
    bool result = g_townScriptFileList.size() <
                  static_cast<std::size_t>(village_index);
    if (!result && village_index >= 0) {
        result = ::MAX_VILLAGE_NUM != 0;
        if (village_index < ::MAX_VILLAGE_NUM) {
            Village* const village = &villages_[village_index];
            if (village->area_count > area_index && area_index >= 0) {
                return village->getArea(area_index)->send_to_all(
                    packet, touched_users);
            }
        }
    }
    return result;
}

// IDA_EA: 0x86CEC84
// IDA_NAME: _ZN9GameWorld21isDungeonEntranceAreaEP5CUser
// DEMANGLED: GameWorld::isDungeonEntranceArea(CUser *)
bool GameWorld::isDungeonEntranceArea(CUser* user) {
    Village* village = getUserVillage(user);
    if (!village) {
        cMyTrace trace(
            "bool GameWorld::isDungeonEntranceArea(CUser*)", 5765, 5);
        trace("getUserVillage call failed");
        return false;
    }

    const int area_index = user->get_area(false);
    Area* area = village->getArea(area_index);
    if (!area) {
        cMyTrace trace(
            "bool GameWorld::isDungeonEntranceArea(CUser*)", 5772, 5);
        trace("getArea call failed (area no:%d)", area_index);
        return false;
    }

    const int area_type = static_cast<int>(area->town_area_type);
    return user->get_charac_level() <= 1 || area_type == 4;
}

// IDA_EA: 0x86C8316
// IDA_NAME: _ZN9GameWorld16get_user_id_listEiiRSt6vectorItSaItEE
// DEMANGLED: GameWorld::get_user_id_list(int,int,std::vector<unsigned short,std::allocator<unsigned short>> &)
bool GameWorld::get_user_id_list(int village_index,
                                 int area_index,
                                 std::vector<uint16_t>& user_ids) {
    if (village_index >= 0 && village_index < ::MAX_VILLAGE_NUM) {
        return villages_[village_index].get_user_id_list(area_index,
                                                         user_ids);
    }
    return false;
}
