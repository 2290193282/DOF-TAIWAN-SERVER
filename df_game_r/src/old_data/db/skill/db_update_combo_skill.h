#ifndef OLDSRV_DB_DB_UPDATE_COMBO_SKILL_H_
#define OLDSRV_DB_DB_UPDATE_COMBO_SKILL_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_game/progression/skill/combo_skill.h"


class MySQL;

// IDA_EA: 0x844A8EA/0x844AA4C/0x844FE86/0x845AC00/0x845AC30.
// IDA unit: 08_quest_skill_growth/DB_UpdateComboSkill.
class DB_UpdateComboSkill : public DBDispatcher {
public:
    DB_UpdateComboSkill();
    virtual ~DB_UpdateComboSkill();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool setComboSkill(MySQL* db_handle,
                       unsigned int charac_no,
                       unsigned char count,
                       comoboSkillData* combo_skill_data,
                       ENUM_SKILL_TREE_KIND skill_tree_kind);
};

#endif  // OLDSRV_DB_DB_UPDATE_COMBO_SKILL_H_
