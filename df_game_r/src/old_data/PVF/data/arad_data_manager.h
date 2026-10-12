#ifndef OLDSRV_OLD_DATA_PVF_DATA_ARAD_DATA_MANAGER_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ARAD_DATA_MANAGER_H_

#include <map>
#include <string>

#include "old_data/PVF/arad/arad_script_interface.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_data/PVF/data/arad_one_plus_card_event_item_list.h"
#include "old_data/PVF/etc/arad_special_use_item_script.h"
#include "old_data/PVF/town/arad_village_attack_reward.h"

class CUser;

namespace ARAD {

// IDA type: ARAD::Arad_InterfaceGameScript (0x08 on i386).
struct Arad_InterfaceGameScript {
    explicit Arad_InterfaceGameScript(std::string script_name);
    virtual ~Arad_InterfaceGameScript();

    virtual bool loadScriptData() = 0;
    virtual void clearScriptData() = 0;

    std::string scriptDataName() const;

    std::string script_name_;
};

// IDA type: ARAD::FN_DeleteGameScript.  The map owns each registered parser.
struct FN_DeleteGameScript {
    void operator()(
        std::pair<const std::string, Arad_InterfaceGameScript*>& value);
};

// IDA type: ARAD::Arad_DataManager (0xBC on i386).  The empty Singleton base
// is first in the old type hierarchy; ScriptInterface is the primary dynamic
// base at offset zero.
class Arad_DataManager : public Singleton<Arad_DataManager>,
                         public ScriptInterface {
public:
    Arad_DataManager();
    ~Arad_DataManager();

    Arad_InterfaceGameScript* findGameScript(std::string name);
    virtual bool Load();
    virtual int Destroy();
    bool initAradData();
    void destroyAradData();
    bool __insertGameScript(Arad_InterfaceGameScript* script);
    PurchaseLimitItem* getPurchaseLimitItem();
    Arad_Mileage_Data* getAradMileageData();
    ExceptionItem* getExceptionItem();
    Arad_VillageAttackRewardItem* getAradVillageAttackReward();

private:
    std::map<std::string, Arad_InterfaceGameScript*> game_scripts_;
    ExceptionItem exception_item_;
    PurchaseLimitItem purchase_limit_item_;
    AradBonusItem bonus_item_;
    Arad_Mileage_Data mileage_data_;
    Arad_OnePlusCardEvent_ItemList one_plus_card_event_item_list_;
    Arad_VillageAttackRewardItem village_attack_reward_item_;
};

template <>
Singleton<Arad_DataManager>::Singleton();

template <>
Arad_DataManager* Singleton<Arad_DataManager>::Get();

// IDA_EA: 0x8196AC8
// IDA_NAME: _ZN4ARAD16CheckAccountItemEP5CUserjRN17PurchaseLimitItem4InfoE
// DEMANGLED: ARAD::CheckAccountItem(CUser *,unsigned int,PurchaseLimitItem::Info &)
bool CheckAccountItem(::CUser* user,
                      uint32_t item_index,
                      PurchaseLimitItem::Info& info);

}  // namespace ARAD

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ARAD::ScriptInterface) == 0x04u,
              "ARAD::ScriptInterface IDA ABI size must stay 0x04");
static_assert(sizeof(ARAD::Arad_InterfaceGameScript) == 0x08u,
              "ARAD::Arad_InterfaceGameScript IDA ABI size must stay 0x08");
static_assert(sizeof(ARAD::Arad_DataManager) == 0xBCu,
              "ARAD::Arad_DataManager IDA ABI size must stay 0xBC");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ARAD_DATA_MANAGER_H_
