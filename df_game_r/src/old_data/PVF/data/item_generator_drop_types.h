#ifndef OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_DROP_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_DROP_TYPES_H_

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "old_data/PVF/data/item_generator_types.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_game/world/region/world_value_types.h"

class CMTRand;
class CMonster;

namespace WongWork {

class CItemGenRateTable {
public:
    CItemGenRateTable();
    ~CItemGenRateTable();
    CItemGenRateTable* init();
    CItemGenRateTable* destroy();
    random_option::CRandomOptionItemHandle*
    set_disable_drop_random_option_item();
    bool generateTable(const unsigned char item_ref_range[201][2],
                       bool include_random_option_items);
    int chooseItem(int drop_type, int rarity, int character_level);

private:
    std::map<unsigned int, int> item_rates_[5][6][201];
    CMTRand* random_;
    random_option::CRandomOptionItemHandle random_option_handle_;
};

class CSecialIMonsterDropItemGenRateTable {
public:
    CSecialIMonsterDropItemGenRateTable();
    ~CSecialIMonsterDropItemGenRateTable();
    CSecialIMonsterDropItemGenRateTable* init();
    CSecialIMonsterDropItemGenRateTable* destroy();
    bool generateTable(const unsigned char item_ref_range[201][2]);
    int chooseItem(int drop_type,
                   int rarity,
                   int character_level,
                   int dungeon_difficulty,
                   int dungeon_type);

private:
    std::map<unsigned int, int> item_rates_[5][3][5][6][201];
    CMTRand* random_;
    random_option::CRandomOptionItemHandle random_option_handle_;
};

// Shared parser-owned header for the normal and special monster generators.
// The fields map one-for-one to the tags read by their RDAR loaders.
class CMonsterDrop_Hell : public CItemGenerator {
public:
    // IDA calls the two vectors below
    // std::vector<CMonsterDrop_Hell::stGenItemProb_t>, not the analogous
    // normal/special-generator row type.  Keeping this distinct preserves
    // both the original container lifetime and its GCC 4.9 template ABI.
    struct stGenItemProb_t {
        int first_level;
        int last_level;
        int weight[5];
    };

    // IDA_EA: 0x853A56A / 0x853A60E.  The old C2 constructs the two
    // probability vectors at +0x18/+0x24 and the rate table at +0x254;
    // D2 destroys them in the reverse order before CItemGenerator::D2.
    explicit CMonsterDrop_Hell(int random_stream_count);
    ~CMonsterDrop_Hell();

    // Vtable 0x8C9A778 has these three overrides at +0x00, +0x04 and +0x0c.
    // Its +0x08 slot remains CItemGenerator::generateRandomItem.
    virtual void setDropRatio(float ratio);
    virtual int loadScript(const char* filename);
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);

    // These are real named members rather than extra vtable slots.
    // IDA_EA: 0x8535A88 / 0x8535B08.
    int _getGenItemProbIdx(int standard_level);
    void hellPartySpecificItem(const stGenerateRefData_t& reference,
                               stGenerateResult_t& result);

protected:
    std::vector<stGenItemProb_t> probabilities_;        // +0x18
    std::vector<stGenItemProb_t> scaled_probabilities_; // +0x24
    int rarity_decision_[5][6];                         // +0x30
    int hell_drop_metadata_[6];                         // +0xa8
    unsigned char item_ref_range_[201][2];              // +0xc0
    unsigned char item_ref_padding_[2];                  // +0x252
    CItemGenRateTable item_rate_table_;                  // +0x254
};

class CSpecialMonsterDrop : public CItemGenerator {
public:
    // IDA names these containers
    // std::vector<CSpecialMonsterDrop::stGenItemProb_t>, which is a distinct
    // GCC 4.9 template instantiation from the normal and HELL generators.
    struct stGenItemProb_t {
        int first_level;
        int last_level;
        int weight[5];
    };

    // IDA_EA: 0x853A426 / 0x853A4CA.
    explicit CSpecialMonsterDrop(int random_stream_count);
    ~CSpecialMonsterDrop();

    // Vtable 0x8C9A760: this class overrides only slots +0x00 and +0x04.
    // The random/specific slots remain CItemGenerator's originals.
    virtual void setDropRatio(float ratio);
    virtual int loadScript(const char* filename);

    // IDA_EA: 0x8536882 / 0x853736E.  These are named helper members, not
    // vtable entries.
    int _getGenItemProbIdx(int standard_level);
    void _generateRandomItem(uint8_t character_level,
                             int8_t monster_role,
                             uint8_t dungeon_standard_level,
                             uint8_t dungeon_difficulty,
                             uint8_t party_member_count,
                             int bourgeois_rate,
                             uint8_t item_drop_dungeon_type,
                             int standard_drop_rate,
                             stGenerateResult_t& result,
                             bool& generated_special_item);

protected:
    std::vector<stGenItemProb_t> probabilities_;        // +0x18
    std::vector<stGenItemProb_t> scaled_probabilities_; // +0x24
    int rarity_decision_[5][6];                         // +0x30
    float party_member_bonus_[5][4];                    // +0xa8
    float dungeon_difficulty_bonus_[5][5];              // +0xf8
    float monster_type_bonus_[5][4];                    // +0x15c
    unsigned char item_ref_range_[201][2];              // +0x1ac
    unsigned char item_ref_padding_[2];                  // +0x33e
    CSecialIMonsterDropItemGenRateTable item_rate_table_; // +0x340
};

class CMonsterDrop : public CItemGenerator {
public:
    // IDA C2 @0x853A6C8 constructs vectors of this exact nested type at
    // +0x18 and +0x24.  It must not share the special/HELL row type merely
    // because all three rows happen to be 0x1c bytes.
    struct stGenItemProb_t {
        int first_level;
        int last_level;
        int weight[5];
    };

    // IDA_EA: 0x853A6AE.  The C2 stores one in both words.  The following
    // normal-drop scale begins immediately after this exact eight-byte
    // subobject, at CMonsterDrop+0x259380.
    struct stFirstMobHuntingWeight_t {
        stFirstMobHuntingWeight_t();

        int first_boss_weight;      // +0x00
        int first_named_mob_weight; // +0x04
    };

    // The normal monster RDAR loader writes this 0x1c-byte suffix at
    // CMonsterDrop+0x259388.  It is consumed by _getBossGoldDrop @0x8535DC8.
    struct stBossGoldDrop_t {
        uint16_t condition_rate[4][2];
        uint16_t gold_quantity[2][2];
        uint16_t gold_volume[2];
    };

    // Vtable 0x8C9A748 has all four concrete slots below.
    explicit CMonsterDrop(int random_stream_count);
    virtual void setDropRatio(float ratio);
    virtual int loadScript(const char* filename);
    virtual void generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result);
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);
    int loadScript(const char* normal_filename,
                   const char* hell_filename,
                   const char* extra_filename);

    // These are named non-vtable helpers.
    int _getGenItemProbIdx(int standard_level);
    // IDA_EA: 0x8535DC8.  The first argument is the zero-based party-member
    // index, even though the outer random generator separately tests role 3.
    int _getBossGoldDrop(uint8_t party_member_index,
                         uint16_t& item_count,
                         uint16_t& gold_multiplier);
    // IDA_EA: 0x8535ED2.
    void _generateRandomItem(uint8_t character_level,
                             int8_t monster_role,
                             uint8_t dungeon_standard_level,
                             uint8_t dungeon_difficulty,
                             uint8_t party_member_count,
                             int bourgeois_rate,
                             int standard_drop_rate,
                             stGenerateResult_t& result,
        bool& generated_special_item);
    // IDA_EA: 0x8536FCE.  Preserve the source spelling "Monseter": it is
    // part of the original exported mangled name.
    void _generateMonseterSpecificItem(const CMonster* monster,
                                       char drop_kind,
                                       stGenerateResult_t& result);

protected:
    std::vector<stGenItemProb_t> probabilities_;        // +0x18
    std::vector<stGenItemProb_t> scaled_probabilities_; // +0x24
    int rarity_decision_[5][6];                         // +0x30
    float party_member_bonus_[5][4];                    // +0xa8
    float dungeon_difficulty_bonus_[5][5];              // +0xf8
    float monster_type_bonus_[5][4];                    // +0x15c
    unsigned char item_ref_range_[201][2];              // +0x1ac
    unsigned char item_ref_padding_[2];                  // +0x33e
    CItemGenRateTable item_rate_table_;                  // +0x340
    CMonsterDrop_Hell hell_drop_;                        // +0x238ac
    CSpecialMonsterDrop special_drop_;                   // +0x4706c
    stFirstMobHuntingWeight_t first_mob_hunting_weight_; // +0x259378
    int normal_drop_scale_;                              // +0x259380
    float normal_drop_scale_rate_;                       // +0x259384
    stBossGoldDrop_t boss_gold_drop_;                    // +0x259388
};

class CObjectDrop : public CMonsterDrop {
public:
    explicit CObjectDrop(int random_stream_count);
    virtual int loadScript(const char* filename);
    // Vtable 0x8C9A730 overrides the inherited random and specific slots.
    virtual void generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result);
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);
};

// IDA C2 @0x853A826 constructs exactly two 201-entry RDAR tables.  The
// one-argument virtual loader is deliberately a no-op; ItemGeneratorMgr uses
// the two-file loader so normal and war-area data fail independently.
class CWorldDrop : public CItemGenerator {
public:
    explicit CWorldDrop(int random_stream_count);
    virtual int loadScript(const char* filename);
    virtual void generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result);
    int loadScript(const char* normal_filename, const char* war_area_filename);

private:
    WorldDropInfo normal_drop_[201];
    WorldDropInfo war_area_drop_[201];
};

// IDA C2 @0x853ABC2 has no tail fields.  Its only derived virtual behavior
// is the weighted specific-item selection at slot +0x0c.
class CDungeonDrop : public CItemGenerator {
public:
    explicit CDungeonDrop(int random_stream_count);
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);
};

class CClearReward : public CItemGenerator {
public:
    struct stGenItemProb_t {
        int first_level;
        int last_level;
        int probability;
    };

    struct stGoldCardBlankItemInfo_t {
        // IDA_EA: 0x853F17E
        ~stGoldCardBlankItemInfo_t();
        int weight;
        int item_id;
        int add_info;
    };

    explicit CClearReward(int random_stream_count);
    virtual void setDropRatio(float ratio);
    virtual int loadScript(const char* filename);
    virtual void generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result);
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);

    // IDA_EA: 0x85385B2
    int getGoldCardCommission(int standard_level);
    int generatePremiumGoldCardItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result,
                                    int rarity,
                                    unsigned int fallback_item_id);
    int power_war_dungeon_gold_clear_reward(
        const stGenerateRefData_t& reference,
        stGenerateResult_t& result,
        int character_level);
    int generatePcRoomCardBlankItem(Inven_Item& item);
    int generateGoldCardBlankItem(Inven_Item& item);
    // IDA_EA: 0x85BF000.  The original returns the stored float through the
    // x87 long-double ABI and is deliberately non-const.
    long double getGoldCardCreateRate();
    int getRewardItemRatePerMapMaxCount(int map_index) const;

private:
    // IDA_EA: 0x8538694
    // IDA_NAME: _ZN8WongWork12CClearReward18_getGenItemProbIdxEiNS_22eClearRewardDropProb_tE
    int _getGenItemProbIdx(int character_level, int drop_probability);
    // IDA_EA: 0x85387C0 / 0x8538BF8.  These are named helper members, not
    // overloads of the vtable slots above.
    int _generateRandomItem(const stGenerateRefClearReward_t& reference,
                            int standard_drop_rate,
                            stGenerateResult_t& result,
                            float clear_reward_scale);
    int _generateSpecificItem(const stGenerateRefClearReward_t& reference,
                              stGenerateResult_t& result);

    std::vector<stGenItemProb_t> configured_probability_[6]; // +0x18
    std::vector<stGenItemProb_t> active_probability_[6];     // +0x60
    int drop_kind_probability_[2];                            // +0xa8
    int drop_kind_reserved_;                                  // +0xb0
    int drop_item_type_probability_[4];                       // +0xb4
    int dungeon_difficulty_drop_bonus_[5];                    // +0xc0
    float dungeon_difficulty_gold_bonus_[5];                  // +0xd4
    float party_member_drop_bonus_[4];                        // +0xe8
    int rarity_decision_[6];                                  // +0xf8
    unsigned char item_ref_range_[201][2];                    // +0x114
    unsigned char item_ref_padding_[2];
    CItemGenRateTable item_rate_table_;                        // +0x2a8
    int gold_card_commission_[201];                           // +0x23814
    float gold_card_create_rate_;                             // +0x23b38
    int gold_card_blank_total_weight_;                        // +0x23b3c
    std::vector<stGoldCardBlankItemInfo_t> gold_card_blank_items_;
    int reward_item_rate_per_map_max_count_[10];
    int pcroom_card_blank_total_weight_;
    std::vector<stGoldCardBlankItemInfo_t> pcroom_card_blank_items_;
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CMonsterDrop_Hell::stGenItemProb_t) == 0x1cu,
              "CMonsterDrop_Hell probability-row ABI");
static_assert(sizeof(CSpecialMonsterDrop::stGenItemProb_t) == 0x1cu,
              "CSpecialMonsterDrop probability-row ABI");
static_assert(sizeof(CMonsterDrop::stGenItemProb_t) == 0x1cu,
              "CMonsterDrop probability-row ABI");
static_assert(sizeof(CItemGenRateTable) == 0x2356cu,
              "CItemGenRateTable ABI");
static_assert(sizeof(CSecialIMonsterDropItemGenRateTable) == 0x211fccu,
              "CSecialIMonsterDropItemGenRateTable ABI");
static_assert(sizeof(CMonsterDrop_Hell) == 0x237c0u,
              "CMonsterDrop_Hell ABI");
static_assert(sizeof(CSpecialMonsterDrop) == 0x21230cu,
              "CSpecialMonsterDrop ABI");
static_assert(sizeof(CMonsterDrop::stFirstMobHuntingWeight_t) == 0x8u,
              "CMonsterDrop first-mob hunting ABI");
static_assert(offsetof(CMonsterDrop::stFirstMobHuntingWeight_t,
                       first_boss_weight) == 0x0u,
              "CMonsterDrop first-boss hunting weight ABI");
static_assert(offsetof(CMonsterDrop::stFirstMobHuntingWeight_t,
                       first_named_mob_weight) == 0x4u,
              "CMonsterDrop first-named hunting weight ABI");
static_assert(sizeof(CMonsterDrop::stBossGoldDrop_t) == 0x1cu,
              "CMonsterDrop boss-gold ABI");
static_assert(sizeof(CMonsterDrop) == 0x2593a4u, "CMonsterDrop ABI");
static_assert(sizeof(CObjectDrop) == 0x2593a4u, "CObjectDrop ABI");
static_assert(sizeof(CWorldDrop) == 0x1f80u, "CWorldDrop ABI");
static_assert(sizeof(CDungeonDrop) == 0x18u, "CDungeonDrop ABI");
static_assert(sizeof(CClearReward::stGenItemProb_t) == 0x0cu,
              "CClearReward probability row ABI");
static_assert(sizeof(CClearReward::stGoldCardBlankItemInfo_t) == 0x0cu,
              "CClearReward card row ABI");
static_assert(sizeof(CClearReward) == 0x23b84u, "CClearReward ABI");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_DROP_TYPES_H_
