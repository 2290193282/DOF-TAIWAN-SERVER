#include "old_game/appearance/avatar/world_avatar_cube_types.h"


typedef char STAncientRareAvatarCube_Size_Must_Be_0xAC[
    (sizeof(STAncientRareAvatarCube) == 0xAC) ? 1 : -1];

// IDA_EA: 0x891AA3A
// IDA_NAME: _ZN23STAncientRareAvatarCubeC2Ev
// DEMANGLED: STAncientRareAvatarCube::STAncientRareAvatarCube(void)
STAncientRareAvatarCube::STAncientRareAvatarCube()
    : grade0_count(0),
      grade1_count(0),
      grade2_count(0),
      grade3_count(0),
      grade4_count(0),
      grade5_count(0),
      grade6_count(0),
      grade7_count(0),
      grade8_count(0),
      grade9_count(0) {
    grade0_items.clear();
    grade1_items.clear();
    grade2_items.clear();
    grade3_items.clear();
    grade4_items.clear();
    grade5_items.clear();
    grade6_items.clear();
    grade7_items.clear();
    grade8_items.clear();
    grade9_items.clear();
    common_items.clear();
}

// IDA_EA: 0x83345DC
// IDA_NAME: _ZN23STAncientRareAvatarCubeD2Ev
// DEMANGLED: STAncientRareAvatarCube::~STAncientRareAvatarCube()
STAncientRareAvatarCube::~STAncientRareAvatarCube() {}

// IDA_EA: 0x891C106
// IDA_NAME: _ZN23STAncientRareAvatarCubeC2ERKS_
// DEMANGLED: STAncientRareAvatarCube::STAncientRareAvatarCube(STAncientRareAvatarCube const&)
STAncientRareAvatarCube::STAncientRareAvatarCube(
    const STAncientRareAvatarCube& rhs)
    : common_items(rhs.common_items),
      grade0_count(rhs.grade0_count),
      grade1_count(rhs.grade1_count),
      grade2_count(rhs.grade2_count),
      grade3_count(rhs.grade3_count),
      grade4_count(rhs.grade4_count),
      grade5_count(rhs.grade5_count),
      grade6_count(rhs.grade6_count),
      grade7_count(rhs.grade7_count),
      grade8_count(rhs.grade8_count),
      grade9_count(rhs.grade9_count),
      grade0_items(rhs.grade0_items),
      grade1_items(rhs.grade1_items),
      grade2_items(rhs.grade2_items),
      grade3_items(rhs.grade3_items),
      grade4_items(rhs.grade4_items),
      grade5_items(rhs.grade5_items),
      grade6_items(rhs.grade6_items),
      grade7_items(rhs.grade7_items),
      grade8_items(rhs.grade8_items),
      grade9_items(rhs.grade9_items) {}

// IDA_EA: 0x894EB9E
// IDA_NAME: _ZN23STAncientRareAvatarCubeaSERKS_
// DEMANGLED: STAncientRareAvatarCube::operator=(STAncientRareAvatarCube const&)
STAncientRareAvatarCube& STAncientRareAvatarCube::operator=(
    const STAncientRareAvatarCube& rhs) {
    common_items = rhs.common_items;
    grade0_count = rhs.grade0_count;
    grade1_count = rhs.grade1_count;
    grade2_count = rhs.grade2_count;
    grade3_count = rhs.grade3_count;
    grade4_count = rhs.grade4_count;
    grade5_count = rhs.grade5_count;
    grade6_count = rhs.grade6_count;
    grade7_count = rhs.grade7_count;
    grade8_count = rhs.grade8_count;
    grade9_count = rhs.grade9_count;
    grade0_items = rhs.grade0_items;
    grade1_items = rhs.grade1_items;
    grade2_items = rhs.grade2_items;
    grade3_items = rhs.grade3_items;
    grade4_items = rhs.grade4_items;
    grade5_items = rhs.grade5_items;
    grade6_items = rhs.grade6_items;
    grade7_items = rhs.grade7_items;
    grade8_items = rhs.grade8_items;
    grade9_items = rhs.grade9_items;
    return *this;
}
