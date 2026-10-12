#ifndef OLDSRV_DB_DB_REQUEST_CHARAC_SKILL_INFO_H_
#define OLDSRV_DB_DB_REQUEST_CHARAC_SKILL_INFO_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


// IDA_EA: _Mastered_skill::_Mastered_skill @0x848F34C. The constructor
// clears the two consecutive byte fields; every skill blob uses unit size 2.
struct _Mastered_skill {
    _Mastered_skill();

    uint8_t skill_index;           // +0: PVF/packet skill identity
    int8_t skill_level;            // +1: signed mastered level
};

// IDA_EA: 0x84544C0/0x8237438
// IDA_NAME: _ZN6Stream12GetOutBufferI29SIG_REQUEST_CHARAC_SKILL_INFOEEPT_v
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_REQUEST_CHARAC_SKILL_INFOEEPT_v
// The old payload uses natural 4-byte alignment; its final three bytes are
// compiler tail alignment, not serialized fields.
struct SIG_REQUEST_CHARAC_SKILL_INFO {
    uint32_t charac_no;          // +0
    _Mastered_skill mastered_skills[204]; // +4
    char skill_tree_index;       // +412
};

static_assert(sizeof(_Mastered_skill) == 2u,
              "_Mastered_skill IDA ABI size must stay 2");
static_assert(offsetof(_Mastered_skill, skill_index) == 0u,
              "_Mastered_skill skill index must stay at +0");
static_assert(offsetof(_Mastered_skill, skill_level) == 1u,
              "_Mastered_skill signed skill level must stay at +1");
static_assert(sizeof(SIG_REQUEST_CHARAC_SKILL_INFO) == 416u,
              "SIG_REQUEST_CHARAC_SKILL_INFO IDA ABI size must stay 416");
static_assert(offsetof(SIG_REQUEST_CHARAC_SKILL_INFO, mastered_skills) == 4u,
              "SIG_REQUEST_CHARAC_SKILL_INFO skill offset must stay 4");
static_assert(offsetof(SIG_REQUEST_CHARAC_SKILL_INFO, skill_tree_index) ==
                  412u,
              "SIG_REQUEST_CHARAC_SKILL_INFO tree index offset must stay 412");

// IDA unit: 04_character_select_create/DB_RequestCharacSkillInfo.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at signal 765.
class DB_RequestCharacSkillInfo : public DBDispatcher {
public:
    DB_RequestCharacSkillInfo();
    virtual ~DB_RequestCharacSkillInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    bool _GetSkillTreeIndex(unsigned int charac_no,
                            char* skill_tree_index);
    bool _GetSkillInfo(unsigned int charac_no,
                       char skill_tree_index,
                       _Mastered_skill* mastered_skill);
    void sendResult(int user_idx, SIG_REQUEST_CHARAC_SKILL_INFO* payload);
};

#endif  // OLDSRV_DB_DB_REQUEST_CHARAC_SKILL_INFO_H_
