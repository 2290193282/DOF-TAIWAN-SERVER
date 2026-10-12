#include "old_data/db/skill/db_update_combo_skill.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



extern const char* getComboSkillTableName(int skill_tree_kind)
    __asm__("_Z22getComboSkillTableName20ENUM_SKILL_TREE_KIND");

namespace {

typedef char DB_UpdateComboSkill_SIG_Size_Must_Be_0xB2[
    (sizeof(SIG_COMBO_SKILL) == 0xB2) ? 1 : -1];

bool BeginComboSkillTransaction(MySQL* db_handle) {
    return db_handle && db_handle->set_query("start transaction") &&
           db_handle->exec(true);
}

void RollbackComboSkillTransaction(MySQL* db_handle) {
    if (db_handle && db_handle->set_query("rollback")) {
        (void)db_handle->exec(true);
    }
}

bool CommitComboSkillTransaction(MySQL* db_handle) {
    return db_handle && db_handle->set_query("commit") &&
           db_handle->exec(true);
}

}  // namespace

// IDA_EA: 0x844FE86
// IDA_NAME: _ZN19DB_UpdateComboSkillC2Ev
// DEMANGLED: DB_UpdateComboSkill::DB_UpdateComboSkill(void)
DB_UpdateComboSkill::DB_UpdateComboSkill() {}

// IDA_EA: 0x845AC00
// IDA_NAME: _ZN19DB_UpdateComboSkillD2Ev
// DEMANGLED: DB_UpdateComboSkill::~DB_UpdateComboSkill()
// IDA_EA: 0x845AC30
// IDA_NAME: _ZN19DB_UpdateComboSkillD0Ev
// DEMANGLED: DB_UpdateComboSkill::~DB_UpdateComboSkill() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateComboSkill::~DB_UpdateComboSkill() {}

// IDA_EA: 0x844A8EA
// IDA_NAME: _ZN19DB_UpdateComboSkill8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateComboSkill::dispatch(int,int,Stream *)
bool DB_UpdateComboSkill::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream || !GlobalData::s_db_mgr) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) {
        return false;
    }
    SIG_COMBO_SKILL* payload = stream->GetOutBuffer<SIG_COMBO_SKILL>();
    if (!payload) {
        return false;
    }

    // CMD506 carries both Black Knight trees in one signal.  Persist the two
    // fixed six-container sets as one unit: exposing tree 0 while tree 1
    // failed is indistinguishable from losing part of ASDFGH on reselect.
    if (!BeginComboSkillTransaction(db_handle)) {
        return false;
    }
    if (setComboSkill(db_handle,
                      payload->charac_no,
                      payload->first_tree_count,
                      payload->first_tree_combos,
                      ENUM_SKILL_TREE_0) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateComboSkill::dispatch(int, int, Stream*)",
            47916,
            5);
        trace("DB_UpdateComboSkill::dispatch, setComboSkill ERROR charac_no=%u",
              payload->charac_no);
        RollbackComboSkillTransaction(db_handle);
        return false;
    }
    if (setComboSkill(db_handle,
                      payload->charac_no,
                      payload->second_tree_count,
                      payload->second_tree_combos,
                      ENUM_SKILL_TREE_1) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateComboSkill::dispatch(int, int, Stream*)",
            47923,
            5);
        trace("DB_UpdateComboSkill::dispatch, setComboSkill ERROR charac_no=%u",
              payload->charac_no);
        RollbackComboSkillTransaction(db_handle);
        return false;
    }
    if (!CommitComboSkillTransaction(db_handle)) {
        RollbackComboSkillTransaction(db_handle);
        return false;
    }
    return true;
}

// IDA_EA: 0x844AA4C
// IDA_NAME: _ZN19DB_UpdateComboSkill13setComboSkillEP5MySQLjhP15comoboSkillData20ENUM_SKILL_TREE_KIND
// DEMANGLED: DB_UpdateComboSkill::setComboSkill(MySQL *,unsigned int,unsigned char,comoboSkillData *,ENUM_SKILL_TREE_KIND)
bool DB_UpdateComboSkill::setComboSkill(MySQL* db_handle,
                                        unsigned int charac_no,
                                        unsigned char count,
                                        comoboSkillData* combo_skill_data,
                                        ENUM_SKILL_TREE_KIND skill_tree_kind) {
    if (!db_handle || !combo_skill_data) {
        return false;
    }
    if (static_cast<unsigned int>(skill_tree_kind) >= 2) {
        cMyTrace trace(
            "bool DB_UpdateComboSkill::setComboSkill(MySQL*, unsigned int, unsigned char, comoboSkillData*, ENUM_SKILL_TREE_KIND)",
            47938,
            5);
        trace("DB_UpdateComboSkill::setComboSkill kind(%d) error characNo(%u)",
              skill_tree_kind,
              charac_no);
        return false;
    }

    for (int index = 0; count > index && index <= 5; ++index) {
        const comoboSkillData& record = combo_skill_data[index];
        const int combo_idx = static_cast<int16_t>(record.skill_id);
        const int value1 = static_cast<int16_t>(record.command[0]);
        const int value2 = static_cast<int16_t>(record.command[1]);
        const int value3 = static_cast<int16_t>(record.command[2]);
        const int value4 = static_cast<int16_t>(record.command[3]);
        const int value5 = static_cast<int16_t>(record.command[4]);
        const int value6 = static_cast<int16_t>(record.command[5]);
        const char* table_name = getComboSkillTableName(skill_tree_kind);
        if (!table_name) {
            return false;
        }
        // A successful UPDATE reports zero affected rows when the persisted
        // values are already identical; that does not mean the row is absent.
        // Use one idempotent statement so repeated saves cannot fall through
        // to a duplicate-key INSERT.
        if (!db_handle->set_query(
            "inSert into %s(charac_no, combo_idx, value1, value2, value3, value4, value5, value6) values(%u, %d, %d, %d, %d, %d, %d, %d) on duplicate key upDate value1=values(value1), value2=values(value2), value3=values(value3), value4=values(value4), value5=values(value5), value6=values(value6)",
            table_name,
            charac_no,
            combo_idx,
            value1,
            value2,
            value3,
            value4,
            value5,
            value6) || db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
