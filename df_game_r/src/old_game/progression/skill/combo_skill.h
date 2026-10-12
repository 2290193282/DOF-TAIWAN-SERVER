#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_COMBO_SKILL_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_COMBO_SKILL_H_

#include "old_game/progression/quest/quest_skill_value_types.h"

#include <functional>
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <vector>

enum ENUM_SKILL_TREE_KIND {
    ENUM_SKILL_TREE_CURRENT = -1,
    ENUM_SKILL_TREE_0 = 0,
    ENUM_SKILL_TREE_1 = 1,
    ENUM_SKILL_TREE_SFP_0 = 2,
    ENUM_SKILL_TREE_SFP_1 = 3,
};


class PacketGuard;

#pragma pack(push, 1)
struct SIG_COMBO_SKILL {
    SIG_COMBO_SKILL();
    void* reset();

    uint32_t charac_no;                    // +0x00
    uint8_t first_tree_count;              // +0x04
    uint8_t second_tree_count;             // +0x05
    uint32_t charac_job;                   // +0x06, packed/unaligned
    comoboSkillData first_tree_combos[6];  // +0x0a
    comoboSkillData second_tree_combos[6]; // +0x5e
};
#pragma pack(pop)

// The 0627 ComboSkill translation unit was built with the old packed
// libstdc++ tree ABI.  IDA proves that its map state is laid out as one byte
// of key comparator followed by the four-dword rb-tree header and the node
// count at +0x11.  A normal i386 GCC 4.9 std::map is 0x18 and therefore cannot
// be embedded here: it would move SkillSlot's first skill tree from +0x46.
//
// This is the typed implementation of that one old specialization.  It owns
// and balances real rb-tree nodes; it is not raw storage, an overlay, or an
// address-keyed side table.  Keeping the physical state in this dedicated
// type also avoids changing every unrelated std::map in a translation unit.
struct ComboSkillRbNodeBase {
    enum Color {
        kRed = 0,
        kBlack = 1,
    };

    unsigned int color;
    ComboSkillRbNodeBase* parent;
    ComboSkillRbNodeBase* left;
    ComboSkillRbNodeBase* right;
};

typedef std::pair<const short, std::vector<int> > ComboSkillMapValue;

struct ComboSkillRbNode : public ComboSkillRbNodeBase {
    explicit ComboSkillRbNode(const ComboSkillMapValue& source);

    ComboSkillMapValue value;
};

#pragma pack(push, 1)
struct ComboSkillMapState {
    std::less<short> key_compare;       // old packed map +0x00
    ComboSkillRbNodeBase header;        // old packed map +0x01
    uint32_t node_count;                // old packed map +0x11
};
#pragma pack(pop)

class ComboSkillMap {
public:
    typedef ComboSkillMapValue value_type;
    typedef uint32_t size_type;

    class const_iterator;

    class iterator {
    public:
        iterator();
        explicit iterator(ComboSkillRbNodeBase* node);

        value_type& operator*() const;
        value_type* operator->() const;
        iterator& operator++();
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        friend class ComboSkillMap;
        friend class const_iterator;
        ComboSkillRbNodeBase* node_;
    };

    class const_iterator {
    public:
        const_iterator();
        explicit const_iterator(const ComboSkillRbNodeBase* node);
        const_iterator(const iterator& other);

        const value_type& operator*() const;
        const value_type* operator->() const;
        const_iterator& operator++();
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;

    private:
        friend class ComboSkillMap;
        const ComboSkillRbNodeBase* node_;
    };

    ComboSkillMap();
    ComboSkillMap(const ComboSkillMap& other);
    ~ComboSkillMap();
    ComboSkillMap& operator=(const ComboSkillMap& other);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    iterator find(short key);
    const_iterator find(short key) const;
    std::pair<iterator, bool> insert(const value_type& value);
    std::pair<iterator, bool> insert(
        const std::pair<short, std::vector<int> >& value);
    std::vector<int>& operator[](short key);
    void clear();
    bool empty() const;
    size_type size() const;

private:
    static ComboSkillRbNodeBase* minimum(ComboSkillRbNodeBase* node);
    static const ComboSkillRbNodeBase* minimum(
        const ComboSkillRbNodeBase* node);
    static ComboSkillRbNodeBase* increment(ComboSkillRbNodeBase* node);
    static const ComboSkillRbNodeBase* increment(
        const ComboSkillRbNodeBase* node);
    static ComboSkillRbNode* as_node(ComboSkillRbNodeBase* node);
    static const ComboSkillRbNode* as_node(
        const ComboSkillRbNodeBase* node);

    ComboSkillRbNodeBase*& root();
    ComboSkillRbNodeBase* root() const;
    void rotate_left(ComboSkillRbNodeBase* node);
    void rotate_right(ComboSkillRbNodeBase* node);
    void rebalance_after_insert(ComboSkillRbNodeBase* node);
    void destroy_subtree(ComboSkillRbNodeBase* node);

    ComboSkillMapState state_;
};

class ComboSkill {
public:
    ComboSkill();
    ~ComboSkill();
    ComboSkill(const ComboSkill& other);
    ComboSkill& operator=(const ComboSkill& other);

    int resetTree(::ENUM_SKILL_TREE_KIND kind);
    void convertComboSkillMap(unsigned char count,
                              const comoboSkillData* data,
                              ::ENUM_SKILL_TREE_KIND kind);
    void convertComboSkillArray(unsigned char& count,
                                comoboSkillData* data,
                                ::ENUM_SKILL_TREE_KIND kind) const;
    int loadData(SIG_COMBO_SKILL* data);
    int saveData(SIG_COMBO_SKILL* data) const;
    int getComboSkillSendList(PacketGuard& packet) const;
    int getComboSkillSendForm(PacketGuard& packet,
                              ::ENUM_SKILL_TREE_KIND kind) const;
    int setComboSkill(::ENUM_SKILL_TREE_KIND kind, ComboSkillMap& map);
    void eraseDuplicationComboSKill(::ENUM_SKILL_TREE_KIND kind,
                                    int skill_slot_index);
    const ComboSkillMap* getComboSkillMap(::ENUM_SKILL_TREE_KIND kind) const;
    ComboSkillMap* getComboSkillMap(::ENUM_SKILL_TREE_KIND kind);

    ComboSkillMap first_tree_;   // IDA +0x00
    ComboSkillMap second_tree_;  // IDA +0x15
};

static_assert(sizeof(SIG_COMBO_SKILL) == 0xb2u,
              "SIG_COMBO_SKILL 0627 payload must stay 0xb2 bytes");
static_assert(offsetof(SIG_COMBO_SKILL, charac_no) == 0x00u,
              "SIG_COMBO_SKILL character number must stay at +0x00");
static_assert(offsetof(SIG_COMBO_SKILL, first_tree_count) == 0x04u,
              "SIG_COMBO_SKILL first-tree count must stay at +0x04");
static_assert(offsetof(SIG_COMBO_SKILL, second_tree_count) == 0x05u,
              "SIG_COMBO_SKILL second-tree count must stay at +0x05");
static_assert(offsetof(SIG_COMBO_SKILL, charac_job) == 0x06u,
              "SIG_COMBO_SKILL character job must stay at +0x06");
static_assert(offsetof(SIG_COMBO_SKILL, first_tree_combos) == 0x0au,
              "SIG_COMBO_SKILL first-tree combos must stay at +0x0a");
static_assert(offsetof(SIG_COMBO_SKILL, second_tree_combos) == 0x5eu,
              "SIG_COMBO_SKILL second-tree combos must stay at +0x5e");

#if defined(__i386__)
static_assert(sizeof(ComboSkillRbNodeBase) == 0x10u,
              "old combo rb-tree node base must stay 0x10");
static_assert(sizeof(ComboSkillMapValue) == 0x10u &&
                  sizeof(ComboSkillRbNode) == 0x20u,
              "old combo rb-tree value/node ABI must stay 0x10/0x20");
static_assert(offsetof(ComboSkillMapState, header) == 0x01u,
              "old combo map header must start at +0x01");
static_assert(offsetof(ComboSkillMapState, node_count) == 0x11u,
              "old combo map node count must stay at +0x11");
static_assert(sizeof(ComboSkillMapState) == 0x15u &&
                  sizeof(ComboSkillMap) == 0x15u,
              "old combo map packed ABI must stay 0x15");
static_assert(offsetof(ComboSkill, first_tree_) == 0x00u &&
                  offsetof(ComboSkill, second_tree_) == 0x15u &&
                  sizeof(ComboSkill) == 0x2au,
              "ComboSkill physical map pair must stay +0/+0x15 (0x2a)");
#endif


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_COMBO_SKILL_H_
