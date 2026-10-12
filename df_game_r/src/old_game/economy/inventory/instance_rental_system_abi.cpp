#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "common/manager/global_data.h"
#include "old_game/dungeon/room/rental_machine_pc_room_runtime.h"
#include "old_game/economy/inventory/rental_machine_dimension_abi.h"
#include <new>

struct RentalMachinePcRoomStorage {
    uint32_t vptr;
    uint32_t impl;
};

static_assert(sizeof(RentalMachinePcRoomStorage) == 0x08u,
              "RentalMachinePcRoom IDA ABI size must stay 0x08");

struct InstanceRentalSystem::RentalSystemImpl {
    RentalSystemImpl();
    ~RentalSystemImpl();

    std::map<int, IRentalMachine*> machines;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(InstanceRentalSystem::RentalSystemImpl) == 0x18u,
              "InstanceRentalSystem::RentalSystemImpl size must stay 0x18");
#endif

InstanceRentalSystem* GlobalData::s_rentalSystem = 0;

// IDA_EA: 0x826FA5E
// IDA_NAME: _ZN14IRentalMachineC2Ev
// DEMANGLED: IRentalMachine::IRentalMachine(void)
IRentalMachine::IRentalMachine() {}

// IDA_EA: 0x826FA1A
// IDA_NAME: _ZN14IRentalMachineD2Ev
// DEMANGLED: IRentalMachine::~IRentalMachine()
IRentalMachine::~IRentalMachine() {}

// IDA_EA: 0x8271A6C
// IDA_NAME: _ZN20InstanceRentalSystem16RentalSystemImplC2Ev
// DEMANGLED: InstanceRentalSystem::RentalSystemImpl::RentalSystemImpl(void)
InstanceRentalSystem::RentalSystemImpl::RentalSystemImpl() : machines() {}

// IDA_EA: 0x8271B3C
// IDA_NAME: _ZN20InstanceRentalSystem16RentalSystemImplD2Ev
// DEMANGLED: InstanceRentalSystem::RentalSystemImpl::~RentalSystemImpl()
InstanceRentalSystem::RentalSystemImpl::~RentalSystemImpl() {}

// IDA_EA: 0x822F02E
// IDA_NAME: _ZN20InstanceRentalSystem9RentInput10RentInElemC2Ev
// DEMANGLED: InstanceRentalSystem::RentInput::RentInElem::RentInElem(void)
InstanceRentalSystem::RentInput::RentInElem::RentInElem()
    : section_index(0),
      package_index(0),
      equip_flag(0),
      rent_type(0),
      expire_time(-1) {}

// IDA_EA: 0x823D630
// IDA_NAME: _ZN20InstanceRentalSystem9RentInput10RentInElemD2Ev
// DEMANGLED: InstanceRentalSystem::RentInput::RentInElem::~RentInElem()
InstanceRentalSystem::RentInput::RentInElem::~RentInElem() {}

// IDA_EA: 0x822F058
// IDA_NAME: _ZN20InstanceRentalSystem9RentInputC2Ev
// DEMANGLED: InstanceRentalSystem::RentInput::RentInput(void)
InstanceRentalSystem::RentInput::RentInput()
    : entries(),
      rental_type(0),
      write_log(0),
      force_pcroom(0) {}

// IDA_EA: 0x82352B2
// IDA_NAME: _ZN20InstanceRentalSystem9RentInputD2Ev
// DEMANGLED: InstanceRentalSystem::RentInput::~RentInput()
InstanceRentalSystem::RentInput::~RentInput() {}

// IDA_EA: 0x82719F6
// IDA_NAME: _ZN20InstanceRentalSystem10RentalInfoC2Ev
// DEMANGLED: InstanceRentalSystem::RentalInfo::RentalInfo(void)
InstanceRentalSystem::RentalInfo::RentalInfo()
    : state(0),
      slot(0),
      item_id(0),
      section_index(0),
      package_index(0),
      equip_flag(0),
      rent_type(0),
      expire_time(-1) {}

// IDA_EA: 0x8272E8A
// IDA_NAME: _ZN20InstanceRentalSystem10RentalInfoD2Ev
// DEMANGLED: InstanceRentalSystem::RentalInfo::~RentalInfo()
InstanceRentalSystem::RentalInfo::~RentalInfo() {}

// IDA_EA: 0x8271A3A
// IDA_NAME: _ZN20InstanceRentalSystem15DeletedRentItemC2Ev
// DEMANGLED: InstanceRentalSystem::DeletedRentItem::DeletedRentItem(void)
InstanceRentalSystem::DeletedRentItem::DeletedRentItem()
    : is_equipment(0), slot(0), item_id(0) {}

// IDA_EA: 0x8273086
// IDA_NAME: _ZN20InstanceRentalSystem15DeletedRentItemD2Ev
// DEMANGLED: InstanceRentalSystem::DeletedRentItem::~DeletedRentItem()
InstanceRentalSystem::DeletedRentItem::~DeletedRentItem() {}

// IDA_EA: 0x8231C4E
// IDA_NAME: _ZN20InstanceRentalSystem10RentOutputC2Ev
// DEMANGLED: InstanceRentalSystem::RentOutput::RentOutput(void)
InstanceRentalSystem::RentOutput::RentOutput()
    : rental_infos(), deleted_items() {}

// IDA_EA: 0x8231C94
// IDA_NAME: _ZN20InstanceRentalSystem10RentOutputD2Ev
// DEMANGLED: InstanceRentalSystem::RentOutput::~RentOutput()
InstanceRentalSystem::RentOutput::~RentOutput() {}

// IDA_EA: 0x8270014
// IDA_NAME: _ZN20InstanceRentalSystemC2Ev
// DEMANGLED: InstanceRentalSystem::InstanceRentalSystem(void)
InstanceRentalSystem::InstanceRentalSystem()
    : impl_(new RentalSystemImpl()) {
    RentalMachinePcRoomStorage* pc_room =
        static_cast<RentalMachinePcRoomStorage*>(
            ::operator new(sizeof(RentalMachinePcRoomStorage)));
    new (pc_room) RentalMachinePcRoom();
    registerRentalMachine(1, reinterpret_cast<IRentalMachine*>(pc_room));

    registerRentalMachine(2, new RentalMachineDimension());
}

// IDA_EA: 0x8270110
// IDA_NAME: _ZN20InstanceRentalSystemD2Ev
// DEMANGLED: InstanceRentalSystem::~InstanceRentalSystem()
InstanceRentalSystem::~InstanceRentalSystem() {
    for (std::map<int, IRentalMachine*>::iterator it =
             impl_->machines.begin();
         it != impl_->machines.end(); ++it) {
        delete it->second;
        it->second = 0;
    }
}

// IDA_EA: 0x82701DC
// IDA_NAME: _ZN20InstanceRentalSystem5ReadyEv
// DEMANGLED: InstanceRentalSystem::Ready(void)
int InstanceRentalSystem::Ready() {
    for (std::map<int, IRentalMachine*>::iterator it =
             impl_->machines.begin();
         it != impl_->machines.end(); ++it) {
        IRentalMachine* machine = it->second;
        if (!machine) {
            return 0;
        }
        if (machine->Ready() != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8270288
// IDA_NAME: _ZN20InstanceRentalSystem4RentER5CUserRKNS_9RentInputERNS_10RentOutputE
// DEMANGLED: InstanceRentalSystem::Rent(CUser &,InstanceRentalSystem::RentInput const&,InstanceRentalSystem::RentOutput &)
int InstanceRentalSystem::Rent(CUser& user,
                               const RentInput& input,
                               RentOutput& output) {
    IRentalMachine* machine = getMachine(input.rental_type);
    if (machine) {
        return machine->Rent(user, input, output);
    }
    return 1;
}

// IDA_EA: 0x82702DC
// IDA_NAME: _ZN20InstanceRentalSystem17CheckRentableItemEij
// DEMANGLED: InstanceRentalSystem::CheckRentableItem(int,unsigned int)
int InstanceRentalSystem::CheckRentableItem(int rental_type,
                                            unsigned int item_id) {
    IRentalMachine* machine = getMachine(rental_type);
    if (machine) {
        return machine->CheckRentableItem(item_id);
    }
    return 1;
}

// IDA_EA: 0x8270320
// IDA_NAME: _ZN20InstanceRentalSystem21registerRentalMachineEiP14IRentalMachine
// DEMANGLED: InstanceRentalSystem::registerRentalMachine(int,IRentalMachine *)
IRentalMachine** InstanceRentalSystem::registerRentalMachine(
    int rental_type,
    IRentalMachine* machine) {
    IRentalMachine*& slot = impl_->machines[rental_type];
    slot = machine;
    return &slot;
}

// IDA_EA: 0x8270348
// IDA_NAME: _ZN20InstanceRentalSystem10getMachineEi
// DEMANGLED: InstanceRentalSystem::getMachine(int)
IRentalMachine* InstanceRentalSystem::getMachine(int rental_type) {
    std::map<int, IRentalMachine*>::iterator found =
        impl_->machines.find(rental_type);
    if (found == impl_->machines.end()) {
        return 0;
    }
    return found->second;
}
