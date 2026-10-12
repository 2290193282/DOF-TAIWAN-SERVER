#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_SHOP_PARAMETER_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_SHOP_PARAMETER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

namespace advancealtar {

// IDA_EA: FieldData @0x88A34FE/0x88A3820; BuyUpgradeData @0x88A2C62;
// BuyShopData @0x88A105C; AdvanceAltarShopParameter @0x88A0F5C/0x88A0FA2.
namespace FieldType {
enum T {
    LEVEL = 0,
    INDEX = 1,
    PRICE = 2,
    NAME = 3,
    COOLTIME = 4,
    NEED_POINT = 5,
    EXPLAIN = 6,
    TYPE_EXPLAIN = 7,
    SET_SLOT = 8,
    IMAGE_INDEX = 9,
    HP = 10,
    DMG = 11,
    PIC = 12,
    RCV = 13,
    SUM = 14,
    MHT = 15,
    RGN = 16,
    MSD = 17,
    ASD = 18,
    SDT = 19,
    FDT = 20,
    ETC = 21,
    LRG = 22,
    SPL = 23,
    ETC2 = 24,
    INVALID = -1
};
}  // namespace FieldType

namespace AdvanceAltarShopType {
enum T {
    UNIT = 0,
    SKILL = 1,
    TOWER = 2
};
}  // namespace AdvanceAltarShopType

// IDA unit: 99_misc_global/advancealtar/FieldData.
struct FieldData {
    FieldData();
    ~FieldData();
    FieldData(const FieldData& other);

    FieldType::T field_type;  // +0x00
    int32_t int_value;        // +0x04
    std::string string_value;  // +0x08
};

static_assert(sizeof(FieldData) == 0x0Cu,
              "advancealtar::FieldData IDA size must stay 0x0C");
static_assert(offsetof(FieldData, int_value) == 0x04u,
              "FieldData int_value offset must stay 0x04");
static_assert(offsetof(FieldData, string_value) == 0x08u,
              "FieldData string_value offset must stay 0x08");

// IDA unit: 99_misc_global/advancealtar/_StarRestFeeByGold.
struct _StarRestFeeByGold {
    _StarRestFeeByGold();

    int32_t gold_fee;  // +0x00
};

static_assert(sizeof(_StarRestFeeByGold) == 0x04u,
              "advancealtar::_StarRestFeeByGold IDA size must stay 0x04");

// IDA unit: 99_misc_global/advancealtar/BuyUpgradeData.
struct BuyUpgradeData {
    BuyUpgradeData();
    ~BuyUpgradeData();
    BuyUpgradeData(const BuyUpgradeData& other);

    BuyUpgradeData& operator=(const BuyUpgradeData& other);
    FieldData* getFieldData(int level, FieldType::T field_type);
    FieldData* getFieldDataPoint(int level, FieldType::T field_type);
    FieldData* getMinLevelFieldDataPoint(FieldType::T field_type, int& level);
    int getStatFirstPos() const;
    FieldType::T getFieldType(int index) const;
    int getStatIconIndex(FieldType::T field_type) const;
    std::string getStatStringIndex(FieldType::T field_type) const;
    int getMaxLevel() const;
    int getMinLevel() const;
    int isValidLevel(int level) const;

    int32_t id;          // +0x00
    int32_t unlock_stage_index;  // +0x04
    uint8_t is_settable_slot;    // +0x08
    uint8_t reserved_09[0x03];   // +0x09
    std::string name;            // +0x0C
    std::vector<FieldType::T> field_types;  // +0x10
    std::map<int, std::map<FieldType::T, FieldData> > level_fields;  // +0x1C
};

static_assert(sizeof(BuyUpgradeData) == 0x34u,
              "advancealtar::BuyUpgradeData IDA size must stay 0x34");
static_assert(offsetof(BuyUpgradeData, is_settable_slot) == 0x08u,
              "BuyUpgradeData is_settable_slot offset must stay 0x08");
static_assert(offsetof(BuyUpgradeData, name) == 0x0Cu,
              "BuyUpgradeData name offset must stay 0x0C");
static_assert(offsetof(BuyUpgradeData, field_types) == 0x10u,
              "BuyUpgradeData field_types offset must stay 0x10");
static_assert(offsetof(BuyUpgradeData, level_fields) == 0x1Cu,
              "BuyUpgradeData level_fields offset must stay 0x1C");

// IDA unit: 07_npc_shop_dialog/advancealtar/BuyShopData.
struct BuyShopData {
    BuyShopData();
    ~BuyShopData();
    BuyShopData(const BuyShopData& other);

    BuyShopData* reset();
    std::vector<BuyUpgradeData>* getBuyUpgradeDataVec(
        AdvanceAltarShopType::T shop_type);

    int32_t ridable_id;                         // +0x00
    std::vector<BuyUpgradeData> unit_upgrades;  // +0x04
    std::vector<BuyUpgradeData> skill_upgrades; // +0x10
    std::vector<BuyUpgradeData> tower_upgrades; // +0x1C
};

static_assert(sizeof(BuyShopData) == 0x28u,
              "advancealtar::BuyShopData IDA size must stay 0x28");
static_assert(offsetof(BuyShopData, unit_upgrades) == 0x04u,
              "BuyShopData unit_upgrades offset must stay 0x04");
static_assert(offsetof(BuyShopData, skill_upgrades) == 0x10u,
              "BuyShopData skill_upgrades offset must stay 0x10");
static_assert(offsetof(BuyShopData, tower_upgrades) == 0x1Cu,
              "BuyShopData tower_upgrades offset must stay 0x1C");

// IDA unit: 07_npc_shop_dialog/advancealtar/AdvanceAltarShopParameter.
class AdvanceAltarShopParameter {
public:
    AdvanceAltarShopParameter();
    ~AdvanceAltarShopParameter();

    std::vector<BuyUpgradeData>* getBuyUpgradeDataVec(
        int ridable_id, AdvanceAltarShopType::T shop_type);
    int importAdvanceAltarShopParameter(const char* filename);
    FieldType::T getFieldType(std::string field_name);
    bool getdataType(FieldType::T field_type);
    int getMaxStarResetGoldFee();

    std::map<int, BuyShopData> buy_shop_data;  // +0x00
    std::map<int, _StarRestFeeByGold> star_reset_fee_by_gold;  // +0x18
};

static_assert(sizeof(AdvanceAltarShopParameter) == 0x30u,
              "advancealtar::AdvanceAltarShopParameter IDA size must stay 0x30");
static_assert(offsetof(AdvanceAltarShopParameter,
                       star_reset_fee_by_gold) == 0x18u,
              "AdvanceAltarShopParameter star map offset must stay 0x18");

}  // namespace advancealtar

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_SHOP_PARAMETER_H_
