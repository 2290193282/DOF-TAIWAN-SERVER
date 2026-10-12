#include "old_game/activity/reward/advance_altar_shop_parameter.h"

#include "infra/log/format/trace.h"


namespace app {
std::string getStr(int id);
}  // namespace app

namespace advancealtar {

// IDA_EA: 0x88A34FE
// IDA_NAME: _ZN12advancealtar9FieldDataC2Ev
// DEMANGLED: advancealtar::FieldData::FieldData(void)
FieldData::FieldData()
    : field_type(FieldType::INVALID),
      int_value(0),
      string_value() {
    string_value.clear();
}

// IDA_EA: 0x88A3820
// IDA_NAME: _ZN12advancealtar9FieldDataD2Ev
// DEMANGLED: advancealtar::FieldData::~FieldData()
FieldData::~FieldData() {}

// IDA_EA: 0x88A3DEE
// IDA_NAME: _ZN12advancealtar9FieldDataC2ERKS0_
// DEMANGLED: advancealtar::FieldData::FieldData(FieldData const&)
FieldData::FieldData(const FieldData& other)
    : field_type(other.field_type),
      int_value(other.int_value),
      string_value(other.string_value) {}

// IDA_EA: 0x88A3760
// IDA_NAME: _ZN12advancealtar18_StarRestFeeByGoldC2Ev
// DEMANGLED: advancealtar::_StarRestFeeByGold::_StarRestFeeByGold(void)
_StarRestFeeByGold::_StarRestFeeByGold()
    : gold_fee(0) {}

// IDA_EA: 0x88A2C62
// IDA_NAME: _ZN12advancealtar14BuyUpgradeData12getFieldDataEiNS_9FieldType1TE
// DEMANGLED: advancealtar::BuyUpgradeData::getFieldData(int,FieldType::T)
FieldData* BuyUpgradeData::getFieldData(int level,
                                        FieldType::T field_type) {
    LogManager::logFormat(
        0,
        "../localglobal/global_RDARScriptAdvanceAltar_Shop.cpp",
        "getFieldData",
        0x26E,
        "BuyUpgradeData::GetFieldData level = %d",
        level);

    FieldData* field_data = getFieldDataPoint(level, field_type);
    if (field_data) {
        return field_data;
    }

    const std::string field_type_name = getStatStringIndex(field_type);
    LogManager::logFormat(
        0,
        "../localglobal/global_RDARScriptAdvanceAltar_Shop.cpp",
        "getFieldData",
        0x27B,
        "AdvanceAltar BuyUpgradeData::GetFieldData Error level = %d, type = %s",
        level,
        field_type_name.c_str());
    return getFieldDataPoint(1, field_type);
}

// IDA_EA: 0x88A2E4C
// IDA_NAME: _ZN12advancealtar14BuyUpgradeData17getFieldDataPointEiNS_9FieldType1TE
// DEMANGLED: advancealtar::BuyUpgradeData::getFieldDataPoint(int,FieldType::T)
FieldData* BuyUpgradeData::getFieldDataPoint(int level,
                                             FieldType::T field_type) {
    std::map<int, std::map<FieldType::T, FieldData> >::iterator level_it =
        level_fields.find(level);
    if (level_it == level_fields.end()) {
        return 0;
    }

    std::map<FieldType::T, FieldData>::iterator field_it =
        level_it->second.find(field_type);
    if (field_it != level_it->second.end()) {
        return &field_it->second;
    }
    return 0;
}

// IDA_EA: 0x88A2F1A
// IDA_NAME: _ZN12advancealtar14BuyUpgradeData25getMinLevelFieldDataPointENS_9FieldType1TERi
// DEMANGLED: advancealtar::BuyUpgradeData::getMinLevelFieldDataPoint(FieldType::T,int&)
FieldData* BuyUpgradeData::getMinLevelFieldDataPoint(
    FieldType::T field_type, int& level) {
    level = 0;
    FieldData* field_data = getFieldDataPoint(level, field_type);
    if (!field_data) {
        level = 1;
        return getFieldDataPoint(level, field_type);
    }
    return field_data;
}

// IDA_EA: 0x88A2F7A
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData15getStatFirstPosEv
// DEMANGLED: advancealtar::BuyUpgradeData::getStatFirstPos(void)const
int BuyUpgradeData::getStatFirstPos() const {
    int position = 0;
    for (std::vector<FieldType::T>::const_iterator it = field_types.begin();
         it != field_types.end(); ++it, ++position) {
        if (*it >= FieldType::HP && *it <= FieldType::ETC2) {
            return position;
        }
    }
    return -1;
}

// IDA_EA: 0x88A3012
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData12getFieldTypeEi
// DEMANGLED: advancealtar::BuyUpgradeData::getFieldType(int)const
FieldType::T BuyUpgradeData::getFieldType(int index) const {
    FieldType::T result = FieldType::INVALID;
    if (index >= 0 &&
        field_types.size() > static_cast<std::vector<FieldType::T>::size_type>(
                                 index)) {
        return field_types[index];
    }
    return result;
}

// IDA_EA: 0x88A3056
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData16getStatIconIndexENS_9FieldType1TE
// DEMANGLED: advancealtar::BuyUpgradeData::getStatIconIndex(FieldType::T)const
int BuyUpgradeData::getStatIconIndex(FieldType::T field_type) const {
    int result = -1;
    switch (field_type) {
        case FieldType::HP:
            result = 0;
            break;
        case FieldType::DMG:
            result = 1;
            break;
        case FieldType::PIC:
            result = 2;
            break;
        case FieldType::RCV:
            result = 3;
            break;
        case FieldType::SUM:
            result = 4;
            break;
        case FieldType::MHT:
            result = 5;
            break;
        case FieldType::RGN:
            result = 6;
            break;
        case FieldType::MSD:
            result = 7;
            break;
        case FieldType::ASD:
            result = 8;
            break;
        case FieldType::SDT:
            result = 9;
            break;
        case FieldType::FDT:
            result = 10;
            break;
        case FieldType::ETC:
            result = 11;
            break;
        case FieldType::LRG:
            result = 12;
            break;
        case FieldType::SPL:
            result = 13;
            break;
        case FieldType::ETC2:
            result = 14;
            break;
        default:
            return result;
    }
    return result;
}

// IDA_EA: 0x88A3106
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData18getStatStringIndexENS_9FieldType1TE
// DEMANGLED: advancealtar::BuyUpgradeData::getStatStringIndex(FieldType::T)const
std::string BuyUpgradeData::getStatStringIndex(
    FieldType::T field_type) const {
    int string_id = 0;
    switch (field_type) {
        case FieldType::HP:
            string_id = 0x11570;
            break;
        case FieldType::DMG:
            string_id = 0x11571;
            break;
        case FieldType::PIC:
            string_id = 0x11573;
            break;
        case FieldType::RCV:
            string_id = 0x11574;
            break;
        case FieldType::SUM:
            string_id = 0x11575;
            break;
        case FieldType::MHT:
            string_id = 0x11576;
            break;
        case FieldType::RGN:
            string_id = 0x1158F;
            break;
        case FieldType::MSD:
            string_id = 0x11590;
            break;
        case FieldType::ASD:
            string_id = 0x11591;
            break;
        case FieldType::SDT:
            string_id = 0x11592;
            break;
        case FieldType::FDT:
            string_id = 0x11593;
            break;
        case FieldType::ETC:
        case FieldType::ETC2:
            string_id = 0x11596;
            break;
        case FieldType::LRG:
            string_id = 0x11572;
            break;
        case FieldType::SPL:
            string_id = 0x11595;
            break;
        default:
            return std::string();
    }
    return app::getStr(string_id);
}

// IDA_EA: 0x88A3322
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData11getMaxLevelEv
// DEMANGLED: advancealtar::BuyUpgradeData::getMaxLevel(void)const
int BuyUpgradeData::getMaxLevel() const {
    int result = 0;
    for (std::map<int, std::map<FieldType::T, FieldData> >::const_iterator it =
             level_fields.begin();
         it != level_fields.end(); ++it) {
        if (it->first > result) {
            result = it->first;
        }
    }
    return result;
}

// IDA_EA: 0x88A33C0
// IDA_NAME: _ZNK12advancealtar14BuyUpgradeData11getMinLevelEv
// DEMANGLED: advancealtar::BuyUpgradeData::getMinLevel(void)const
int BuyUpgradeData::getMinLevel() const {
    int result = 0;
    for (std::map<int, std::map<FieldType::T, FieldData> >::const_iterator it =
             level_fields.begin();
         it != level_fields.end(); ++it) {
        if (it->first < result) {
            result = it->first;
        }
    }
    return result;
}

// IDA_EA: 0x88A345E
// IDA_NAME: _ZN12advancealtar14BuyUpgradeData12isValidLevelEi
// DEMANGLED: advancealtar::BuyUpgradeData::isValidLevel(int)
int BuyUpgradeData::isValidLevel(int level) const {
    if (getMinLevel() > level || getMaxLevel() < level) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x88A3570
// IDA_NAME: _ZN12advancealtar14BuyUpgradeDataC2Ev
// DEMANGLED: advancealtar::BuyUpgradeData::BuyUpgradeData(void)
BuyUpgradeData::BuyUpgradeData()
    : id(0),
      unlock_stage_index(0),
      is_settable_slot(1),
      reserved_09(),
      name(),
      field_types(),
      level_fields() {
    field_types.clear();
    level_fields.clear();
}

// IDA_EA: 0x88A3630
// IDA_NAME: _ZN12advancealtar14BuyUpgradeDataD2Ev
// DEMANGLED: advancealtar::BuyUpgradeData::~BuyUpgradeData()
BuyUpgradeData::~BuyUpgradeData() {}

// IDA_EA: 0x88A5674
// IDA_NAME: _ZN12advancealtar14BuyUpgradeDataC2ERKS0_
// DEMANGLED: advancealtar::BuyUpgradeData::BuyUpgradeData(BuyUpgradeData const&)
BuyUpgradeData::BuyUpgradeData(const BuyUpgradeData& other)
    : id(other.id),
      unlock_stage_index(other.unlock_stage_index),
      is_settable_slot(other.is_settable_slot),
      reserved_09(),
      name(other.name),
      field_types(other.field_types),
      level_fields(other.level_fields) {}

// IDA_EA: 0x88A5784
// IDA_NAME: _ZN12advancealtar14BuyUpgradeDataaSERKS0_
// DEMANGLED: advancealtar::BuyUpgradeData::operator=(BuyUpgradeData const&)
BuyUpgradeData& BuyUpgradeData::operator=(const BuyUpgradeData& other) {
    if (this != &other) {
        id = other.id;
        unlock_stage_index = other.unlock_stage_index;
        is_settable_slot = other.is_settable_slot;
        name = other.name;
        field_types = other.field_types;
        level_fields = other.level_fields;
    }
    return *this;
}

// IDA_EA: 0x88A105C
// IDA_NAME: _ZN12advancealtar11BuyShopData20getBuyUpgradeDataVecENS_20AdvanceAltarShopType1TE
// DEMANGLED: advancealtar::BuyShopData::getBuyUpgradeDataVec(AdvanceAltarShopType::T)
std::vector<BuyUpgradeData>* BuyShopData::getBuyUpgradeDataVec(
    AdvanceAltarShopType::T shop_type) {
    if (shop_type == AdvanceAltarShopType::SKILL) {
        return &skill_upgrades;
    }
    if (shop_type == AdvanceAltarShopType::TOWER) {
        return &tower_upgrades;
    }
    if (shop_type) {
        return 0;
    }
    return &unit_upgrades;
}

// IDA_EA: 0x88A1090
// IDA_NAME: _ZN12advancealtar11BuyShopData5resetEv
// DEMANGLED: advancealtar::BuyShopData::reset(void)
BuyShopData* BuyShopData::reset() {
    ridable_id = 0;
    unit_upgrades.clear();
    skill_upgrades.clear();
    tower_upgrades.clear();
    return this;
}

// IDA_EA: 0x88A36A4
// IDA_NAME: _ZN12advancealtar11BuyShopDataC2Ev
// DEMANGLED: advancealtar::BuyShopData::BuyShopData(void)
BuyShopData::BuyShopData()
    : ridable_id(0),
      unit_upgrades(),
      skill_upgrades(),
      tower_upgrades() {
    unit_upgrades.clear();
    skill_upgrades.clear();
    tower_upgrades.clear();
}

// IDA_EA: 0x88A37AC
// IDA_NAME: _ZN12advancealtar11BuyShopDataD2Ev
// DEMANGLED: advancealtar::BuyShopData::~BuyShopData()
BuyShopData::~BuyShopData() {}

// IDA_EA: 0x88A4138
// IDA_NAME: _ZN12advancealtar11BuyShopDataC2ERKS0_
// DEMANGLED: advancealtar::BuyShopData::BuyShopData(BuyShopData const&)
BuyShopData::BuyShopData(const BuyShopData& other)
    : ridable_id(other.ridable_id),
      unit_upgrades(other.unit_upgrades),
      skill_upgrades(other.skill_upgrades),
      tower_upgrades(other.tower_upgrades) {}

// IDA_EA: 0x88A0F5C
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameterC2Ev
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::AdvanceAltarShopParameter(void)
AdvanceAltarShopParameter::AdvanceAltarShopParameter()
    : buy_shop_data(),
      star_reset_fee_by_gold() {}

// IDA_EA: 0x88A0FA2
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameterD2Ev
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::~AdvanceAltarShopParameter()
AdvanceAltarShopParameter::~AdvanceAltarShopParameter() {}

// IDA_EA: 0x88A0FE8
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameter20getBuyUpgradeDataVecEiNS_20AdvanceAltarShopType1TE
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::getBuyUpgradeDataVec(int,AdvanceAltarShopType::T)
std::vector<BuyUpgradeData>*
AdvanceAltarShopParameter::getBuyUpgradeDataVec(
    int ridable_id, AdvanceAltarShopType::T shop_type) {
    std::map<int, BuyShopData>::iterator it = buy_shop_data.find(ridable_id);
    if (it == buy_shop_data.end()) {
        return 0;
    }
    return it->second.getBuyUpgradeDataVec(shop_type);
}

// IDA_EA: 0x88A27A2
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameter12getFieldTypeESs
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::getFieldType(std::string)
FieldType::T AdvanceAltarShopParameter::getFieldType(
    std::string field_name) {
    FieldType::T result = FieldType::INVALID;
    if (!field_name.compare("level")) {
        return FieldType::LEVEL;
    }
    if (!field_name.compare("index")) {
        return FieldType::INDEX;
    }
    if (!field_name.compare("price")) {
        return FieldType::PRICE;
    }
    if (!field_name.compare("name")) {
        return FieldType::NAME;
    }
    if (!field_name.compare("cooltime")) {
        return FieldType::COOLTIME;
    }
    if (!field_name.compare("need point")) {
        return FieldType::NEED_POINT;
    }
    if (!field_name.compare("explain")) {
        return FieldType::EXPLAIN;
    }
    if (!field_name.compare("type explain")) {
        return FieldType::TYPE_EXPLAIN;
    }
    if (!field_name.compare("set slot")) {
        return FieldType::SET_SLOT;
    }
    if (!field_name.compare("image index")) {
        return FieldType::IMAGE_INDEX;
    }
    if (!field_name.compare("hp")) {
        return FieldType::HP;
    }
    if (!field_name.compare("dmg")) {
        return FieldType::DMG;
    }
    if (!field_name.compare("pic")) {
        return FieldType::PIC;
    }
    if (!field_name.compare("rcv")) {
        return FieldType::RCV;
    }
    if (!field_name.compare("sum")) {
        return FieldType::SUM;
    }
    if (!field_name.compare("mht")) {
        return FieldType::MHT;
    }
    if (!field_name.compare("rgn")) {
        return FieldType::RGN;
    }
    if (!field_name.compare("msd")) {
        return FieldType::MSD;
    }
    if (!field_name.compare("asd")) {
        return FieldType::ASD;
    }
    if (!field_name.compare("sdt")) {
        return FieldType::SDT;
    }
    if (!field_name.compare("fdt")) {
        return FieldType::FDT;
    }
    if (!field_name.compare("etc")) {
        return FieldType::ETC;
    }
    if (!field_name.compare("lrg")) {
        return FieldType::LRG;
    }
    if (!field_name.compare("spl")) {
        return FieldType::SPL;
    }
    if (!field_name.compare("etc2")) {
        return FieldType::ETC2;
    }
    return result;
}

// IDA_EA: 0x88A2B8E
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameter11getdataTypeENS_9FieldType1TE
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::getdataType(FieldType::T)
bool AdvanceAltarShopParameter::getdataType(FieldType::T field_type) {
    const unsigned int value = static_cast<unsigned int>(field_type);
    return value > 7 || ((1u << value) & 0xC8u) == 0;
}

// IDA_EA: 0x88A2BC0
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameter22getMaxStarResetGoldFeeEv
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::getMaxStarResetGoldFee(void)
int AdvanceAltarShopParameter::getMaxStarResetGoldFee() {
    int max_star = 0;
    for (std::map<int, _StarRestFeeByGold>::iterator it =
             star_reset_fee_by_gold.begin();
         it != star_reset_fee_by_gold.end(); ++it) {
        if (it->first > max_star) {
            max_star = it->first;
        }
    }
    return star_reset_fee_by_gold[max_star].gold_fee;
}

}  // namespace advancealtar
