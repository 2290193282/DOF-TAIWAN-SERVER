#include "old_game/appearance/avatar/compound_avatar_value_types.h"

#include <stddef.h>

// IDA_EA: 0x891BE88
// IDA_NAME: _ZN17tagCompoundAvatarC2Ev
// DEMANGLED: tagCompoundAvatar::tagCompoundAvatar(void)
tagCompoundAvatar::tagCompoundAvatar() {
    // The old C2 constructs the ownership-bearing members in declaration
    // order but deliberately leaves every scalar field uninitialized.  Do not
    // call Clean() here and do not add scalar member initializers.
}

// IDA_EA: 0x891C1F8
// IDA_NAME: _ZN17tagCompoundAvatarC2ERKS_
// DEMANGLED: tagCompoundAvatar::tagCompoundAvatar(tagCompoundAvatar const&)
tagCompoundAvatar::tagCompoundAvatar(const tagCompoundAvatar& rhs)
    : grade(rhs.grade),
      rare_rate_by_part(rhs.rare_rate_by_part),
      normal_avatar_total_rate_by_part(rhs.normal_avatar_total_rate_by_part),
      rare_avatar_total_rate_by_part(rhs.rare_avatar_total_rate_by_part),
      normal_avatar_items(rhs.normal_avatar_items),
      rare_avatar_items(rhs.rare_avatar_items),
      upper_rare_avatar_total_rate_by_part(
          rhs.upper_rare_avatar_total_rate_by_part),
      upper_rare_avatar_items(rhs.upper_rare_avatar_items),
      ancient_rare_cube(rhs.ancient_rare_cube),
      material_items(rhs.material_items),
      material_upper_items(rhs.material_upper_items),
      material_lower_items(rhs.material_lower_items),
      upper_grade(rhs.upper_grade),
      upper_rare_rate_by_part(rhs.upper_rare_rate_by_part),
      material_master_items(rhs.material_master_items) {
}

// IDA_EA: 0x833463C
// IDA_NAME: _ZN17tagCompoundAvatarD2Ev
// DEMANGLED: tagCompoundAvatar::~tagCompoundAvatar()
tagCompoundAvatar::~tagCompoundAvatar() {
}

// IDA_EA: 0x891AB56
// IDA_NAME: _ZN17tagCompoundAvatar5CleanEv
// DEMANGLED: tagCompoundAvatar::Clean(void)
void tagCompoundAvatar::Clean() {
    grade = 0;
    for (int part = 0; part != 10; ++part) {
        rare_rate_by_part[part] = 0;
    }

    for (int part = 0; part != 10; ++part) {
        normal_avatar_items[part].clear();
        rare_avatar_items[part].clear();
        normal_avatar_total_rate_by_part[part] = 0;
        rare_avatar_total_rate_by_part[part] = 0;
        upper_rare_avatar_total_rate_by_part[part] = 0;
        upper_rare_avatar_items[part].clear();
    }

    material_items.clear();
    upper_grade = 0;
    for (int part = 0; part != 10; ++part) {
        upper_rare_rate_by_part[part] = 0;
    }
    material_upper_items.clear();
    material_lower_items.clear();
    material_master_items.clear();

    // The old Clean() does not touch ancient_rare_cube.  Its independent
    // lifecycle/state must survive this reset exactly as in the IDB body.
}

// IDA_EA: 0x894EC3A
// IDA_NAME: _ZN17tagCompoundAvataraSERKS_
// DEMANGLED: tagCompoundAvatar::operator=(tagCompoundAvatar const&)
tagCompoundAvatar& tagCompoundAvatar::operator=(const tagCompoundAvatar& rhs) {
    grade = rhs.grade;
    rare_rate_by_part = rhs.rare_rate_by_part;
    normal_avatar_total_rate_by_part = rhs.normal_avatar_total_rate_by_part;
    rare_avatar_total_rate_by_part = rhs.rare_avatar_total_rate_by_part;
    normal_avatar_items = rhs.normal_avatar_items;
    rare_avatar_items = rhs.rare_avatar_items;
    upper_rare_avatar_total_rate_by_part =
        rhs.upper_rare_avatar_total_rate_by_part;
    upper_rare_avatar_items = rhs.upper_rare_avatar_items;
    ancient_rare_cube = rhs.ancient_rare_cube;
    material_items = rhs.material_items;
    material_upper_items = rhs.material_upper_items;
    material_lower_items = rhs.material_lower_items;
    upper_grade = rhs.upper_grade;
    upper_rare_rate_by_part = rhs.upper_rare_rate_by_part;
    material_master_items = rhs.material_master_items;
    return *this;
}

// IDA_EA: 0x833431C
// IDA_NAME: _ZN22tagCompoundAvatarTableC2Ev
// DEMANGLED: tagCompoundAvatarTable::tagCompoundAvatarTable(void)
tagCompoundAvatarTable::tagCompoundAvatarTable()
    : entries() {
}

// IDA_EA: 0x8334330
// IDA_NAME: _ZN22tagCompoundAvatarTableD2Ev
// DEMANGLED: tagCompoundAvatarTable::~tagCompoundAvatarTable()
tagCompoundAvatarTable::~tagCompoundAvatarTable() {
}

// IDA_EA: 0x8903FCC
// IDA_NAME: _ZNK22tagCompoundAvatarTable21getCompoundAvatarInfoEi
// DEMANGLED: tagCompoundAvatarTable::getCompoundAvatarInfo(int)const
tagCompoundAvatar* tagCompoundAvatarTable::getCompoundAvatarInfo(
    int requested_grade) const {
    if (requested_grade <= 0 ||
        entries.size() <= static_cast<std::size_t>(requested_grade)) {
        return 0;
    }

    tagCompoundAvatar* candidate = const_cast<tagCompoundAvatar*>(
        &entries.at(static_cast<std::size_t>(requested_grade)));
    return candidate->grade == requested_grade ? candidate : 0;
}
