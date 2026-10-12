#ifndef OLDSRV_OLD_GAME_INVENTORY_INVENTORY_H_
#define OLDSRV_OLD_GAME_INVENTORY_INVENTORY_H_

#include <stdint.h>
#include <stddef.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/manager/global_effect_manager_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/types/stackable_value_types.h"
#include "old_game/activity/schedule/lottery_info.h"
#include "old_game/activity/schedule/multi_box_lottery_info.h"
#include "old_game/economy/inventory/arad_item_info.h"
#include "old_game/economy/inventory/random_item_table.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"
#include "old_game/economy/inventory/recipe_info.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "infra/network/packet/PacketEnum.h"

namespace user_creature {
class CCreatureMgr;
}
namespace WongWork {
class CAvatarItemMgr;
}
namespace ItemUpgradeType {
enum T : int;
}
class CUser;
class CUserCharacInfo;
class DB_SaveAccountCargo;
struct STEquipmentScript;
struct STStackableScript;
struct stStackableBooster_t;
struct stStackableBoosterItemInfo_t;
struct stLimitCubeInfo;
struct WpBonusPointData;
struct ExpBonusPointData;
struct AvatarOptionChangeType;
struct AvatarCoin;
class CStackableItem;
struct CStackableItemLayoutVerifier;
class CMTRand;
enum ENUM_ITEMSPACE : int;
enum INVEN_TYPE : int;
namespace StackableAction {
enum Type : int;
enum UsablePlace : int;
enum UsableServerType : int;

// IDA runtime argument shared by StackableAction::Action and event handlers.
// The two leading dwords are the original action/request context; the live
// gameplay objects are read at +0x08 and +0x0C by Bingo, lottery and fatigue.
struct input_param {
    uint32_t action_context;
    uint32_t request_context;
    CUser* user;
    CStackableItem* stackable_item;
};
}

static_assert(sizeof(StackableAction::input_param) == 0x10u,
              "StackableAction::input_param IDA ABI size must stay 0x10");

// C-owned mail serialization projection.  This carries only the named
// fields consumed by SIG_MAILBOX_SEND_NEW_MAIL::Request; the conversion stays
// inside the canonical inventory owner so mail/gameplay callers do not
// reinterpret the 0x3d-byte Inven_Item ABI themselves.
struct InvenItemMailRecord {
    uint8_t seal_flag;
    uint32_t item_id;
    uint32_t add_info;
    uint8_t item_attr;
    uint16_t endurance;
    uint32_t extend_info;
    uint8_t item_guid[10];
    uint8_t amplify_option;
    uint16_t amplify_value;
    uint32_t chronicle_value;
    uint8_t random_option[14];
    uint8_t separate_upgrade;
    bool avatar_flag;
    bool creature_flag;
};

// IDA unit: Inven_Item / Inven_Item_Compatible.  This is the canonical
// inventory item ABI: all gameplay code that reads, saves, packets, trades,
// equips, or stores an item shares this one 0x3d-byte record layout.
struct __attribute__((packed)) Inven_Item {
    enum ITEM_TYPE {
        ITEM_TYPE_UNKNOWN = 0,
    };

    Inven_Item();
    // IDA_EA: 0x823D36C
    // IDA_NAME: _ZN10Inven_ItemD2Ev
    // DEMANGLED: Inven_Item::~Inven_Item()
    // The old ELF exports D1/D2 at the same address as STB_WEAK.  Keeping the
    // empty body visible to callers preserves the original inline lifetime:
    // ordinary by-value inventory calls do not emit an out-of-line D1 call.
    ~Inven_Item() __attribute__((used)) {}

    // IDA_EA: 0x80CB7D8
    // IDA_NAME: _ZN10Inven_Item5resetEv
    // DEMANGLED: Inven_Item::reset(void)
    Inven_Item* reset();
    Inven_Item* set_add_info(int add_info);

    Inven_Item* ResetItemAttr();
    Inven_Item* SetUpgrade(uint8_t upgrade);
    int GetUpgrade() const;
    int get_add_info() const;
    bool isEmpty() const;
    Inven_Item* SetTradeLimitCount(uint8_t count);
    char GetTradeLimitCount() const;
    Inven_Item* setCopy(const Inven_Item& rhs);
    int GetItemAttr() const;
    Inven_Item* SetItemAttr(uint8_t attr);
    bool isAvatarItemType() const;
    bool isEquipableItemType() const;
    char GetReSealCount() const;
    Inven_Item* SetReSealCount(uint8_t count);
    bool IsCreatureItemType() const;
    int GetInventoryType();
    int GetItemSpace() const;
    unsigned int getPackedCode1() const;
    int getKey() const;
    int IncUpgrade();

    // Non-IDA owner-local projection for the proven mail request fields.
    // It is deliberately not a second Inven_Item layout or an ABI alias.
    void ExportMailRecord(InvenItemMailRecord& record) const;

    // IDA offsets: reset @0x80CB7D8 writes the scalar prefix and calls the
    // named subobject reset functions.  C2 @0x80CB854 constructs the two
    // non-trivial tail members in declaration order before calling reset.
    uint8_t seal_or_state_;                      // +0x00
    uint8_t item_category_;                      // +0x01
    int32_t item_id_;                            // +0x02
    uint8_t item_attr_;                          // +0x06
    int32_t add_info_;                           // +0x07
    uint16_t avatar_select_;                     // +0x0b
    int32_t extend_info_;                        // +0x0d
    stAmplifyOption_t amplify_option_;           // +0x11
    itemGloballyUniqueIdentifier_t globally_unique_id_; // +0x15
    ChronicleRandomOptionInfo chronicle_random_option_; // +0x1f
    UpgradeSeparateInfo upgrade_separate_;       // +0x33
    ReservedCapacity reserved_capacity_;         // +0x34
};

// IDA 0x833F38E/0x833F418 use this packet-compatible 0x26-byte value to
// transfer only selected Inven_Item members.  Its compiler gaps are explicit
// so the object extent remains the original packed wire layout.
struct __attribute__((packed)) Inven_Item_Compatible {
    Inven_Item_Compatible* setCopy(const Inven_Item& item);
    Inven_Item* makeInvenItem(Inven_Item& item);

    uint8_t seal_or_state_;                 // IDA +0x00
    uint8_t item_category_;                 // IDA +0x01
    uint8_t padding_02_[2];                 // IDA +0x02
    int32_t item_id_;                       // IDA +0x04
    uint8_t item_attr_;                     // IDA +0x08
    uint8_t padding_09_[3];                 // IDA +0x09
    int32_t add_info_;                      // IDA +0x0c
    uint16_t avatar_select_;                // IDA +0x10
    uint8_t padding_12_[2];                 // IDA +0x12
    int32_t extend_info_;                   // IDA +0x14
    stAmplifyOption_t amplify_option_;      // IDA +0x18
    itemGloballyUniqueIdentifier_t globally_unique_id_; // IDA +0x1c
};

static_assert(sizeof(Inven_Item) == 0x3d,
               "Inven_Item IDA ABI size must stay 0x3d");
static_assert(offsetof(Inven_Item, seal_or_state_) == 0x00u,
              "Inven_Item state must retain IDA +0x00");
static_assert(offsetof(Inven_Item, item_category_) == 0x01u,
              "Inven_Item category must retain IDA +0x01");
static_assert(offsetof(Inven_Item, item_id_) == 0x02u,
              "Inven_Item item id must retain IDA +0x02");
static_assert(offsetof(Inven_Item, item_attr_) == 0x06u,
              "Inven_Item item attribute must retain IDA +0x06");
static_assert(offsetof(Inven_Item, add_info_) == 0x07u,
              "Inven_Item additional information must retain IDA +0x07");
static_assert(offsetof(Inven_Item, avatar_select_) == 0x0bu,
              "Inven_Item avatar selection must retain IDA +0x0b");
static_assert(offsetof(Inven_Item, extend_info_) == 0x0du,
              "Inven_Item extension value must retain IDA +0x0d");
static_assert(offsetof(Inven_Item, amplify_option_) == 0x11u,
              "Inven_Item amplify option must retain IDA +0x11");
static_assert(offsetof(Inven_Item, globally_unique_id_) == 0x15u,
              "Inven_Item GUID must retain IDA +0x15");
static_assert(offsetof(Inven_Item, chronicle_random_option_) == 0x1fu,
              "Inven_Item Chronicle random data must retain IDA +0x1f");
static_assert(offsetof(Inven_Item, upgrade_separate_) == 0x33u,
              "Inven_Item separate upgrade must retain IDA +0x33");
static_assert(offsetof(Inven_Item, reserved_capacity_) == 0x34u,
              "Inven_Item reserved capacity must retain IDA +0x34");
static_assert(sizeof(Inven_Item_Compatible) == 0x26,
              "Inven_Item_Compatible IDA ABI size must stay 0x26");
static_assert(offsetof(Inven_Item_Compatible, seal_or_state_) == 0x00u,
              "Inven_Item_Compatible state must retain IDA +0x00");
static_assert(offsetof(Inven_Item_Compatible, item_category_) == 0x01u,
              "Inven_Item_Compatible category must retain IDA +0x01");
static_assert(offsetof(Inven_Item_Compatible, item_id_) == 0x04u,
              "Inven_Item_Compatible item id must retain IDA +0x04");
static_assert(offsetof(Inven_Item_Compatible, item_attr_) == 0x08u,
              "Inven_Item_Compatible attribute must retain IDA +0x08");
static_assert(offsetof(Inven_Item_Compatible, add_info_) == 0x0cu,
              "Inven_Item_Compatible add info must retain IDA +0x0c");
static_assert(offsetof(Inven_Item_Compatible, avatar_select_) == 0x10u,
              "Inven_Item_Compatible selector must retain IDA +0x10");
static_assert(offsetof(Inven_Item_Compatible, extend_info_) == 0x14u,
              "Inven_Item_Compatible extension must retain IDA +0x14");
static_assert(offsetof(Inven_Item_Compatible, amplify_option_) == 0x18u,
              "Inven_Item_Compatible amplify data must retain IDA +0x18");
static_assert(offsetof(Inven_Item_Compatible, globally_unique_id_) == 0x1cu,
              "Inven_Item_Compatible GUID must retain IDA +0x1c");

// CCreatureMgr/CAvatarCoin are embedded only by the later CInventory layout.
// Deferring their headers until the complete 0x3d-byte Inven_Item declaration
// breaks the real inventory -> mail -> DB payload include cycle without
// changing either runtime layout.
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/avatar_coin.h"

// IDA_EA: 0x85002E0
// IDA_NAME: _Z14CheckEquipableRK10Inven_ItemS1_
// DEMANGLED: CheckEquipable(Inven_Item const&,Inven_Item const&)
bool CheckEquipable(const Inven_Item& source, const Inven_Item& target);

// IDA_EA: 0x84E91A4
// IDA_NAME: _Z18GetItemIdByAddInfoP10Inven_Itemii
// DEMANGLED: GetItemIdByAddInfo(Inven_Item *,int,int)
int GetItemIdByAddInfo(Inven_Item* items, int item_count, int add_info);

// IDA_EA: 0x85003BE
// IDA_NAME: _Z11IsEquipableP5CUserPK5CItemi
// DEMANGLED: IsEquipable(CUser *,CItem const*,int)
int IsEquipable(CUser* user, const CItem* item, int slot);


struct InventoryMemory {
    // IDA_EA: 0x822D3B8
    // IDA_NAME: _ZN15InventoryMemoryC2Ev
    // DEMANGLED: InventoryMemory::InventoryMemory(void)
    InventoryMemory();

    // IDA_EA: 0x822D472
    // IDA_NAME: _ZN15InventoryMemory5resetEv
    // DEMANGLED: InventoryMemory::reset(void)
    void* reset();

    // InventoryMemory::C2 @0x822D3B8 constructs these contiguous item ranges
    // in declaration order.  The main range uses the target client's five
    // 56-slot pages; the avatar, creature, and cargo capacities are unchanged.
    Inven_Item inventory_items[oldsrv::config::kMainInventoryItemCount];
    Inven_Item avatar_items[oldsrv::config::kAvatarInventoryItemCount];
    Inven_Item creature_items[oldsrv::config::kCreatureInventoryItemCount];
    Inven_Item cargo_items[oldsrv::config::kCargoInventoryItemCount];
};

static_assert(sizeof(Inven_Item) ==
                  oldsrv::config::kInventoryItemRecordSize,
              "inventory records must preserve the 0x3D wire/DB ABI");
static_assert(offsetof(InventoryMemory, inventory_items) == 0x0000u,
              "InventoryMemory inventory range must begin at +0x0000");
static_assert(offsetof(InventoryMemory, avatar_items) == 0x53e0u,
              "InventoryMemory avatar range must begin at +0x53e0");
static_assert(offsetof(InventoryMemory, creature_items) == 0x6ce5u,
              "InventoryMemory creature range must begin at +0x6ce5");
static_assert(offsetof(InventoryMemory, cargo_items) == 0xa68fu,
              "InventoryMemory cargo range must begin at +0xa68f");
static_assert(sizeof(InventoryMemory) == 0xcac7u,
              "InventoryMemory target ABI size must stay 0xcac7");
struct stAvatarEmblemInfo_t;
struct stAvatarExpansionInfo_t;
struct RecipeInfo;

// IDA_EA: 0x898C79D
// IDA_NAME: _Z21getLevelLinearAbilityiiiii
// DEMANGLED: getLevelLinearAbility(int,int,int,int,int)
int getLevelLinearAbility(int begin_level,
                          int end_level,
                          int begin_value,
                          int end_value,
                          int charac_level);

// IDA keeps this enum as a named CEquipItem parameter in the exported ABI.
enum ENUM_AVATAR_FUNC_FILTER : int;

// IDA unit: CEquipItem. CItemList::load_item_files allocates exactly 0x23c
// bytes before C2 @0x851547E.  The old set_item body writes the final dword at
// +0x238, so 0x238 was a member offset, never the object size.
class CEquipItem : public CItem {
public:
    // IDA_EA: 0x851547E / 0x8514B70 / 0x8514D08.
    CEquipItem();
    ~CEquipItem();

    // IDA_EA: 0x850FD84
    // IDA_NAME: _ZN10CEquipItem8set_itemER17STEquipmentScript
    CEquipItem* set_item(STEquipmentScript& script);

    // IDA_EA: 0x851098A
    // IDA_NAME: _ZNK10CEquipItem9make_itemER10Inven_Item
    int make_item(Inven_Item& item) const;

    // IDA_EA: 0x8151120
    // IDA_NAME: _ZNK10CEquipItem15getAvatarPeriodEh
    // DEMANGLED: CEquipItem::getAvatarPeriod(unsigned char)const
    int getAvatarPeriod(unsigned char avatar_select) const;
    // IDA_EA: 0x8510818
    // IDA_NAME: _ZN10CEquipItem6verifyEv
    // DEMANGLED: CEquipItem::verify(void)
    bool verify();
    // IDA_EA: 0x851353E
    // IDA_NAME: _ZNK10CEquipItem19IsAvatarOptionValidEc
    // DEMANGLED: CEquipItem::IsAvatarOptionValid(char)const
    bool IsAvatarOptionValid(char option) const;
    // IDA_EA: 0x851388E
    // IDA_NAME: _ZNK10CEquipItem24checkItemUpgradePossibleEv
    // DEMANGLED: CEquipItem::checkItemUpgradePossible(void)const
    bool checkItemUpgradePossible() const;
    // IDA_EA: 0x85136EC
    // IDA_NAME: _ZN10CEquipItem17isUpgradableLevelEN15ItemUpgradeType1TEi
    // DEMANGLED: CEquipItem::isUpgradableLevel(ItemUpgradeType::T,int)
    bool isUpgradableLevel(ItemUpgradeType::T upgrade_type,
                           int charac_level);
    // IDA_EA: 0x851378A
    // IDA_NAME: _ZN10CEquipItem25isExistUpgradableLevelTagEv
    // DEMANGLED: CEquipItem::isExistUpgradableLevelTag(void)
    bool isExistUpgradableLevelTag();
    // IDA_EA: 0x8513530
    // IDA_NAME: _ZNK10CEquipItem16getExpBonusPointEv
    // DEMANGLED: CEquipItem::getExpBonusPoint(void)const
    const ExpBonusPointData* getExpBonusPoint() const;
    // IDA_EA: 0x85137B8
    // IDA_NAME: _ZNK10CEquipItem14getGrowthGradeEi
    // DEMANGLED: CEquipItem::getGrowthGrade(int)const
    int getGrowthGrade(int charac_level) const;
    // IDA_EA: 0x851381C
    // IDA_NAME: _ZNK10CEquipItem19getGrowthRepairCostEi
    // DEMANGLED: CEquipItem::getGrowthRepairCost(int)const
    int getGrowthRepairCost(int charac_level) const;
    // IDA_EA: 0x8514DDE
    // IDA_NAME: _ZNK10CEquipItem17getUsableItemTypeEi
    // DEMANGLED: CEquipItem::getUsableItemType(int)const
    int getUsableItemType(int item_type) const;
    // IDA_EA: 0x854B528
    // IDA_NAME: _ZN10CEquipItem25GetEmancipateMaterialListEv
    // DEMANGLED: CEquipItem::GetEmancipateMaterialList(void)
    char* GetEmancipateMaterialList();
    // IDA_EA: 0x854B536
    // IDA_NAME: _ZN10CEquipItem23GetEmancipateResultListEv
    // DEMANGLED: CEquipItem::GetEmancipateResultList(void)
    char* GetEmancipateResultList();

    // IDA_EA: 0x8514E04
    // IDA_NAME: _ZNK10CEquipItem21GetGuildPowerWarPointEv
    // DEMANGLED: CEquipItem::GetGuildPowerWarPoint(void)const
    int GetGuildPowerWarPoint() const;
    // IDA_EA: 0x8514D26
    // IDA_NAME: _ZNK10CEquipItem11GetItemTypeEv
    // DEMANGLED: CEquipItem::GetItemType(void)const
    int GetItemType() const;
    // IDA_EA: 0x8514E6E
    // IDA_NAME: _ZNK10CEquipItem20GetRandomOptionGradeEv
    // DEMANGLED: CEquipItem::GetRandomOptionGrade(void)const
    int GetRandomOptionGrade() const;
    // IDA_EA: 0x832E036
    // IDA_NAME: _ZNK10CEquipItem22GetUsableEquipmentTypeEv
    // DEMANGLED: CEquipItem::GetUsableEquipmentType(void)const
    int GetUsableEquipmentType() const;
    // IDA_EA: 0x84E9700
    // IDA_NAME: _ZNK10CEquipItem17GetUsableMaxLevelEv
    // DEMANGLED: CEquipItem::GetUsableMaxLevel(void)const
    int GetUsableMaxLevel() const;
    // IDA_EA: 0x8514E12
    // IDA_NAME: _ZNK10CEquipItem20GetUserPowerWarPointEv
    // DEMANGLED: CEquipItem::GetUserPowerWarPoint(void)const
    int GetUserPowerWarPoint() const;
    // IDA_EA: 0x85136C6
    // IDA_NAME: _ZNK10CEquipItem22IsAvatarColorVariationEi
    // DEMANGLED: CEquipItem::IsAvatarColorVariation(int)const
    int IsAvatarColorVariation(int index) const;
    // IDA_EA: 0x8514D46
    // IDA_NAME: _ZNK10CEquipItem12IsAvatarItemEv
    // DEMANGLED: CEquipItem::IsAvatarItem(void)const
    int IsAvatarItem() const;
    // IDA_EA: 0x8514DA0
    // IDA_NAME: _ZNK10CEquipItem13IsClearAvatarEv
    // DEMANGLED: CEquipItem::IsClearAvatar(void)const
    int IsClearAvatar() const;
    // Tests the equipment prototype's valueless [full avatar] PVF flag.
    // This classification is distinct from [weapon full avatar].
    bool IsFullAvatar() const;
    // Tests the target-client [skin emblem socket] prototype capability.
    // This is static PVF state; the per-avatar socket/emblem records remain
    // owned by CAvatarItemMgr and user_items.jewel_socket.
    bool HasSkinEmblemSocket() const;
    // IDA_EA: 0x8514D72
    // IDA_NAME: _ZNK10CEquipItem14IsCreatureItemEv
    // DEMANGLED: CEquipItem::IsCreatureItem(void)const
    int IsCreatureItem() const;
    // IDA_EA: 0x822CA70
    // IDA_NAME: _ZN10CEquipItem12IsEmancipateEv
    // DEMANGLED: CEquipItem::IsEmancipate(void)
    int IsEmancipate();
    // IDA_EA: 0x8514DB0
    // IDA_NAME: _ZNK10CEquipItem13IsEnchantalbeEv
    // DEMANGLED: CEquipItem::IsEnchantalbe(void)const
    int IsEnchantalbe() const;
    // IDA_EA: 0x8514E2E
    // IDA_NAME: _ZNK10CEquipItem19IsImpossibleAmplifyEv
    // DEMANGLED: CEquipItem::IsImpossibleAmplify(void)const
    int IsImpossibleAmplify() const;
    // IDA_EA: 0x8514E3E
    // IDA_NAME: _ZNK10CEquipItem20IsImpossibleDisjointEv
    // DEMANGLED: CEquipItem::IsImpossibleDisjoint(void)const
    int IsImpossibleDisjoint() const;
    // IDA_EA: 0x8514E4E
    // IDA_NAME: _ZNK10CEquipItem16IsImpossibleGiftEv
    // DEMANGLED: CEquipItem::IsImpossibleGift(void)const
    int IsImpossibleGift() const;
    // IDA_EA: 0x8514E1E
    // IDA_NAME: _ZNK10CEquipItem19IsImpossibleUpgradeEv
    // DEMANGLED: CEquipItem::IsImpossibleUpgrade(void)const
    int IsImpossibleUpgrade() const;
    // IDA_EA: 0x8514E5E
    // IDA_NAME: _ZNK10CEquipItem14IsRandomOptionEv
    // DEMANGLED: CEquipItem::IsRandomOption(void)const
    int IsRandomOption() const;
    // IDA_EA: 0x85108FC
    // IDA_NAME: _ZN10CEquipItem19check_able_To_equipEcii
    // DEMANGLED: CEquipItem::check_able_To_equip(char,int,int)
    int check_able_To_equip(char slot, int charac_job, int charac_level);
    // IDA_EA: 0x85108C6
    // IDA_NAME: _ZNK10CEquipItem11check_eTypeEc
    // DEMANGLED: CEquipItem::check_eType(char)const
    int check_eType(char slot) const;
    // IDA_EA: 0x81803BC
    // IDA_NAME: _ZNK10CEquipItem19getAvatarOptionSizeEv
    // DEMANGLED: CEquipItem::getAvatarOptionSize(void)const
    int getAvatarOptionSize() const;
    // 0828 stores avatar abilities by an explicit sparse option key.  The
    // legacy ABI keeps a vector, so random callers select a valid ordinal and
    // translate it back to the real wire/DB key through this accessor.
    int getAvatarOptionKeyByOrdinal(int ordinal) const;
    // IDA_EA: 0x8150F36
    // IDA_NAME: _ZNK10CEquipItem15getAvatarSocketEiR20stAvatarEmblemInfo_t
    // DEMANGLED: CEquipItem::getAvatarSocket(int,stAvatarEmblemInfo_t &)const
    char getAvatarSocket(int avatar_select, stAvatarEmblemInfo_t& emblem) const;
    // IDA_EA: 0x822CA80
    // IDA_NAME: _ZNK10CEquipItem22getUpgradeCostDiscountEv
    // DEMANGLED: CEquipItem::getUpgradeCostDiscount(void)const
    float getUpgradeCostDiscount() const;
    // IDA_EA: 0x854B552
    // IDA_NAME: _ZNK10CEquipItem22getUpgradeProbIncreaseEv
    // DEMANGLED: CEquipItem::getUpgradeProbIncrease(void)const
    int getUpgradeProbIncrease() const;
    // IDA_EA: 0x8510C0C
    // IDA_NAME: _ZN10CEquipItem28get_aura_avatar_option_valueEv
    // DEMANGLED: CEquipItem::get_aura_avatar_option_value(void)
    int get_aura_avatar_option_value();
    // IDA_EA: 0x833EEE8
    // IDA_NAME: _ZNK10CEquipItem26get_creature_minimum_levelEv
    // DEMANGLED: CEquipItem::get_creature_minimum_level(void)const
    int get_creature_minimum_level() const;
    // IDA_EA: 0x833EEBE
    // IDA_NAME: _ZNK10CEquipItem20get_creature_speciesEv
    // DEMANGLED: CEquipItem::get_creature_species(void)const
    int get_creature_species() const;
    // IDA_EA: 0x833EEA6
    // IDA_NAME: _ZNK10CEquipItem21get_experience_amountEv
    // DEMANGLED: CEquipItem::get_experience_amount(void)const
    float get_experience_amount() const;
    // IDA_EA: 0x833EEDA
    // IDA_NAME: _ZNK10CEquipItem17get_hatch_item_idEv
    // DEMANGLED: CEquipItem::get_hatch_item_id(void)const
    int get_hatch_item_id() const;
    // IDA_EA: 0x8514E92
    // IDA_NAME: _ZNK10CEquipItem26get_parent_booster_item_idEv
    // DEMANGLED: CEquipItem::get_parent_booster_item_id(void)const
    int get_parent_booster_item_id() const;
    // IDA_EA: 0x833EECC
    // IDA_NAME: _ZNK10CEquipItem12get_sub_typeEv
    // DEMANGLED: CEquipItem::get_sub_type(void)const
    int get_sub_type() const;
    // IDA_EA: 0x85136B6
    // IDA_NAME: _ZNK10CEquipItem22isAmplifyPollutionItemEv
    // DEMANGLED: CEquipItem::isAmplifyPollutionItem(void)const
    int isAmplifyPollutionItem() const;
    // IDA_EA: 0x8514DE8
    // IDA_NAME: _ZNK10CEquipItem18isEnableAvatarFuncE23ENUM_AVATAR_FUNC_FILTER
    // DEMANGLED: CEquipItem::isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER)const
    bool isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER filter) const;
    // IDA_EA: 0x85137A8
    // IDA_NAME: _ZNK10CEquipItem12isGrowthTypeEv
    // DEMANGLED: CEquipItem::isGrowthType(void)const
    int isGrowthType() const;
    // IDA_EA: 0x8510BDC
    // IDA_NAME: _ZN10CEquipItem21is_aura_avatar_optionEc
    // DEMANGLED: CEquipItem::is_aura_avatar_option(char)
    int is_aura_avatar_option(char option);
    // IDA_EA: 0x8514D34
    // IDA_NAME: _ZN10CEquipItem19setAvatarOptionSizeEi
    // DEMANGLED: CEquipItem::setAvatarOptionSize(int)
    CEquipItem* setAvatarOptionSize(int size);
    // IDA_EA: 0x8514E80
    // IDA_NAME: _ZN10CEquipItem26set_parent_booster_item_idEm
    // DEMANGLED: CEquipItem::set_parent_booster_item_id(unsigned long)
    CEquipItem* set_parent_booster_item_id(unsigned long item_id);

    // IDA_EA: 0x811ED98
    // IDA_NAME: _ZNK10CEquipItem13get_enduranceEv
    // DEMANGLED: CEquipItem::get_endurance(void)const
    int get_endurance() const;

    // IDA_EA: 0x8150F28
    // IDA_NAME: _ZNK10CEquipItem19getAvatarTypeSelectEv
    // DEMANGLED: CEquipItem::getAvatarTypeSelect(void)const
    char* getAvatarTypeSelect() const;

    // IDA_EA: 0x8513522
    // IDA_NAME: _ZNK10CEquipItem15getWpBonusPointEv
    // DEMANGLED: CEquipItem::getWpBonusPoint(void)const
    const WpBonusPointData* getWpBonusPoint() const;

    // IDA_EA: 0x854B544
    // IDA_NAME: _ZNK10CEquipItem20getEnchantSystemDataEv
    // DEMANGLED: CEquipItem::getEnchantSystemData(void)const
    char* getEnchantSystemData() const;

public:
    // Physical 0627 CEquipItem suffix.  Non-trivial members are exactly the
    // subobjects constructed by C2 @0x851547E and destroyed by D2 @0x8514B70.
    RandomItemTable random_item_table_;           // +0x148
    float experience_amount_;                     // +0x158
    int32_t creature_species_;                     // +0x15c
    int32_t sub_type_;                             // +0x160
    int32_t hatch_item_id_;                        // +0x164
    int32_t creature_minimum_level_;               // +0x168
    uint8_t clear_avatar_;                         // +0x16c
    uint8_t reserved_16d_[3];                     // +0x16d
    int32_t maximum_endurance_;                    // +0x170
    std::vector<stAvatarTypeSelect_t> avatar_type_selects_; // +0x174
    std::vector<uint16_t> avatar_socket_types_;    // +0x180
    int32_t avatar_function_mask_;                 // +0x18c
    int32_t growth_repair_cost_;                   // +0x190
    uint8_t emancipate_;                           // +0x194
    uint8_t reserved_195_[3];                     // +0x195
    std::vector<std::pair<int, int> > emancipate_materials_; // +0x198
    std::vector<std::pair<int, int> > emancipate_results_;   // +0x1a4
    std::vector<STEnchantSystemData> enchant_system_data_;   // +0x1b0
    int32_t usable_equipment_type_;                // +0x1bc
    int32_t usable_max_level_;                     // +0x1c0
    int32_t avatar_option_size_;                   // +0x1c4
    uint8_t random_option_;                        // +0x1c8
    int8_t random_option_grade_;                   // +0x1c9
    uint8_t reserved_1ca_[2];                     // +0x1ca
    float upgrade_cost_discount_;                 // +0x1cc
    int32_t upgrade_probability_increase_;         // +0x1d0
    int32_t script_value_1d4_;                     // +0x1d4
    uint8_t aura_avatar_option_type_;              // +0x1d8
    uint8_t reserved_1d9_[3];                     // +0x1d9
    int32_t aura_avatar_option_value_;             // +0x1dc
    WpBonusPointData wp_bonus_point_;              // +0x1e0
    ExpBonusPointData exp_bonus_point_;            // +0x1e8
    std::vector<STAvatarAbilityVariation> avatar_ability_variations_; // +0x1f0
    uint8_t amplify_pollution_item_;               // +0x1fc
    uint8_t reserved_1fd_[3];                     // +0x1fd
    std::map<ItemUpgradeType::T, std::pair<int, int> >
        upgrade_level_limits_;                     // +0x200
    uint8_t avatar_color_variations_[2];           // +0x218
    uint8_t growth_type_;                          // +0x21a
    uint8_t reserved_21b_;                         // +0x21b
    int32_t growth_begin_level_;                   // +0x21c
    int32_t growth_end_level_;                     // +0x220
    int32_t growth_begin_grade_;                   // +0x224
    int32_t growth_end_grade_;                     // +0x228
    int32_t growth_begin_repair_cost_;             // +0x22c
    int32_t growth_end_repair_cost_;               // +0x230
    int32_t item_type_;                            // +0x234
    int32_t default_avatar_period_;                // +0x238
    uint8_t full_avatar_flag_;                      // +0x23c
    uint8_t skin_emblem_socket_enabled_;            // +0x23d
    uint8_t reserved_23e_[2];                      // +0x23e
};

static_assert(offsetof(CEquipItem, random_item_table_) == 0x148u,
              "CEquipItem random item table must stay at +0x148");
static_assert(offsetof(CEquipItem, avatar_type_selects_) == 0x174u,
              "CEquipItem avatar selections must stay at +0x174");
static_assert(offsetof(CEquipItem, emancipate_materials_) == 0x198u,
              "CEquipItem emancipate materials must stay at +0x198");
static_assert(offsetof(CEquipItem, enchant_system_data_) == 0x1b0u,
              "CEquipItem enchant data must stay at +0x1b0");
static_assert(offsetof(CEquipItem, wp_bonus_point_) == 0x1e0u,
              "CEquipItem WP bonus must stay at +0x1e0");
static_assert(offsetof(CEquipItem, upgrade_level_limits_) == 0x200u,
              "CEquipItem upgrade limits must stay at +0x200");
static_assert(offsetof(CEquipItem, maximum_endurance_) == 0x170u,
              "CEquipItem maximum endurance must stay at +0x170");
static_assert(offsetof(CEquipItem, item_type_) == 0x234u,
              "CEquipItem item type must stay at +0x234");
static_assert(offsetof(CEquipItem, default_avatar_period_) == 0x238u,
              "CEquipItem final avatar period must stay at +0x238");
static_assert(offsetof(CEquipItem, full_avatar_flag_) == 0x23cu,
              "CEquipItem full-avatar flag must stay at +0x23c");
static_assert(offsetof(CEquipItem, skin_emblem_socket_enabled_) == 0x23du,
              "CEquipItem skin-emblem flag must stay at +0x23d");
static_assert(sizeof(CEquipItem) == 0x240u,
              "CEquipItem size must include target avatar capability state");

// IDA shared inventory ABI enums. Call sites retain their original numeric
// values; the names are required for the original member symbol identities.
enum INVEN_TYPE : int {
    INVEN_TYPE_ABI_PLACEHOLDER = 0,
};

enum eItemDelReason {
    ITEM_DEL_REASON_ABI_PLACEHOLDER = 0,
};

// IDA keeps these as named enum parameters. Their enumerator tables belong to
// their individual gameplay call sites; only the ABI type is shared here.
enum eItemAddReason : int;
enum eMoneyAddReason : int;
enum eAvatarItemAddReason : int;
enum eCoinSubReason : int;

// IDA ABI: the original enumerator table is restored separately from the
// inventory state machine. The fixed underlying representation preserves the
// verified eMoneySubReason parameter ABI without inventing numeric meanings.
enum eMoneySubReason : int;

// IDA ABI enum identities used by CInventory::MakeEquipList.  Numeric policy
// remains with the USERINFO/equipment callers.
enum ENUM_USERINFO : int;
enum ENUM_EQUIPSLOT : int;

// IDA_EA: 0x850D159
// IDA_NAME: _Z18IsCreatureItemTypei
// DEMANGLED: IsCreatureItemType(int)
int IsCreatureItemType(int item_type);

// IDA_EA: 0x850D17C
// IDA_NAME: _Z19isEquipableItemTypei
// DEMANGLED: isEquipableItemType(int)
int isEquipableItemType(int item_type);

// IDA unit: CInventory.  The C2/D2/reset-proven prefix through +0x650 is
// physical below; the unclosed tail remains intentionally outside this slice.
class CInventory {
    // CUser::SetCharacInfo @0x8649C6A writes the owned equipment and avatar
    // visibility subobjects directly in the original source relationship.
    // Friendship exposes names only and adds no storage or ABI change.
    friend class CUser;

public:
    struct AvatarVisibleData {
        // IDA_EA: 0x850D348
        // IDA_NAME: _ZN10CInventory17AvatarVisibleData4initEv
        // DEMANGLED: CInventory::AvatarVisibleData::init(void)
        AvatarVisibleData* init();

        uint32_t item_id;
        uint16_t avatar_expansion_0;
        uint16_t avatar_expansion_1;
    };
    static_assert(sizeof(AvatarVisibleData) == 0x08u,
                  "AvatarVisibleData IDA ABI size must stay 0x08");

    // IDA_EA: 0x822D52C
    // IDA_NAME: _ZN10CInventoryC2Ev
    // DEMANGLED: CInventory::CInventory(void)
    CInventory();
    // IDA_EA: 0x8699476
    // IDA_NAME: _ZN10CInventoryC2ERKS_
    // DEMANGLED: CInventory::CInventory(CInventory const&)
    CInventory(const CInventory& other);
    // IDA_EA: 0x822D600
    // IDA_NAME: _ZN10CInventoryD2Ev
    // DEMANGLED: CInventory::~CInventory(void)
    ~CInventory();
    // IDA_EA: 0x869D73C
    // IDA_NAME: _ZN10CInventoryaSERKS_
    // DEMANGLED: CInventory::operator=(CInventory const&)
    CInventory& operator=(const CInventory& other);
    // IDA_EA: 0x84FA314
    // IDA_NAME: _ZN10CInventory7setCopyERKS_
    // DEMANGLED: CInventory::setCopy(CInventory const&)
    CInventory* setCopy(const CInventory& source);

    // IDA_EA: 0x84FA91C
    // IDA_NAME: _ZN10CInventory11getHashCodeEPh
    // DEMANGLED: CInventory::getHashCode(unsigned char *)
    int getHashCode(unsigned char* output);

    // IDA_EA: 0x8509A06
    // IDA_NAME: _ZN10CInventory5resetEv
    // DEMANGLED: CInventory::reset(void)
    bool reset();

    // IDA_EA: 0x84FE2E2
    // IDA_NAME: _ZNK10CInventory13MakeEquipListEPvb13ENUM_USERINFO14ENUM_EQUIPSLOT
    // DEMANGLED: CInventory::MakeEquipList(void *,bool,ENUM_USERINFO,ENUM_EQUIPSLOT)const
    int MakeEquipList(void* packet,
                      bool full,
                      ENUM_USERINFO userinfo,
                      ENUM_EQUIPSLOT equip_slot) const;

    // Returns the Equipment.lst item id consumed by the minimum USERINFO
    // appearance reader. Skin avatars occupy worn slot 8.
    uint32_t GetFullAvatarEquipmentItemId() const;

    // IDA_EA: 0x84FAC70
    // IDA_NAME: _ZNK10CInventory12FindItemLockEhR14ENUM_ITEMSPACERi
    // DEMANGLED: CInventory::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
    int FindItemLock(unsigned char lock,
                     ENUM_ITEMSPACE& item_space,
                     int& slot) const;

    // IDA_EA: 0x850D374
    // IDA_NAME: _ZNK10CInventory14GetClearAvatarEi
    // DEMANGLED: CInventory::GetClearAvatar(int)const
    int GetClearAvatar(int slot) const;

    // IDA_EA: 0x869498A
    // IDA_NAME: _ZNK10CInventory14GetClearAvatarEPm
    // DEMANGLED: CInventory::GetClearAvatar(unsigned long *)const
    bool GetClearAvatar(unsigned long* clear_avatar) const;

    // IDA_EA: 0x850CE40
    // IDA_NAME: _ZN10CInventory19checkEquipmentStateEiiRiS0_S0_S0_S0_S0_
    // DEMANGLED: CInventory::checkEquipmentState(int,int,int &,int &,int &,int &,int &,int &)
    void checkEquipmentState(int charac_job,
                             int first_grow_type,
                             int& empty_equipment_count,
                             int& mastery_mismatch_count,
                             int& mastery_mismatch_state,
                             int& unselected_avatar_count,
                             int& creature_artifact_count,
                             int& mastery_equipment_count);

    // IDA_EA: 0x8509B9E
    // IDA_NAME: _ZN10CInventory13AddAvatarItemEiibccPKc20eAvatarItemAddReasonbt
    // DEMANGLED: CInventory::AddAvatarItem(int,int,bool,char,char,char const*,eAvatarItemAddReason,bool,unsigned short)
    int AddAvatarItem(int item_id,
                      int usable_period,
                      bool flag,
                      char first_flag,
                      char avatar_select,
                      const char* ipg_no,
                      eAvatarItemAddReason reason,
                      bool account_bound,
                      uint16_t hidden_option);

    // IDA_EA: 0x8509E78
    // IDA_NAME: _ZN10CInventory15_SaveAvatarItemEiibcRK20stAvatarEmblemInfo_tPKcmRK23stAvatarExpansionInfo_t
    // DEMANGLED: CInventory::_SaveAvatarItem(int,int,bool,char,stAvatarEmblemInfo_t const&,char const*,unsigned long,stAvatarExpansionInfo_t const&)
    int _SaveAvatarItem(int slot,
                        int expire_date,
                        bool flag,
                        char first_flag,
                        const stAvatarEmblemInfo_t& emblem,
                        const char* ipg_no,
                        unsigned long add_info,
                        const stAvatarExpansionInfo_t& expansion);

    // IDA_EA: 0x850367C
    // IDA_NAME: _ZN10CInventory25insertAvatarIntoInventoryE10Inven_Item20eAvatarItemAddReasonb
    // DEMANGLED: CInventory::insertAvatarIntoInventory(Inven_Item,eAvatarItemAddReason,bool)
    int insertAvatarIntoInventory(Inven_Item item,
                                  eAvatarItemAddReason reason,
                                  bool log_history);

    // IDA_EA: 0x85037D8
    // IDA_NAME: _ZN10CInventory28insertAvatarIntoSpecificSlotE10Inven_Itemi20eAvatarItemAddReasonb
    // DEMANGLED: CInventory::insertAvatarIntoSpecificSlot(Inven_Item,int,eAvatarItemAddReason,bool)
    int insertAvatarIntoSpecificSlot(Inven_Item item,
                                    int slot,
                                    eAvatarItemAddReason reason,
                                    bool log_history);

    // IDA_EA: 0x85039E8
    // IDA_NAME: _ZN10CInventory24insert_item_special_slotE10Inven_Itemii15eMoneyAddReason14eItemAddReason
    // DEMANGLED: CInventory::insert_item_special_slot(Inven_Item,int,int,eMoneyAddReason,eItemAddReason)
    int insert_item_special_slot(Inven_Item item,
                                 int item_count,
                                 int original_slot,
                                 eMoneyAddReason money_reason,
                                 eItemAddReason item_reason);

    // IDA_EA: 0x84FB01E
    // IDA_NAME: _ZNK10CInventory14CheckValidSlotEii
    // DEMANGLED: CInventory::CheckValidSlot(int,int)const
    int CheckValidSlot(int inventory_type, int slot) const;

    // IDA_EA: 0x84FB11A
    // IDA_NAME: _ZN10CInventory12SetInvenDataEiPvi
    // DEMANGLED: CInventory::SetInvenData(int,void *,int)
    void* SetInvenData(int inventory_type, void* source, int bytes);

    // IDA_EA: 0x84FB222
    // IDA_NAME: _ZNK10CInventory12GetInvenDataEiPvi
    // DEMANGLED: CInventory::GetInvenData(int,void *,int)const
    void* GetInvenData(int inventory_type, void* destination, int bytes) const;

    // IDA_EA: 0x84FC1DE
    // IDA_NAME: _ZNK10CInventory11GetInvenRefEii
    // DEMANGLED: CInventory::GetInvenRef(int,int)const
    const Inven_Item* GetInvenRef(int inventory_type, int slot) const;

    // IDA_EA: 0x84FC2AE
    // IDA_NAME: _ZN10CInventory11GetInvenRefEii
    // DEMANGLED: CInventory::GetInvenRef(int,int)
    Inven_Item* GetInvenRef(int inventory_type, int slot);

    // IDA_EA: 0x84FC37E
    // IDA_NAME: _ZN10CInventory11ChangeEquipE10INVEN_TYPEii
    // DEMANGLED: CInventory::ChangeEquip(INVEN_TYPE,int,int)
    // This is the local equipment transition used by CMD19 before the
    // higher-level CUser inter-space transaction publishes its response.
    int ChangeEquip(INVEN_TYPE inventory_type,
                    int source_slot,
                    int equipment_slot);

    // IDA_EA: 0x8500688
    // IDA_NAME: _ZN10CInventory9move_itemE10INVEN_TYPEiS0_i
    // DEMANGLED: CInventory::move_item(INVEN_TYPE,int,INVEN_TYPE,int)
    int move_item(INVEN_TYPE source_type,
                  int source_slot,
                  INVEN_TYPE destination_type,
                  int destination_slot);

    // IDA_EA: 0x84FB918
    // IDA_NAME: _ZNK10CInventory12GetInvenSlotEii
    // DEMANGLED: CInventory::GetInvenSlot(int,int)const
    Inven_Item GetInvenSlot(int inventory_type, int slot) const;

    // IDA_EA: 0x84FB824
    // IDA_NAME: _ZNK10CInventory14get_empty_slotE10INVEN_TYPEN10Inven_Item9ITEM_TYPEE
    // DEMANGLED: CInventory::get_empty_slot(INVEN_TYPE,Inven_Item::ITEM_TYPE)const
    int get_empty_slot(INVEN_TYPE inventory_type,
                       Inven_Item::ITEM_TYPE item_type) const;

    // IDA_EA: 0x822D67C
    // IDA_NAME: _ZN10CInventory7SetCoinEj
    // DEMANGLED: CInventory::SetCoin(unsigned int)
    CInventory* SetCoin(unsigned int coin);

    // IDA_EA: 0x8110C7A
    // IDA_NAME: _ZNK10CInventory12GetEventCoinEv
    // DEMANGLED: CInventory::GetEventCoin(void)const
    int GetEventCoin() const;

    // IDA_EA: 0x8110C86
    // IDA_NAME: _ZN10CInventory12SetEventCoinEj
    // DEMANGLED: CInventory::SetEventCoin(unsigned int)
    CInventory* SetEventCoin(unsigned int coin);

    // IDA_EA: 0x81AC72E
    // IDA_NAME: _ZNK10CInventory10GetPayCoinEv
    // DEMANGLED: CInventory::GetPayCoin(void)const
    int GetPayCoin() const;

    // IDA_EA: 0x81AC73A
    // IDA_NAME: _ZN10CInventory10SetPayCoinEj
    // DEMANGLED: CInventory::SetPayCoin(unsigned int)
    CInventory* SetPayCoin(unsigned int coin);

    // IDA_EA: 0x822D652
    // IDA_NAME: _ZN10CInventory9set_moneyEj
    // DEMANGLED: CInventory::set_money(unsigned int)
    CInventory* set_money(unsigned int money);

    // IDA_EA: 0x822D660
    // IDA_NAME: _ZNK10CInventory12getTotalCoinEv
    // DEMANGLED: CInventory::getTotalCoin(void)const
    int getTotalCoin() const;

    // IDA_EA: 0x822D696
    // IDA_NAME: _ZNK10CInventory12IsCoinUsableEv
    // DEMANGLED: CInventory::IsCoinUsable(void)const
    bool IsCoinUsable() const;

    // IDA_EA: 0x84FA9D4
    // IDA_NAME: _ZN10CInventory9UseUpCoinEj
    // DEMANGLED: CInventory::UseUpCoin(unsigned int)
    int UseUpCoin(unsigned int coin);

    // IDA_EA: 0x84FAA98
    // IDA_NAME: _ZN10CInventory7UseCoinE14eCoinSubReason
    // DEMANGLED: CInventory::UseCoin(eCoinSubReason)
    int UseCoin(eCoinSubReason reason);

    // IDA_EA: 0x84FA4AC
    // IDA_NAME: _ZN10CInventory19getPackedCodeArray1EPmi
    // DEMANGLED: CInventory::getPackedCodeArray1(unsigned long *,int)
    int getPackedCodeArray1(unsigned long* codes, int capacity);
    // IDA_EA: 0x84FA68A
    // IDA_NAME: _ZN10CInventory17getSortedKeyArrayEPmi
    // DEMANGLED: CInventory::getSortedKeyArray(unsigned long *,int)
    int getSortedKeyArray(unsigned long* keys, int capacity);
    // IDA_EA: 0x84FAE0A
    // IDA_NAME: _ZNK10CInventory16SendItemLockListEPK10Inven_Itemi14ENUM_ITEMSPACE
    // DEMANGLED: CInventory::SendItemLockList(Inven_Item const*,int,ENUM_ITEMSPACE)const
    void SendItemLockList(const Inven_Item* items,
                          int item_count,
                          ENUM_ITEMSPACE item_space) const;
    // IDA_EA: 0x84FAF8E
    // IDA_NAME: _ZNK10CInventory21SendItemLockListInvenEv
    // DEMANGLED: CInventory::SendItemLockListInven(void)const
    void SendItemLockListInven() const;
    // IDA_EA: 0x84FAFBE
    // IDA_NAME: _ZNK10CInventory22SendItemLockListAvatarEv
    // DEMANGLED: CInventory::SendItemLockListAvatar(void)const
    void SendItemLockListAvatar() const;
    // IDA_EA: 0x84FAFEE
    // IDA_NAME: _ZNK10CInventory24SendItemLockListCreatureEv
    // DEMANGLED: CInventory::SendItemLockListCreature(void)const
    void SendItemLockListCreature() const;
    // IDA_EA: 0x84FB32A
    // IDA_NAME: _ZN10CInventory9ResetSlotEii
    // DEMANGLED: CInventory::ResetSlot(int,int)
    Inven_Item* ResetSlot(int inventory_type, int slot);
    // IDA_EA: 0x84FB3E4
    // IDA_NAME: _ZNK10CInventory14get_item_countEi
    // DEMANGLED: CInventory::get_item_count(int)const
    int get_item_count(int inventory_type) const;
    // IDA_EA: 0x84FB500
    // IDA_NAME: _ZNK10CInventory14get_item_countEim
    // DEMANGLED: CInventory::get_item_count(int,unsigned long)const
    int get_item_count(int inventory_type, unsigned long item_id) const;
    // IDA_EA: 0x84FBC5A
    // IDA_NAME: _ZN10CInventory17GetInvenSlotByRefEiiR10Inven_Item
    // DEMANGLED: CInventory::GetInvenSlotByRef(int,int,Inven_Item &)
    int GetInvenSlotByRef(int inventory_type, int slot, Inven_Item& item);
    // IDA_EA: 0x84FF8B4
    // IDA_NAME: _ZN10CInventory15UpdateDailyCoinEi
    // DEMANGLED: CInventory::UpdateDailyCoin(int)
    CUserCharacInfo* UpdateDailyCoin(int daily_coin);
    // IDA_EA: 0x85049C8
    // IDA_NAME: _ZN10CInventory30MoveItemToEmptySlotOfInventoryEiii
    // DEMANGLED: CInventory::MoveItemToEmptySlotOfInventory(int,int,int)
    int MoveItemToEmptySlotOfInventory(int source_slot,
                                       int begin_slot,
                                       int end_slot);
    // IDA_EA: 0x8504A8A
    // IDA_NAME: _ZN10CInventory13SortInvenSlotEN10Inven_Item9ITEM_TYPEE
    // DEMANGLED: CInventory::SortInvenSlot(Inven_Item::ITEM_TYPE)
    int SortInvenSlot(Inven_Item::ITEM_TYPE item_type);

    // IDA_EA: 0x8504D80
    // IDA_NAME: _ZN10CInventory4sortE10INVEN_TYPE
    // DEMANGLED: CInventory::sort(INVEN_TYPE)
    int sort(INVEN_TYPE inventory_type);
    // IDA_EA: 0x8504F24
    // IDA_NAME: _ZNK10CInventory16check_slot_emptyEi
    // DEMANGLED: CInventory::check_slot_empty(int)const
    bool check_slot_empty(int slot) const;
    // IDA_EA: 0x850562A
    // IDA_NAME: _ZNK10CInventory18possible_to_insertEi
    // DEMANGLED: CInventory::possible_to_insert(int)const
    bool possible_to_insert(int item_id) const;

    // IDA_EA: 0x85056CA
    // IDA_NAME: _ZN10CInventory19erase_repeated_itemERKSt6vectorISt4pairIiiESaIS2_EERS4_
    // DEMANGLED: CInventory::erase_repeated_item(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
    int erase_repeated_item(const std::vector<std::pair<int, int> >& requested,
                            std::vector<std::pair<int, int> >& remaining);

    // IDA_EA: 0x85058DA
    // IDA_NAME: _ZN10CInventory19tryDeleteEventItemsERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CInventory::tryDeleteEventItems(std::vector<std::pair<int,int> > const&)
    int tryDeleteEventItems(
        const std::vector<std::pair<int, int> >& items);

    // IDA_EA: 0x850675E
    // IDA_NAME: _ZN10CInventory19tryInsertEventItemsERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CInventory::tryInsertEventItems(std::vector<std::pair<int,int> > const&)
    int tryInsertEventItems(
        const std::vector<std::pair<int, int> >& items);

    // IDA_EA: 0x8505DB4
    // IDA_NAME: _ZN10CInventory18delete_event_itemsERKSt6vectorISt4pairIiiESaIS2_EERS0_IiSaIiEERb15eMoneySubReason14eItemDelReasonb
    // DEMANGLED: CInventory::delete_event_items(std::vector<std::pair<int,int> > const&,std::vector<int>&,bool&,eMoneySubReason,eItemDelReason,bool)
    int delete_event_items(
        const std::vector<std::pair<int, int> >& items,
        std::vector<int>& deleted_slots,
        bool& equipment_changed,
        eMoneySubReason money_reason,
        eItemDelReason item_reason,
        bool cube_failure);

    // IDA_EA: 0x8507ACC
    // IDA_NAME: _ZNK10CInventory17CheckNeedItemListERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CInventory::CheckNeedItemList(std::vector<std::pair<int,int> > const&)const
    int CheckNeedItemList(
        const std::vector<std::pair<int, int> >& items) const;

    // IDA_EA: 0x8507F1A
    // IDA_NAME: _ZNK10CInventory27CheckEquipmentItemConditionERKSt6vectorISt4pairIiiESaIS2_EER16stEquipConditionR10Inven_Item
    // DEMANGLED: CInventory::CheckEquipmentItemCondition(std::vector<std::pair<int,int> > const&,stEquipCondition&,Inven_Item&)const
    int CheckEquipmentItemCondition(
        const std::vector<std::pair<int, int> >& items,
        stEquipCondition& condition,
        Inven_Item& material) const;
    // IDA_EA: 0x85083D0
    // IDA_NAME: _ZNK10CInventory27CheckEquipmentItemConditionE10INVEN_TYPEsR28stEquipConditionForChronicle
    // DEMANGLED: CInventory::CheckEquipmentItemCondition(INVEN_TYPE,short,stEquipConditionForChronicle &)const
    int CheckEquipmentItemCondition(
        INVEN_TYPE inventory_type,
        int16_t slot,
        stEquipConditionForChronicle& condition) const;

    // IDA_EA: 0x850C8A8
    // IDA_NAME: _ZNK10CInventory37GetExpertJobCompoundMaterialVariationE20ENUM_EXPERT_JOB_TYPEi
    // DEMANGLED: CInventory::GetExpertJobCompoundMaterialVariation(ENUM_EXPERT_JOB_TYPE,int)const
    float GetExpertJobCompoundMaterialVariation(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level) const;
    // IDA_EA: 0x850C95A
    // IDA_NAME: _ZNK10CInventory33GetExpertJobCompoundRateVariationE20ENUM_EXPERT_JOB_TYPEi
    // DEMANGLED: CInventory::GetExpertJobCompoundRateVariation(ENUM_EXPERT_JOB_TYPE,int)const
    float GetExpertJobCompoundRateVariation(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level) const;
    // IDA_EA: 0x850CA0C
    // IDA_NAME: _ZNK10CInventory35GetExpertJobCompoundResultVariationE20ENUM_EXPERT_JOB_TYPEiRN12STItemScript10SEXPERTJOB31stExpertCompoundResultVariationE
    // DEMANGLED: CInventory::GetExpertJobCompoundResultVariation(ENUM_EXPERT_JOB_TYPE,int,STItemScript::SEXPERTJOB::stExpertCompoundResultVariation&)const
    void GetExpertJobCompoundResultVariation(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level,
        STItemScript::SEXPERTJOB::stExpertCompoundResultVariation& value) const;
    // IDA_EA: 0x850CAE2 / 0x850CB94.  These sum expert-job equipment
    // modifiers across slots 10..21 for a portable disjoint machine owner.
    float GetExpertJobSelfDisjointBigWinRate(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level) const;
    float GetExpertJobSelfDisjointResultVariation(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level) const;
    // IDA_EA: 0x850CC46
    // IDA_NAME: _ZNK10CInventory24GetExpertJobAddtionalExpE20ENUM_EXPERT_JOB_TYPEiRN12STItemScript10SEXPERTJOB24stExpertJobAdditionalExpE
    // DEMANGLED: CInventory::GetExpertJobAddtionalExp(ENUM_EXPERT_JOB_TYPE,int,STItemScript::SEXPERTJOB::stExpertJobAdditionalExp &)const
    void GetExpertJobAddtionalExp(
        ENUM_EXPERT_JOB_TYPE expert_job_type,
        int expert_job_level,
        STItemScript::SEXPERTJOB::stExpertJobAdditionalExp& value) const;

    // IDA_EA: 0x8508536
    // IDA_NAME: _ZNK10CInventory12CalBlankSlotEPiRKSt6vectorISt4pairIiiESaIS3_EE
    // DEMANGLED: CInventory::CalBlankSlot(int *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)const
    bool CalBlankSlot(int* blank_slots,
                      const std::vector<std::pair<int, int> >& requested) const;

    // IDA_EA: 0x850891A
    // IDA_NAME: _ZNK10CInventory11CalNeedSlotEPiRKSt6vectorISt4pairIiiESaIS3_EE
    // DEMANGLED: CInventory::CalNeedSlot(int *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)const
    bool CalNeedSlot(int* needed_slots,
                     const std::vector<std::pair<int, int> >& requested) const;
    // IDA_EA: 0x8509880
    // IDA_NAME: _ZN10CInventory20ChecknFixWineAndMoruEv
    // DEMANGLED: CInventory::ChecknFixWineAndMoru(void)
    bool ChecknFixWineAndMoru();

    // Original dependency used by ChecknFixWineAndMoru; its provider is not
    // part of this exact-100 closure batch.
    int IsOverStackableItem(int item_id, unsigned char item_type);

    // IDA_EA: 0x84E9990
    // IDA_NAME: _ZN10CInventory14SetClearAvatarEii
    // DEMANGLED: CInventory::SetClearAvatar(int,int)
    void SetClearAvatar(int slot, int item_id);

    // IDA_EA: 0x84E99B4
    // IDA_NAME: _ZN10CInventory14SetClearAvatarEPm
    // DEMANGLED: CInventory::SetClearAvatar(unsigned long *)
    void SetClearAvatar(unsigned long* clear_avatar);

    // IDA_EA: 0x84FB638
    // IDA_NAME: _ZNK10CInventory13IsEquipAvatarEv
    // DEMANGLED: CInventory::IsEquipAvatar(void)const
    int IsEquipAvatar() const;

    // IDA_EA: 0x850AE7A
    // IDA_NAME: _ZNK10CInventory12makeCheckSumEii
    // DEMANGLED: CInventory::makeCheckSum(int,int)const
    int makeCheckSum(int pvp_win_point, int seed) const;

    // IDA_EA: 0x850CD1C
    // IDA_NAME: _ZNK10CInventory18get_avatar_slot_noEi
    // DEMANGLED: CInventory::get_avatar_slot_no(int)const
    int get_avatar_slot_no(int item_id) const;

    // IDA_EA: 0x850CD62
    // IDA_NAME: _ZNK10CInventory17get_inven_slot_noEi
    // DEMANGLED: CInventory::get_inven_slot_no(int)const
    int get_inven_slot_no(int item_id) const;

    // IDA_EA: 0x850CDAC
    // IDA_NAME: _ZNK10CInventory29getEquipWeaponUpgradeSeparateEv
    // DEMANGLED: CInventory::getEquipWeaponUpgradeSeparate(void)const
    int getEquipWeaponUpgradeSeparate() const;

    // IDA_EA: 0x850CDD8
    // IDA_NAME: _ZN10CInventory13getValidMoneyEi
    // DEMANGLED: CInventory::getValidMoney(int)
    int getValidMoney(int money);

    // IDA_EA: 0x8501886
    // IDA_NAME: _ZNK10CInventory11StartEndPosEN10Inven_Item9ITEM_TYPEERiS2_
    // DEMANGLED: CInventory::StartEndPos(Inven_Item::ITEM_TYPE,int &,int &)const
    int StartEndPos(Inven_Item::ITEM_TYPE item_type,
                    int& start,
                    int& end) const;

    // IDA_EA: 0x850185E
    // IDA_NAME: _ZNK10CInventory14getStartEndPosEN10Inven_Item9ITEM_TYPEERiS2_
    // DEMANGLED: CInventory::getStartEndPos(Inven_Item::ITEM_TYPE,int &,int &)const
    int getStartEndPos(Inven_Item::ITEM_TYPE item_type,
                       int& start,
                       int& end) const;

    // IDA_EA: 0x850A016
    // IDA_NAME: _ZNK10CInventory16IsExistStackableEiN10Inven_Item9ITEM_TYPEE
    // DEMANGLED: CInventory::IsExistStackable(int,Inven_Item::ITEM_TYPE)const
    int IsExistStackable(int item_id, Inven_Item::ITEM_TYPE item_type) const;

    // IDA_EA: 0x84FB67A
    // IDA_NAME: _ZNK10CInventory17GetRemainCapacityE10INVEN_TYPEN10Inven_Item9ITEM_TYPEE
    // DEMANGLED: CInventory::GetRemainCapacity(INVEN_TYPE,Inven_Item::ITEM_TYPE)const
    int GetRemainCapacity(INVEN_TYPE inventory_type,
                          Inven_Item::ITEM_TYPE item_type) const;

    // IDA_EA: 0x85018D2
    // IDA_NAME: _ZNK10CInventory11GetItemTypeEi
    // DEMANGLED: CInventory::GetItemType(int)const
    int GetItemType(int item_id) const;

    // IDA_EA: 0x8505038
    // IDA_NAME: _ZNK10CInventory15QuickEmptyCountEv
    // DEMANGLED: CInventory::QuickEmptyCount(void)const
    int QuickEmptyCount() const;
    // IDA_EA: 0x8505080
    // IDA_NAME: _ZNK10CInventory15InvenEmptycountEN10Inven_Item9ITEM_TYPEE
    // DEMANGLED: CInventory::InvenEmptycount(Inven_Item::ITEM_TYPE)const
    int InvenEmptycount(Inven_Item::ITEM_TYPE item_type) const;
    // IDA_EA: 0x8504F64
    // IDA_NAME: _ZNK10CInventory17check_empty_countEN10Inven_Item9ITEM_TYPEEi
    // DEMANGLED: CInventory::check_empty_count(Inven_Item::ITEM_TYPE,int)const
    int check_empty_count(Inven_Item::ITEM_TYPE item_type,
                          int required_count) const;

    // IDA_EA: 0x85052BE
    // IDA_NAME: _ZNK10CInventory36CheckExistItemInInventoryOrEquipmentEiRiRb
    // DEMANGLED: CInventory::CheckExistItemInInventoryOrEquipment(int,int &,bool &)const
    int CheckExistItemInInventoryOrEquipment(int item_id,
                                             int& slot,
                                             bool& equipment) const;

    // IDA_EA: 0x850542A
    // IDA_NAME: _ZNK10CInventory30ShowAllExistItemsForErrorCheckEv
    // DEMANGLED: CInventory::ShowAllExistItemsForErrorCheck(void)const
    bool ShowAllExistItemsForErrorCheck() const;

    // IDA_EA: 0x8505172
    // IDA_NAME: _ZNK10CInventory16check_item_existEi
    // DEMANGLED: CInventory::check_item_exist(int)const
    int check_item_exist(int item_id) const;

    // IDA_EA: 0x850552A
    // IDA_NAME: _ZNK10CInventory32check_vilmark_dungeon_item_existEi
    // DEMANGLED: CInventory::check_vilmark_dungeon_item_exist(int)const
    int check_vilmark_dungeon_item_exist(int item_id) const;

    // IDA_EA: 0x84FBF2C
    // IDA_NAME: _ZNK10CInventory12GetInvenDataEiR10Inven_Item
    // DEMANGLED: CInventory::GetInvenData(int,Inven_Item &)const
    int GetInvenData(int item_id, Inven_Item& out) const;

    // IDA_EA: 0x84FF762
    // IDA_NAME: _ZN10CInventory9use_equipEi
    // DEMANGLED: CInventory::use_equip(int)
    int use_equip(int slot);

    // IDA_EA: 0x85000AE
    // IDA_NAME: _ZN10CInventory11update_itemE10INVEN_TYPEi10Inven_Item
    // DEMANGLED: CInventory::update_item(INVEN_TYPE,int,Inven_Item)
    int update_item(INVEN_TYPE inventory_type, int slot, Inven_Item item);

    // IDA_EA: 0x84FF8DE
    // IDA_NAME: _ZN10CInventory8use_itemEiii
    // DEMANGLED: CInventory::use_item(int,int,int)
    int use_item(int slot, int count, int reason);

    // IDA_EA: 0x84FFFBA
    // IDA_NAME: _ZNK10CInventory12try_use_itemEiii
    // DEMANGLED: CInventory::try_use_item(int,int,int)const
    int try_use_item(int slot, int count, int reason) const;

    // IDA_EA: 0x8500354
    // IDA_NAME: _ZN10CInventory31isEquipmentPlaceableAtInventoryEi
    // DEMANGLED: CInventory::isEquipmentPlaceableAtInventory(int)
    bool isEquipmentPlaceableAtInventory(int slot);

    // IDA_EA: 0x8500384
    // IDA_NAME: _ZNK10CInventory15GetSlotItemTypeEi
    // DEMANGLED: CInventory::GetSlotItemType(int)const
    int GetSlotItemType(int slot) const;

    // IDA_EA: 0x8501B70
    // IDA_NAME: _ZNK10CInventory31tryInsertInvenItemIntoQuickSlotE10Inven_Item
    // DEMANGLED: CInventory::tryInsertInvenItemIntoQuickSlot(Inven_Item)const
    int tryInsertInvenItemIntoQuickSlot(Inven_Item item) const;
    // IDA_EA: 0x8501BF6
    // IDA_NAME: _ZNK10CInventory30tryStackInvenItemIntoQuickSlotE10Inven_Item
    // DEMANGLED: CInventory::tryStackInvenItemIntoQuickSlot(Inven_Item)const
    int tryStackInvenItemIntoQuickSlot(Inven_Item item) const;
    // IDA_EA: 0x8501CEA
    int tryStackInvenItemIntoCorrespondingSpace(Inven_Item item) const;
    // IDA_EA: 0x8501E12
    int tryInsertInvenItemIntoCoresspondingSpace(Inven_Item item) const;
    // IDA_EA: 0x85035F4
    // IDA_NAME: _ZNK10CInventory28tryInsertAvatarIntoInventoryE10Inven_Item
    // DEMANGLED: CInventory::tryInsertAvatarIntoInventory(Inven_Item)const
    int tryInsertAvatarIntoInventory(Inven_Item item) const;

    // IDA_EA: 0x8501ECA
    // IDA_NAME: _ZNK10CInventory26tryInsertItemIntoInventoryE10Inven_Item
    // DEMANGLED: CInventory::tryInsertItemIntoInventory(Inven_Item)const
    int tryInsertItemIntoInventory(Inven_Item item) const;

    // IDA_EA: 0x8501AFA
    // IDA_NAME: _ZNK10CInventory26tryInsertItemIntoEquipmentE10Inven_Item
    // DEMANGLED: CInventory::tryInsertItemIntoEquipment(Inven_Item)const
    int tryInsertItemIntoEquipment(Inven_Item item) const;

    // IDA_EA: 0x8502470
    // IDA_NAME: _ZN10CInventory28insertInvenItemIntoQuickSlotE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::insertInvenItemIntoQuickSlot(Inven_Item,eItemAddReason,bool)
    int insertInvenItemIntoQuickSlot(Inven_Item item,
                                     eItemAddReason reason,
                                     bool log_history);

    // IDA_EA: 0x85025F8
    // IDA_NAME: _ZN10CInventory37insertInvenItemIntoCoresspondingSpaceE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::insertInvenItemIntoCoresspondingSpace(Inven_Item,eItemAddReason,bool)
    int insertInvenItemIntoCoresspondingSpace(Inven_Item item,
                                              eItemAddReason reason,
                                              bool log_history);

    // IDA_EA: 0x85027B2
    // IDA_NAME: _ZN10CInventory27stackInvenItemIntoQuickSlotE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::stackInvenItemIntoQuickSlot(Inven_Item,eItemAddReason,bool)
    int stackInvenItemIntoQuickSlot(Inven_Item item,
                                    eItemAddReason reason,
                                    bool log_history);

    // IDA_EA: 0x8502A7E
    // IDA_NAME: _ZN10CInventory36stackInvenItemIntoCorrespondingSpaceE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::stackInvenItemIntoCorrespondingSpace(Inven_Item,eItemAddReason,bool)
    int stackInvenItemIntoCorrespondingSpace(Inven_Item item,
                                             eItemAddReason reason,
                                             bool log_history);

    // IDA_EA: 0x8503392
    // IDA_NAME: _ZN10CInventory18doesExistEquipItemEii
    // DEMANGLED: CInventory::doesExistEquipItem(int,int)
    int doesExistEquipItem(int item_id, int item_key);

    // IDA_EA: 0x8503418
    // IDA_NAME: _ZN10CInventory17getExistEquipItemEiii
    // DEMANGLED: CInventory::getExistEquipItem(int,int,int)
    int getExistEquipItem(int item_id, int item_key, int excluded_slot);

    // IDA_EA: 0x85034AE
    // IDA_NAME: _ZN10CInventory24removeDupliacteEquipItemEv
    // DEMANGLED: CInventory::removeDupliacteEquipItem(void)
    int removeDupliacteEquipItem();

    // IDA_EA: 0x8503638
    // IDA_NAME: _ZNK10CInventory18getAvatarEmptySlotEii
    // DEMANGLED: CInventory::getAvatarEmptySlot(int,int)const
    int getAvatarEmptySlot(int first_slot, int last_slot) const;

    // IDA_EA: 0x8502344
    // IDA_NAME: _ZN10CInventory23insertItemIntoEquipmentE10Inven_Item14eItemAddReasonbb
    // DEMANGLED: CInventory::insertItemIntoEquipment(Inven_Item,eItemAddReason,bool,bool)
    int insertItemIntoEquipment(Inven_Item item,
                                eItemAddReason reason,
                                bool preserve_state,
                                bool log_history);

    // IDA_EA: 0x8502D86
    // IDA_NAME: _ZN10CInventory23insertItemIntoInventoryE10Inven_Item14eItemAddReasonbb
    // DEMANGLED: CInventory::insertItemIntoInventory(Inven_Item,eItemAddReason,bool,bool)
    int insertItemIntoInventory(Inven_Item item,
                                eItemAddReason reason,
                                bool log_history,
                                bool creature_flag);

    // IDA_EA: 0x8506BB2
    // IDA_NAME: _ZN10CInventory18insert_event_itemsERKSt6vectorISt4pairIiiESaIS2_EERS0_IiSaIiEE15eMoneyAddReason14eItemAddReasonbPKcSD_
    // DEMANGLED: CInventory::insert_event_items(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector&<int,std::allocator<int>>,eMoneyAddReason,eItemAddReason,bool,char const*,char const*)
    int insert_event_items(
        const std::vector<std::pair<int, int> >& items,
        std::vector<int>& inserted_slots,
        eMoneyAddReason money_reason,
        eItemAddReason item_reason,
        bool notify,
        const char* success_message,
        const char* failure_message);

    // IDA_EA: 0x850A86A
    // IDA_NAME: _ZN10CInventory22insertItemIntoCreatureER10Inven_Item14eItemAddReasonbb
    // DEMANGLED: CInventory::insertItemIntoCreature(Inven_Item &,eItemAddReason,bool,bool)
    int insertItemIntoCreature(Inven_Item& item,
                               eItemAddReason reason,
                               bool log_history,
                               bool creature_flag);

    // IDA_EA: 0x850A3B6
    // IDA_NAME: _ZNK10CInventory20GetExistCreatureSlotER10Inven_Item
    // DEMANGLED: CInventory::GetExistCreatureSlot(Inven_Item &)const
    int GetExistCreatureSlot(Inven_Item& item) const;

    // IDA_EA: 0x850A436
    // IDA_NAME: _ZNK10CInventory20GetEmptyCreatureSlotER10Inven_Item
    // DEMANGLED: CInventory::GetEmptyCreatureSlot(Inven_Item &)const
    int GetEmptyCreatureSlot(Inven_Item& item) const;

    // IDA_EA: 0x850A4B0
    // IDA_NAME: _ZNK10CInventory16WriteCreatureLogE10INVEN_TYPEiiii14eItemAddReason
    // DEMANGLED: CInventory::WriteCreatureLog(INVEN_TYPE,int,int,int,int,eItemAddReason)const
    void WriteCreatureLog(INVEN_TYPE inventory_type,
                          int item_id,
                          int resulting_count,
                          int added_count,
                          int item_type,
                          eItemAddReason reason) const;

    // IDA_EA: 0x850A52C
    // IDA_NAME: _ZN10CInventory31insertItemIntoCreatureStackableE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::insertItemIntoCreatureStackable(Inven_Item,eItemAddReason,bool)
    int insertItemIntoCreatureStackable(Inven_Item item,
                                        eItemAddReason reason,
                                        bool log_history);

    // IDA_EA: 0x850A762
    // IDA_NAME: _ZN10CInventory31insertItemIntoCreatureEquipmentE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::insertItemIntoCreatureEquipment(Inven_Item,eItemAddReason,bool)
    int insertItemIntoCreatureEquipment(Inven_Item item,
                                        eItemAddReason reason,
                                        bool log_history);

    // IDA_EA: 0x850A7C4
    // IDA_NAME: _ZN10CInventory31insertItemIntoArtifactEquipmentE10Inven_Item14eItemAddReasonb
    // DEMANGLED: CInventory::insertItemIntoArtifactEquipment(Inven_Item,eItemAddReason,bool)
    int insertItemIntoArtifactEquipment(Inven_Item item,
                                        eItemAddReason reason,
                                        bool log_history);

    // IDA_EA: 0x850AAA4
    // IDA_NAME: _ZNK10CInventory25tryInsertItemIntoCreatureER10Inven_Item
    // DEMANGLED: CInventory::tryInsertItemIntoCreature(Inven_Item &)const
    int tryInsertItemIntoCreature(Inven_Item& item) const;

    // IDA_EA: 0x850AB10
    // IDA_NAME: _ZN10CInventory30insertCreatureIntoSpecificSlotE10Inven_Itemi14eItemAddReasonb
    // DEMANGLED: CInventory::insertCreatureIntoSpecificSlot(Inven_Item,int,eItemAddReason,bool)
    int insertCreatureIntoSpecificSlot(Inven_Item item,
                                       int slot,
                                       eItemAddReason reason,
                                       bool log_history);

    // IDA_EA: 0x81347D6
    // IDA_NAME: _ZNK10CInventory9get_moneyEv
    // DEMANGLED: CInventory::get_money(void)const
    int get_money() const;

    // IDA_EA: 0x822D68A
    // IDA_NAME: _ZNK10CInventory7GetCoinEv
    // DEMANGLED: CInventory::GetCoin(void)const
    int GetCoin() const;

    // IDA_EA: 0x822D6FC
    // IDA_NAME: _ZNK10CInventory22get_inventory_capacityEv
    // DEMANGLED: CInventory::get_inventory_capacity(void)const
    int get_inventory_capacity() const;

    // IDA_EA: 0x84FF54C
    // IDA_NAME: _ZN10CInventory9use_moneyEi15eMoneySubReasonb
    // DEMANGLED: CInventory::use_money(int,eMoneySubReason,bool)
    int use_money(int money, eMoneySubReason reason, bool log_history);

    // IDA_EA: 0x84FF29C
    // IDA_NAME: _ZN10CInventory10gain_moneyEi15eMoneyAddReasonbi
    // DEMANGLED: CInventory::gain_money(int,eMoneyAddReason,bool,int)
    int gain_money(int money,
                   eMoneyAddReason reason,
                   bool log_history,
                   int excluded_money);

    // IDA_EA: 0x8508C2E
    // IDA_NAME: _ZNK10CInventory15SendAvatarEventEiiiii
    // DEMANGLED: CInventory::SendAvatarEvent(int,int,int,int,int)const
    void SendAvatarEvent(int source,
                         int first_slot,
                         int second_slot,
                         int value_1,
                         int value_2) const;

    // IDA_EA: 0x8508FD0
    // IDA_NAME: _ZN10CInventory15MoveClearAvatarEiiiiii
    // DEMANGLED: CInventory::MoveClearAvatar(int,int,int,int,int,int)
    void MoveClearAvatar(int target_avatar_slot,
                         int source_avatar_key,
                         int source_avatar_item_id,
                         int equipment_slot,
                         int clear_avatar_key,
                         int clear_avatar_item_id);

    // IDA_EA: 0x850937E
    // IDA_NAME: _ZNK10CInventory22isAnyExpiredAvatarItemEv
    // DEMANGLED: CInventory::isAnyExpiredAvatarItem(void)const
    int isAnyExpiredAvatarItem() const;
    // IDA_EA: 0x8509466
    // IDA_NAME: _ZN10CInventory17ExpiredAvatarItemEP11PacketGuard
    // DEMANGLED: CInventory::ExpiredAvatarItem(PacketGuard *)
    bool ExpiredAvatarItem(PacketGuard* packet);

    // IDA_EA: 0x822D6C6
    // IDA_NAME: _ZN10CInventory10set_parentEP5CUser
    user_creature::CCreatureMgr* set_parent(CUser* user);
    // IDA_EA: 0x822D4F6
    // IDA_NAME: _ZN10CInventory18SetInventoryMemoryEP15InventoryMemory
    CInventory* SetInventoryMemory(InventoryMemory* memory);
    // IDA_EA: 0x80DD568
    // IDA_NAME: _ZN10CInventory15GetCreatureMgrWEv
    user_creature::CCreatureMgr* GetCreatureMgrW();
    // IDA_EA: 0x81B7408
    // IDA_NAME: _ZNK10CInventory15GetCreatureMgrREv
    // DEMANGLED: CInventory::GetCreatureMgrR(void)const
    const user_creature::CCreatureMgr* GetCreatureMgrR() const;
    // IDA_EA: 0x8110C6C
    // IDA_NAME: _ZN10CInventory17GetAvatarItemMgrWEv
    WongWork::CAvatarItemMgr* GetAvatarItemMgrW();
    // IDA_EA: 0x80DD576
    // IDA_NAME: _ZNK10CInventory17GetAvatarItemMgrREv
    const WongWork::CAvatarItemMgr* GetAvatarItemMgrR() const;

    // IDA_EA: 0x84FC6BC
    // IDA_NAME: _ZNK10CInventory14MakeItemPacketE10INVEN_TYPEiR11PacketGuard
    // DEMANGLED: CInventory::MakeItemPacket(INVEN_TYPE,int,PacketGuard &)const
    int MakeItemPacket(INVEN_TYPE inventory_type,
                       int slot,
                       PacketGuard& packet) const;

    // IDA_EA: 0x84FD6D2
    // IDA_NAME: _ZNK10CInventory24MakeSecuRewarditemPacketE10INVEN_TYPEiR11PacketGuard
    // DEMANGLED: CInventory::MakeSecuRewarditemPacket(INVEN_TYPE,int,PacketGuard &)const
    int MakeSecuRewarditemPacket(INVEN_TYPE inventory_type,
                                 int slot,
                                 PacketGuard& packet) const;

    // IDA_EA: 0x84FD7B6
    // IDA_NAME: _ZNK10CInventory12MakeItemListE10INVEN_TYPEPv
    // DEMANGLED: CInventory::MakeItemList(INVEN_TYPE,void *)const
    int MakeItemList(INVEN_TYPE inventory_type, void* packet) const;

    // IDA_EA: 0x850400C
    // IDA_NAME: _ZN10CInventory11delete_itemE10INVEN_TYPEii14eItemDelReasonb
    // DEMANGLED: CInventory::delete_item(INVEN_TYPE,int,int,eItemDelReason,bool)
    int delete_item(INVEN_TYPE inventory_type,
                    int slot,
                    int count,
                    eItemDelReason reason,
                    bool log_history);

    // IDA_EA: 0x8509610
    // IDA_NAME: _ZNK10CInventory10checkToFixEv
    // DEMANGLED: CInventory::checkToFix(void)const
    int checkToFix() const;

    // IDA_EA: 0x8509738
    // IDA_NAME: _ZN10CInventory9CheckNFixEv
    // DEMANGLED: CInventory::CheckNFix(void)
    bool CheckNFix();

    // IDA_EA: 0x822D6EE
    // IDA_NAME: _ZN10CInventory22set_inventory_capacityEi
    // DEMANGLED: CInventory::set_inventory_capacity(int)
    CInventory* set_inventory_capacity(int capacity);

    // IDA_EA: 0x84FB0A0
    // IDA_NAME: _ZNK10CInventory13GetInvenStartEi
    // DEMANGLED: CInventory::GetInvenStart(int)const
    Inven_Item* GetInvenStart(int inventory_type) const;

    // IDA_EA: 0x850A096
    // IDA_NAME: _ZN10CInventory12RemoveKCItemEP10Inven_ItemS1_
    // DEMANGLED: CInventory::RemoveKCItem(Inven_Item *,Inven_Item *)
    static bool RemoveKCItem(Inven_Item* equipment_items,
                             Inven_Item* stackable_items);

    // IDA_EA: 0x850A298
    // IDA_NAME: _ZN10CInventory12RemoveKCItemEv
    // DEMANGLED: CInventory::RemoveKCItem(void)
    bool RemoveKCItem();

    // IDA_EA: 0x850A2BC
    // IDA_NAME: _ZN10CInventory12RemoveFPItemEP10Inven_ItemS1_
    // DEMANGLED: CInventory::RemoveFPItem(Inven_Item *,Inven_Item *)
    static bool RemoveFPItem(Inven_Item* equipment_items,
                             Inven_Item* stackable_items);

    // IDA_EA: 0x850A392
    // IDA_NAME: _ZN10CInventory12RemoveFPItemEv
    // DEMANGLED: CInventory::RemoveFPItem(void)
    bool RemoveFPItem();

private:
    // The original CUser/CUserCharacInfo inventory persistence bodies address
    // these embedded fields directly.  Keep that owner relationship explicit
    // instead of recreating their offsets in the caller translation units.
    friend class CUser;
    friend class CUserCharacInfo;
    // LAYOUT_EVIDENCE: C2 keeps the recovered member order, with the native
    // weapon-avatar record inserted at equipment slot 10.
    // initializes the three InventoryMemory table pointers and the embedded
    // AvatarCoin/creature/avatar managers in this declaration order.
    CUser* parent_;                       // IDA +0x00
    uint32_t money_;                      // IDA +0x04
    uint32_t coin_;                       // IDA +0x08
    uint32_t pay_coin_;                   // IDA +0x0c
    uint32_t event_coin_;                 // IDA +0x10
    uint32_t reset_state_014_;             // IDA +0x14, reset/copy dword
    int32_t inventory_capacity_;           // IDA +0x18
    Inven_Item equipment_items_[oldsrv::config::kEquipmentTypeCount];
    // i386 supplies one alignment byte at native +0x68b.
    Inven_Item* stackable_items_;          // native +0x68c, InventoryMemory[0]
    Inven_Item* avatar_items_;             // native +0x690, InventoryMemory[352]

public:
    // Native +0x694. This is an embedded scalar object constructed by
    // CInventory::C2 and reset through AvatarCoin::Set(0).
    AvatarCoin avatar_coin_;

private:
    int clear_avatar_[oldsrv::config::kPersistedClearAvatarCount];
    AvatarVisibleData avatar_visible_data_[
        oldsrv::config::kCharacterViewEquipmentTypeCount]; // +0x6c4..+0x72b
    Inven_Item* creature_items_;            // native +0x72c, InventoryMemory[457]
    user_creature::CCreatureMgr creature_manager_; // native +0x730..+0x77b
    WongWork::CAvatarItemMgr avatar_item_manager_; // native +0x77c..+0x7ff

    friend struct CInventoryPrefixLayoutVerifier;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CInventoryPrefixLayoutVerifier {
    static_assert(offsetof(CInventory, parent_) == 0x000u,
                  "CInventory parent must stay at IDA +0x000");
    static_assert(offsetof(CInventory, money_) == 0x004u,
                  "CInventory money must stay at IDA +0x004");
    static_assert(offsetof(CInventory, coin_) == 0x008u,
                  "CInventory coin must stay at IDA +0x008");
    static_assert(offsetof(CInventory, pay_coin_) == 0x00cu,
                  "CInventory pay coin must stay at IDA +0x00c");
    static_assert(offsetof(CInventory, event_coin_) == 0x010u,
                  "CInventory event coin must stay at IDA +0x010");
    static_assert(offsetof(CInventory, reset_state_014_) == 0x014u,
                  "CInventory reset dword must stay at IDA +0x014");
    static_assert(offsetof(CInventory, inventory_capacity_) == 0x018u,
                  "CInventory capacity must stay at IDA +0x018");
    static_assert(offsetof(CInventory, equipment_items_) == 0x01cu,
                  "CInventory equipment table must stay at IDA +0x01c");
    static_assert(offsetof(CInventory, stackable_items_) == 0x68cu,
                  "CInventory stackable table must stay at native +0x68c");
    static_assert(offsetof(CInventory, avatar_items_) == 0x690u,
                  "CInventory avatar table must stay at native +0x690");
    static_assert(offsetof(CInventory, avatar_coin_) == 0x694u,
                  "CInventory AvatarCoin must stay at native +0x694");
    static_assert(offsetof(CInventory, clear_avatar_) == 0x698u,
                  "CInventory clear-avatar list must stay at native +0x698");
    static_assert(offsetof(CInventory, avatar_visible_data_) == 0x6c4u,
                  "CInventory avatar-visible data must stay at target +0x6c4");
    static_assert(offsetof(CInventory, creature_items_) == 0x72cu,
                  "CInventory creature table must stay at target +0x72c");
    static_assert(offsetof(CInventory, creature_manager_) == 0x730u,
                  "CInventory creature manager must stay at target +0x730");
    static_assert(offsetof(CInventory, avatar_item_manager_) == 0x77cu,
                  "CInventory avatar manager must stay at target +0x77c");
};
static_assert(sizeof(CInventory) == 0x800u,
              "CInventory target ABI size must stay 0x800");
#endif

struct CCargoLayoutAssert;

class CCargo {
public:
    // IDA_EA: 0x86949DA
    // IDA_NAME: _ZN6CCargoC2Ev
    // DEMANGLED: CCargo::CCargo(void)
    CCargo();

    // IDA_EA: 0x86949F8
    // IDA_NAME: _ZN6CCargoD2Ev
    // DEMANGLED: CCargo::~CCargo()
    ~CCargo();

    // IDA_EA: 0x850B0C2
    // IDA_NAME: _ZN6CCargo5resetEv
    // DEMANGLED: CCargo::reset(void)
    void* reset();

    // IDA_EA: 0x86949FE
    // IDA_NAME: _ZN6CCargo10set_parentEP5CUser
    // DEMANGLED: CCargo::set_parent(CUser *)
    void* set_parent(CUser* user);

    // IDA_EA: 0x86949C6
    // IDA_NAME: _ZN6CCargo18SetInventoryMemoryEP15InventoryMemory
    // DEMANGLED: CCargo::SetInventoryMemory(InventoryMemory *)
    void* SetInventoryMemory(InventoryMemory* memory);

    // IDA_EA: 0x84B4262
    // IDA_NAME: _ZNK6CCargo15get_cargo_startEv
    // DEMANGLED: CCargo::get_cargo_start(void)const
    Inven_Item* get_cargo_start() const;

    // IDA_EA: 0x822D708
    // IDA_NAME: _ZNK6CCargo11GetCapacityEv
    // DEMANGLED: CCargo::GetCapacity(void)const
    int GetCapacity() const;

    // IDA_EA: 0x850C656
    // IDA_NAME: _ZN6CCargo12UpgradeCargoEi
    // DEMANGLED: CCargo::UpgradeCargo(int)
    int UpgradeCargo(int capacity);

    // IDA_EA: 0x850C698
    // IDA_NAME: _ZNK6CCargo12isUpgradableEi
    // DEMANGLED: CCargo::isUpgradable(int)const
    int isUpgradable(int capacity) const;

    // IDA_EA: 0x84E99F0
    // IDA_NAME: _ZNK6CCargo12GetItemCountEv
    // DEMANGLED: CCargo::GetItemCount(void)const
    int GetItemCount() const;

    // IDA_EA: 0x850BEE2
    // IDA_NAME: _ZNK6CCargo12MakeItemListEP11PacketGuard
    // DEMANGLED: CCargo::MakeItemList(PacketGuard *)const
    int MakeItemList(PacketGuard* packet) const;

    // IDA_EA: 0x850C160
    // IDA_NAME: _ZNK6CCargo14MakeItemPacketEP11PacketGuardi
    // DEMANGLED: CCargo::MakeItemPacket(PacketGuard *,int)const
    int MakeItemPacket(PacketGuard* packet, int slot) const;

    // IDA_EA: 0x850C836
    // IDA_NAME: _ZNK6CCargo12FindItemLockEhR14ENUM_ITEMSPACERi
    // DEMANGLED: CCargo::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
    int FindItemLock(unsigned char lock,
                     ENUM_ITEMSPACE& item_space,
                     int& slot) const;

    // IDA_EA: 0x850BD50
    // IDA_NAME: _ZNK6CCargo16SendItemLockListEv
    // DEMANGLED: CCargo::SendItemLockList(void)const
    void SendItemLockList() const;

    // IDA_EA: 0x850BD0C
    // IDA_NAME: _ZN6CCargo4sortEv
    // DEMANGLED: CCargo::sort(void)
    void* sort();

    // IDA_EA: 0x850B116
    // IDA_NAME: _ZN6CCargo9set_cargoEPv
    // DEMANGLED: CCargo::set_cargo(void *)
    int set_cargo(void* source);

    // IDA_EA: 0x850B164
    // IDA_NAME: _ZNK6CCargo9get_cargoEPv
    // DEMANGLED: CCargo::get_cargo(void *)const
    int get_cargo(void* destination) const;

    // IDA_EA: 0x850B2B4
    // IDA_NAME: _ZNK6CCargo14get_cargo_slotEi
    // DEMANGLED: CCargo::get_cargo_slot(int)const
    Inven_Item get_cargo_slot(int slot) const;

    // IDA_EA: 0x850B390
    // IDA_NAME: _ZNK6CCargo11GetCargoRefEi
    // DEMANGLED: CCargo::GetCargoRef(int)const
    const Inven_Item* GetCargoRef(int slot) const;

    // IDA_EA: 0x850B3C8
    // IDA_NAME: _ZN6CCargo11GetCargoRefEi
    // DEMANGLED: CCargo::GetCargoRef(int)
    Inven_Item* GetCargoRef(int slot);

    // IDA_EA: 0x850B400
    // IDA_NAME: _ZN6CCargo11insert_itemERK10Inven_Item
    // DEMANGLED: CCargo::insert_item(Inven_Item const&)
    int insert_item(const Inven_Item& item);

    // IDA_EA: 0x850BB00
    // IDA_NAME: _ZN6CCargo11update_itemEiRK10Inven_Item
    // DEMANGLED: CCargo::update_item(int,Inven_Item const&)
    CCargo* update_item(int slot, const Inven_Item& item);

    // IDA_EA: 0x850B4B0
    // IDA_NAME: _ZNK6CCargo16_tryAddStackItemERK10Inven_Itemi
    // DEMANGLED: CCargo::_tryAddStackItem(Inven_Item const&,int)const
    int _tryAddStackItem(const Inven_Item& item, int slot) const;

    // IDA_EA: 0x850B570
    // IDA_NAME: _ZNK6CCargo24tryInsertItemSpecialSlotERK10Inven_Itemi
    // DEMANGLED: CCargo::tryInsertItemSpecialSlot(Inven_Item const&,int)const
    int tryInsertItemSpecialSlot(const Inven_Item& item, int slot) const;

    // IDA_EA: 0x850B672
    // IDA_NAME: _ZN6CCargo24insert_item_special_slotER10Inven_Itemi
    // DEMANGLED: CCargo::insert_item_special_slot(Inven_Item &,int)
    int insert_item_special_slot(Inven_Item& item, int slot);

    // IDA_EA: 0x850B904
    // IDA_NAME: _ZN6CCargo9move_itemEii
    // DEMANGLED: CCargo::move_item(int,int)
    int move_item(int source_slot, int destination_slot);

    // IDA_EA: 0x850B1A0
    // IDA_NAME: _ZN6CCargo10_ResetSlotEi14eItemDelReason
    // DEMANGLED: CCargo::_ResetSlot(int,eItemDelReason)
    CCargo* _ResetSlot(int slot, eItemDelReason reason);

    // IDA_EA: 0x850B794
    // IDA_NAME: _ZN6CCargo11delete_itemEii14eItemDelReason
    // DEMANGLED: CCargo::delete_item(int,int,eItemDelReason)
    int delete_item(int slot, int count, eItemDelReason reason);

    // IDA_EA: 0x850BBCA
    // IDA_NAME: _ZNK6CCargo16check_slot_emptyEi
    // DEMANGLED: CCargo::check_slot_empty(int)const
    bool check_slot_empty(int slot) const;

    // IDA_EA: 0x850BC14
    // IDA_NAME: _ZNK6CCargo16check_item_existEi
    // DEMANGLED: CCargo::check_item_exist(int)const
    int check_item_exist(int item_id) const;

    // IDA_EA: 0x850BC5A
    // IDA_NAME: _ZN6CCargo12_CompareSlotEPKvS1_
    // DEMANGLED: CCargo::_CompareSlot(void const*,void const*)
    static int _CompareSlot(const void* left, const void* right);

    // IDA_EA: 0x850C406
    // IDA_NAME: _ZNK6CCargo14get_empty_slotEv
    // DEMANGLED: CCargo::get_empty_slot(void)const
    int get_empty_slot() const;

    // IDA_EA: 0x850C462
    // IDA_NAME: _ZN6CCargo13_GetItemCountEv
    // DEMANGLED: CCargo::_GetItemCount(void)
    int _GetItemCount();

    // IDA_EA: 0x850C4BA
    // IDA_NAME: _ZN6CCargo8_AddItemERK10Inven_Itemi
    // DEMANGLED: CCargo::_AddItem(Inven_Item const&,int)
    CCargo* _AddItem(const Inven_Item& item, int slot);

    // IDA_EA: 0x850C546
    // IDA_NAME: _ZN6CCargo13_AddStackItemERK10Inven_Itemi
    // DEMANGLED: CCargo::_AddStackItem(Inven_Item const&,int)
    int _AddStackItem(const Inven_Item& item, int slot);

    // IDA_EA: 0x850C6E0
    // IDA_NAME: _ZNK6CCargo12GetCargoDataEiR10Inven_Item
    // DEMANGLED: CCargo::GetCargoData(int,Inven_Item &)const
    int GetCargoData(int item_id, Inven_Item& item) const;

    // IDA_EA: 0x850C79E
    // IDA_NAME: _ZN6CCargo18calcCargoItemCountEv
    // DEMANGLED: CCargo::calcCargoItemCount(void)
    int calcCargoItemCount();

    // IDA_EA: 0x850C7B2
    // IDA_NAME: _ZN6CCargo17_IsExistEquipItemEii
    // DEMANGLED: CCargo::_IsExistEquipItem(int,int)
    int _IsExistEquipItem(int item_id, int item_key);

    // IDA_EA: 0x850D39C
    // IDA_NAME: _ZNK6CCargo14IsOperatorableEi
    // DEMANGLED: CCargo::IsOperatorable(int)const
    bool IsOperatorable(int slot) const;

    // IDA_EA: 0x8694A0C
    // IDA_NAME: _ZN6CCargo11SetCapacityEi
    // DEMANGLED: CCargo::SetCapacity(int)
    CCargo* SetCapacity(int capacity);

private:
    // IDA layout: owner pointer, 152-slot backing store, item count,
    // capacity, then the cargo-sort flag.
    CUser* owner_;
    Inven_Item* cargo_;
    int item_count_;
    int capacity_;
    bool cargo_sort_flag_;
    uint8_t reserved_11_[3];

    friend struct CCargoLayoutAssert;
};

static_assert(sizeof(CCargo) == 0x14u,
              "CCargo IDA ABI size must stay 0x14");
struct CCargoLayoutAssert {
    static_assert(offsetof(CCargo, capacity_) == 0x0cu,
                  "CCargo capacity must stay at IDA +0x0c");
};

class CAccountCargo {
    friend class CUser;
    friend class DB_SaveAccountCargo;
    friend struct CAccountCargoLayoutAssert;

public:
    // IDA_EA: 0x8289794
    // IDA_NAME: _ZN13CAccountCargoC2Ev
    // DEMANGLED: CAccountCargo::CAccountCargo(void)
    CAccountCargo();

    // IDA_EA: 0x822F012
    // IDA_NAME: _ZN13CAccountCargo11GetCapacityEv
    // DEMANGLED: CAccountCargo::GetCapacity(void)
    int GetCapacity();

    // IDA_EA: 0x8695A0C
    // IDA_NAME: _ZN13CAccountCargo7IsAlterEv
    // DEMANGLED: CAccountCargo::IsAlter(void)
    int IsAlter();

    // IDA_EA: 0x844DC16
    // IDA_NAME: _ZN13CAccountCargo9SetStableEv
    // DEMANGLED: CAccountCargo::SetStable(void)
    CAccountCargo* SetStable();

    // IDA_EA: 0x822F020
    // IDA_NAME: _ZN13CAccountCargo8GetMoneyEv
    // DEMANGLED: CAccountCargo::GetMoney(void)
    int GetMoney();

    // IDA_EA: 0x8289816
    // IDA_NAME: _ZN13CAccountCargo9SetDBDataEP5CUserP10Inven_Itemjj
    // DEMANGLED: CAccountCargo::SetDBData(CUser *,Inven_Item *,unsigned int,unsigned int)
    CAccountCargo* SetDBData(CUser* user,
                             Inven_Item* items,
                             unsigned int money,
                             unsigned int capacity);

    // IDA_EA: 0x828986C
    // IDA_NAME: _ZN13CAccountCargo5ClearEv
    // DEMANGLED: CAccountCargo::Clear(void)
    void* Clear();

    // IDA_EA: 0x82898C0
    // IDA_NAME: _ZN13CAccountCargo9ResetSlotEi
    // DEMANGLED: CAccountCargo::ResetSlot(int)
    Inven_Item* ResetSlot(int slot);

    // IDA_EA: 0x82898F8
    // IDA_NAME: _ZN13CAccountCargo7GetSlotEi
    // DEMANGLED: CAccountCargo::GetSlot(int)
    Inven_Item GetSlot(int slot);

    // IDA_EA: 0x8289A0C
    // IDA_NAME: _ZN13CAccountCargo10GetSlotRefEi
    // DEMANGLED: CAccountCargo::GetSlotRef(int)
    Inven_Item* GetSlotRef(int slot);

    // IDA_EA: 0x8289A4A
    // IDA_NAME: _ZN13CAccountCargo20CheckInsertConditionER10Inven_Item
    // DEMANGLED: CAccountCargo::CheckInsertCondition(Inven_Item &)
    bool CheckInsertCondition(Inven_Item& item);

    // IDA_EA: 0x8289C82
    // IDA_NAME: _ZN13CAccountCargo10InsertItemER10Inven_Itemi
    // DEMANGLED: CAccountCargo::InsertItem(Inven_Item &,int)
    int InsertItem(Inven_Item& item, int slot);

    // Feature-owned CMD551 insertion.  CargoTransportStone deliberately moves
    // character-bound equipment that the ordinary account-cargo API must
    // continue to reject.  The dispatcher proves the PVF rule, trade state,
    // material cost and exact empty destination before calling this narrow
    // commit primitive; this owner still rechecks the record/slot boundary and
    // copies the complete 0x3d-byte equipment identity (including its GUID).
    int InsertExactEquipmentForCargoTransport(const Inven_Item& item,
                                               int slot);

    // Native cross-container packet owners use this only for compensating a
    // partially committed transaction.  It restores the exact prior blob and
    // marks the existing account-cargo save owner dirty; no new persistent
    // state or journal table is introduced.
    bool RestoreSlotSnapshotForTransaction(int slot,
                                           const Inven_Item& snapshot);

    // IDA_EA: 0x8289E3C
    // IDA_NAME: _ZN13CAccountCargo10DeleteItemEii
    // DEMANGLED: CAccountCargo::DeleteItem(int,int)
    int DeleteItem(int slot, int count);

    // IDA_EA: 0x8289F26
    // IDA_NAME: _ZN13CAccountCargo8MoveItemEii
    // DEMANGLED: CAccountCargo::MoveItem(int,int)
    int MoveItem(int source_slot, int destination_slot);

    // IDA_EA: 0x828A554
    // IDA_NAME: _ZNK13CAccountCargo14CheckValidSlotEi
    // DEMANGLED: CAccountCargo::CheckValidSlot(int)const
    bool CheckValidSlot(int slot) const;

    // IDA_EA: 0x828A580
    // IDA_NAME: _ZN13CAccountCargo12GetEmptySlotEv
    // DEMANGLED: CAccountCargo::GetEmptySlot(void)
    int GetEmptySlot();

    // IDA_EA: 0x828A5D4
    // IDA_NAME: _ZN13CAccountCargo14CheckSlotEmptyEi
    // DEMANGLED: CAccountCargo::CheckSlotEmpty(int)
    bool CheckSlotEmpty(int slot);

    // IDA_EA: 0x828A61A
    // IDA_NAME: _ZN13CAccountCargo19GetSpecificItemSlotEi
    // DEMANGLED: CAccountCargo::GetSpecificItemSlot(int)
    int GetSpecificItemSlot(int item_id);

    // IDA_EA: 0x828A670
    // IDA_NAME: _ZN13CAccountCargo15CheckStackLimitEiii
    // DEMANGLED: CAccountCargo::CheckStackLimit(int,int,int)
    bool CheckStackLimit(int slot, int item_id, int count);

    // IDA_EA: 0x828A742
    // IDA_NAME: _ZN13CAccountCargo8AddMoneyEj
    // DEMANGLED: CAccountCargo::AddMoney(unsigned int)
    void AddMoney(unsigned int money);

    // IDA_EA: 0x828A764
    // IDA_NAME: _ZN13CAccountCargo8SubMoneyEj
    // DEMANGLED: CAccountCargo::SubMoney(unsigned int)
    void SubMoney(unsigned int money);

    // IDA_EA: 0x828A794
    // IDA_NAME: _ZN13CAccountCargo12GetItemCountEv
    // DEMANGLED: CAccountCargo::GetItemCount(void)
    int GetItemCount();

    // IDA_EA: 0x828A88A
    // IDA_NAME: _ZN13CAccountCargo12SendItemListEv
    // DEMANGLED: CAccountCargo::SendItemList(void)
    int SendItemList();

    // IDA_EA: 0x828AB1C
    // IDA_NAME: _ZNK13CAccountCargo14MakeItemPacketEP11PacketGuardi
    // DEMANGLED: CAccountCargo::MakeItemPacket(PacketGuard *,int)const
    int MakeItemPacket(PacketGuard* packet, int slot) const;

    // IDA_EA: 0x828A12A
    // IDA_NAME: _ZN13CAccountCargo12DepositMoneyEj
    // DEMANGLED: CAccountCargo::DepositMoney(unsigned int)
    int DepositMoney(unsigned int money);

    // IDA_EA: 0x828A2F6
    // IDA_NAME: _ZN13CAccountCargo13WithdrawMoneyEj
    // DEMANGLED: CAccountCargo::WithdrawMoney(unsigned int)
    int WithdrawMoney(unsigned int money);

    // IDA_EA: 0x828A4CA
    // IDA_NAME: _ZN13CAccountCargo15CheckMoneyLimitEj
    // DEMANGLED: CAccountCargo::CheckMoneyLimit(unsigned int)
    bool CheckMoneyLimit(unsigned int money);

    // IDA_EA: 0x828A7DC
    // IDA_NAME: _ZN13CAccountCargo15SendNotifyMoneyE14ENUM_CMDPACKET
    // DEMANGLED: CAccountCargo::SendNotifyMoney(ENUM_CMDPACKET)
    void SendNotifyMoney(ENUM_CMDPACKET command);

    // IDA_EA: 0x828AD72
    // IDA_NAME: _ZN13CAccountCargo18CreateAccountCargoEP5CUser
    // DEMANGLED: CAccountCargo::CreateAccountCargo(CUser *)
    static int CreateAccountCargo(CUser* user);

    // IDA_EA: 0x828B0B2
    // IDA_NAME: _ZN13CAccountCargo19UpgradeAccountCargoEP5CUser
    // DEMANGLED: CAccountCargo::UpgradeAccountCargo(CUser *)
    static int UpgradeAccountCargo(CUser* user);

    // IDA_EA: 0x828B4EC
    // IDA_NAME: _ZN13CAccountCargo18DeleteAccountCargoEP5CUser
    // DEMANGLED: CAccountCargo::DeleteAccountCargo(CUser *)
    static int DeleteAccountCargo(CUser* user);

    // IDA_EA: 0x828B3D0
    // IDA_NAME: _ZN13CAccountCargo16SendNotifyRecipeEP5CUserib
    // DEMANGLED: CAccountCargo::SendNotifyRecipe(CUser *,int,bool)
    static void SendNotifyRecipe(CUser* user,
                                 int item_slot,
                                 bool deleted_required_item);

    // IDA_EA: 0x84EBE46
    // IDA_NAME: _ZN13CAccountCargo11SetCapacityEj
    // DEMANGLED: CAccountCargo::SetCapacity(unsigned int)
    CAccountCargo* SetCapacity(unsigned int capacity);

private:
    CUser* user_;                    // +0x000
    Inven_Item items_[56];           // +0x004, 56 * 0x3d
    uint32_t money_;                 // +0xd5c
    uint32_t capacity_;              // +0xd60
    uint8_t altered_;                // +0xd64
    uint8_t alignment_[3];           // +0xd65
};

static_assert(sizeof(CAccountCargo) == 0xd68u,
              "CAccountCargo IDA ABI size must stay 0xd68");
struct CAccountCargoLayoutAssert {
    static_assert(offsetof(CAccountCargo, capacity_) == 0xd60u,
                  "CAccountCargo capacity must stay at IDA +0xd60");
    static_assert(offsetof(CAccountCargo, altered_) == 0xd64u,
                  "CAccountCargo altered flag must stay at IDA +0xd64");
};

struct __attribute__((packed)) CStackableCoolTimeOption {
    uint16_t value;
    uint8_t enabled;
};

struct __attribute__((packed)) CStackableEffectItemOption {
    uint16_t value;
    uint8_t enabled;
};

static_assert(sizeof(CStackableCoolTimeOption) == 0x03u,
              "CStackable cooldown option must stay three bytes");
static_assert(offsetof(CStackableCoolTimeOption, value) == 0x00u &&
                  offsetof(CStackableCoolTimeOption, enabled) == 0x02u,
              "CStackable cooldown option fields drifted");
static_assert(sizeof(CStackableEffectItemOption) == 0x03u,
              "CStackable effect option must stay three bytes");
static_assert(offsetof(CStackableEffectItemOption, value) == 0x00u &&
                  offsetof(CStackableEffectItemOption, enabled) == 0x02u,
              "CStackable effect option fields drifted");

// IDA: CStackableItem +0x2dc.  set_item @0x850D81E copies the
// [stat change duration] value to duration_ms and retains the two byte flags
// independently.  _processEffectItem @0x865EC0E tests duration_tag first,
// then uses timer_enabled to decide whether TIMER_MESSAGE 118 is scheduled.
struct StatChangeInfo {
    // IDA_EA: 0x85141A8
    // IDA_NAME: _ZN14StatChangeInfoC2Ev
    // DEMANGLED: StatChangeInfo::StatChangeInfo(void)
    StatChangeInfo() __attribute__((noinline));
    int32_t duration_ms;        // +0x00
    uint8_t duration_tag;       // +0x04
    uint8_t timer_enabled;      // +0x05
    uint8_t reserved_06_07[2];  // +0x06
};

typedef StatChangeInfo CStackableStatChangeInfo;

// IDA: CStackableItem +0x2d0..+0x2ef.  CStackableItem::set_item
// @0x850D81E populates this entire scalar segment from STStackableScript;
// CUser's continuous-item flow consumes it at 0x865EA62..0x865EC0E.
struct CStackableContinuousItemState {
    int32_t cool_time_ms;  // +0x00, STStackableScript::item.values_5c[3]
    int32_t cool_time_group;  // +0x04
    CountinuousItemOption cool_time_maintenance;  // +0x08
    StatChangeInfo stat_change;  // +0x0c
    CountinuousItemOption effect_maintenance;  // +0x14
    uint8_t exp_affect_enabled;  // +0x17
    float exp_affect_ratio;  // +0x18
    uint8_t shared_effect_item;  // +0x1c, STStackableScript::value_1d0
    uint8_t reserved_1d;  // +0x1d
    uint16_t emancipation_value;  // +0x1e, STStackableScript::value_394
};

static_assert(sizeof(CStackableStatChangeInfo) == 0x08u,
              "CStackable stat-change segment must stay 0x08");
static_assert(offsetof(CStackableStatChangeInfo, duration_ms) == 0x00u &&
                  offsetof(CStackableStatChangeInfo, duration_tag) == 0x04u &&
                  offsetof(CStackableStatChangeInfo, timer_enabled) == 0x05u &&
                  offsetof(CStackableStatChangeInfo, reserved_06_07) == 0x06u,
              "CStackable stat-change fields drifted");
static_assert(sizeof(CStackableContinuousItemState) == 0x20u,
              "CStackable continuous-item segment must stay 0x20");
static_assert(offsetof(CStackableContinuousItemState, cool_time_ms) == 0x00u &&
                  offsetof(CStackableContinuousItemState, cool_time_group) == 0x04u &&
                  offsetof(CStackableContinuousItemState, cool_time_maintenance) == 0x08u &&
                  offsetof(CStackableContinuousItemState, stat_change) == 0x0cu &&
                  offsetof(CStackableContinuousItemState, effect_maintenance) == 0x14u &&
                  offsetof(CStackableContinuousItemState, exp_affect_enabled) == 0x17u &&
                  offsetof(CStackableContinuousItemState, exp_affect_ratio) == 0x18u &&
                  offsetof(CStackableContinuousItemState, shared_effect_item) == 0x1cu &&
                  offsetof(CStackableContinuousItemState, reserved_1d) == 0x1du &&
                  offsetof(CStackableContinuousItemState, emancipation_value) == 0x1eu,
              "CStackable continuous-item fields drifted");

// IDA: CStackableItem +0x324 .. +0x33f.  CStackableItem::set_item copies
// STStackableScript::values_448 in the non-linear original order.  The
// [shop doll] parser writes values_448[2], which becomes +0x32c; private
// store validation consumes that concrete field and must not treat it as a
// CItem index or an untyped byte offset.
struct CStackablePrivateStoreInfo {
    int32_t expert_job_value_1;       // +0x324 <- script +0x44c
    int32_t expert_job_value_3;       // +0x328 <- script +0x454
    int32_t shop_doll_item_index;     // +0x32c <- script +0x450
    int32_t expert_job_value_4;       // +0x330 <- script +0x458
    int32_t expert_job_value_5;       // +0x334 <- script +0x45c
    int32_t expert_job_value_6;       // +0x338 <- script +0x460
    int32_t expert_job_value_7;       // +0x33c <- script +0x464
};

static_assert(sizeof(CStackablePrivateStoreInfo) == 0x1cu,
              "CStackableItem private-store script segment must stay 0x1c");
static_assert(offsetof(CStackablePrivateStoreInfo, expert_job_value_1) == 0x00u &&
                  offsetof(CStackablePrivateStoreInfo, expert_job_value_3) == 0x04u &&
                  offsetof(CStackablePrivateStoreInfo, shop_doll_item_index) == 0x08u &&
                  offsetof(CStackablePrivateStoreInfo, expert_job_value_4) == 0x0cu &&
                  offsetof(CStackablePrivateStoreInfo, expert_job_value_5) == 0x10u &&
                  offsetof(CStackablePrivateStoreInfo, expert_job_value_6) == 0x14u &&
                  offsetof(CStackablePrivateStoreInfo, expert_job_value_7) == 0x18u,
              "CStackableItem private-store fields drifted");

class CStackableItem : public CItem {
public:
    friend struct CStackableItemLayoutVerifier;
    // IDA_EA: 0x8515762 / 0x85146B4 / 0x8514A5C.
    CStackableItem();
    ~CStackableItem();

    // IDA_EA: 0x850D81E
    // IDA_NAME: _ZN14CStackableItem8set_itemER17STStackableScript
    int set_item(STStackableScript& script);

    // IDA_EA: 0x850F8B8
    // IDA_NAME: _ZNK14CStackableItem9make_itemER10Inven_Item
    int make_item(Inven_Item& item) const;

    // IDA_EA: 0x822C9D2
    // IDA_NAME: _ZNK14CStackableItem12get_sub_typeEv
    // DEMANGLED: CStackableItem::get_sub_type(void)const
    int get_sub_type() const;
    // IDA_EA: 0x822C9E0
    // IDA_NAME: _ZN14CStackableItem22get_2nd_material_indexEv
    // DEMANGLED: CStackableItem::get_2nd_material_index(void)
    int get_2nd_material_index();
    // IDA_EA: 0x822C9EE
    // IDA_NAME: _ZN14CStackableItem22get_2nd_material_countEv
    // DEMANGLED: CStackableItem::get_2nd_material_count(void)
    int get_2nd_material_count();
    // IDA_EA: 0x822C9FC
    // IDA_NAME: _ZNK14CStackableItem17getStackableLimitEv
    // DEMANGLED: CStackableItem::getStackableLimit(void)const
    int getStackableLimit() const;
    // IDA_EA: 0x822CA0A
    // IDA_NAME: _ZNK14CStackableItem19getGlobalEffectDataEv
    // DEMANGLED: CStackableItem::getGlobalEffectData(void)const
    const std::vector<stGlobalEffect_t>& getGlobalEffectData() const;
    // IDA_EA: 0x822CA18
    // IDA_NAME: _ZN14CStackableItem18IsSharedEffectItemEv
    // DEMANGLED: CStackableItem::IsSharedEffectItem(void)
    int IsSharedEffectItem();
    // IDA_EA: 0x822CA28
    // IDA_NAME: _ZNK14CStackableItem20getJewelTargetSocketEv
    // DEMANGLED: CStackableItem::getJewelTargetSocket(void)const
    int getJewelTargetSocket() const;
    // IDA_EA: 0x822CA38
    // IDA_NAME: _ZN14CStackableItem12IsEmancipateEv
    // DEMANGLED: CStackableItem::IsEmancipate(void)
    int IsEmancipate();
    // IDA_EA: 0x822CA48
    // IDA_NAME: _ZNK14CStackableItem15IsDeletableItemEv
    // DEMANGLED: CStackableItem::IsDeletableItem(void)const
    int IsDeletableItem() const;
    // IDA_EA: 0x849F530
    // IDA_NAME: _ZN14CStackableItem16GetMonsterCardIdEv
    // DEMANGLED: CStackableItem::GetMonsterCardId(void)
    int GetMonsterCardId();
    // IDA_EA: 0x84B4222
    // IDA_NAME: _ZN14CStackableItem17IsReinforceTicketEv
    // DEMANGLED: CStackableItem::IsReinforceTicket(void)
    int IsReinforceTicket();
    // IDA_EA: 0x84B4232
    // IDA_NAME: _ZN14CStackableItem16GetReinforceRateEv
    // DEMANGLED: CStackableItem::GetReinforceRate(void)
    int GetReinforceRate();
    // IDA_EA: 0x84B4242
    // IDA_NAME: _ZN14CStackableItem24IsAmplifyReinforceTicketEv
    // DEMANGLED: CStackableItem::IsAmplifyReinforceTicket(void)
    int IsAmplifyReinforceTicket();
    // IDA_EA: 0x84B4252
    // IDA_NAME: _ZN14CStackableItem23GetAmplifyReinforceRateEv
    // DEMANGLED: CStackableItem::GetAmplifyReinforceRate(void)
    int GetAmplifyReinforceRate();
    // IDA_EA: 0x850D780
    // IDA_NAME: _ZNK14CStackableItem17getUsableItemTypeEi
    // DEMANGLED: CStackableItem::getUsableItemType(int)const
    bool getUsableItemType(int item_type) const;
    // IDA_EA: 0x850EB42
    // IDA_NAME: _ZNK14CStackableItem24IsAvatarOptionChangeItemEv
    // DEMANGLED: CStackableItem::IsAvatarOptionChangeItem(void)const
    int IsAvatarOptionChangeItem() const;
    // IDA_EA: 0x850EB52
    // IDA_NAME: _ZN14CStackableItem17check_able_To_useEii
    // DEMANGLED: CStackableItem::check_able_To_use(int,int)
    int check_able_To_use(int charac_job, int charac_level);
    // IDA_EA: 0x850EBA2
    // IDA_NAME: _ZNK14CStackableItem21check_change_job_typeEcc
    // DEMANGLED: CStackableItem::check_change_job_type(char,char)const
    bool check_change_job_type(char grow_type, char charac_job) const;
    // IDA_EA: 0x850F732
    // IDA_NAME: _ZN14CStackableItem18quickslot_priorityEv
    // DEMANGLED: CStackableItem::quickslot_priority(void)
    bool quickslot_priority();
    // IDA_EA: 0x850FA14
    // IDA_NAME: _ZN14CStackableItem6verifyEv
    // DEMANGLED: CStackableItem::verify(void)
    bool verify();
    // IDA_EA: 0x850FA8A
    // IDA_NAME: _ZN14CStackableItem35is_included_material_for_limit_cubeEj
    // DEMANGLED: CStackableItem::is_included_material_for_limit_cube(unsigned int)
    int is_included_material_for_limit_cube(unsigned int item_id);
    // IDA_EA: 0x850FB32
    // IDA_NAME: _ZN14CStackableItem22get_1st_material_countEj
    // DEMANGLED: CStackableItem::get_1st_material_count(unsigned int)
    int get_1st_material_count(unsigned int item_id);
    // IDA_EA: 0x850FBE4
    // IDA_NAME: _ZN14CStackableItem19get_limit_cube_itemERSt4pairIiiE
    // DEMANGLED: CStackableItem::get_limit_cube_item(std::pair<int,int> &)
    std::pair<int, int>* get_limit_cube_item(std::pair<int, int>& output);
    // IDA_EA: 0x8513444
    // IDA_NAME: _ZN14CStackableItem27get_amplify_reinforce_countEi
    // DEMANGLED: CStackableItem::get_amplify_reinforce_count(int)
    int get_amplify_reinforce_count(int random_value);
    // CMD205 hardening view over the already-owned +0x348 PVF map.  The
    // official random selector assumes a complete 0..99 distribution.
    bool has_valid_random_amplify_reinforce_table() const;
    // IDA_EA: 0x8514A84
    // IDA_NAME: _ZNK14CStackableItem11GetItemTypeEv
    // DEMANGLED: CStackableItem::GetItemType(void)const
    int GetItemType() const;
    // IDA_EA: 0x8514A7A
    int IsEnchantalbe() const;
    // IDA_EA: 0x8514A92
    int IsAvatarItem() const;
    // IDA_EA: 0x8514A9C
    // IDA_NAME: _ZNK14CStackableItem14IsCreatureItemEv
    // DEMANGLED: CStackableItem::IsCreatureItem(void)const
    int IsCreatureItem() const;
    // IDA_EA: 0x8514ACA
    int IsClearAvatar() const;
    // IDA_EA: 0x8514AD4
    bool isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER filter) const;
    // IDA_EA: 0x8514ADE
    // IDA_NAME: _ZNK14CStackableItem21GetGuildPowerWarPointEv
    // DEMANGLED: CStackableItem::GetGuildPowerWarPoint(void)const
    int GetGuildPowerWarPoint() const;
    // IDA_EA: 0x8514AEC
    // IDA_NAME: _ZNK14CStackableItem20GetUserPowerWarPointEv
    // DEMANGLED: CStackableItem::GetUserPowerWarPoint(void)const
    int GetUserPowerWarPoint() const;
    // IDA_EA: 0x8514AF8
    // IDA_NAME: _ZNK14CStackableItem16IsImpossibleGiftEv
    // DEMANGLED: CStackableItem::IsImpossibleGift(void)const
    int IsImpossibleGift() const;
    // IDA_EA: 0x8514B08
    // IDA_NAME: _ZNK14CStackableItem20IsRandomOptionUnsealEv
    // DEMANGLED: CStackableItem::IsRandomOptionUnseal(void)const
    int IsRandomOptionUnseal() const;
    // IDA_EA: 0x8514B18
    // IDA_NAME: _ZN14CStackableItem26set_parent_booster_item_idEm
    // DEMANGLED: CStackableItem::set_parent_booster_item_id(unsigned long)
    CStackableItem* set_parent_booster_item_id(unsigned long item_id);
    // IDA_EA: 0x8514B2A
    // IDA_NAME: _ZNK14CStackableItem26get_parent_booster_item_idEv
    // DEMANGLED: CStackableItem::get_parent_booster_item_id(void)const
    int get_parent_booster_item_id() const;
    // IDA_EA: 0x8514B38
    // IDA_NAME: _ZNK14CStackableItem18getTradeLimitCountEv
    // DEMANGLED: CStackableItem::getTradeLimitCount(void)const
    int getTradeLimitCount() const;
    // IDA_EA: 0x854B4E8
    // IDA_NAME: _ZN14CStackableItem13GetEmancipateEv
    // DEMANGLED: CStackableItem::GetEmancipate(void)
    int GetEmancipate();
    // IDA_EA: 0x854B4F8
    // IDA_NAME: _ZN14CStackableItem17GetReinforceVauleEv
    // DEMANGLED: CStackableItem::GetReinforceVaule(void)
    int GetReinforceVaule();
    // IDA_EA: 0x854B508
    // IDA_NAME: _ZN14CStackableItem24GetAmplifyReinforceValueEv
    // DEMANGLED: CStackableItem::GetAmplifyReinforceValue(void)
    int GetAmplifyReinforceValue();
    // IDA_EA: 0x854B518
    // IDA_NAME: _ZN14CStackableItem34is_random_amplify_reinforce_ticketEv
    // DEMANGLED: CStackableItem::is_random_amplify_reinforce_ticket(void)
    int is_random_amplify_reinforce_ticket();
    // IDA_EA: 0x86946D2
    // IDA_NAME: _ZN14CStackableItem17GetCoolTimeOptionEv
    // DEMANGLED: CStackableItem::GetCoolTimeOption(void)
    CStackableCoolTimeOption GetCoolTimeOption();
    // IDA_EA: 0x86946F4
    // IDA_NAME: _ZN14CStackableItem19GetEffectItemOptionEv
    // DEMANGLED: CStackableItem::GetEffectItemOption(void)
    CStackableEffectItemOption GetEffectItemOption();
    // IDA_EA: 0x8694716
    // IDA_NAME: _ZN14CStackableItem17GetStatChangeInfoEv
    // DEMANGLED: CStackableItem::GetStatChangeInfo(void)
    CStackableStatChangeInfo GetStatChangeInfo();
    // IDA_EA: 0x8694736
    // IDA_NAME: _ZN14CStackableItem11IsExpAffectEv
    // DEMANGLED: CStackableItem::IsExpAffect(void)
    int IsExpAffect();
    // IDA_EA: 0x8694746
    // IDA_NAME: _ZN14CStackableItem17GetExpAffectRatioEv
    // DEMANGLED: CStackableItem::GetExpAffectRatio(void)
    long double GetExpAffectRatio();
    // IDA_EA: 0x869475E
    // IDA_NAME: _ZNK14CStackableItem26IsUse2ndUpgradeTableRecipeEv
    // DEMANGLED: CStackableItem::IsUse2ndUpgradeTableRecipe(void)const
    int IsUse2ndUpgradeTableRecipe() const;
    // IDA_EA: 0x822CA58
    // IDA_NAME: _ZNK14CStackableItem21verifyStackableActionEN15StackableAction4TypeE
    // DEMANGLED: CStackableItem::verifyStackableAction(StackableAction::Type)const
    bool verifyStackableAction(StackableAction::Type action) const;

    // IDA_EA: 0x850E9F4
    // IDA_NAME: _ZN14CStackableItem17verifyUsablePlaceEP5CUser
    // DEMANGLED: CStackableItem::verifyUsablePlace(CUser *)
    int verifyUsablePlace(CUser* user);

    // IDA_EA: 0x850E8DE
    // IDA_NAME: _ZN14CStackableItem22verifyUsableServerTypeEv
    // DEMANGLED: CStackableItem::verifyUsableServerType(void)
    int verifyUsableServerType();

    // IDA_EA: 0x850EBFC
    // IDA_NAME: _ZN14CStackableItem14get_lotto_itemEPc
    // DEMANGLED: CStackableItem::get_lotto_item(char *)
    Inven_Item* get_lotto_item(char* output);
    // IDA_EA: 0x850EDDA
    // IDA_NAME: _ZN14CStackableItem25get_multibox_lottery_itemEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
    void get_multibox_lottery_item(
        std::vector<std::pair<Inven_Item, bool> >* output);
    // IDA_EA: 0x850F136
    // IDA_NAME: _ZN14CStackableItem25get_multibox_default_itemEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
    void get_multibox_default_item(
        std::vector<std::pair<Inven_Item, bool> >* output);
    // IDA_EA: 0x850F38A
    // IDA_NAME: _ZN14CStackableItem30get_multibox_default_item_aradEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
    void get_multibox_default_item_arad(
        std::vector<std::pair<Inven_Item, bool> >* output);

    // CStackableItem::C2/D2 construct/destruct stStackableBooster_t at +0x240.
    const stStackableBooster_t& booster_data() const {
        return booster_data_;
    }

    // CStackableItem::C2/D2 construct/destroy BoosterSelectInfo at +0x264;
    // set_item copies STStackableScript::booster_select into that exact
    // subobject for item type 34.  CMD163 reads its value_08 selection count.
    const BoosterSelectInfo& booster_select_info() const {
        return booster_select_info_;
    }

    // IDA C2/D2 @ 0x8515762/0x85146B4 construct/destroy this exact child at
    // +0x1e4.  Locked multibox validation consumes item_id/third_dword from
    // the real PVF object; it must not reinterpret an anonymous tail.
    const MultiBoxLotteryInfo& multibox_lottery_info() const {
        return multibox_lottery_info_;
    }

    // CStackableItem::set_item copies the PVF [enchant system] map to +0x2f4
    // and the two selection-rate bytes to +0x30c/+0x30d.  Third-chronicle
    // CItemUpgrade consumes exactly these fields for CMD371; this owner-local
    // view prevents that transaction from treating a PVF prototype as raw
    // anonymous storage.
    const std::map<STEnchantSystemMapKey, STEnchantSystemMapData>&
    third_chronicle_enchant_map() const {
        return third_chronicle_enchant_map_;
    }

    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>&
    third_chronicle_enchant_map() {
        return third_chronicle_enchant_map_;
    }

    uint8_t third_chronicle_enchant_group() const {
        return third_chronicle_enchant_group_;
    }

    char* third_chronicle_enchant_rates() const {
        return const_cast<char*>(third_chronicle_enchant_rates_);
    }

    // IDA_EA: 0x850F75E
    // IDA_NAME: _ZNK14CStackableItem14getBoosterItemERSt6vectorI28stStackableBoosterItemInfo_tSaIS1_EE
    // DEMANGLED: CStackableItem::getBoosterItem(std::vector<stStackableBoosterItemInfo_t,std::allocator<stStackableBoosterItemInfo_t>> &)const
    bool getBoosterItem(
        std::vector<stStackableBoosterItemInfo_t>& output) const;

    // CStackableItem::C2/D2 construct the package vectors at +0x21c and
    // +0x234.  set_item @0x850D81E copies the corresponding PVF package
    // rows for item types 14 and 38.  CeraShop package redemption consumes
    // these same typed subobjects at 0x827c3ab and 0x827c3d1.
    const std::vector<STStackablePackageData>& package_data() const {
        return package_data_;
    }

    const std::vector<std::vector<STStackablePackageData> >&
    package_selection_data() const {
        return package_selection_data_;
    }

    // Owner-local typed projection of RecipeInfo::input_items.  IDA
    // expert_job::GetRecipeInputItems @0x849B734 iterates the first vector in
    // RecipeInfo stored at the exact CStackableItem +0x174 member.
    const std::vector<std::pair<int, int> >& recipe_input_items() const {
        return recipe_info_.pairs_00;
    }

    // CStackableItem::C2 constructs RecipeInfo at +0x174.  The alchemist
    // recipe path reads its +0x25 mode flag through this same
    // subobject, so retain a typed projection rather than a byte surrogate.
    const RecipeInfo& recipe_info() const {
        return recipe_info_;
    }

    // CStackableItem::C2 constructs AradItemInfo at +0x148.  The secret-shop
    // event delivery path reads its rabbit_20 vector at +0x168 through this
    // owner-local subobject rather than treating the item as raw storage.
    const AradItemInfo& arad_item_info() const {
        return arad_item_info_;
    }

    // The limit-cube parser populates this exact stLimitCubeInfo subobject;
    // CMD339 must use the owned table and its owned random stream rather than
    // reconstructing a second rate table from the wire request.
    const stLimitCubeInfo& limit_cube_info() const {
        return limit_cube_info_;
    }

    const std::vector<CRabbitBox>& additional_items() const {
        return arad_item_info().rabbit_20;
    }

    // Verified IDA layout view used by StackableAction event consumers.
    // reqBingoMark/proc_addFatigue read the first script value from +0x3A0.
    const std::vector<int>& action_values() const {
        return action_values_;
    }

    // CStackableItem::set_item @0x850D81E copies the PVF scalar at script
    // +0x474 into the exact +0x340 member.  CUser's gold-lottery broadcast
    // leaf @0x868D1EC compares the award with this value after confirming
    // the item is stackable.
    int lottery_gold_broadcast_minimum() const {
        return lottery_gold_broadcast_minimum_;
    }

    // STStackableScript "[check usable itemlevel]" maps pair_43c and
    // value_438 to these exact CStackableItem offsets in set_item
    // @0x850D81E.
    int usable_item_level_min() const {
        return usable_item_level_min_;
    }
    int usable_item_level_max() const {
        return usable_item_level_max_;
    }
    bool has_usable_item_level_range() const {
        return has_usable_item_level_range_ == 1u;
    }

    // CStackableItem::set_item @0x850D81E copies STStackableScript's
    // [evolution creature] value to +0x360.  CMD324 compares this exact
    // value with CCreatureMgr::GetCreatureIndex before it permits the
    // existing ItemEvolute transaction.
    int evolution_creature_id() const {
        return evolution_creature_id_;
    }

    // STStackableScript::summon_monster_314.enabled is copied by
    // CStackableItem::set_item @0x850D81E to this exact byte.  CMD047 uses
    // it to reject monster-summon consumables in tournament dungeons except
    // for the original item-type-18 exception.
    bool has_summon_monster() const {
        return summon_monster_enabled_ != 0u;
    }

    // IDA C2/D2 @0x8515762/0x85146B4 construct the concrete three-byte
    // AvatarOptionChangeType at +0x365 followed by the two std::set<int>
    // members at +0x368/+0x380.  set_item @0x850D81E copies exactly
    // STStackableScript +0x47c/+0x480/+0x498 to the same locations.  These
    // are owner-local typed views of already-constructed objects; they do
    // not introduce an additional lifetime or a byte-offset surrogate.
    const AvatarOptionChangeType& avatar_option_change_type() const {
        return avatar_option_change_type_;
    }

    const std::set<int>& usable_avatar_item_ids() const {
        return usable_avatar_item_ids_;
    }

    const std::set<int>& prohibited_avatar_options() const {
        return prohibited_avatar_options_;
    }

    // IDA CStackableItem::C2/D2 @0x8515762/0x85146B4 construct and
    // destroy this exact std::set<int> at +0x3d8.  set_item
    // @0x850D81E copies STStackableScript::set_50c to it.  CMD275 looks up
    // the requested target prototype in the same owned set before starting
    // the bead-enchant transaction; expose that real subobject rather than
    // reading a synthetic integer at the caller.
    const std::set<int>& bead_target_item_ids() const {
        return bead_target_item_ids_;
    }

    const std::vector<std::string>& stackable_appendage_scripts() const {
        return stackable_appendage_scripts_;
    }
    bool verifies_stack_number() const {
        return verify_stack_number_ != 0u;
    }
    int all_skill_plus() const { return all_skill_plus_; }
    int levelup_mode() const { return levelup_mode_; }
    const std::vector<StackableDisguiseAppendageModifier>&
    disguise_appendages() const {
        return disguise_appendages_;
    }
    const std::vector<StackableEmancipateGrowGroup>&
    emancipate_grow_groups() const {
        return emancipate_grow_groups_;
    }
    const std::vector<std::pair<int, int> >&
    emancipate_material_revisions() const {
        return emancipate_material_revisions_;
    }
    const std::set<int>& stackable_action_limit_indexes() const {
        return stackable_action_limit_indexes_;
    }
    const std::vector<StackableRandomImageEntry>& random_image_entries() const {
        return random_image_entries_;
    }

public:
    // Physical 0627 derived layout.  Every STL/business child below is a
    // real C++ member; numeric offsets are retained only by the ABI gates.
    AradItemInfo arad_item_info_;                    // +0x148
    RecipeInfo recipe_info_;                         // +0x174
    LotteryInfo lottery_info_;                       // +0x1c0

private:
    // set_item @0x850D81E creates this per-prototype MT stream.  The old D2
    // does not delete it, so pointer ownership deliberately remains external.
    CMTRand* limit_cube_random_;                      // +0x1e0

public:
    MultiBoxLotteryInfo multibox_lottery_info_;      // +0x1e4
    std::vector<STStackablePackageData> package_data_; // +0x21c
    std::vector<STStackablePackageData> alternate_package_data_; // +0x228
    std::vector<std::vector<STStackablePackageData> >
        package_selection_data_;                     // +0x234
    stStackableBooster_t booster_data_;              // +0x240
    BoosterSelectInfo booster_select_info_;          // +0x264
    std::vector<stGlobalEffect_t> global_effect_data_; // +0x280
    int32_t stackable_limit_;                        // +0x28c
    stLimitCubeInfo limit_cube_info_;                // +0x290
    int32_t item_type_;                              // +0x2b0
    uint16_t jewel_target_socket_;                   // +0x2b4
    uint16_t reserved_2b6_;
    int32_t monster_card_id_;                        // +0x2b8
    int32_t sub_type_;                               // +0x2bc
    std::vector<int> usable_equipment_types_;        // +0x2c0
    int32_t script_value_2cc_;                       // +0x2cc
    CStackableContinuousItemState continuous_item_state_; // +0x2d0
    uint8_t emancipate_;                             // +0x2f0
    uint8_t reserved_2f1_2f3_[3];
    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>
        third_chronicle_enchant_map_;                // +0x2f4
    uint8_t third_chronicle_enchant_group_;          // +0x30c
    char third_chronicle_enchant_rates_[2];          // +0x30d
    uint8_t reinforce_ticket_;                       // +0x30f
    uint16_t reinforce_value_;                       // +0x310
    uint8_t reinforce_rate_;                         // +0x312
    uint8_t amplify_reinforce_ticket_;               // +0x313
    uint16_t amplify_reinforce_value_;               // +0x314
    uint8_t amplify_reinforce_rate_;                 // +0x316
    uint8_t reserved_317_;
    int32_t usable_item_level_min_;                  // +0x318
    int32_t usable_item_level_max_;                  // +0x31c
    uint8_t has_usable_item_level_range_;            // +0x320
    uint8_t deletable_item_;                         // +0x321
    uint8_t reserved_322_323_[2];
    CStackablePrivateStoreInfo private_store_info_;  // +0x324
    int32_t lottery_gold_broadcast_minimum_;         // +0x340
    uint8_t random_amplify_reinforce_ticket_;        // +0x344
    uint8_t reserved_345_347_[3];
    std::map<int, int> amplify_reinforce_count_rates_; // +0x348
    int32_t evolution_creature_id_;                  // +0x360
    uint8_t summon_monster_enabled_;                 // +0x364
    AvatarOptionChangeType avatar_option_change_type_; // +0x365
    std::set<int> usable_avatar_item_ids_;           // +0x368
    std::set<int> prohibited_avatar_options_;        // +0x380
    uint8_t avatar_option_change_item_;              // +0x398
    uint8_t reserved_399_39b_[3];
    int32_t stackable_action_type_;                  // +0x39c
    std::vector<int> action_values_;                 // +0x3a0
    std::vector<StackableAction::UsablePlace> usable_places_; // +0x3ac
    std::vector<StackableAction::UsableServerType>
        usable_server_types_;                        // +0x3b8
    uint8_t random_option_unseal_;                   // +0x3c4
    // STStackableScript's `[daily delete item]` byte is copied here by
    // CStackableItem::set_item @0x850D81E.  The original consumer tests it
    // as a signed byte, so retain that exact positive-value predicate.
    uint8_t daily_delete_item_;                      // +0x3c5
    uint8_t reserved_3c6_3c7_[2];
    int32_t booster_data_mode_;                      // +0x3c8
    int32_t script_value_3cc_;                       // +0x3cc
    int32_t script_value_3d0_;                       // +0x3d0
    int32_t trade_limit_count_;                      // +0x3d4
    std::set<int> bead_target_item_ids_;             // +0x3d8
    std::set<int> limit_dungeon_ids_;                // +0x3f0
    std::set<int> add_socket_limited_avatar_ids_;    // +0x408
    int32_t resent_day_value_;                       // +0x420
    uint8_t cera_coupon_;                            // +0x424
    uint8_t exclusive_dungeon_tooltip_enabled_;      // +0x425
    uint8_t reserved_426_427_[2];
    std::set<int> exclusive_dungeon_ids_;            // +0x428
    std::vector<int> avatar_booster_preview_item_ids_; // +0x440
    // Native 0828 suffix promoted from STStackableScript.  The legacy 0x44c
    // prefix above remains untouched for old vtable consumers.
    std::vector<std::string> stackable_appendage_scripts_; // +0x44c
    uint8_t verify_stack_number_;                         // +0x458
    uint8_t reserved_459_45b_[3];
    int32_t all_skill_plus_;                              // +0x45c
    int32_t levelup_mode_;                                // +0x460
    std::vector<StackableDisguiseAppendageModifier>
        disguise_appendages_;                            // +0x464
    std::vector<StackableEmancipateGrowGroup>
        emancipate_grow_groups_;                         // +0x470
    std::vector<std::pair<int, int> >
        emancipate_material_revisions_;                  // +0x47c
    std::set<int> stackable_action_limit_indexes_;       // +0x488
    std::vector<StackableRandomImageEntry>
        random_image_entries_;                           // +0x4a0

    bool is_daily_delete_item() const {
        return static_cast<int8_t>(daily_delete_item_) > 0;
    }

    // An empty target set means that the socket tool follows its ordinary
    // action-value rules.  A non-empty [add socket limited avatar] block is a
    // strict Equipment.lst allowlist, as used by target item 10014841.
    bool can_add_socket_to_avatar(int avatar_item_id) const {
        return add_socket_limited_avatar_ids_.empty() ||
               add_socket_limited_avatar_ids_.count(avatar_item_id) != 0u;
    }

};

static_assert(sizeof(CStackableItem) == 0x4acu,
              "CStackableItem 0828 extension size must stay 0x4AC");

// Friend-only compile-time ledger so the private RNG pointer is checked along
// with every public member.  These are ABI gates, not runtime offset accessors.
struct CStackableItemLayoutVerifier {
    static_assert(offsetof(CStackableItem, arad_item_info_) == 0x148u &&
                      offsetof(CStackableItem, recipe_info_) == 0x174u &&
                      offsetof(CStackableItem, lottery_info_) == 0x1c0u &&
                      offsetof(CStackableItem, limit_cube_random_) == 0x1e0u &&
                      offsetof(CStackableItem, multibox_lottery_info_) == 0x1e4u,
                  "CStackableItem leading business objects drifted");
    static_assert(offsetof(CStackableItem, package_data_) == 0x21cu &&
                      offsetof(CStackableItem, alternate_package_data_) == 0x228u &&
                      offsetof(CStackableItem, package_selection_data_) == 0x234u &&
                      offsetof(CStackableItem, booster_data_) == 0x240u &&
                      offsetof(CStackableItem, booster_select_info_) == 0x264u &&
                      offsetof(CStackableItem, global_effect_data_) == 0x280u &&
                      offsetof(CStackableItem, stackable_limit_) == 0x28cu &&
                      offsetof(CStackableItem, limit_cube_info_) == 0x290u,
                  "CStackableItem package/booster/limit-cube objects drifted");
    static_assert(offsetof(CStackableItem, item_type_) == 0x2b0u &&
                      offsetof(CStackableItem, jewel_target_socket_) == 0x2b4u &&
                      offsetof(CStackableItem, reserved_2b6_) == 0x2b6u &&
                      offsetof(CStackableItem, monster_card_id_) == 0x2b8u &&
                      offsetof(CStackableItem, sub_type_) == 0x2bcu &&
                      offsetof(CStackableItem, usable_equipment_types_) == 0x2c0u &&
                      offsetof(CStackableItem, script_value_2cc_) == 0x2ccu &&
                      offsetof(CStackableItem, continuous_item_state_) == 0x2d0u,
                  "CStackableItem item-type and continuous state drifted");
    static_assert(offsetof(CStackableItem, emancipate_) == 0x2f0u &&
                      offsetof(CStackableItem, reserved_2f1_2f3_) == 0x2f1u &&
                      offsetof(CStackableItem, third_chronicle_enchant_map_) == 0x2f4u &&
                      offsetof(CStackableItem, third_chronicle_enchant_group_) == 0x30cu &&
                      offsetof(CStackableItem, third_chronicle_enchant_rates_) == 0x30du &&
                      offsetof(CStackableItem, reinforce_ticket_) == 0x30fu &&
                      offsetof(CStackableItem, reinforce_value_) == 0x310u &&
                      offsetof(CStackableItem, reinforce_rate_) == 0x312u,
                  "CStackableItem enchant/reinforce prefix drifted");
    static_assert(offsetof(CStackableItem, amplify_reinforce_ticket_) == 0x313u &&
                      offsetof(CStackableItem, amplify_reinforce_value_) == 0x314u &&
                      offsetof(CStackableItem, amplify_reinforce_rate_) == 0x316u &&
                      offsetof(CStackableItem, reserved_317_) == 0x317u &&
                      offsetof(CStackableItem, usable_item_level_min_) == 0x318u &&
                      offsetof(CStackableItem, usable_item_level_max_) == 0x31cu &&
                      offsetof(CStackableItem, has_usable_item_level_range_) == 0x320u &&
                      offsetof(CStackableItem, deletable_item_) == 0x321u &&
                      offsetof(CStackableItem, reserved_322_323_) == 0x322u,
                  "CStackableItem amplify and usable-level fields drifted");
    static_assert(offsetof(CStackableItem, private_store_info_) == 0x324u &&
                      offsetof(CStackableItem, lottery_gold_broadcast_minimum_) == 0x340u &&
                      offsetof(CStackableItem, random_amplify_reinforce_ticket_) == 0x344u &&
                      offsetof(CStackableItem, reserved_345_347_) == 0x345u &&
                      offsetof(CStackableItem, amplify_reinforce_count_rates_) == 0x348u &&
                      offsetof(CStackableItem, evolution_creature_id_) == 0x360u &&
                      offsetof(CStackableItem, summon_monster_enabled_) == 0x364u &&
                      offsetof(CStackableItem, avatar_option_change_type_) == 0x365u,
                  "CStackableItem store/amplify/evolution fields drifted");
    static_assert(offsetof(CStackableItem, usable_avatar_item_ids_) == 0x368u &&
                      offsetof(CStackableItem, prohibited_avatar_options_) == 0x380u &&
                      offsetof(CStackableItem, avatar_option_change_item_) == 0x398u &&
                      offsetof(CStackableItem, reserved_399_39b_) == 0x399u &&
                      offsetof(CStackableItem, stackable_action_type_) == 0x39cu &&
                      offsetof(CStackableItem, action_values_) == 0x3a0u &&
                      offsetof(CStackableItem, usable_places_) == 0x3acu &&
                      offsetof(CStackableItem, usable_server_types_) == 0x3b8u,
                  "CStackableItem avatar/action containers drifted");
    static_assert(offsetof(CStackableItem, random_option_unseal_) == 0x3c4u &&
                      offsetof(CStackableItem, daily_delete_item_) == 0x3c5u &&
                      offsetof(CStackableItem, reserved_3c6_3c7_) == 0x3c6u &&
                      offsetof(CStackableItem, booster_data_mode_) == 0x3c8u &&
                      offsetof(CStackableItem, script_value_3cc_) == 0x3ccu &&
                      offsetof(CStackableItem, script_value_3d0_) == 0x3d0u &&
                      offsetof(CStackableItem, trade_limit_count_) == 0x3d4u &&
                      offsetof(CStackableItem, bead_target_item_ids_) == 0x3d8u,
                  "CStackableItem terminal policy fields drifted");
    static_assert(
        offsetof(CStackableItem, limit_dungeon_ids_) == 0x3f0u &&
            offsetof(CStackableItem, add_socket_limited_avatar_ids_) ==
                0x408u &&
            offsetof(CStackableItem, resent_day_value_) == 0x420u &&
            offsetof(CStackableItem, cera_coupon_) == 0x424u &&
            offsetof(CStackableItem, exclusive_dungeon_tooltip_enabled_) ==
                0x425u &&
            offsetof(CStackableItem, exclusive_dungeon_ids_) == 0x428u &&
            offsetof(CStackableItem, avatar_booster_preview_item_ids_) ==
                0x440u &&
            offsetof(CStackableItem, stackable_appendage_scripts_) ==
                0x44cu &&
            offsetof(CStackableItem, verify_stack_number_) == 0x458u &&
            offsetof(CStackableItem, all_skill_plus_) == 0x45cu &&
            offsetof(CStackableItem, levelup_mode_) == 0x460u &&
            offsetof(CStackableItem, disguise_appendages_) == 0x464u &&
            offsetof(CStackableItem, emancipate_grow_groups_) == 0x470u &&
            offsetof(CStackableItem, emancipate_material_revisions_) ==
                0x47cu &&
            offsetof(CStackableItem, stackable_action_limit_indexes_) ==
                0x488u &&
            offsetof(CStackableItem, random_image_entries_) == 0x4a0u,
        "CStackableItem target stackable extension layout drifted");
};

#endif  // OLDSRV_OLD_GAME_INVENTORY_INVENTORY_H_
