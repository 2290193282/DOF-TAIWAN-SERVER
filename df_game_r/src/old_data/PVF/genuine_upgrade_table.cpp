#include "old_data/PVF/genuine_upgrade_table.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <new>
#include <string>

int importGenuineUpgradeTable(const char* filename,
                              GenuineDamageUpgradeTable* table)
    __asm__("_Z25importGenuineUpgradeTablePKcP25GenuineDamageUpgradeTable");

// IDA_EA: 0x891BAF0
// IDA_NAME: _ZN24GenuineDamageUpgradeInfoC2Ev
// DEMANGLED: GenuineDamageUpgradeInfo::GenuineDamageUpgradeInfo(void)
GenuineDamageUpgradeInfo::GenuineDamageUpgradeInfo()
    : normal_damage(0),
      reinforced_damage(0),
      material_rate(0.0f),
      grade_rate(0.0f) {}

// IDA_EA: 0x891BB1E
// IDA_NAME: _ZN21OutLineEFFECTInfoTempC2Ev
// DEMANGLED: OutLineEFFECTInfoTemp::OutLineEFFECTInfoTemp(void)
OutLineEFFECTInfoTemp::OutLineEFFECTInfoTemp()
    : effect_index(0), color(0), effect_type(0), outline_width(0) {}

// IDA_EA: 0x811EE5E
// IDA_NAME: _ZN25GenuineDamageUpgradeTableC2Ev
// DEMANGLED: GenuineDamageUpgradeTable::GenuineDamageUpgradeTable(void)
GenuineDamageUpgradeTable::GenuineDamageUpgradeTable()
    : material_by_grade(), info_by_upgrade(), effect_by_upgrade() {}

// IDA_EA: 0x811EEDA
// IDA_NAME: _ZN25GenuineDamageUpgradeTableD2Ev
// DEMANGLED: GenuineDamageUpgradeTable::~GenuineDamageUpgradeTable()
GenuineDamageUpgradeTable::~GenuineDamageUpgradeTable() {}

// IDA_EA: 0x8912A54
// IDA_NAME: _ZN25GenuineDamageUpgradeTable23getGenuineUpgradeDamageE11ENUM_RARITYiiiib
// DEMANGLED: GenuineDamageUpgradeTable::getGenuineUpgradeDamage(ENUM_RARITY,int,int,int,int,bool)
int GenuineDamageUpgradeTable::getGenuineUpgradeDamage(
    ENUM_RARITY rarity,
    int weapon_type,
    int stat_type,
    int separate_upgrade,
    int growth_grade,
    bool reinforced) {
    if (separate_upgrade == 0) {
        return 0;
    }

    GenuineDamageUpgradeInfo& info = info_by_upgrade[separate_upgrade];
    const int base_damage =
        reinforced ? info.reinforced_damage : info.normal_damage;
    const float damage =
        static_cast<float>(base_damage) *
        (static_cast<float>(growth_grade) / static_cast<float>(level) + 1.0f) *
        weapon_weights[weapon_type][stat_type] *
        stat_weights_by_rarity[static_cast<int>(rarity)];
    return static_cast<int>(damage + 0.5f);
}

// IDA_EA: 0x8912B88
// IDA_NAME: _ZNK25GenuineDamageUpgradeTable28getGenuineUpgradeMaterialCntEhi18ENUM_EQUIPMENTTYPE11ENUM_RARITY
// DEMANGLED: GenuineDamageUpgradeTable::getGenuineUpgradeMaterialCnt(unsigned char,int,ENUM_EQUIPMENTTYPE,ENUM_RARITY)const
int GenuineDamageUpgradeTable::getGenuineUpgradeMaterialCnt(
    unsigned char growth_grade,
    int upgrade_level,
    ENUM_EQUIPMENTTYPE equipment_type,
    ENUM_RARITY rarity) const {
    if (equipment_type != static_cast<ENUM_EQUIPMENTTYPE>(
                              oldsrv::config::kEquipmentTypeWeapon) ||
        growth_grade > 99) {
        return -1;
    }

    float rate = item_weights_by_rarity[static_cast<int>(rarity)];
    std::map<int, GenuineDamageUpgradeInfo>::const_iterator info_it =
        info_by_upgrade.find(upgrade_level + 1);
    if (info_it == info_by_upgrade.end()) {
        return -1;
    }
    rate *= info_it->second.grade_rate;
    rate += 0.0000001f;

    std::map<int, GenuineDamageUpgradeMaterialInfo>::const_iterator material_it =
        material_by_grade.find(static_cast<int>(growth_grade));
    if (material_it == material_by_grade.end()) {
        return -1;
    }

    const int count =
        static_cast<int>(static_cast<long double>(material_it->second.item_count) *
                         static_cast<long double>(rate));
    return count != 0 ? count : 1;
}

// IDA_EA: 0x8912D10
// IDA_NAME: _ZN25GenuineDamageUpgradeTable13getEffectInfoEi
// DEMANGLED: GenuineDamageUpgradeTable::getEffectInfo(int)
OutLineEFFECTInfoTemp GenuineDamageUpgradeTable::getEffectInfo(
    int separate_upgrade) {
    static OutLineEFFECTInfoTemp temp;
    if (separate_upgrade_max >= separate_upgrade) {
        std::map<int, OutLineEFFECTInfoTemp>::const_iterator it =
            effect_by_upgrade.find(separate_upgrade);
        if (it != effect_by_upgrade.end()) {
            return it->second;
        }
    }
    return temp;
}

// IDA_EA: 0x891229D
// IDA_NAME: _Z22getGenuineUpgradeTablev
// DEMANGLED: getGenuineUpgradeTable(void)
GenuineDamageUpgradeTable* getGenuineUpgradeTable()
    __asm__("_Z22getGenuineUpgradeTablev");
GenuineDamageUpgradeTable* getGenuineUpgradeTable() {
    static GenuineDamageUpgradeTable* genuine_damage_upgrade_table = 0;
    if (genuine_damage_upgrade_table != 0) {
        return genuine_damage_upgrade_table;
    }

    void* storage = ::operator new(0x188u);
    std::memset(storage, 0, 0x188u);
    genuine_damage_upgrade_table =
        new (storage) GenuineDamageUpgradeTable();
    if (importGenuineUpgradeTable("Etc/Upgrade_Separate.etc",
                                  genuine_damage_upgrade_table) == 1) {
        return genuine_damage_upgrade_table;
    }
    return 0;
}

// IDA_EA: 0x8912336
// IDA_NAME: _Z25importGenuineUpgradeTablePKcP25GenuineDamageUpgradeTable
// DEMANGLED: importGenuineUpgradeTable(char const*,GenuineDamageUpgradeTable *)
int importGenuineUpgradeTable(const char* filename,
                              GenuineDamageUpgradeTable* table) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[table]") {
            int index = 1;
            table->info_by_upgrade.clear();
            while (1) {
                GenuineDamageUpgradeInfo info;
                info.normal_damage = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.reinforced_damage = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int material_rate = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.material_rate =
                    static_cast<float>(material_rate) / 10000.0f;
                info.grade_rate = ScanFloat(&success);
                if (!success) {
                    break;
                }
                table->info_by_upgrade.insert(std::make_pair(index++, info));
            }
        } else if (tag == "[separate upgrade max]") {
            table->separate_upgrade_max = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importGenuineUpgradeTable",
                    0x2F9E,
                    "importGenuineUpgradeTable() - [separate_upgrade max] false");
            }
        } else if (tag == "[level]") {
            table->level = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importGenuineUpgradeTable",
                    0x2FA5,
                    "importGenuineUpgradeTable() - [level] false");
            }
        } else if (tag == "[item weights by grade]") {
            while (1) {
                const int grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                GenuineDamageUpgradeMaterialInfo info;
                info.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                table->material_by_grade.insert(std::make_pair(grade, info));
            }
        } else if (tag == "[item weights by rarity]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                table->item_weights_by_rarity[rarity] =
                    ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importGenuineUpgradeTable",
                        0x2FBE,
                        "importGenuineUpgradeTable() - [item weights by rarity] false");
                }
            }
        } else if (tag == "[stat weights by rarity]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                table->stat_weights_by_rarity[rarity] =
                    ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importGenuineUpgradeTable",
                        0x2FC9,
                        "importGenuineUpgradeTable() - [stat weights by rarity] false");
                }
            }
        } else if (tag == "[weapon weights]") {
            for (int weapon = 0; weapon <= 10; ++weapon) {
                for (int rarity = 0; rarity <= 5; ++rarity) {
                    table->weapon_weights[weapon][rarity] =
                        ScanFloat(&success);
                    if (!success) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptEtcParameter.cpp",
                            "importGenuineUpgradeTable",
                            0x2FD5,
                            "importGenuineUpgradeTable() - [weapon weights] false");
                    }
                }
            }
        } else if (tag == "[separate upgrade effect]") {
            int index = 1;
            while (1) {
                OutLineEFFECTInfoTemp info;
                const int red = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int green = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int blue = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.color = 0xFF000000u |
                             (static_cast<unsigned int>(blue) & 0xFFu) << 16 |
                             (static_cast<unsigned int>(green) & 0xFFu) << 8 |
                             (static_cast<unsigned int>(red) & 0xFFu);
                info.outline_width = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.effect_type = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.effect_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                table->effect_by_upgrade.insert(std::make_pair(index++, info));
            }
            break;
        }
    }
    return 1;
}

// IDA_EA: 0x811ECA8
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate17LoadUpgradeScriptEPKc
// DEMANGLED: WongWork::CItemUpgrade_Separate::LoadUpgradeScript(char const*)
int WongWork::CItemUpgrade_Separate::LoadUpgradeScript(
    const char* filename) {
    return importGenuineUpgradeTable(filename, &genuine_upgrade_table_);
}

// IDA_EA: 0x811EE16
// IDA_NAME: _ZNK8WongWork21CItemUpgrade_Separate19getUpgradeInfoTableEv
// DEMANGLED: WongWork::CItemUpgrade_Separate::getUpgradeInfoTable(void)const
GenuineDamageUpgradeTable*
WongWork::CItemUpgrade_Separate::getUpgradeInfoTable() const {
    return const_cast<GenuineDamageUpgradeTable*>(&genuine_upgrade_table_);
}
