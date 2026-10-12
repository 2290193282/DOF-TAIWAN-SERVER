#ifndef OLDSRV_GLOBAL_ITEM_H_
#define OLDSRV_GLOBAL_ITEM_H_

#include <stddef.h>
#include <stdint.h>

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "old_data/PVF/equipment/equipment_script.h"

enum ENUM_WORLD_TYPE : int;
enum ENUM_EXPERT_JOB_TYPE : int;
struct Inven_Item;
struct CItemVTable;

// IDA CItem::C2/D2 at 0x85152D4 / 0x85141EE construct and destroy the
// vector explicitly among other non-trivial CItem subobjects.  Retain that
// original lifetime order while giving the +0x8c/+0x90 increase-status pair
// a typed representation.
struct CItemIncreaseStatusData {
    int32_t type;
    std::vector<int> values;
};

// IDA unit: 99_misc_global/CItem. Public pointer ABI only; full item
// IDA_EA: get_index @0x8110C48, GetItemName @0x811ED82,
// getAdvanceAltarNeedGage @0x81347CA, getExpirationDate @0x80F1306.
// hierarchy and PVF population remain separate restoration work.
class CItem {
public:
    // The old object begins with its Itanium C++ vtable pointer.  Consumers
    // with proven virtual-slot evidence may invoke only named slots from the
    // typed CItemVTable below.
    const CItemVTable* vtable_;

    // IDA_EA: get_index @0x8110C48, get_grade @0x8110C54,
    // get_price @0x822C84A, GetCashPrice @0x822C856, and
    // GetMedalPrice @0x822C862.  These five scalar members are the
    // beginning of the true CItem layout; retaining their names prevents a
    // private-store caller from depending on an untyped ABI tail.
    int32_t item_index;       // +0x04
    int32_t grade;            // +0x08
    int32_t price;            // +0x0c
    int32_t cash_price;       // +0x10
    int32_t medal_price;      // +0x14

    int get_rarity() const;
    int getUsableLevel() const;
    // IDA_EA: 0x80F12E2
    // IDA_NAME: _ZNK5CItem13GetAttachTypeEv
    // DEMANGLED: CItem::GetAttachType(void)const
    int GetAttachType() const;
    // CLIENT_EA: CItem_GetRentalServiceType @0x9607D0.
    // The target importer accepts only categories 1..3; category 1 is
    // independently identified by 243 PC-room rental equipment samples and
    // by every confirmed client equality consumer.
    int GetRentalServiceType() const;
    int is_stackable() const;
    // IDA_EA: 0x822C86E
    // IDA_NAME: _ZNK5CItem10GetDyeInfoEv
    // DEMANGLED: CItem::GetDyeInfo(void)const
    // C2/D2 construct/destroy this exact DyeInfo subobject at +0x9c.
    DyeInfo* GetDyeInfo() const;
    // IDA_EA: 0x8150F18
    // IDA_NAME: _ZNK5CItem13IsRoutingItemEv
    // DEMANGLED: CItem::IsRoutingItem(void)const
    int IsRoutingItem() const;
    // IDA_EA: 0x846996A
    // IDA_NAME: _ZNK5CItem17isEpicRoutingItemEv
    // DEMANGLED: CItem::isEpicRoutingItem(void)const
    bool isEpicRoutingItem() const;
    // IDA_EA: 0x850D756
    // IDA_NAME: _ZNK5CItem18check_item_routingEv
    // DEMANGLED: CItem::check_item_routing(void)const
    bool check_item_routing() const;
    // IDA_EA: 0x85BEEC8
    // IDA_NAME: _ZN5CItem13IsPriorityJobEii
    // DEMANGLED: CItem::IsPriorityJob(int,int)
    int IsPriorityJob(int charac_job, int grow_type);
    int32_t get_index() const;
    // These four PVF drop-selection accessors are consumed by the B-owned
    // ItemGenerator rate tables.  Their exact bodies retain the original
    // item-script offsets and are provided with that data closure.
    int get_gen_rate() const;
    int isSpecialMonsterDropItem();
    int isDropItemDungeonDifficulty(int difficulty);
    int isDropItemDungeonType(int dungeon_type);
    // IDA_EA: 0x80F1312
    // IDA_NAME: _ZNK5CItem16getItemGroupNameEv
    // DEMANGLED: CItem::getItemGroupName(void)const
    int getItemGroupName() const;
    int get_grade() const;
    // IDA_EA: 0x8110C60
    // IDA_NAME: _ZNK5CItem15getUsablePeriodEv
    // DEMANGLED: CItem::getUsablePeriod(void)const
    int getUsablePeriod() const;
    const char* GetItemName() const;
    // IDA_EA: 0x85143EC
    // IDA_NAME: _ZNK5CItem15GetItemIconNameEv
    // DEMANGLED: CItem::GetItemIconName(void)const
    const char* GetItemIconName() const;
    int getExpirationDate() const;
    int getAdvanceAltarNeedGage() const;
    // IDA_EA: 0x8179FB4
    // IDA_NAME: _ZNK5CItem21isCerashopPackageItemEv
    // DEMANGLED: CItem::isCerashopPackageItem(void)const
    // The original invokes the +0x0c item-type virtual slot twice: it
    // short-circuits on type 14, then tests type 15.
    bool isCerashopPackageItem() const;
    int GetSellPrice() const;
    // IDA_EA: 0x828B5B4
    // IDA_NAME: _ZNK5CItem12isPackagableEv
    // DEMANGLED: CItem::isPackagable(void)const
    bool isPackagable() const;
    // IDA_EA: 0x822C84A
    // IDA_NAME: _ZNK5CItem9get_priceEv
    // DEMANGLED: CItem::get_price(void)const
    int get_price() const;
    // IDA_EA: 0x822C856
    // IDA_NAME: _ZNK5CItem12GetCashPriceEv
    // DEMANGLED: CItem::GetCashPrice(void)const
    int GetCashPrice() const;
    // IDA_EA: 0x822C862
    // IDA_NAME: _ZNK5CItem13GetMedalPriceEv
    // DEMANGLED: CItem::GetMedalPrice(void)const
    int GetMedalPrice() const;
    // IDA_EA: 0x850D6F4
    // IDA_NAME: _ZNK5CItem15GetNeedMaterialEv
    // DEMANGLED: CItem::GetNeedMaterial(void)const
    // The 0828 body first consults the manager override and then falls back
    // to this prototype's physical +0x24 PVF pair when no override exists.
    const std::pair<int, int>* GetNeedMaterial() const;
    // IDA_EA: 0x817EDEC
    // IDA_NAME: _ZNK5CItem14IsHiddenOptionEv
    // DEMANGLED: CItem::IsHiddenOption(void)const
    int IsHiddenOption() const;
    // IDA_EA: 0x822C83E
    // IDA_NAME: _ZNK5CItem19GetFinishPointPriceEv
    // DEMANGLED: CItem::GetFinishPointPrice(void)const
    int GetFinishPointPrice() const;
    // IDA_EA: 0x85F7BE0
    // IDA_NAME: _ZNK5CItem15IsBanRedeemItemEv
    // DEMANGLED: CItem::IsBanRedeemItem(void)const
    int IsBanRedeemItem() const;
    // IDA_EA: 0x84E95BC
    // IDA_NAME: _ZNK5CItem15check_low_levelEi
    // DEMANGLED: CItem::check_low_level(int)const
    bool check_low_level(int charac_level) const;
    // IDA_EA: 0x850D728
    // IDA_NAME: _ZNK5CItem14check_job_typeEc
    // DEMANGLED: CItem::check_job_type(char)const
    int check_job_type(char job) const;
    // IDA_EA: 0x84E95CE
    // IDA_NAME: _ZNK5CItem17isExpertJobUsableE20ENUM_EXPERT_JOB_TYPEi
    // DEMANGLED: CItem::isExpertJobUsable(ENUM_EXPERT_JOB_TYPE,int)const
    bool isExpertJobUsable(ENUM_EXPERT_JOB_TYPE expert_job_type,
                           int expert_job_level) const;
    // IDA_EA: 0x850D2C2
    // IDA_NAME: _ZNK5CItem35GetExpertJobCompoundResultVariationERN12STItemScript10SEXPERTJOB31stExpertCompoundResultVariationE
    // DEMANGLED: CItem::GetExpertJobCompoundResultVariation(STItemScript::SEXPERTJOB::stExpertCompoundResultVariation &)const
    int GetExpertJobCompoundResultVariation(
        STItemScript::SEXPERTJOB::stExpertCompoundResultVariation& value) const;
    // IDA_EA: 0x850D27A
    // IDA_NAME: _ZNK5CItem14get_need_skillEv
    // DEMANGLED: CItem::get_need_skill(void)const
    int get_need_skill() const;
    // IDA_EA: 0x850D286
    // IDA_NAME: _ZNK5CItem23getFootControlRateLimitEv
    // DEMANGLED: CItem::getFootControlRateLimit(void)const
    int getFootControlRateLimit() const;
    // IDA_EA: 0x8545FCC
    // IDA_NAME: _ZNK5CItem14GetUpSkillTypeEv
    // DEMANGLED: CItem::GetUpSkillType(void)const
    int GetUpSkillType() const;
    // IDA_EA: 0x8545FDA
    // IDA_NAME: _ZNK5CItem12GetNeedLevelEv
    // DEMANGLED: CItem::GetNeedLevel(void)const
    int GetNeedLevel() const;

    // IDA_EA: 0x822C87C
    // IDA_NAME: _ZNK5CItem13IsEnableWorldE15ENUM_WORLD_TYPE
    // DEMANGLED: CItem::IsEnableWorld(ENUM_WORLD_TYPE)const
    bool IsEnableWorld(ENUM_WORLD_TYPE world_type) const;
    // IDA_EA: 0x822C8EA
    // IDA_NAME: _ZNK5CItem13IsEnableWorldEPKS_
    // DEMANGLED: CItem::IsEnableWorld(CItem const*)const
    bool IsEnableWorld(const CItem* item) const;

    // IDA_EA: 0x850D292
    // IDA_NAME: _ZNK5CItem37GetExpertJobCompoundMaterialVariationEv
    // DEMANGLED: CItem::GetExpertJobCompoundMaterialVariation(void)const
    float GetExpertJobCompoundMaterialVariation() const;
    // IDA_EA: 0x850D2AA
    // IDA_NAME: _ZNK5CItem33GetExpertJobCompoundRateVariationEv
    // DEMANGLED: CItem::GetExpertJobCompoundRateVariation(void)const
    float GetExpertJobCompoundRateVariation() const;
    // IDA_EA: 0x850D2DE / 0x850D2F6.  The two scalar PVF fields immediately
    // follow the compound modifiers and drive portable disjoint bonuses.
    float GetExpertJobSelfDisjointBigWinRate() const;
    float GetExpertJobSelfDisjointResultVariation() const;
    // IDA_EA: 0x850D30E
    // IDA_NAME: _ZNK5CItem25GetExpertJobAdditionalExpERN12STItemScript10SEXPERTJOB24stExpertJobAdditionalExpE
    // DEMANGLED: CItem::GetExpertJobAdditionalExp(STItemScript::SEXPERTJOB::stExpertJobAdditionalExp &)const
    void GetExpertJobAdditionalExp(
        STItemScript::SEXPERTJOB::stExpertJobAdditionalExp& value) const;

    // These members cover the inherited tail of the 26-slot item vtable.
    // Their addresses and slot ordering are verified from df_game_r's
    // CItem/CEquipItem/CStackableItem vtables.
    int IsImpossibleUpgrade() const;
    int IsImpossibleAmplify() const;
    int IsImpossibleDisjoint() const;
    int IsImpossibleGift() const;
    int IsImpossibleCharacCargo() const;
    int IsPossibleKiriProtection() const;
    int PeriodAfterUnsealing() const;
    int GetQuestItemDropBouns() const;
    // 0828 PVF [event reward item] prototype classification.  The fixed
    // 0627 base layout is preserved; the implementation resolves the
    // immutable registry populated by CEquipItem/CStackableItem set_item.
    bool IsEventRewardItem() const;
    int IsRandomOption() const;
    // IDA_EA: 0x851441C, vptr +0x50 (slot 20).
    int GetRandomOptionGrade() const;
    // IDA_EA: 0x8514426, vptr +0x54 (slot 21).
    int IsRandomOptionUnseal() const;
    // IDA_EA: 0x85136AC, vptr +0x58 (slot 22).
    int isAmplifyPollutionItem() const;
    // IDA_EA: 0x8513884, vptr +0x5c (slot 23).
    int checkItemUpgradePossible() const;

public:
    // Physical 0627 CItem layout.  The old constructor @0x85152D4 proves
    // every non-trivial subobject below; accessors prove the scalar fields.
    // Unknown scalar meanings stay evidence-neutral instead of being hidden
    // behind this+offset arithmetic.
    int32_t script_values_18_[3];                  // +0x18
    std::pair<int, int> script_value_pair_;        // +0x24
    std::string item_name_;                        // +0x2c
    int32_t sell_price_;                           // +0x30
    int32_t attach_type_;                          // +0x34
    int32_t rarity_;                               // +0x38
    uint8_t usable_job_flags_[oldsrv::config::kCharacterJobCount];  // +0x3c
    int32_t usable_level_;                         // +0x48
    int32_t script_value_4c_;                      // +0x4c
    int32_t generation_rate_;                      // +0x50
    uint8_t stackable_;                            // +0x54
    uint8_t reserved_55_[3];                       // +0x55
    int32_t need_skill_;                           // +0x58
    int32_t foot_control_rate_limit_;              // +0x5c
    uint8_t packagable_;                           // +0x60
    uint8_t reserved_61_[3];                       // +0x61
    int32_t finish_point_price_;                   // +0x64
    int32_t advance_altar_need_gage_;              // +0x68
    int32_t usable_period_;                        // +0x6c
    int32_t expiration_date_;                      // +0x70
    int32_t item_group_name_;                      // +0x74
    uint8_t script_flag_78_;                       // +0x78
    uint8_t epic_routing_item_;                    // +0x79
    uint8_t reserved_7a_[2];                       // +0x7a
    int32_t user_power_war_point_;                 // +0x7c
    int32_t guild_power_war_point_;                // +0x80
    uint8_t impossible_upgrade_;                   // +0x84
    uint8_t impossible_amplify_;                   // +0x85
    uint8_t impossible_disjoint_;                  // +0x86
    uint8_t impossible_gift_;                      // +0x87
    uint8_t impossible_charac_cargo_;              // +0x88
    uint8_t possible_kiri_protection_;             // +0x89
    uint8_t reserved_8a_[2];                       // +0x8a
    CItemIncreaseStatusData increase_status_;      // +0x8c
    DyeInfo dye_info_;                             // +0x9c
    int32_t period_after_unsealing_;               // +0xac
    std::string item_icon_name_;                   // +0xb0
    STSpecialMonsterDrop special_monster_drop_;    // +0xb4
    uint8_t routing_item_;                         // +0xbd
    uint8_t reserved_be_[2];                       // +0xbe
    std::vector<std::pair<int, int> > priority_jobs_; // +0xc0
    int32_t quest_item_drop_bonus_;                // +0xcc
    uint8_t script_flag_d0_;                       // +0xd0
    uint8_t reserved_d1_[3];                       // +0xd1
    std::set<int> enabled_worlds_;                 // +0xd4
    int32_t usable_pvp_rank_;                      // +0xec
    int32_t up_skill_type_;                        // +0xf0
    int32_t need_level_;                           // +0xf4
    uint8_t hidden_option_;                        // +0xf8
    uint8_t reserved_f9_[3];                       // +0xf9
    float expert_job_material_variation_;          // +0xfc
    float expert_job_compound_rate_variation_;     // +0x100
    std::vector<STItemScript::SEXPERTJOB::SUSABLE>
        expert_job_usable_;                        // +0x104
    STItemScript::SEXPERTJOB::stExpertCompoundResultVariation
        expert_job_result_variation_;              // +0x110
    float expert_job_disjoint_big_win_rate_;       // +0x118
    float expert_job_disjoint_result_variation_;  // +0x11c
    STItemScript::SEXPERTJOB::stExpertJobAdditionalExp
        expert_job_additional_exp_;                // +0x120
    std::set<STItemScript::ENUM_ITEM_CATEGORY> categories_; // +0x128
    // Both old derived constructors begin their first owned subobject at
    // +0x148.  Their virtual parent-booster accessors and CItem's nonvirtual
    // IsBanRedeemItem prove these two fields are the common base tail.
    unsigned long parent_booster_item_id_;          // +0x140
    uint8_t ban_redeem_item_;                       // +0x144
    uint8_t rental_service_type_;                   // +0x145
    uint8_t reserved_146_[2];                       // +0x146

    CItem();
    ~CItem();
    CItem(const CItem&);
    CItem& operator=(const CItem&);

    // IDA_EA: 0x8694658
    // IDA_NAME: _ZNK5CItem24GetIncreaseStatusIntDataEiRi
    // DEMANGLED: CItem::GetIncreaseStatusIntData(int,int &)const
    int GetIncreaseStatusIntData(int index, int& value) const;
    // IDA_EA: 0x86946B6
    // IDA_NAME: _ZNK5CItem21GetIncreaseStatusTypeEv
    // DEMANGLED: CItem::GetIncreaseStatusType(void)const
    int GetIncreaseStatusType() const;
    // IDA_EA: 0x86946C4
    // IDA_NAME: _ZNK5CItem16GetUsablePvPRankEv
    // DEMANGLED: CItem::GetUsablePvPRank(void)const
    int GetUsablePvPRank() const;
    // IDA_EA: 0x822C97A
    // IDA_NAME: _ZNK5CItem11hasCategoryEN12STItemScript18ENUM_ITEM_CATEGORYE
    // CItem::C2/D2 own the queried set at +0x128; this member only exposes
    // the existing prototype state and never creates a second set.
    int hasCategory(STItemScript::ENUM_ITEM_CATEGORY category) const;
};

// IDA vtable view consumed by CExpandEquipslot::Move_Item.  The old call
// sites dispatch IsAvatarItem at slot 4, IsCreatureItem at slot 5, and
// PeriodAfterUnsealing at slot 18.
struct CItemVTable {
    typedef void (*Entry)(const CItem*);

    Entry destructor;
    Entry deleting_destructor;
    // IDA call sites such as CFairPvPScore::loadData @0x84A52EC dispatch
    // slot 2 as CItem::make_item(Inven_Item &).
    void (*make_item)(const CItem*, Inven_Item&);
    // IDA vtable +0x0c.  Item-lock eligibility distinguishes type 22
    // (equipment state) from type 11 (avatar attachment), so this must retain
    // its integer return rather than the generic void Entry signature.
    int (*get_item_type)(const CItem*);
    bool (*is_avatar_item)(const CItem*);
    bool (*is_creature_item)(const CItem*);
    Entry slots_6_through_9[4];
    int (*get_guild_power_war_point)(const CItem*);
    int (*get_user_power_war_point)(const CItem*);
    int (*is_impossible_upgrade)(const CItem*);
    int (*is_impossible_amplify)(const CItem*);
    int (*is_impossible_disjoint)(const CItem*);
    int (*is_impossible_gift)(const CItem*);
    int (*is_impossible_charac_cargo)(const CItem*);
    int (*is_possible_kiri_protection)(const CItem*);
    int (*period_after_unsealing)(const CItem*);
    int (*is_random_option)(const CItem*);
    int (*get_random_option_grade)(const CItem*);
    int (*is_random_option_unseal)(const CItem*);
    int (*is_amplify_pollution_item)(const CItem*);
    int (*check_item_upgrade_possible)(const CItem*);
    CItem* (*set_parent_booster_item_id)(CItem*, unsigned long);
    int (*get_parent_booster_item_id)(const CItem*);
};

static_assert(offsetof(CItemVTable, is_avatar_item) == 0x10u,
              "CItem vtable IsAvatarItem slot must stay 4");
static_assert(offsetof(CItemVTable, is_creature_item) == 0x14u,
              "CItem vtable IsCreatureItem slot must stay 5");
static_assert(offsetof(CItemVTable, get_guild_power_war_point) == 0x28u,
              "CItem vtable guild power-war cost must stay slot 10");
static_assert(offsetof(CItemVTable, get_user_power_war_point) == 0x2cu,
              "CItem vtable user power-war cost must stay slot 11");
static_assert(offsetof(CItemVTable, is_impossible_upgrade) == 0x30u,
              "CItem vtable IsImpossibleUpgrade slot must stay 12");
static_assert(offsetof(CItemVTable, is_impossible_amplify) == 0x34u,
              "CItem vtable IsImpossibleAmplify slot must stay 13");
static_assert(offsetof(CItemVTable, is_possible_kiri_protection) == 0x44u,
              "CItem vtable IsPossibleKiriProtection slot must stay 17");
static_assert(offsetof(CItemVTable, period_after_unsealing) == 0x48u,
              "CItem vtable PeriodAfterUnsealing slot must stay 18");
static_assert(offsetof(CItemVTable, is_random_option) == 0x4cu,
              "CItem vtable IsRandomOption slot must stay 19");
static_assert(offsetof(CItemVTable, get_random_option_grade) == 0x50u,
              "CItem vtable GetRandomOptionGrade slot must stay 20");
static_assert(offsetof(CItemVTable, is_random_option_unseal) == 0x54u,
              "CItem vtable IsRandomOptionUnseal slot must stay 21");
static_assert(offsetof(CItemVTable, is_amplify_pollution_item) == 0x58u,
              "CItem vtable isAmplifyPollutionItem slot must stay 22");
static_assert(offsetof(CItemVTable, check_item_upgrade_possible) == 0x5cu,
              "CItem vtable checkItemUpgradePossible slot must stay 23");
static_assert(offsetof(CItemVTable, set_parent_booster_item_id) == 0x60u,
              "CItem vtable parent-booster setter slot must stay 24");
static_assert(offsetof(CItemVTable, get_parent_booster_item_id) == 0x64u,
              "CItem vtable parent-booster getter slot must stay 25");
static_assert(sizeof(CItemIncreaseStatusData) == 0x10u,
              "CItem increase-status data must retain its i386 ABI");
static_assert(offsetof(CItemIncreaseStatusData, values) == 0x04u,
              "CItem increase-status values must follow type");
static_assert(offsetof(CItem, script_value_pair_) == 0x24u,
              "CItem script pair must remain at +0x24");
static_assert(offsetof(CItem, item_name_) == 0x2cu,
              "CItem name must remain at +0x2c");
static_assert(offsetof(CItem, usable_level_) == 0x48u,
              "CItem usable level must remain at +0x48");
static_assert(offsetof(CItem, generation_rate_) == 0x50u,
              "CItem generation rate must remain at +0x50");
static_assert(offsetof(CItem, increase_status_) == 0x8cu,
              "CItem increase-status data must remain at +0x8c");
static_assert(offsetof(CItem, dye_info_) == 0x9cu,
              "CItem dye information must remain at +0x9c");
static_assert(offsetof(CItem, item_icon_name_) == 0xb0u,
              "CItem icon name must remain at +0xb0");
static_assert(offsetof(CItem, enabled_worlds_) == 0xd4u,
              "CItem enabled worlds must remain at +0xd4");
static_assert(offsetof(CItem, rarity_) == 0x38u,
              "CItem rarity must remain at +0x38");
static_assert(offsetof(CItem, epic_routing_item_) == 0x79u,
              "CItem epic-routing flag must remain at +0x79");
static_assert(offsetof(CItem, usable_pvp_rank_) == 0xecu,
              "CItem PvP rank must remain at +0xec");
static_assert(offsetof(CItem, expert_job_usable_) == 0x104u,
              "CItem expert-job usable list must remain at +0x104");
static_assert(offsetof(CItem, expert_job_result_variation_) == 0x110u,
              "CItem expert result variation must remain at +0x110");
static_assert(offsetof(CItem, categories_) == 0x128u,
              "CItem category set must remain at +0x128");
static_assert(offsetof(CItem, parent_booster_item_id_) == 0x140u,
              "CItem parent booster id must remain at +0x140");
static_assert(offsetof(CItem, ban_redeem_item_) == 0x144u,
              "CItem redeem policy flag must remain at +0x144");
static_assert(offsetof(CItem, rental_service_type_) == 0x145u,
              "CItem rental-service type must remain at +0x145");
static_assert(sizeof(CItem) == 0x148u,
              "CItem ABI extent must stay 0x148 bytes");


#endif  // OLDSRV_GLOBAL_ITEM_H_
