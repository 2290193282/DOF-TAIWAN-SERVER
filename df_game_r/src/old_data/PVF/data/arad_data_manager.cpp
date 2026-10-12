#include "old_data/PVF/data/arad_data_manager.h"

#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/arad_statistics_event_item_list.h"

#include <algorithm>
#include <string>

namespace ARAD {

// IDA_EA: 0x80E2454
// IDA_NAME: _ZN4ARAD15ScriptInterfaceC2Ev
// DEMANGLED: ARAD::ScriptInterface::ScriptInterface(void)
ScriptInterface::ScriptInterface() {}

// IDA_EA: 0x8188B18
// IDA_NAME: _ZNK4ARAD24Arad_InterfaceGameScript14scriptDataNameEv
// DEMANGLED: ARAD::Arad_InterfaceGameScript::scriptDataName(void)const
std::string Arad_InterfaceGameScript::scriptDataName() const {
    return script_name_;
}

// IDA_EA: 0x8195C5C
// IDA_NAME: _ZN4ARAD24Arad_InterfaceGameScriptC2ESs
// DEMANGLED: ARAD::Arad_InterfaceGameScript::Arad_InterfaceGameScript(std::string)
Arad_InterfaceGameScript::Arad_InterfaceGameScript(std::string script_name)
    : script_name_(script_name) {}

// IDA_EA: 0x8195C82
// IDA_NAME: _ZN4ARAD24Arad_InterfaceGameScriptD2Ev
// DEMANGLED: ARAD::Arad_InterfaceGameScript::~Arad_InterfaceGameScript()
Arad_InterfaceGameScript::~Arad_InterfaceGameScript() {}

// IDA_EA: 0x8195CB6
// IDA_NAME: _ZN4ARAD24Arad_InterfaceGameScriptD0Ev
// DEMANGLED: ARAD::Arad_InterfaceGameScript::~Arad_InterfaceGameScript()
// The virtual destructor above emits this deleting-destructor entry.

// IDA_EA: 0x8188C56
// IDA_NAME: _ZN4ARAD19FN_DeleteGameScriptclERSt4pairIKSsPNS_24Arad_InterfaceGameScriptEE
// DEMANGLED: ARAD::FN_DeleteGameScript::operator()(std::pair<std::string const,ARAD::Arad_InterfaceGameScript *> &)
void FN_DeleteGameScript::operator()(
    std::pair<const std::string, Arad_InterfaceGameScript*>& value) {
    if (value.second) {
        value.second->clearScriptData();
        delete value.second;
        value.second = 0;
    }
}

// IDA_EA: 0x8188F66
// IDA_NAME: _ZN4ARAD9SingletonINS_16Arad_DataManagerEEC2Ev
// DEMANGLED: ARAD::Singleton<ARAD::Arad_DataManager>::Singleton(void)
template <>
Singleton<Arad_DataManager>::Singleton() {}

// IDA_EA: 0x8197EFA
// IDA_NAME: _ZN4ARAD9SingletonINS_16Arad_DataManagerEE3GetEv
// DEMANGLED: ARAD::Singleton<ARAD::Arad_DataManager>::Get(void)
template <>
Arad_DataManager* Singleton<Arad_DataManager>::Get() {
    static Arad_DataManager obj;
    return &obj;
}

// IDA_EA: 0x8188030
// IDA_NAME: _ZN4ARAD16Arad_DataManagerC2Ev
// DEMANGLED: ARAD::Arad_DataManager::Arad_DataManager(void)
Arad_DataManager::Arad_DataManager()
    : Singleton<Arad_DataManager>(),
      ScriptInterface(),
      game_scripts_(),
      exception_item_(),
      purchase_limit_item_(),
      bonus_item_(),
      mileage_data_(),
      one_plus_card_event_item_list_(),
      village_attack_reward_item_() {}

// IDA_EA: 0x8188162
// IDA_NAME: _ZN4ARAD16Arad_DataManagerD2Ev
// DEMANGLED: ARAD::Arad_DataManager::~Arad_DataManager()
Arad_DataManager::~Arad_DataManager() {
    Destroy();
}

// IDA_EA: 0x81882AC
// IDA_NAME: _ZN4ARAD16Arad_DataManager14findGameScriptESs
// DEMANGLED: ARAD::Arad_DataManager::findGameScript(std::string)
Arad_InterfaceGameScript* Arad_DataManager::findGameScript(std::string name) {
    return game_scripts_[name];
}

// IDA_EA: 0x81882CC
// IDA_NAME: _ZN4ARAD16Arad_DataManager4LoadEv
// DEMANGLED: ARAD::Arad_DataManager::Load(void)
bool Arad_DataManager::Load() {
    return initAradData();
}

// IDA_EA: 0x81882E0
// IDA_NAME: _ZN4ARAD16Arad_DataManager7DestroyEv
// DEMANGLED: ARAD::Arad_DataManager::Destroy(void)
int Arad_DataManager::Destroy() {
    destroyAradData();
    return 1;
}

// IDA_EA: 0x81882F8
// IDA_NAME: _ZN4ARAD16Arad_DataManager12initAradDataEv
// DEMANGLED: ARAD::Arad_DataManager::initAradData(void)
bool Arad_DataManager::initAradData() {
    cMyTrace loading_event_period("initAradData", 72, 9, true, false);
    loading_event_period("\t- Loading Event Period Script - ");

    Arad_InterfaceGameScript* game_script =
        new Arad_EventPeriodDataManager();
    if (!game_script || game_script->loadScriptData() != true) {
        cMyTrace fail("initAradData", 76, 9, true, true);
        fail("Fail");
        return false;
    }
    if (__insertGameScript(game_script) != true) {
        cMyTrace fail("initAradData", 82, 9, true, true);
        fail("Fail");
        return false;
    }
    cMyTrace event_period_success("initAradData", 85, 9, false, true);
    event_period_success("Success");

    cMyTrace loading_statistics("initAradData", 89, 9, true, false);
    loading_statistics("\t- Loading Statistics Item List - ");
    game_script = new Arad_StatisticsEventItemList();
    if (!game_script || game_script->loadScriptData() != true) {
        cMyTrace fail("initAradData", 93, 9, true, true);
        fail("Fail");
        return false;
    }
    if (__insertGameScript(game_script) != true) {
        cMyTrace fail("initAradData", 99, 9, true, true);
        fail("Fail");
        return false;
    }
    cMyTrace statistics_success("initAradData", 102, 9, false, true);
    statistics_success("Success");

    cMyTrace loading_nexon("initAradData", 148, 9, true, false);
    loading_nexon("\t- Loading NexonFolder Script - ");
    int line = exception_item_.open("Nexon/AradSpecialUseItems.txt");
    if (line != 0) {
        cMyTrace fail("initAradData", 151, 9, true, true);
        fail("\t- AradSpecialUseItems script load fail. (line:%d)", line);
        return false;
    }
    line = purchase_limit_item_.open("Nexon/AradSpecialUseItems.txt");
    if (line != 0) {
        cMyTrace fail("initAradData", 157, 9, true, true);
        fail("\t- AradSpecialUseItems script load fail. (line:%d)", line);
        return false;
    }
    line = bonus_item_.open("Nexon/AradSpecialUseItems.txt");
    if (line != 0) {
        cMyTrace fail("initAradData", 163, 9, true, true);
        fail("\t- AradSpecialUseItems script load fail. (line:%d)", line);
        return false;
    }
    line = mileage_data_.open("Nexon/CeraShopMileage.txt");
    if (line != 0) {
        cMyTrace fail("initAradData", 178, 9, true, true);
        fail("\t- CeraShopMileage script load fail. (line:%d)", line);
        return false;
    }
    line = village_attack_reward_item_.open(
        "Nexon/VillageAttackRewardItem.txt");
    if (line != 0) {
        cMyTrace fail("initAradData", 187, 9, true, true);
        fail("\t- VillageAttackRewardItem script load fail. (line:%d)", line);
        return false;
    }
    return true;
}

// IDA_EA: 0x8188952
// IDA_NAME: _ZN4ARAD16Arad_DataManager15destroyAradDataEv
// DEMANGLED: ARAD::Arad_DataManager::destroyAradData(void)
void Arad_DataManager::destroyAradData() {
    std::for_each(game_scripts_.begin(), game_scripts_.end(),
                  FN_DeleteGameScript());
    game_scripts_.clear();
}

// IDA_EA: 0x81889BC
// IDA_NAME: _ZN4ARAD16Arad_DataManager18__insertGameScriptEPNS_24Arad_InterfaceGameScriptE
// DEMANGLED: ARAD::Arad_DataManager::__insertGameScript(ARAD::Arad_InterfaceGameScript *)
bool Arad_DataManager::__insertGameScript(
    Arad_InterfaceGameScript* script) {
    if (game_scripts_.find(script->scriptDataName()) != game_scripts_.end()) {
        return false;
    }

    game_scripts_[script->scriptDataName()] = script;
    return true;
}

// IDA_EA: 0x8197EB6
// IDA_NAME: _ZN4ARAD16Arad_DataManager20getPurchaseLimitItemEv
// DEMANGLED: ARAD::Arad_DataManager::getPurchaseLimitItem(void)
PurchaseLimitItem* Arad_DataManager::getPurchaseLimitItem() {
    return &purchase_limit_item_;
}

// IDA_EA: 0x8196AC8
// IDA_NAME: _ZN4ARAD16CheckAccountItemEP5CUserjRN17PurchaseLimitItem4InfoE
// DEMANGLED: ARAD::CheckAccountItem(CUser *,unsigned int,PurchaseLimitItem::Info &)
bool CheckAccountItem(CUser*,
                      uint32_t item_index,
                      PurchaseLimitItem::Info& info) {
    Arad_DataManager* const manager = Singleton<Arad_DataManager>::Get();
    PurchaseLimitItem* const purchase_limit = manager->getPurchaseLimitItem();
    return purchase_limit->find(item_index, info) == 0;
}

// IDA_EA: 0x819E6A6
// IDA_NAME: _ZN4ARAD16Arad_DataManager18getAradMileageDataEv
// DEMANGLED: ARAD::Arad_DataManager::getAradMileageData(void)
Arad_Mileage_Data* Arad_DataManager::getAradMileageData() {
    return &mileage_data_;
}

// IDA_EA: 0x84ED168
// IDA_NAME: _ZN4ARAD16Arad_DataManager16getExceptionItemEv
// DEMANGLED: ARAD::Arad_DataManager::getExceptionItem(void)
ExceptionItem* Arad_DataManager::getExceptionItem() {
    return &exception_item_;
}

// IDA_EA: 0x84ED174
// IDA_NAME: _ZN4ARAD16Arad_DataManager26getAradVillageAttackRewardEv
// DEMANGLED: ARAD::Arad_DataManager::getAradVillageAttackReward(void)
Arad_VillageAttackRewardItem*
Arad_DataManager::getAradVillageAttackReward() {
    return &village_attack_reward_item_;
}

}  // namespace ARAD
