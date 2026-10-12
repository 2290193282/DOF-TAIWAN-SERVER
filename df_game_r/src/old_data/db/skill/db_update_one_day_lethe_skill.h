#ifndef OLDSRV_DB_DB_UPDATE_ONE_DAY_LETHE_SKILL_H_
#define OLDSRV_DB_DB_UPDATE_ONE_DAY_LETHE_SKILL_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"


// IDA ABI: the three producers at 0x84D94C4, 0x84D98A4 and 0x85C4842
// populate these exact slots before DB signal 92.  The 4-byte stream_context
// slot is carried by the wire type but is not read by this DB dispatcher.
struct SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD {
    uint32_t account_id;       // +0x000
    uint32_t charac_no;        // +0x004
    uint32_t stream_context;   // +0x008
    uint8_t state;             // +0x00C
    char skill_slot[408];      // +0x00D
    uint8_t lethe_flag;        // +0x1A5
    uint8_t skill_tree_kind;   // +0x1A6
    uint8_t reserved_1a7;      // +0x1A7 alignment byte
};

// IDA ABI: dispatch @0x8433ECA sends this exact 0x1A0-byte response to
// DISPATCH_Q.  memcpy length 0x199 is the skill slot plus lethe_flag.
struct SIG_PREMIUM_LETHE_FLAG {
    uint32_t success;          // +0x000
    uint8_t state;             // +0x004
    char skill_slot[408];      // +0x005
    uint8_t lethe_flag;        // +0x19D
    uint8_t skill_tree_kind;   // +0x19E
    uint8_t reserved_19f;      // +0x19F alignment byte
};

static_assert(sizeof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD) == 0x1A8u,
              "SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD size must stay 0x1A8");
static_assert(offsetof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD, state) == 0x0Cu,
              "premium Lethe request state offset must stay 0x0C");
static_assert(offsetof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD, skill_slot) ==
                  0x0Du,
              "premium Lethe request skill_slot offset must stay 0x0D");
static_assert(offsetof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD, lethe_flag) ==
                  0x1A5u,
              "premium Lethe request flag offset must stay 0x1A5");
static_assert(offsetof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD,
                       skill_tree_kind) == 0x1A6u,
              "premium Lethe request tree-kind offset must stay 0x1A6");
static_assert(offsetof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD,
                       reserved_1a7) == 0x1A7u,
              "premium Lethe request alignment byte must stay 0x1A7");
static_assert(sizeof(SIG_PREMIUM_LETHE_FLAG) == 0x1A0u,
              "SIG_PREMIUM_LETHE_FLAG size must stay 0x1A0");
static_assert(offsetof(SIG_PREMIUM_LETHE_FLAG, skill_slot) == 0x05u &&
                  offsetof(SIG_PREMIUM_LETHE_FLAG, lethe_flag) == 0x19Du &&
                  offsetof(SIG_PREMIUM_LETHE_FLAG, skill_tree_kind) == 0x19Eu &&
                  offsetof(SIG_PREMIUM_LETHE_FLAG, reserved_19f) == 0x19Fu,
              "premium Lethe response tree-kind offset must stay 0x19E");


// IDA_EA: 0x8433ECA/0x843410C/0x84341FC/0x8434340/0x84345A4/
// 0x844EE8E/0x845D6A8/0x845D6D8.
// IDA unit: 08_quest_skill_growth/DB_UpdateOneDayLetheSkill.
class DB_UpdateOneDayLetheSkill : public DBDispatcher {
public:
    DB_UpdateOneDayLetheSkill();
    virtual ~DB_UpdateOneDayLetheSkill();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    bool ProcessInitLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload);
    bool ProcessUseLethe(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload);
    bool ProcessSkillConfirm(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload, int user_idx);
    bool ProcessInitLetheSkill(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* payload);
};

#endif  // OLDSRV_DB_DB_UPDATE_ONE_DAY_LETHE_SKILL_H_
