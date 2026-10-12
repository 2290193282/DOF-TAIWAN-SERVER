#ifndef OLDSRV_OLD_DATA_PVF_GENUINE_UPGRADE_TABLE_H_
#define OLDSRV_OLD_DATA_PVF_GENUINE_UPGRADE_TABLE_H_

#include <stddef.h>

#include <map>

#include "old_data/PVF/equipment/item_upgrade.h"

class CItem;

struct GenuineDamageUpgradeInfo {
    GenuineDamageUpgradeInfo();

    int normal_damage;
    int reinforced_damage;
    float material_rate;
    float grade_rate;
};

struct GenuineDamageUpgradeMaterialInfo {
    int item_index;
    int item_count;
};

struct OutLineEFFECTInfoTemp {
    OutLineEFFECTInfoTemp();

    int effect_index;
    unsigned int color;
    int effect_type;
    int outline_width;
};

class GenuineDamageUpgradeTable {
public:
    GenuineDamageUpgradeTable();
    ~GenuineDamageUpgradeTable();

    int getGenuineUpgradeDamage(ENUM_RARITY rarity,
                                int weapon_type,
                                int stat_type,
                                int separate_upgrade,
                                int growth_grade,
                                bool reinforced);
    int getGenuineUpgradeMaterialCnt(unsigned char growth_grade,
                                     int upgrade_level,
                                     ENUM_EQUIPMENTTYPE equipment_type,
                                     ENUM_RARITY rarity) const;
    OutLineEFFECTInfoTemp getEffectInfo(int separate_upgrade);

    int level;
    int separate_upgrade_max;
    float item_weights_by_rarity[6];
    float stat_weights_by_rarity[6];
    float weapon_weights[11][6];
    std::map<int, GenuineDamageUpgradeMaterialInfo> material_by_grade;
    std::map<int, GenuineDamageUpgradeInfo> info_by_upgrade;
    std::map<int, OutLineEFFECTInfoTemp> effect_by_upgrade;
};

namespace WongWork {

struct STUpgradeInfo {
    int material_count;
    int success_rate_10000;
};

// IDA_EA: C2 @0x811DFC0, D2 @0x811E0BE.  The table begins immediately
// after IItemUpgrade's two-slot vptr and CMTRand/range occupy +0x18c/+0x190.
class CItemUpgrade_Separate : public IItemUpgrade {
public:
    CItemUpgrade_Separate();
    ~CItemUpgrade_Separate();

    virtual int ProcUpgrade(CUser* user,
                            MSG_ITEM_UPGRADE* message,
                            PacketGuard* packet);
    virtual int LoadUpgradeScript(const char* filename);
    GenuineDamageUpgradeTable* getUpgradeInfoTable() const;

private:
    int _CheckCondition(const CUser* user,
                        const Inven_Item& target,
                        const CItem* item,
                        const Inven_Item& material,
                        const GenuineDamageUpgradeTable& table,
                        int charac_level) const;
    int _GetUpgradeInfo(const Inven_Item& target,
                        const CItem* item,
                        const GenuineDamageUpgradeTable& table,
                        STUpgradeInfo* result,
                        int charac_level);
    int _GetPenaltyType(unsigned char separate_upgrade,
                        ENUM_RARITY rarity,
                        const GenuineDamageUpgradeTable& table,
                        const GenuineDamageUpgradeInfo* upgrade_info);
    int _DoProcUpgrade(CUser* user,
                       Inven_Item* target,
                       const STUpgradeInfo& info);
    void _NoticeUpgrade(const CUser* user,
                        const Inven_Item& target,
                        bool succeeded);

public:
    // Kept public solely so the i386 layout assertions below can guard the
    // binary provider boundary; gameplay mutates these only through methods.
    GenuineDamageUpgradeTable genuine_upgrade_table_;
    CMTRand* random_;
    unsigned int random_limit_;
};

}  // namespace WongWork

GenuineDamageUpgradeTable* getGenuineUpgradeTable();
int importGenuineUpgradeTable(const char* filename,
                              GenuineDamageUpgradeTable* table);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(GenuineDamageUpgradeInfo) == 0x10u,
              "GenuineDamageUpgradeInfo IDA ABI size must stay 0x10");
static_assert(sizeof(GenuineDamageUpgradeMaterialInfo) == 0x08u,
              "GenuineDamageUpgradeMaterialInfo IDA ABI size must stay 0x08");
static_assert(sizeof(OutLineEFFECTInfoTemp) == 0x10u,
              "OutLineEFFECTInfoTemp IDA ABI size must stay 0x10");
static_assert(sizeof(GenuineDamageUpgradeTable) == 0x188u,
              "GenuineDamageUpgradeTable IDA ABI size must stay 0x188");
static_assert(sizeof(WongWork::STUpgradeInfo) == 0x08u,
              "STUpgradeInfo IDA ABI size must stay 0x08");
static_assert(offsetof(WongWork::CItemUpgrade_Separate,
                       genuine_upgrade_table_) == 0x04u,
              "CItemUpgrade_Separate table must begin at IDA +0x04");
static_assert(offsetof(WongWork::CItemUpgrade_Separate, random_) == 0x18cu,
              "CItemUpgrade_Separate random pointer must stay at IDA +0x18c");
static_assert(sizeof(WongWork::CItemUpgrade_Separate) == 0x194u,
              "CItemUpgrade_Separate IDA ABI size must stay 0x194");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_GENUINE_UPGRADE_TABLE_H_
