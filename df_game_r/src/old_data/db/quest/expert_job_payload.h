#ifndef OLDSRV_OLD_DATA_DB_QUEST_EXPERT_JOB_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_QUEST_EXPERT_JOB_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

// IDA payload used by slot-0 charac_expand persistence and DBTask slots
// 384/385.  Stream specializations consume exactly 0x30 bytes.
struct SIG_EXPERT_JOB {
    uint32_t charac_no;             // +0x00
    uint16_t expert_job_giveup_cnt; // +0x04
    uint8_t reserved_06[2];         // +0x06
    int32_t expert_job_info;        // +0x08, disjoint-machine endurance
    int32_t expert_job_info_ex;     // +0x0C, disjoint-machine grade
    uint8_t recipe[30];             // +0x10, 240 learned-recipe bits
    uint8_t reserved_2e[2];         // +0x2E
};

#pragma pack(pop)

static_assert(sizeof(SIG_EXPERT_JOB) == 0x30u,
              "SIG_EXPERT_JOB old-server size must stay 0x30");
static_assert(offsetof(SIG_EXPERT_JOB, expert_job_giveup_cnt) == 0x04u,
              "SIG_EXPERT_JOB giveup count offset must stay 0x04");
static_assert(offsetof(SIG_EXPERT_JOB, expert_job_info) == 0x08u,
              "SIG_EXPERT_JOB info offset must stay 0x08");
static_assert(offsetof(SIG_EXPERT_JOB, expert_job_info_ex) == 0x0Cu,
              "SIG_EXPERT_JOB info-ex offset must stay 0x0C");
static_assert(offsetof(SIG_EXPERT_JOB, recipe) == 0x10u,
              "SIG_EXPERT_JOB recipe offset must stay 0x10");

#endif  // OLDSRV_OLD_DATA_DB_QUEST_EXPERT_JOB_PAYLOAD_H_
