#ifndef OLDSRV_GLOBAL_INSTANCE_RENTAL_SYSTEM_ABI_H_
#define OLDSRV_GLOBAL_INSTANCE_RENTAL_SYSTEM_ABI_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

class CUser;
class CGlobalEffectManager;
class IRentalMachine;
class CAuctionServerProxy;
class CCeraAuctionServerProxy;
class CTimeGate;
class CMonitorServerProxy;
class CItemAmplifier;
template <typename T>
class CServerProxyMgr;
class ItemVendingMachine;
class CSecu_ProtectionField;

namespace WongWork {
class CCeraShop;
class CGMAccounts;
}  // namespace WongWork

struct RefPvpGrade;

// IDA unit: 99_misc_global/InstanceRentalSystem.
// IDA_EA: InstanceRentalSystem ctor/dtor/dispatch @0x8270014..0x8270348.
// IDA_EA: RentInput/RentOutput/RentalInfo values @0x822F02E..0x8273086.
// IDA_EA: IRentalMachine lifecycle @0x826FA1A/@0x826FA40/@0x826FA5E.
// IDA_EA: GlobalData::s_rentalSystem lifecycle @0x8299FA0/@0x829E0B2.
// The old type lives in the global namespace. Keep this ABI separate from
// GlobalData so B/C raw old-symbol declarations continue to link.
class InstanceRentalSystem {
public:
    struct RentalInfo {
        RentalInfo();
        ~RentalInfo();

        uint8_t state;
        uint8_t reserved_01;
        uint16_t slot;
        uint32_t item_id;
        uint8_t section_index;
        uint8_t package_index;
        uint8_t equip_flag;
        uint8_t rent_type;
        int32_t expire_time;
    };

    struct DeletedRentItem {
        DeletedRentItem();
        ~DeletedRentItem();

        uint8_t is_equipment;
        uint8_t reserved_01;
        uint16_t slot;
        uint32_t item_id;
    };

    struct RentInput {
        struct RentInElem {
            RentInElem();
            ~RentInElem();

            uint8_t section_index;
            uint8_t package_index;
            uint8_t equip_flag;
            uint8_t rent_type;
            int32_t expire_time;
        };

        RentInput();
        ~RentInput();

        std::vector<RentInElem> entries;
        int32_t rental_type;
        uint8_t write_log;
        uint8_t force_pcroom;
        uint8_t reserved_12[2];
    };

    struct RentOutput {
        RentOutput();
        ~RentOutput();

        std::vector<RentalInfo> rental_infos;
        std::vector<DeletedRentItem> deleted_items;
    };

    struct RentalSystemImpl;

    InstanceRentalSystem();
    ~InstanceRentalSystem();

    int Ready();
    int Rent(CUser& user, const RentInput& input, RentOutput& output);
    int CheckRentableItem(int rental_type, unsigned int item_id);
    IRentalMachine** registerRentalMachine(int rental_type,
                                           IRentalMachine* machine);
    IRentalMachine* getMachine(int rental_type);

private:
    std::auto_ptr<RentalSystemImpl> impl_;

    InstanceRentalSystem(const InstanceRentalSystem&);
    InstanceRentalSystem& operator=(const InstanceRentalSystem&);
};

static_assert(sizeof(InstanceRentalSystem) == 0x04u,
              "InstanceRentalSystem IDA ABI size must stay 0x04");
static_assert(sizeof(InstanceRentalSystem::RentalInfo) == 0x10u,
              "InstanceRentalSystem::RentalInfo IDA ABI size must stay 0x10");
static_assert(sizeof(InstanceRentalSystem::DeletedRentItem) == 0x08u,
              "InstanceRentalSystem::DeletedRentItem IDA ABI size must stay 0x08");
static_assert(sizeof(InstanceRentalSystem::RentInput::RentInElem) == 0x08u,
              "InstanceRentalSystem::RentInput::RentInElem size must stay 0x08");
static_assert(sizeof(InstanceRentalSystem::RentInput) == 0x14u,
              "InstanceRentalSystem::RentInput IDA ABI size must stay 0x14");
static_assert(sizeof(InstanceRentalSystem::RentOutput) == 0x18u,
              "InstanceRentalSystem::RentOutput IDA ABI size must stay 0x18");

class IRentalMachine {
public:
    IRentalMachine();
    virtual ~IRentalMachine();

    virtual int Rent(CUser& user,
                     const InstanceRentalSystem::RentInput& input,
                     InstanceRentalSystem::RentOutput& output) = 0;
    virtual int Ready() = 0;
    virtual int CheckRentableItem(unsigned int item_id) = 0;

private:
    IRentalMachine(const IRentalMachine&);
    IRentalMachine& operator=(const IRentalMachine&);
};

static_assert(sizeof(IRentalMachine) == 0x04u,
              "IRentalMachine IDA ABI size must stay 0x04");

#endif  // OLDSRV_GLOBAL_INSTANCE_RENTAL_SYSTEM_ABI_H_
