#include "common/manager/premium_public_abi.h"

#include "common/config.h"

typedef char SPremiumInfo_Size_Must_Be_0x120[
    (sizeof(SPremiumInfo) == 0x120) ? 1 : -1];
typedef char SPremiumOverEquipableInfo_Size_Must_Be_0x70[
    (sizeof(SPremiumOverEquipableInfo) == 0x70) ? 1 : -1];
typedef char SPremiumReturnItemInfo_Size_Must_Be_0x2c[
    (sizeof(SPremiumReturnItemInfo) == 0x2c) ? 1 : -1];
typedef char STPremiumItemData_Size_Must_Be_0x0c[
    (sizeof(STPremiumItemData) == 0x0c) ? 1 : -1];

SPremiumInfo g_SPremiumInfo[oldsrv::config::kPremiumDefinitionTypeCount];
std::vector<SPremiumOverEquipableInfo> g_vecPremiumOverEquipableInfo;
std::vector<SPremiumReturnItemInfo> g_vecPremiumReturnItemInfo;
std::map<unsigned long, STPremiumItemData> g_map_premium_item_data;

// IDA_EA: 0x86B16D4
// IDA_NAME: _ZN12SPremiumInfoC2Ev
// DEMANGLED: SPremiumInfo::SPremiumInfo(void)
SPremiumInfo::SPremiumInfo()
    : advantage_bucket(0),
      uses_time_window(0),
      expire_time(0),
      item_rewards(),
      fatigue_rate(0),
      reserved_036(0),
      coin_bonus(0),
      exp_bonus(0),
      unlimited_fatigue(false),
      reserved_041(0),
      reserved_042(0),
      add_exp_bonus(0),
      add_quest_item_drop_rate(0),
      independent_drop_items(),
      over_skill_level(0),
      cera_shop_discount_rate(0),
      apply_target_server(0),
      gold_bonus(false),
      reserved_061(),
      over_equipable_count(0),
      over_equipable(),
      return_item_count(0),
      return_items(),
      apply_weekdays(),
      additional_info(),
      name() {}

// IDA_EA: 0x86B1732
// IDA_NAME: _ZN12SPremiumInfoD2Ev
// DEMANGLED: SPremiumInfo::~SPremiumInfo()
SPremiumInfo::~SPremiumInfo() {}

// IDA_EA: 0x8939B4E
// IDA_NAME: _ZN25SPremiumOverEquipableInfoD2Ev
// DEMANGLED: SPremiumOverEquipableInfo::~SPremiumOverEquipableInfo()
SPremiumOverEquipableInfo::~SPremiumOverEquipableInfo() {}

// IDA_EA: 0x8939E04
// IDA_NAME: _ZN22SPremiumReturnItemInfoD2Ev
// DEMANGLED: SPremiumReturnItemInfo::~SPremiumReturnItemInfo()
SPremiumReturnItemInfo::~SPremiumReturnItemInfo() {}

// IDA_EA: 0x86B0826
// IDA_NAME: _ZN17STPremiumItemDataC2Ev
// DEMANGLED: STPremiumItemData::STPremiumItemData(void)
STPremiumItemData::STPremiumItemData()
    : premium_type(0),
      term(0),
      restrict_flag(0),
      reserved_06(),
      target_item_id(0) {}

// C owns CUserPremium gameplay members.  The former public-ABI approximation
// of SetPremiumItemData used a conflicting class layout and is intentionally
// not provided from global/.
