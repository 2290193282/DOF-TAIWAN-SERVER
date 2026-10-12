#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_VALUE_TYPES_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_VALUE_TYPES_H_

#include <stddef.h>

#include <array>
#include <utility>
#include <vector>

#include "old_game/appearance/avatar/world_avatar_cube_types.h"

typedef std::pair<int, int> CompoundAvatarItem;
typedef std::vector<CompoundAvatarItem> CompoundAvatarItemList;

// IDA unit: 06_userinfo_stat_inventory/tagCompoundAvatar.
//
// Member labels are evidence names derived from the old RDAR parser and
// consumers, rather than a claim about original source spelling.  The field
// order is physical: C2/D2, Clean, copy C2, and assignment all prove it.
struct tagCompoundAvatar {
    // IDA_EA: 0x891BE88
    // IDA_NAME: _ZN17tagCompoundAvatarC2Ev
    // DEMANGLED: tagCompoundAvatar::tagCompoundAvatar(void)
    tagCompoundAvatar();

    // IDA_EA: 0x891C1F8
    // IDA_NAME: _ZN17tagCompoundAvatarC2ERKS_
    // DEMANGLED: tagCompoundAvatar::tagCompoundAvatar(tagCompoundAvatar const&)
    tagCompoundAvatar(const tagCompoundAvatar& rhs);

    // IDA_EA: 0x833463C
    // IDA_NAME: _ZN17tagCompoundAvatarD2Ev
    // DEMANGLED: tagCompoundAvatar::~tagCompoundAvatar()
    ~tagCompoundAvatar();

    // IDA_EA: 0x891AB56
    // IDA_NAME: _ZN17tagCompoundAvatar5CleanEv
    // DEMANGLED: tagCompoundAvatar::Clean(void)
    void Clean();

    // IDA_EA: 0x894EC3A
    // IDA_NAME: _ZN17tagCompoundAvataraSERKS_
    // DEMANGLED: tagCompoundAvatar::operator=(tagCompoundAvatar const&)
    tagCompoundAvatar& operator=(const tagCompoundAvatar& rhs);

    int grade;                                                    // +0x000
    std::array<int, 10> rare_rate_by_part;                       // +0x004
    std::array<int, 10> normal_avatar_total_rate_by_part;        // +0x02c
    std::array<int, 10> rare_avatar_total_rate_by_part;          // +0x054
    std::array<CompoundAvatarItemList, 10> normal_avatar_items;  // +0x07c
    std::array<CompoundAvatarItemList, 10> rare_avatar_items;    // +0x0f4
    std::array<int, 10> upper_rare_avatar_total_rate_by_part;    // +0x16c
    std::array<CompoundAvatarItemList, 10> upper_rare_avatar_items;  // +0x194
    STAncientRareAvatarCube ancient_rare_cube;                   // +0x20c
    CompoundAvatarItemList material_items;                       // +0x2b8
    CompoundAvatarItemList material_upper_items;                 // +0x2c4
    CompoundAvatarItemList material_lower_items;                 // +0x2d0
    int upper_grade;                                              // +0x2dc
    std::array<int, 10> upper_rare_rate_by_part;                 // +0x2e0
    // 0828 PVF extension: [material_master].  Appended after the confirmed
    // 0627 prefix so the old table fields retain their ABI offsets.
    CompoundAvatarItemList material_master_items;                 // +0x308
};

// IDA unit: 06_userinfo_stat_inventory/tagCompoundAvatarTable.
struct tagCompoundAvatarTable {
    // IDA_EA: 0x833431C
    // IDA_NAME: _ZN22tagCompoundAvatarTableC2Ev
    // DEMANGLED: tagCompoundAvatarTable::tagCompoundAvatarTable(void)
    tagCompoundAvatarTable();

    // IDA_EA: 0x8334330
    // IDA_NAME: _ZN22tagCompoundAvatarTableD2Ev
    // DEMANGLED: tagCompoundAvatarTable::~tagCompoundAvatarTable()
    ~tagCompoundAvatarTable();

    // IDA_EA: 0x8903FCC
    // IDA_NAME: _ZNK22tagCompoundAvatarTable21getCompoundAvatarInfoEi
    // DEMANGLED: tagCompoundAvatarTable::getCompoundAvatarInfo(int)const
    tagCompoundAvatar* getCompoundAvatarInfo(int grade) const;

    std::vector<tagCompoundAvatar> entries;  // +0x00
};

// IDA_EA: 0x89031D2
int importCompoundAvatarTableCore(const char* directory,
                                  const char* filename,
                                  tagCompoundAvatarTable* table);

// IDA_EA: 0x8903EA7
int importCompoundAvatarTable(const char* directory,
                              const char* filename,
                              tagCompoundAvatarTable* table);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CompoundAvatarItemList) == 0x0cu,
              "Compound avatar item-vector ABI size must stay 0x0c");
static_assert(offsetof(tagCompoundAvatar, grade) == 0x000u,
              "tagCompoundAvatar grade must stay at +0x000");
static_assert(offsetof(tagCompoundAvatar, rare_rate_by_part) == 0x004u,
              "tagCompoundAvatar rare rates must stay at +0x004");
static_assert(offsetof(tagCompoundAvatar, normal_avatar_total_rate_by_part) ==
                   0x02cu,
              "tagCompoundAvatar normal totals must stay at +0x02c");
static_assert(offsetof(tagCompoundAvatar, rare_avatar_total_rate_by_part) ==
                   0x054u,
              "tagCompoundAvatar rare totals must stay at +0x054");
static_assert(offsetof(tagCompoundAvatar, normal_avatar_items) == 0x07cu,
              "tagCompoundAvatar normal item vectors must stay at +0x07c");
static_assert(offsetof(tagCompoundAvatar, rare_avatar_items) == 0x0f4u,
              "tagCompoundAvatar rare item vectors must stay at +0x0f4");
static_assert(offsetof(tagCompoundAvatar,
                       upper_rare_avatar_total_rate_by_part) == 0x16cu,
              "tagCompoundAvatar upper-rare totals must stay at +0x16c");
static_assert(offsetof(tagCompoundAvatar, upper_rare_avatar_items) == 0x194u,
              "tagCompoundAvatar upper-rare vectors must stay at +0x194");
static_assert(offsetof(tagCompoundAvatar, ancient_rare_cube) == 0x20cu,
              "tagCompoundAvatar ancient cube must stay at +0x20c");
static_assert(offsetof(tagCompoundAvatar, material_items) == 0x2b8u,
              "tagCompoundAvatar material items must stay at +0x2b8");
static_assert(offsetof(tagCompoundAvatar, material_upper_items) == 0x2c4u,
              "tagCompoundAvatar upper material must stay at +0x2c4");
static_assert(offsetof(tagCompoundAvatar, material_lower_items) == 0x2d0u,
              "tagCompoundAvatar lower material must stay at +0x2d0");
static_assert(offsetof(tagCompoundAvatar, upper_grade) == 0x2dcu,
              "tagCompoundAvatar upper grade must stay at +0x2dc");
static_assert(offsetof(tagCompoundAvatar, upper_rare_rate_by_part) == 0x2e0u,
              "tagCompoundAvatar upper rare rates must stay at +0x2e0");
static_assert(offsetof(tagCompoundAvatar, material_master_items) == 0x308u,
              "tagCompoundAvatar master material must start at +0x308");
static_assert(sizeof(tagCompoundAvatar) == 0x314u,
              "tagCompoundAvatar 0828 extension size must stay 0x314");
static_assert(sizeof(tagCompoundAvatarTable) == 0x0cu,
              "tagCompoundAvatarTable IDA ABI size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_VALUE_TYPES_H_
