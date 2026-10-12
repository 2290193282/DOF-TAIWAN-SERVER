#include "old_data/db/skill/db_update_item_making_skill.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/skill/item_making_skill_payload.h"
#include "infra/network/packet/stream.h"


// IDA_EA: 0x844F74E
// IDA_NAME: _ZN24DB_UpdateItemMakingSkillC2Ev
// DEMANGLED: DB_UpdateItemMakingSkill::DB_UpdateItemMakingSkill(void)
DB_UpdateItemMakingSkill::DB_UpdateItemMakingSkill() {}

// IDA_EA: 0x845BC26
// IDA_NAME: _ZN24DB_UpdateItemMakingSkillD2Ev
// DEMANGLED: DB_UpdateItemMakingSkill::~DB_UpdateItemMakingSkill()
// IDA_EA: 0x845BC56
// IDA_NAME: _ZN24DB_UpdateItemMakingSkillD0Ev
// DEMANGLED: DB_UpdateItemMakingSkill::~DB_UpdateItemMakingSkill()
DB_UpdateItemMakingSkill::~DB_UpdateItemMakingSkill() {}

// IDA_EA: 0x8440F3E
// IDA_NAME: _ZN24DB_UpdateItemMakingSkill8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateItemMakingSkill::dispatch(int,int,Stream *)
bool DB_UpdateItemMakingSkill::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    SIG_ITEM_MAKING_SKILL* payload =
        stream->GetOutBuffer<SIG_ITEM_MAKING_SKILL>();
    db_handle->set_query(
        "upDate item_making_skill_info set weapon=%d, cloth=%d, leather=%d, light_armor=%d, heavy_armor=%d, plate=%d, amulet=%d, wrist=%d, ring=%d, support=%d, magic_stone=%d where charac_no =%u",
        payload->weapon,
        payload->cloth,
        payload->leather,
        payload->light_armor,
        payload->heavy_armor,
        payload->plate,
        payload->amulet,
        payload->wrist,
        payload->ring,
        payload->support,
        payload->magic_stone,
        payload->charac_no);
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            return true;
        }
        db_handle->set_query(
            "inSert into item_making_skill_info (charac_no, weapon, cloth, leather, light_armor, heavy_armor, plate, amulet, wrist, ring, support, magic_stone) values(%u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )",
            payload->charac_no,
            payload->weapon,
            payload->cloth,
            payload->leather,
            payload->light_armor,
            payload->heavy_armor,
            payload->plate,
            payload->amulet,
            payload->wrist,
            payload->ring,
            payload->support,
            payload->magic_stone);
        if (db_handle->exec(true) == true) {
            return true;
        }
        cMyTrace trace(
            "virtual bool DB_UpdateItemMakingSkill::dispatch(int, int, Stream*)",
            40949,
            5);
        trace("DB_UpdateItemMakingSkill::InsertItemMakingSkill insert, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    cMyTrace trace(
        "virtual bool DB_UpdateItemMakingSkill::dispatch(int, int, Stream*)",
        40926,
        5);
    trace("DB_UpdateItemMakingSkill::UpdateItemMakingSkill update, exec() ERROR charac_no=%u",
          payload->charac_no);
    return false;
}
