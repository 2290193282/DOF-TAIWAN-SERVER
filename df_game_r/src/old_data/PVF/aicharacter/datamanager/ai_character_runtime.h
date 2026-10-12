#ifndef OLDSRV_DATAMANAGER_AI_CHARACTER_RUNTIME_H_
#define OLDSRV_DATAMANAGER_AI_CHARACTER_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "old_data/PVF/etc/independent_drop_types.h"

struct STAICharacterCommonScript;
struct Inven_Item;
namespace WongWork {
struct stGenerateRefData_t;
}

// IDA unit: 04_character_select_create/CAICharacter.  These records are
// populated from the AI-character PVF/RDAR script and are therefore owned by
// DataManager, even though the original call sites include C04 gameplay code.
struct CAICharacter {
    CAICharacter();
    ~CAICharacter();

    int getIdx() const;
    int getLevel() const;
    int getAppearancePoint() const;
    CAICharacter* set(int index, const STAICharacterCommonScript& script);
    // IDA_EA: 0x834A13A
    // IDA_NAME: _ZNK12CAICharacter23generateIndependentItemERKN8WongWork19stGenerateRefData_tERSt6vectorI10Inven_ItemSaIS5_EE
    // DEMANGLED: CAICharacter::generateIndependentItem(
    //             WongWork::stGenerateRefData_t const&,
    //             std::vector<Inven_Item>&)const
    int generateIndependentItem(
        const WongWork::stGenerateRefData_t& reference,
        std::vector<Inven_Item>& items) const;
    // IDA_EA: 0x834A15E
    // IDA_NAME: _ZNK12CAICharacter22generateDropAvatarItemEhhRSt6vectorI10Inven_ItemSaIS1_EE
    // DEMANGLED: CAICharacter::generateDropAvatarItem(
    //             unsigned char,unsigned char,std::vector<Inven_Item>&)const
    bool generateDropAvatarItem(uint8_t grade_index,
                                uint8_t part_index,
                                std::vector<Inven_Item>& items) const;
    bool deathTowerSpecifyItemDrop(std::vector<unsigned long>& item_ids) const;
    int getPvPWinPoint() const;
    int getRarity() const;
    std::vector<stMonsterIndependentDrop_t> getIndepentDropItem() const;
    const char* GetName() const;
    int IsHellMonster() const;

    int32_t idx;
    int32_t level;
    int32_t rarity;
    int32_t appearance_point;
    std::vector<std::pair<int32_t, int32_t> > death_tower_items;
    int32_t pvp_win_point_min;
    int32_t pvp_win_point_max;
    std::vector<stMonsterIndependentDrop_t> independent_drop_items;
    stEnamyDropAvatarInfo_t drop_avatar_info;
    std::string name;
    uint8_t hell_monster;
};

// IDA unit: 04_character_select_create/CAICharacterList.  The containers are
// intentionally ordered to preserve the original i386 offset map: the
// level-array starts at +0x18, the AP map at +0x4c0, rarity lists at +0x4d8,
// and the AP-count map at +0x520.
struct CAICharacterList {
    CAICharacterList();
    ~CAICharacterList();

    int getMinAP();
    const std::map<int, int>& getAPCount() const;
    int init();
    int destroy();
    int _add(CAICharacter* character);
    CAICharacter* get(uint32_t idx);
    CAICharacter* getByLevel(uint32_t standard_level);
    CAICharacter* getByAP(uint32_t appearance_point);

    std::map<int, CAICharacter*> characters;
    std::vector<CAICharacter*> by_level[99];
    int32_t min_ap;
    std::map<int, std::vector<CAICharacter*> > by_appearance_point;
    std::vector<CAICharacter*> by_rarity[6];
    std::map<int, int> ap_count;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(CAICharacter, drop_avatar_info) == 0x30u,
              "CAICharacter avatar data must retain IDA +0x30");
static_assert(sizeof(CAICharacter) == 0x74u,
              "CAICharacter IDA ABI size must stay 0x74");
static_assert(sizeof(CAICharacterList) == 0x538u,
              "CAICharacterList IDA ABI size must stay 0x538");
#endif

#endif  // OLDSRV_DATAMANAGER_AI_CHARACTER_RUNTIME_H_
