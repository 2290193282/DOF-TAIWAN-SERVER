#include "old_data/db/skill/db_save_skill.h"

#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/core/mysql.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>


extern const char* get_skill_table_name(unsigned char table_index)
    __asm__("_Z20get_skill_table_nameh");

namespace {

void CountPersistedSkills(const _Mastered_skill* tree,
                          unsigned int count,
                          unsigned int* indexed_slots,
                          unsigned int* positive_levels,
                          unsigned int* level_sum) {
    *indexed_slots = 0u;
    *positive_levels = 0u;
    *level_sum = 0u;
    for (unsigned int index = 0; index < count; ++index) {
        if (!tree[index].skill_index) continue;
        ++*indexed_slots;
        const unsigned int level =
            static_cast<unsigned char>(tree[index].skill_level);
        if (level) ++*positive_levels;
        *level_sum += level;
    }
}

}  // namespace

// IDA_EA: 0x8417770
// IDA_NAME: _ZN12DB_SaveSkillC2Ev
// DEMANGLED: DB_SaveSkill::DB_SaveSkill(void)
DB_SaveSkill::DB_SaveSkill() {}

// IDA_EA: 0x841778C
// IDA_NAME: _ZN12DB_SaveSkillD2Ev
// DEMANGLED: DB_SaveSkill::~DB_SaveSkill()
// IDA_EA: 0x84177BC
// IDA_NAME: _ZN12DB_SaveSkillD0Ev
// DEMANGLED: DB_SaveSkill::~DB_SaveSkill() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveSkill::~DB_SaveSkill() {}

// IDA_EA: 0x84177DA
// IDA_NAME: _ZN12DB_SaveSkill8dispatchEiiP6Stream
// DEMANGLED: DB_SaveSkill::dispatch(int,int,Stream *)
bool DB_SaveSkill::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_LOAD_SKILL payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    return SaveSkill(&payload) == true;
}
// IDA_EA: 0x841782A
// IDA_NAME: _ZN12DB_SaveSkill9SaveSkillEP14SIG_LOAD_SKILL
// DEMANGLED: DB_SaveSkill::SaveSkill(SIG_LOAD_SKILL *)
bool DB_SaveSkill::SaveSkill(SIG_LOAD_SKILL* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    char* skill_slot_blob = 0;
    if (put_compressed_blob_data(db_handle,
                                 0,
                                 reinterpret_cast<char*>(
                                     payload_data->first_tree),
                                 sizeof(payload_data->first_tree),
                                 &skill_slot_blob) != 1) {
        return false;
    }
    char* skill_slot_2nd_blob = 0;
    if (put_compressed_blob_data(db_handle,
                                 1,
                                 reinterpret_cast<char*>(
                                     payload_data->second_tree),
                                 sizeof(payload_data->second_tree),
                                 &skill_slot_2nd_blob) != 1) {
        return false;
    }
    char* skill_command_blob = 0;
    if (put_compressed_blob_data(db_handle,
                                 4,
                                 reinterpret_cast<char*>(
                                     payload_data->skill_commands),
                                 sizeof(payload_data->skill_commands),
                                 &skill_command_blob) != 1) {
        return false;
    }

    const char* skill_table =
        get_skill_table_name(payload_data->skill_table_index);
    db_handle->set_query(
        "upDate %s set remain_sp=%u,skill_slot='%s',remain_sp_2nd=%u, skill_slot_2nd='%s', remain_sfp_1st=%u, remain_sfp_2nd=%u, skill_command ='%s',script_version=%d where charac_no=%u",
        skill_table,
        payload_data->remain_sp,
        skill_slot_blob,
        payload_data->remain_sp_second,
        skill_slot_2nd_blob,
        payload_data->remain_sfp_first,
        payload_data->remain_sfp_second,
        skill_command_blob,
        payload_data->script_version,
        payload_data->charac_no);
    const bool saved = db_handle->exec(true) == true;
    unsigned int first_slots = 0u;
    unsigned int first_positive = 0u;
    unsigned int first_levels = 0u;
    unsigned int second_slots = 0u;
    unsigned int second_positive = 0u;
    unsigned int second_levels = 0u;
    CountPersistedSkills(payload_data->first_tree, 204u, &first_slots,
                         &first_positive, &first_levels);
    CountPersistedSkills(payload_data->second_tree, 204u, &second_slots,
                         &second_positive, &second_levels);
    LogManager::logFormat(
        1, "skill_persistence_diag.cpp", "DB_SaveSkill::SaveSkill", 0,
        "[SKILL_PERSIST][DB_SAVE] table=%u script_version=%u "
        "tree0_slots=%u tree0_positive=%u tree0_levels=%u "
        "tree1_slots=%u tree1_positive=%u tree1_levels=%u "
        "sp0=%u sp1=%u sfp0=%u sfp1=%u result=%d affected=%u",
        static_cast<unsigned int>(payload_data->skill_table_index),
        static_cast<unsigned int>(payload_data->script_version),
        first_slots, first_positive, first_levels,
        second_slots, second_positive, second_levels,
        payload_data->remain_sp, payload_data->remain_sp_second,
        payload_data->remain_sfp_first, payload_data->remain_sfp_second,
        saved ? 1 : 0,
        saved ? db_handle->getAffectedRowCount() : 0u);
    return saved;
}

// IDA_EA: 0x8417A08
// IDA_NAME: _ZN12DB_SaveSkill10SendResultEiiP14SIG_LOAD_SKILL
// DEMANGLED: DB_SaveSkill::SendResult(int,int,SIG_LOAD_SKILL *)
bool DB_SaveSkill::SendResult(int signal,
                              int user_idx,
                              SIG_LOAD_SKILL* payload) {
    (void)signal;
    (void)user_idx;
    (void)payload;
    return true;
}
