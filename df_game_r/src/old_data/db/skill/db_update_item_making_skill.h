#ifndef OLDSRV_OLD_DATA_DB_SKILL_DB_UPDATE_ITEM_MAKING_SKILL_H_
#define OLDSRV_OLD_DATA_DB_SKILL_DB_UPDATE_ITEM_MAKING_SKILL_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_UpdateItemMakingSkill.
class DB_UpdateItemMakingSkill : public DBDispatcher {
public:
    DB_UpdateItemMakingSkill();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    virtual ~DB_UpdateItemMakingSkill();
};

#if defined(__i386__)
static_assert(sizeof(DB_UpdateItemMakingSkill) == 4u,
              "DB_UpdateItemMakingSkill IDA ABI size must stay 4");
#endif

#endif  // OLDSRV_OLD_DATA_DB_SKILL_DB_UPDATE_ITEM_MAKING_SKILL_H_
