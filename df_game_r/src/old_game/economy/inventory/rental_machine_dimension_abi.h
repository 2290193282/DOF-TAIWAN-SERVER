#ifndef OLDSRV_GLOBAL_RENTAL_MACHINE_DIMENSION_ABI_H_
#define OLDSRV_GLOBAL_RENTAL_MACHINE_DIMENSION_ABI_H_

#include "old_game/economy/inventory/instance_rental_system_abi.h"

#include <memory>
#include <set>
#include <vector>

class CUser;
struct EventRewardItemInfo;

// IDA unit: 99_misc_global/RentalMachineDimension.
//
// C2 @ 0x826EAFC constructs the IRentalMachine base, stores the derived
// vtable, then creates RentalMachineDimensionImpl at +0x04.  The 0x11c
// implementation layout is recovered from C2/D2 plus all implementation
// accesses: a 260-byte log filename followed by std::set<unsigned int>.
class RentalMachineDimension : public IRentalMachine {
public:
    RentalMachineDimension();
    virtual ~RentalMachineDimension();

    virtual int Rent(CUser& user,
                     const InstanceRentalSystem::RentInput& input,
                     InstanceRentalSystem::RentOutput& output);
    virtual int Ready();
    virtual int CheckRentableItem(unsigned int item_id);

private:
    class RentalMachineDimensionImpl {
    public:
        RentalMachineDimensionImpl();
        ~RentalMachineDimensionImpl();

        char log_file_name[260];
        std::set<unsigned int> rentable_item_ids;
    };

    static_assert(sizeof(RentalMachineDimensionImpl) == 0x11cu,
                  "RentalMachineDimensionImpl IDA ABI size must stay 0x11c");

    int log(CUser& user, int item_id);
    int initLoggingEnvironment();
    int checkAllRentalItemScript();
    int checkSelectionListScript(
        const std::vector<std::vector<EventRewardItemInfo> >& selections);
    int CheckupAllRentalItems(
        CUser& user,
        const InstanceRentalSystem::RentInput& input);
    int checkRentalLevel(int charac_level);
    int checkRentalGrowJob(int first_grow_type);
    std::vector<std::vector<EventRewardItemInfo> >* GetRewardItemGiveList(
        int charac_job,
        int first_grow_type);
    int GetRentItemInfo(
        CUser& user,
        const InstanceRentalSystem::RentInput::RentInElem& input,
        EventRewardItemInfo& item_info);

    std::auto_ptr<RentalMachineDimensionImpl> impl_;

    RentalMachineDimension(const RentalMachineDimension&);
    RentalMachineDimension& operator=(const RentalMachineDimension&);
};

static_assert(sizeof(RentalMachineDimension) == 0x08u,
              "RentalMachineDimension IDA ABI size must stay 0x08");

#endif  // OLDSRV_GLOBAL_RENTAL_MACHINE_DIMENSION_ABI_H_
