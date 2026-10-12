#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_RENTAL_MACHINE_PC_ROOM_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_RENTAL_MACHINE_PC_ROOM_RUNTIME_H_

#include "old_game/economy/inventory/instance_rental_system_abi.h"

#include <memory>
#include <set>

class CUser;
struct EventRewardItemInfo;

// IDA unit: 09_dungeon_pvp_party/RentalMachinePcRoom.
//
// C2 @ 0x82703C0 constructs the IRentalMachine base, installs the derived
// vtable, and places the auto_ptr implementation at +0x04.  The nested
// object is exactly 0x11c: its zeroed 260-byte log path is followed by the
// std::set used by CheckRentableItem/Ready.
class RentalMachinePcRoom : public IRentalMachine {
public:
    RentalMachinePcRoom();
    virtual ~RentalMachinePcRoom();

    virtual int Rent(CUser& user,
                     const InstanceRentalSystem::RentInput& input,
                     InstanceRentalSystem::RentOutput& output);
    virtual int Ready();
    virtual int CheckRentableItem(unsigned int item_id);

private:
    class RentalMachinePcRoomImpl {
    public:
        RentalMachinePcRoomImpl();
        ~RentalMachinePcRoomImpl();

        char log_file_name[260];
        std::set<unsigned int> rentable_item_ids;
    };

    static_assert(sizeof(RentalMachinePcRoomImpl) == 0x11cu,
                  "RentalMachinePcRoomImpl IDA ABI size must stay 0x11c");

    int log(CUser& user, int item_id);
    int initLoggingEnvironment();
    int checkAllRentalItemScript();
    int CheckupAllRentalItems(
        CUser& user,
        const InstanceRentalSystem::RentInput& input);

    std::auto_ptr<RentalMachinePcRoomImpl> impl_;

    RentalMachinePcRoom(const RentalMachinePcRoom&);
    RentalMachinePcRoom& operator=(const RentalMachinePcRoom&);
};

static_assert(sizeof(RentalMachinePcRoom) == 0x08u,
              "RentalMachinePcRoom IDA ABI size must stay 0x08");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_RENTAL_MACHINE_PC_ROOM_RUNTIME_H_
