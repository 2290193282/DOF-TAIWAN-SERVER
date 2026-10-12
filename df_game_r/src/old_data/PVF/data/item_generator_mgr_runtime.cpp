#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_data/PVF/data/item_generator_drop_types.h"

#include <new>
#include <string.h>

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

namespace WongWork {

// These are the concrete-family C2/load entry points.  Their declarations
// preserve the original member ABI while the family layouts are reconstructed
// in item_generator_drop_import_runtime.cpp.
// IDA_EA: 0x8374EF4
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgrC2Ev
// DEMANGLED: WongWork::CItemGeneratorMgr::CItemGeneratorMgr(void)
CItemGeneratorMgr::CItemGeneratorMgr() : common_data_() {}

// IDA_EA: 0x8374F0C
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgrD2Ev
// DEMANGLED: WongWork::CItemGeneratorMgr::~CItemGeneratorMgr()
CItemGeneratorMgr::~CItemGeneratorMgr() {}

// IDA_EA: 0x814A7D0
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgr12getGeneratorENS0_15eGenerateType_tE
// DEMANGLED: WongWork::CItemGeneratorMgr::getGenerator(WongWork::CItemGeneratorMgr::eGenerateType_t)
CItemGenerator* CItemGeneratorMgr::getGenerator(eGenerateType_t type) {
    return generators_[type];
}

// IDA_EA: 0x8533BC8
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgr10initializeEv
// DEMANGLED: WongWork::CItemGeneratorMgr::initialize(void)
int CItemGeneratorMgr::initialize() {
    generators_[kMonsterDrop] =
        static_cast<CItemGenerator*>(::operator new(0x2593A4u));
    new ((generators_[kMonsterDrop])) WongWork::CMonsterDrop(3);
    generators_[kObjectDrop] =
        static_cast<CItemGenerator*>(::operator new(0x2593A4u));
    new ((generators_[kObjectDrop])) WongWork::CObjectDrop(3);
    generators_[kClearReward] =
        static_cast<CItemGenerator*>(::operator new(0x23B84u));
    new ((generators_[kClearReward])) WongWork::CClearReward(5);
    generators_[kWorldDrop] =
        static_cast<CItemGenerator*>(::operator new(0x1F80u));
    new ((generators_[kWorldDrop])) WongWork::CWorldDrop(2);
    generators_[kDungeonDrop] =
        static_cast<CItemGenerator*>(::operator new(0x18u));
    new ((generators_[kDungeonDrop])) WongWork::CDungeonDrop(2);
    generators_[kPcRoomWorldDrop] =
        static_cast<CItemGenerator*>(::operator new(0x1F80u));
    new ((generators_[kPcRoomWorldDrop])) WongWork::CWorldDrop(2);
    generators_[kPcRoom2WorldDrop] =
        static_cast<CItemGenerator*>(::operator new(0x1F80u));
    new ((generators_[kPcRoom2WorldDrop])) WongWork::CWorldDrop(2);
    generators_[kReservedWorldDrop] =
        static_cast<CItemGenerator*>(::operator new(0x1F80u));
    new ((generators_[kReservedWorldDrop])) WongWork::CWorldDrop(2);

    for (int type = kMonsterDrop; type <= kDungeonDrop; ++type) {
        generators_[type]->setGeneratorType(type);
    }

    int status = static_cast<CMonsterDrop*>(
        generators_[kMonsterDrop])->loadScript(
        "Etc/ItemDropInfo_Monseter.etc",
        "Etc/ItemDropInfo_Monster_Hell.etc",
        "Etc/ItemDropInfo_Monseter_Extra.etc");
    if (status) {
        throw status;
    }
    status = generators_[kObjectDrop]->loadScript("Etc/ItemDropInfo_Object.etc");
    if (status) {
        throw status;
    }
    status = generators_[kClearReward]->loadScript("Etc/ItemDropInfo_ClearReward.etc");
    if (status) {
        throw status;
    }
    status = static_cast<CWorldDrop*>(generators_[kWorldDrop])->loadScript(
        "Etc/WorldDrop.etc",
        "Etc/WorldDropWarArea.etc");
    if (status) {
        throw status;
    }
    status = generators_[kDungeonDrop]->loadScript("");
    if (status) {
        throw status;
    }
    status = static_cast<CWorldDrop*>(
        generators_[kPcRoomWorldDrop])->loadScript(
        "Etc/WorldDropPCRoom.etc",
        "Etc/WorldDropWarAreaPCRoom.etc");
    if (status) {
        throw status;
    }
    status = static_cast<CWorldDrop*>(
        generators_[kPcRoom2WorldDrop])->loadScript(
        "Etc/WorldDropPCRoom2.etc",
        "Etc/WorldDropWarAreaPCRoom.etc");
    if (status) {
        throw status;
    }

    status = common_data_.loadScript("Etc/ItemDropInfo_Common.etc");
    if (status) {
        throw status;
    }
    for (int index = kMonsterDrop; index <= kReservedWorldDrop; ++index) {
        generators_[index]->setCommonData(&common_data_);
    }

    ::memset(generate_statistics_, 0, sizeof(generate_statistics_));
    TimerUpdateItemGenerateStatistics::registNextTimer(
        GlobalData::s_systemTime_.getCurSec());
    setDropRatio(1.0f);
    return 1;
}

// IDA_EA: 0x85342C6
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgr23onTryGenerateRandomItemENS0_15eGenerateType_tENS_15eItemDropType_tE11ENUM_RARITY
// DEMANGLED: WongWork::CItemGeneratorMgr::onTryGenerateRandomItem(WongWork::CItemGeneratorMgr::eGenerateType_t,WongWork::eItemDropType_t,ENUM_RARITY)
int CItemGeneratorMgr::onTryGenerateRandomItem(eGenerateType_t generator_type,
                                                int drop_type,
                                                int rarity) {
    const int index = rarity + 30 * generator_type + 6 * drop_type;
    ++generate_statistics_[index];
    return index;
}

// IDA_EA: 0x8534322
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgr12onTimerEventEi
// DEMANGLED: WongWork::CItemGeneratorMgr::onTimerEvent(int)
void CItemGeneratorMgr::onTimerEvent(int timer_id) {
    if (timer_id == 107) {
        ::memset(generate_statistics_, 0, sizeof(generate_statistics_));
    }
}

}  // namespace WongWork
