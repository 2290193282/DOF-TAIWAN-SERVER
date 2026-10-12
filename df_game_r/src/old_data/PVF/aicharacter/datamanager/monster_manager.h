#ifndef DATAMANAGER_MONSTER_MANAGER_H_
#define DATAMANAGER_MONSTER_MANAGER_H_

#include <stdint.h>

#include <ext/hash_map>

#include <map>
#include <utility>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/monster_script.h"

struct Inven_Item;
namespace WongWork {
struct stGenerateRefData_t;
}

class CMonster {
public:
    CMonster();
    ~CMonster();

    STMonsterScript* getMonsterScript() const;
    // IDA_EA: 0x834F876
    // IDA_NAME: _ZNK8CMonster23generateIndependentItemERKN8WongWork19stGenerateRefData_tERSt6vectorI10Inven_ItemSaIS5_EE
    // DEMANGLED: CMonster::generateIndependentItem(
    //             WongWork::stGenerateRefData_t const&,
    //             std::vector<Inven_Item>&)const
    int generateIndependentItem(
        const WongWork::stGenerateRefData_t& reference,
        std::vector<Inven_Item>& items) const;
    // IDA_EA: 0x834F8A6
    // IDA_NAME: _ZNK8CMonster22generateDropAvatarItemEhhRSt6vectorI10Inven_ItemSaIS1_EE
    // DEMANGLED: CMonster::generateDropAvatarItem(
    //             unsigned char,unsigned char,std::vector<Inven_Item>&)const
    bool generateDropAvatarItem(uint8_t grade_index,
                                uint8_t part_index,
                                std::vector<Inven_Item>& items) const;
    // IDA_EA: 0x834F8E8
    // IDA_NAME: _ZNK8CMonster24generateCatchMonsterItemERSt6vectorI10Inven_ItemSaIS1_EE
    // DEMANGLED: CMonster::generateCatchMonsterItem(std::vector<Inven_Item>&)const
    int generateCatchMonsterItem(std::vector<Inven_Item>& items) const;
    // IDA_EA: 0x834F746.  Only this query validates the caller-supplied
    // drop-kind index; the two selectors below intentionally preserve the
    // original unchecked indexed access.
    int get_specialItem_list_size(unsigned int drop_kind) const;
    // IDA_EA: 0x834F776 / 0x834F7E4.  The overload that accepts a random
    // value uses it only for the ordinary per-kind table: the global
    // special-drop table always rolls against its own total weight.
    int drop_item_index(unsigned int drop_kind) const;
    int drop_item_index(unsigned int drop_kind, int random_value) const;
    // IDA_EA: 0x834F84A.
    int deathTowerSpecifyItemDrop() const;
    int set_monster(STMonsterScript& script);
    int get_index();
    int IsHellMonster() const;

    // 140828 PVF-backed monster state queries.  CMonster owns a full script
    // copy, so these are the runtime projection used by dungeon code rather
    // than parser-only sidecar values.
    bool is_getup_invincible() const;
    bool is_deathless() const;
    bool is_hp_recovery_impossible() const;
    bool is_appendage_disabled() const;
    float champion_fastmove_factor() const;
    float champion_fastattack_factor() const;

private:
    // CDataManager::Verify @0x8360D94 reports the two validated level
    // bounds while intentionally keeping this diagnostic non-fatal.
    friend class CDataManager;
    friend class WarField;

    int raw_00_;                                           // +0x00
    int index_;                                            // +0x04
    std::vector<std::pair<int, int> > special_item_lists_[4]; // +0x08
    int special_drop_total_rate_;                          // +0x38
    std::vector<std::pair<int, int> > special_drop_items_; // +0x3c
    std::vector<std::pair<int, int> > death_tower_specify_items_; // +0x48
    unsigned char is_hell_monster_;                        // +0x54
    int raw_58_;                                           // +0x58
    int raw_5c_;                                           // +0x5c
    STMonsterScript script_;                               // +0x60
    // WarField::ConsistMap performs weighted selection using this script
    // scalar.  It is a real CMonster field, not a shadow layout access.
    int war_room_spawn_weight_;                            // +0x1d34
};

// IDA_EA: 0x8349672
// IDA_NAME: _Z16_drop_item_indexiRKSt6vectorISt4pairIiiESaIS1_EE
// The helper is deliberately a global provider: CMonster's overloaded
// selectors and the Death Tower path all share its cumulative-weight rule.
int _drop_item_index(int random_value,
                     const std::vector<std::pair<int, int> >& items)
    __asm__("_Z16_drop_item_indexiRKSt6vectorISt4pairIiiESaIS1_EE");

class CMonsterManager {
public:
    CMonsterManager();
    ~CMonsterManager();

    int destroy();
    int load_monsters();
    int insert_monster(int category, int monster_index);
    int insert_monster(int monster_index, CMonster* monster);
    CMonster* find_monster(int monster_index);
    int get_levelrandom_mob(int level);

private:
    // CDataManager::Verify @0x8360D94 traverses this canonical monster map.
    friend class CDataManager;
    std::multimap<int, int> category_monsters_;
    __gnu_cxx::hash_map<int, CMonster*> monsters_;
};

static_assert(sizeof(CMonster) == 0x1D38u,
              "CMonster 0828 size must stay 0x1D38");
static_assert(sizeof(CMonsterManager) == 0x2Cu,
              "CMonsterManager IDA ABI size must stay 0x2C");

#endif  // DATAMANAGER_MONSTER_MANAGER_H_
