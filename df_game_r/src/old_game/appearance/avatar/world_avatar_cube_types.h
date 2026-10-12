#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AVATAR_CUBE_TYPES_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AVATAR_CUBE_TYPES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x891AA3A in the companion source.


#include <stdint.h>

#include <utility>
#include <vector>


// IDA unit: 05_world_enter_village/STAncientRareAvatarCube.
struct STAncientRareAvatarCube {
    STAncientRareAvatarCube();
    STAncientRareAvatarCube(const STAncientRareAvatarCube& rhs);
    ~STAncientRareAvatarCube();

    STAncientRareAvatarCube& operator=(const STAncientRareAvatarCube& rhs);

    std::vector<std::pair<int, int> > common_items;
    uint32_t grade0_count;
    uint32_t grade1_count;
    uint32_t grade2_count;
    uint32_t grade3_count;
    uint32_t grade4_count;
    uint32_t grade5_count;
    uint32_t grade6_count;
    uint32_t grade7_count;
    uint32_t grade8_count;
    uint32_t grade9_count;
    std::vector<std::pair<int, int> > grade0_items;
    std::vector<std::pair<int, int> > grade1_items;
    std::vector<std::pair<int, int> > grade2_items;
    std::vector<std::pair<int, int> > grade3_items;
    std::vector<std::pair<int, int> > grade4_items;
    std::vector<std::pair<int, int> > grade5_items;
    std::vector<std::pair<int, int> > grade6_items;
    std::vector<std::pair<int, int> > grade7_items;
    std::vector<std::pair<int, int> > grade8_items;
    std::vector<std::pair<int, int> > grade9_items;
};


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AVATAR_CUBE_TYPES_H_
