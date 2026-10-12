#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_PUBLIC_ABI_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_PUBLIC_ABI_H_

#include "old_game/economy/inventory/instance_rental_system_abi.h"

#include <vector>

class CUser;
struct EventRewardItemInfo;
struct EventRewardSection;

// IDA unit: 09_dungeon_pvp_party/pc_room. This namespace supplies the shared
// rental policy used by both original rental-machine implementations.
namespace pc_room {

int IsPcRoomUser(CUser& user);
int CheckRentableEnvironment(CUser& user);
int CheckRentableEnvironmentAtChannelType(CUser& user);
int CheckInventoryEquipmentEmptyCount(CUser& user, int count);
int CheckInventoryEmptyCount(CUser& user, int count);
int GetSelectionIndex(CUser& user, int& selection_index, int rental_type);
int GetCharacterJob(CUser& user, char& character_job);
std::vector<std::vector<EventRewardItemInfo> >* GetRewardItemSelectionList(
    int package_key,
    int rental_type);
EventRewardSection* GetRewardSection(int pvp_grade, int rental_type);
int GetRentItemInfo(CUser& user,
                    char section_index,
                    char package_index,
                    EventRewardItemInfo& item_info,
                    int rental_type);
int ProcessEquipItem(
    CUser& user,
    const InstanceRentalSystem::RentInput::RentInElem& input,
    const EventRewardItemInfo& item_info,
    InstanceRentalSystem::RentOutput& output,
    int reason);
int ProcessInsertItem(
    CUser& user,
    const InstanceRentalSystem::RentInput::RentInElem& input,
    const EventRewardItemInfo& item_info,
    InstanceRentalSystem::RentOutput& output,
    int reason);
int DeleteAllRentalItemInSameSectionForNewRent(
    CUser& user,
    std::vector<InstanceRentalSystem::DeletedRentItem>& deleted_items,
    int rental_type);

}  // namespace pc_room

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_PUBLIC_ABI_H_
