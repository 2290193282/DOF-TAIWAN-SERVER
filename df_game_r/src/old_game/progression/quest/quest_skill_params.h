#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PARAMS_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PARAMS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8230F18, 0x83480B4, and saveQuestNotifyParam
// 0x8261756/0x8261772/0x82617A2 in the companion source.


#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "old_game/social/party/packet_dispatcher_group_business_043.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "infra/network/packet/message_base.h"


class CUser;

// IDA unit: 08_quest_skill_growth/stSelectQuestParam.
struct stSelectQuestParam {
    explicit stSelectQuestParam(CUser* user);

    int charac_level;       // +0x00
    int charac_job;         // +0x04
    int first_grow_type;    // +0x08
    int second_grow_type;   // +0x0C
    int pvp_grade;          // +0x10
    uint8_t power_side;     // +0x14
    uint8_t pad_15[7];      // +0x15
    int expert_job_type;    // +0x1C
    int expert_job_level;   // +0x20
    uint32_t urgent_quest_index; // +0x24
};

// IDA unit: 08_quest_skill_growth/stUseSkillParam.
struct stUseSkillParam : public ParamBase {
    stUseSkillParam();
    virtual ~stUseSkillParam();

    int result;
};

void stUseSkillParam_deleting_destructor(stUseSkillParam* self);

// IDA unit: 08_quest_skill_growth/stOnBuySkillParam.
struct stOnBuySkillParam : public ParamBase {
    stOnBuySkillParam();
    virtual ~stOnBuySkillParam();

    uint8_t field_04;
    uint8_t pad_05[3];
    buySkillInfo buy_skill_infos[70];
    int field_350;
    uint8_t field_354;
    uint8_t field_355;
    uint8_t pad_356[2];
};

void stOnBuySkillParam_deleting_destructor(stOnBuySkillParam* self);

#if defined(__i386__)
static_assert(offsetof(stOnBuySkillParam, buy_skill_infos) == 0x08u &&
                  offsetof(stOnBuySkillParam, field_350) == 0x350u &&
                  sizeof(stOnBuySkillParam) == 0x358u,
              "stOnBuySkillParam named skill rows must retain the old ABI");
#endif

// IDA unit: 08_quest_skill_growth/stOnChangeSkillParam.
struct stOnChangeSkillParam : public ParamBase {
    stOnChangeSkillParam();
    virtual ~stOnChangeSkillParam();

    int8_t field_04;
    int8_t field_05;
    uint8_t pad_06[2];
};

void stOnChangeSkillParam_deleting_destructor(stOnChangeSkillParam* self);

// IDA unit: 08_quest_skill_growth/stRefundSkill.
struct stRefundSkill : public ParamBase {
    stRefundSkill();
    virtual ~stRefundSkill();

    // Dispatcher_RefundSkill writes these response slots directly.  The old
    // empty declaration concealed the 0x1c-byte state-0 template object.
    int8_t skill_tree_index;     // IDA +0x04
    uint8_t special_skill;       // IDA +0x05
    uint16_t remain_point;       // IDA +0x06 (SP or SFP)
    int skill_slot;              // IDA +0x08
    uint8_t skill_index;         // IDA +0x0c
    uint8_t skill_level;         // IDA +0x0d
    uint8_t pad_0e[2];           // IDA +0x0e
    int inventory_item_id;       // IDA +0x10
    uint8_t refund_count;        // IDA +0x14
    uint8_t pad_15[3];           // IDA +0x15
    int error_code;              // IDA +0x18; check_error/refund reason
};

void stRefundSkill_deleting_destructor(stRefundSkill* self);

// IDA unit: 08_quest_skill_growth/stSkillCommandParam.
struct stSkillCommandParam : public ParamBase {
    stSkillCommandParam();
    virtual ~stSkillCommandParam();

    int result;
};

void stSkillCommandParam_deleting_destructor(stSkillCommandParam* self);

// IDA unit: 08_quest_skill_growth/stChangeAnotherSkillTree.
struct stChangeAnotherSkillTree : public ParamBase {
    stChangeAnotherSkillTree();
    virtual ~stChangeAnotherSkillTree();

    // Live state-0 template storage at 0x82530AC proves the response ABI is
    // ParamBase plus this result dword and response tree byte.  C2 deliberately
    // initializes only ParamBase; these fields are written by CMD264.
    int result;               // IDA +0x04
    uint8_t skill_tree_index; // IDA +0x08
};

void stChangeAnotherSkillTree_deleting_destructor(
    stChangeAnotherSkillTree* self);

// IDA unit: 08_quest_skill_growth/stSendClearedMission.
struct stSendClearedMission : public ParamBase {
    stSendClearedMission();
    virtual ~stSendClearedMission();

    std::vector<int> cleared_missions;
};

void stSendClearedMission_deleting_destructor(stSendClearedMission* self);

// IDA unit: 08_quest_skill_growth/stRequestAssaultPrice.
struct stRequestAssaultPrice : public ParamBase {
    stRequestAssaultPrice();
    virtual ~stRequestAssaultPrice();

    // Dispatcher_RequestAssaultPrice stores its result at +0x04 and the
    // (already equipment-discounted) quoted price at +0x08.  The historical
    // empty declaration hid these live dispatch-template slots.
    int result_code;       // IDA +0x04
    int requested_money;   // IDA +0x08
};

void stRequestAssaultPrice_deleting_destructor(
    stRequestAssaultPrice* self);

// IDA unit: 08_quest_skill_growth/stRequestBlackListParam.
struct stRequestBlackListParam : public ParamBase {
    stRequestBlackListParam();
    virtual ~stRequestBlackListParam();

    int result;
};

void stRequestBlackListParam_deleting_destructor(
    stRequestBlackListParam* self);

// IDA unit: 13_database_dispatch/stRegisterToBlackListParam.
struct stRegisterToBlackListParam : public ParamBase {
    stRegisterToBlackListParam();
    virtual ~stRegisterToBlackListParam();

    int result;
};

void stRegisterToBlackListParam_deleting_destructor(
    stRegisterToBlackListParam* self);

// IDA unit: 13_database_dispatch/stDeleteToBlackListParam.
struct stDeleteToBlackListParam : public ParamBase {
    stDeleteToBlackListParam();
    virtual ~stDeleteToBlackListParam();

    int result;
};

void stDeleteToBlackListParam_deleting_destructor(
    stDeleteToBlackListParam* self);

// IDA unit: 99_misc_global/stGuildBoardParam.
struct stGuildBoardParam : public ParamBase {
    stGuildBoardParam();
    virtual ~stGuildBoardParam();

    int result;
};

void stGuildBoardParam_deleting_destructor(stGuildBoardParam* self);

// IDA unit: 08_quest_skill_growth/stRequestDisjointItemParam.
struct stRequestDisjointItemParam : public ParamBase {
    stRequestDisjointItemParam();
    virtual ~stRequestDisjointItemParam();

    int result;
};

void stRequestDisjointItemParam_deleting_destructor(
    stRequestDisjointItemParam* self);

// Native 140828 CMD486 result state.  The retail request is a batch and the
// currently proven response is a cursor-complete empty batch.  Do not retain
// the 130627 single-advertisement object merely to preserve an obsolete stack
// extent; non-empty 140828 response records remain evidence-blocked.
struct stRequestIngameAdvertisement : public ParamBase {
    stRequestIngameAdvertisement();
    virtual ~stRequestIngameAdvertisement();

    int result;
    uint8_t response_count;
    uint8_t reserved_09[3];
};
#if defined(__i386__)
static_assert(offsetof(stRequestIngameAdvertisement, result) == 0x04u &&
                  offsetof(stRequestIngameAdvertisement, response_count) ==
                      0x08u &&
                  sizeof(stRequestIngameAdvertisement) == 0x0cu,
              "native CMD486 result state ABI changed");
#endif

void stRequestIngameAdvertisement_deleting_destructor(
    stRequestIngameAdvertisement* self);

// IDA unit: 08_quest_skill_growth/saveQuestNotifyParam.
struct saveQuestNotifyParam : public ParamBase {
    saveQuestNotifyParam();
    virtual ~saveQuestNotifyParam();

    int result;
};

void saveQuestNotifyParam_deleting_destructor(saveQuestNotifyParam* self);

// IDA unit: 08_quest_skill_growth/stGrowthPowerParam.
struct stGrowthPowerParam : public ParamBase {
    stGrowthPowerParam();
    virtual ~stGrowthPowerParam();

    int result;
};

void stGrowthPowerParam_deleting_destructor(stGrowthPowerParam* self);

// IDA unit: 08_quest_skill_growth/stComboSkill.
struct stComboSkill : public ParamBase {
    stComboSkill();
    virtual ~stComboSkill();

    int result;
};

void stComboSkill_deleting_destructor(stComboSkill* self);

// IDA unit: 08_quest_skill_growth/stSkillInit.
struct stSkillInit : public ParamBase {
    stSkillInit();
    virtual ~stSkillInit();

    uint32_t dword_04;
    uint8_t byte_08;
    uint8_t pad_09[3];
};

void stSkillInit_deleting_destructor(stSkillInit* self);


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PARAMS_H_
