#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_ITEM_MAKING_SKILL_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_ITEM_MAKING_SKILL_H_

#include "old_game/player/core/charac_expand_cdata.h"

#include <stdint.h>


class CMTRand;
class CStackableItem;
class CUser;
enum ENUM_ITEM_MAKING_TYPE : int;

// IDA unit: 08_quest_skill_growth/CItemMakingSkill.  This is the persisted
// character expansion in slot 10; the derived object reuses CData's tail
// padding exactly as the old i386 ABI does.
class CItemMakingSkill : public charac_expand::CData {
public:
    CItemMakingSkill();
    virtual ~CItemMakingSkill();

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;

    void GainItemMakingSkillPoint(const CStackableItem* item, CUser* user);
    void UpSkillLevel(ENUM_ITEM_MAKING_TYPE type, int point, CUser* user);
    int Send_ItemMakingSkillList(CUser* user) const;
    int GetPrevExpGap(int level);
    int GetSkillLevel(ENUM_ITEM_MAKING_TYPE type, CUser* user) const;
    int GetItemMakingSkillLevel(const CStackableItem* item, CUser* user);
    int SetItemMakingSkillPoint(int type, int point, CUser* user);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    int16_t item_making_skill_[11];  // IDA +0x06..+0x1b
    CMTRand* random_;                // IDA +0x1c
};

static_assert(sizeof(CItemMakingSkill) == 0x20u,
              "CItemMakingSkill IDA ABI size must stay 0x20");

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_ITEM_MAKING_SKILL_H_
