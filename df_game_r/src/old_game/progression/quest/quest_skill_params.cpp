#include "old_game/progression/quest/quest_skill_params.h"

#include "old_game/progression/quest/quest_state_runtime.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <new>


typedef char stSelectQuestParam_Size_Must_Be_0x28[
    (sizeof(stSelectQuestParam) == 0x28) ? 1 : -1];
typedef char stUseSkillParam_Size_Must_Be_0x08[
    (sizeof(stUseSkillParam) == 0x08) ? 1 : -1];
typedef char stOnBuySkillParam_Size_Must_Be_0x358[
    (sizeof(stOnBuySkillParam) == 0x358) ? 1 : -1];
typedef char stOnChangeSkillParam_Size_Must_Be_0x08[
    (sizeof(stOnChangeSkillParam) == 0x08) ? 1 : -1];
typedef char stRefundSkill_Size_Must_Be_0x1c[
    (sizeof(stRefundSkill) == 0x1c) ? 1 : -1];
typedef char stSkillCommandParam_Size_Must_Be_0x08[
    (sizeof(stSkillCommandParam) == 0x08) ? 1 : -1];
typedef char stChangeAnotherSkillTree_Size_Must_Be_0x0c[
    (sizeof(stChangeAnotherSkillTree) == 0x0c) ? 1 : -1];
typedef char stSendClearedMission_Size_Must_Be_0x10[
    (sizeof(stSendClearedMission) == 0x10) ? 1 : -1];
typedef char stRequestAssaultPrice_Size_Must_Be_0x0c[
    (sizeof(stRequestAssaultPrice) == 0x0c) ? 1 : -1];
typedef char stRequestBlackListParam_Size_Must_Be_0x08[
    (sizeof(stRequestBlackListParam) == 0x08) ? 1 : -1];
typedef char stRegisterToBlackListParam_Size_Must_Be_0x08[
    (sizeof(stRegisterToBlackListParam) == 0x08) ? 1 : -1];
typedef char stDeleteToBlackListParam_Size_Must_Be_0x08[
    (sizeof(stDeleteToBlackListParam) == 0x08) ? 1 : -1];
typedef char stGuildBoardParam_Size_Must_Be_0x08[
    (sizeof(stGuildBoardParam) == 0x08) ? 1 : -1];
typedef char stRequestDisjointItemParam_Size_Must_Be_0x08[
    (sizeof(stRequestDisjointItemParam) == 0x08) ? 1 : -1];
typedef char stRequestIngameAdvertisement_Size_Must_Be_0x0c[
    (sizeof(stRequestIngameAdvertisement) == 0x0c) ? 1 : -1];
typedef char stRequestMemberEnterParam_Size_Must_Be_0x08[
    (sizeof(stRequestMemberEnterParam) == 0x08) ? 1 : -1];
typedef char saveQuestNotifyParam_Size_Must_Be_0x08[
    (sizeof(saveQuestNotifyParam) == 0x08) ? 1 : -1];
typedef char stGrowthPowerParam_Size_Must_Be_0x08[
    (sizeof(stGrowthPowerParam) == 0x08) ? 1 : -1];
typedef char stComboSkill_Size_Must_Be_0x08[
    (sizeof(stComboSkill) == 0x08) ? 1 : -1];
typedef char stSkillInit_Size_Must_Be_0x0c[
    (sizeof(stSkillInit) == 0x0c) ? 1 : -1];

// IDA_EA: 0x83480B4
// IDA_NAME: _ZN18stSelectQuestParamC2EP5CUser
// DEMANGLED: stSelectQuestParam::stSelectQuestParam(CUser *)
stSelectQuestParam::stSelectQuestParam(CUser* user)
    : charac_level(user->get_charac_level()),
      charac_job(user->get_charac_job()),
      first_grow_type(user->getCurCharFirstGrowType()),
      second_grow_type(user->getCurCharSecondGrowType()),
      pvp_grade(user->get_pvp_grade()),
      power_side(static_cast<uint8_t>(user->getPowerSide())),
      pad_15(),
      expert_job_type(user->GetCurCharacExpertJobType()),
      expert_job_level(user->GetCurExpertJobLevel(
          static_cast<int>(user->GetCurCharacExpertJobExp()))),
      urgent_quest_index(user->getCurCharacQuestR()->urgent_quest_index) {}

// IDA_EA: 0x8230F18
// IDA_NAME: _ZN15stUseSkillParamC2Ev
// DEMANGLED: stUseSkillParam::stUseSkillParam(void)
stUseSkillParam::stUseSkillParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247D08
// IDA_NAME: _ZN15stUseSkillParamD2Ev
// DEMANGLED: stUseSkillParam::~stUseSkillParam()
stUseSkillParam::~stUseSkillParam() {}

// IDA_EA: 0x8247D38
// IDA_NAME: _ZN15stUseSkillParamD0Ev
// DEMANGLED: stUseSkillParam::~stUseSkillParam() deleting destructor
void stUseSkillParam_deleting_destructor(stUseSkillParam* self) {
    if (self) {
        self->~stUseSkillParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230A04
// IDA_NAME: _ZN17stOnBuySkillParamC2Ev
// DEMANGLED: stOnBuySkillParam::stOnBuySkillParam(void)
stOnBuySkillParam::stOnBuySkillParam()
    : ParamBase(),
      field_04(0),
      buy_skill_infos(),
      field_350(-1),
      field_354(0),
      field_355(0) {
    std::memset(buy_skill_infos, 0, sizeof(buy_skill_infos));
}

// IDA_EA: 0x8248458
// IDA_NAME: _ZN17stOnBuySkillParamD2Ev
// DEMANGLED: stOnBuySkillParam::~stOnBuySkillParam()
stOnBuySkillParam::~stOnBuySkillParam() {}

// IDA_EA: 0x8248488
// IDA_NAME: _ZN17stOnBuySkillParamD0Ev
// DEMANGLED: stOnBuySkillParam::~stOnBuySkillParam() deleting destructor
void stOnBuySkillParam_deleting_destructor(stOnBuySkillParam* self) {
    if (self) {
        self->~stOnBuySkillParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230A94
// IDA_NAME: _ZN20stOnChangeSkillParamC2Ev
// DEMANGLED: stOnChangeSkillParam::stOnChangeSkillParam(void)
stOnChangeSkillParam::stOnChangeSkillParam()
    : ParamBase(),
      field_04(-1),
      field_05(-1) {}

// IDA_EA: 0x824840A
// IDA_NAME: _ZN20stOnChangeSkillParamD2Ev
// DEMANGLED: stOnChangeSkillParam::~stOnChangeSkillParam()
stOnChangeSkillParam::~stOnChangeSkillParam() {}

// IDA_EA: 0x824843A
// IDA_NAME: _ZN20stOnChangeSkillParamD0Ev
// DEMANGLED: stOnChangeSkillParam::~stOnChangeSkillParam() deleting destructor
void stOnChangeSkillParam_deleting_destructor(stOnChangeSkillParam* self) {
    if (self) {
        self->~stOnChangeSkillParam();
        operator delete(self);
    }
}

// IDA_EA: 0x824E97E
// IDA_NAME: _ZN13stRefundSkillC2Ev
// DEMANGLED: stRefundSkill::stRefundSkill(void)
stRefundSkill::stRefundSkill()
    : ParamBase() {}

// IDA_EA: 0x824E99A
// IDA_NAME: _ZN13stRefundSkillD2Ev
// DEMANGLED: stRefundSkill::~stRefundSkill()
stRefundSkill::~stRefundSkill() {}

// IDA_EA: 0x824E9CA
// IDA_NAME: _ZN13stRefundSkillD0Ev
// DEMANGLED: stRefundSkill::~stRefundSkill() deleting destructor
void stRefundSkill_deleting_destructor(stRefundSkill* self) {
    if (self) {
        self->~stRefundSkill();
        operator delete(self);
    }
}

// IDA_EA: 0x8231500
// IDA_NAME: _ZN19stSkillCommandParamC2Ev
// DEMANGLED: stSkillCommandParam::stSkillCommandParam(void)
stSkillCommandParam::stSkillCommandParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247174
// IDA_NAME: _ZN19stSkillCommandParamD2Ev
// DEMANGLED: stSkillCommandParam::~stSkillCommandParam()
stSkillCommandParam::~stSkillCommandParam() {}

// IDA_EA: 0x82471A4
// IDA_NAME: _ZN19stSkillCommandParamD0Ev
// DEMANGLED: stSkillCommandParam::~stSkillCommandParam() deleting destructor
void stSkillCommandParam_deleting_destructor(stSkillCommandParam* self) {
    if (self) {
        self->~stSkillCommandParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8253042
// IDA_NAME: _ZN24stChangeAnotherSkillTreeC2Ev
// DEMANGLED: stChangeAnotherSkillTree::stChangeAnotherSkillTree(void)
stChangeAnotherSkillTree::stChangeAnotherSkillTree()
    : ParamBase() {}

// IDA_EA: 0x825305E
// IDA_NAME: _ZN24stChangeAnotherSkillTreeD2Ev
// DEMANGLED: stChangeAnotherSkillTree::~stChangeAnotherSkillTree()
stChangeAnotherSkillTree::~stChangeAnotherSkillTree() {}

// IDA_EA: 0x825308E
// IDA_NAME: _ZN24stChangeAnotherSkillTreeD0Ev
// DEMANGLED: stChangeAnotherSkillTree::~stChangeAnotherSkillTree()
// deleting destructor
void stChangeAnotherSkillTree_deleting_destructor(
    stChangeAnotherSkillTree* self) {
    if (self) {
        self->~stChangeAnotherSkillTree();
        operator delete(self);
    }
}

// IDA_EA: 0x8250D1E
// IDA_NAME: _ZN20stSendClearedMissionC2Ev
// DEMANGLED: stSendClearedMission::stSendClearedMission(void)
stSendClearedMission::stSendClearedMission()
    : ParamBase(),
      cleared_missions() {}

// IDA_EA: 0x8250D6C
// IDA_NAME: _ZN20stSendClearedMissionD2Ev
// DEMANGLED: stSendClearedMission::~stSendClearedMission()
stSendClearedMission::~stSendClearedMission() {}

// IDA_EA: 0x8250DCE
// IDA_NAME: _ZN20stSendClearedMissionD0Ev
// DEMANGLED: stSendClearedMission::~stSendClearedMission() deleting destructor
void stSendClearedMission_deleting_destructor(stSendClearedMission* self) {
    if (self) {
        self->~stSendClearedMission();
        operator delete(self);
    }
}

// IDA_EA: 0x824C756
// IDA_NAME: _ZN21stRequestAssaultPriceC2Ev
// DEMANGLED: stRequestAssaultPrice::stRequestAssaultPrice(void)
stRequestAssaultPrice::stRequestAssaultPrice()
    : ParamBase() {}

// IDA_EA: 0x824C772
// IDA_NAME: _ZN21stRequestAssaultPriceD2Ev
// DEMANGLED: stRequestAssaultPrice::~stRequestAssaultPrice()
stRequestAssaultPrice::~stRequestAssaultPrice() {}

// IDA_EA: 0x824C7A2
// IDA_NAME: _ZN21stRequestAssaultPriceD0Ev
// DEMANGLED: stRequestAssaultPrice::~stRequestAssaultPrice()
// deleting destructor
void stRequestAssaultPrice_deleting_destructor(
    stRequestAssaultPrice* self) {
    if (self) {
        self->~stRequestAssaultPrice();
        operator delete(self);
    }
}

// IDA_EA: 0x82311CC
// IDA_NAME: _ZN23stRequestBlackListParamC2Ev
// DEMANGLED: stRequestBlackListParam::stRequestBlackListParam(void)
stRequestBlackListParam::stRequestBlackListParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82477DA
// IDA_NAME: _ZN23stRequestBlackListParamD2Ev
// DEMANGLED: stRequestBlackListParam::~stRequestBlackListParam()
stRequestBlackListParam::~stRequestBlackListParam() {}

// IDA_EA: 0x824780A
// IDA_NAME: _ZN23stRequestBlackListParamD0Ev
// DEMANGLED: stRequestBlackListParam::~stRequestBlackListParam()
// deleting destructor
void stRequestBlackListParam_deleting_destructor(
    stRequestBlackListParam* self) {
    if (self) {
        self->~stRequestBlackListParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231180
// IDA_NAME: _ZN26stRegisterToBlackListParamC2Ev
// DEMANGLED: stRegisterToBlackListParam::stRegisterToBlackListParam(void)
stRegisterToBlackListParam::stRegisterToBlackListParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247876
// IDA_NAME: _ZN26stRegisterToBlackListParamD2Ev
// DEMANGLED: stRegisterToBlackListParam::~stRegisterToBlackListParam()
stRegisterToBlackListParam::~stRegisterToBlackListParam() {}

// IDA_EA: 0x82478A6
// IDA_NAME: _ZN26stRegisterToBlackListParamD0Ev
// DEMANGLED: stRegisterToBlackListParam::~stRegisterToBlackListParam()
// deleting destructor
void stRegisterToBlackListParam_deleting_destructor(
    stRegisterToBlackListParam* self) {
    if (self) {
        self->~stRegisterToBlackListParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82311A6
// IDA_NAME: _ZN24stDeleteToBlackListParamC2Ev
// DEMANGLED: stDeleteToBlackListParam::stDeleteToBlackListParam(void)
stDeleteToBlackListParam::stDeleteToBlackListParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247828
// IDA_NAME: _ZN24stDeleteToBlackListParamD2Ev
// DEMANGLED: stDeleteToBlackListParam::~stDeleteToBlackListParam()
stDeleteToBlackListParam::~stDeleteToBlackListParam() {}

// IDA_EA: 0x8247858
// IDA_NAME: _ZN24stDeleteToBlackListParamD0Ev
// DEMANGLED: stDeleteToBlackListParam::~stDeleteToBlackListParam()
// deleting destructor
void stDeleteToBlackListParam_deleting_destructor(
    stDeleteToBlackListParam* self) {
    if (self) {
        self->~stDeleteToBlackListParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231526
// IDA_NAME: _ZN17stGuildBoardParamC2Ev
// DEMANGLED: stGuildBoardParam::stGuildBoardParam(void)
stGuildBoardParam::stGuildBoardParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247126
// IDA_NAME: _ZN17stGuildBoardParamD2Ev
// DEMANGLED: stGuildBoardParam::~stGuildBoardParam()
stGuildBoardParam::~stGuildBoardParam() {}

// IDA_EA: 0x8247156
// IDA_NAME: _ZN17stGuildBoardParamD0Ev
// DEMANGLED: stGuildBoardParam::~stGuildBoardParam()
// deleting destructor
void stGuildBoardParam_deleting_destructor(stGuildBoardParam* self) {
    if (self) {
        self->~stGuildBoardParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82312A4
// IDA_NAME: _ZN26stRequestDisjointItemParamC2Ev
// DEMANGLED: stRequestDisjointItemParam::stRequestDisjointItemParam(void)
stRequestDisjointItemParam::stRequestDisjointItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247606
// IDA_NAME: _ZN26stRequestDisjointItemParamD2Ev
// DEMANGLED: stRequestDisjointItemParam::~stRequestDisjointItemParam()
stRequestDisjointItemParam::~stRequestDisjointItemParam() {}

// IDA_EA: 0x8247636
// IDA_NAME: _ZN26stRequestDisjointItemParamD0Ev
// DEMANGLED: stRequestDisjointItemParam::~stRequestDisjointItemParam()
// deleting destructor
void stRequestDisjointItemParam_deleting_destructor(
    stRequestDisjointItemParam* self) {
    if (self) {
        self->~stRequestDisjointItemParam();
        operator delete(self);
    }
}

// IDA_EA: 0x824A15C
// IDA_NAME: _ZN28stRequestIngameAdvertisementC2Ev
// DEMANGLED: stRequestIngameAdvertisement::stRequestIngameAdvertisement(void)
stRequestIngameAdvertisement::stRequestIngameAdvertisement()
    : ParamBase(),
      result(0),
      response_count(0),
      reserved_09() {}

// IDA_EA: 0x824A178
// IDA_NAME: _ZN28stRequestIngameAdvertisementD2Ev
// DEMANGLED: stRequestIngameAdvertisement::~stRequestIngameAdvertisement()
stRequestIngameAdvertisement::~stRequestIngameAdvertisement() {}

// IDA_EA: 0x824A1A8
// IDA_NAME: _ZN28stRequestIngameAdvertisementD0Ev
// DEMANGLED: stRequestIngameAdvertisement::~stRequestIngameAdvertisement()
// deleting destructor
void stRequestIngameAdvertisement_deleting_destructor(
    stRequestIngameAdvertisement* self) {
    if (self) {
        self->~stRequestIngameAdvertisement();
        operator delete(self);
    }
}

// IDA_EA: 0x8261756
// IDA_NAME: _ZN20saveQuestNotifyParamC2Ev
// DEMANGLED: saveQuestNotifyParam::saveQuestNotifyParam(void)
saveQuestNotifyParam::saveQuestNotifyParam()
    : ParamBase() {}

// IDA_EA: 0x8261772
// IDA_NAME: _ZN20saveQuestNotifyParamD2Ev
// DEMANGLED: saveQuestNotifyParam::~saveQuestNotifyParam()
saveQuestNotifyParam::~saveQuestNotifyParam() {}

// IDA_EA: 0x82617A2
// IDA_NAME: _ZN20saveQuestNotifyParamD0Ev
// DEMANGLED: saveQuestNotifyParam::~saveQuestNotifyParam()
// deleting destructor
void saveQuestNotifyParam_deleting_destructor(saveQuestNotifyParam* self) {
    if (self) {
        self->~saveQuestNotifyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82318AC
// IDA_NAME: _ZN18stGrowthPowerParamC2Ev
// DEMANGLED: stGrowthPowerParam::stGrowthPowerParam(void)
stGrowthPowerParam::stGrowthPowerParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8246BE2
// IDA_NAME: _ZN18stGrowthPowerParamD2Ev
// DEMANGLED: stGrowthPowerParam::~stGrowthPowerParam()
stGrowthPowerParam::~stGrowthPowerParam() {}

// IDA_EA: 0x8246C12
// IDA_NAME: _ZN18stGrowthPowerParamD0Ev
// DEMANGLED: stGrowthPowerParam::~stGrowthPowerParam()
// deleting destructor
void stGrowthPowerParam_deleting_destructor(stGrowthPowerParam* self) {
    if (self) {
        self->~stGrowthPowerParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82604FA
// IDA_NAME: _ZN12stComboSkillC2Ev
// DEMANGLED: stComboSkill::stComboSkill(void)
stComboSkill::stComboSkill()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8260CFE
// IDA_NAME: _ZN12stComboSkillD2Ev
// DEMANGLED: stComboSkill::~stComboSkill()
stComboSkill::~stComboSkill() {}

// IDA_EA: 0x8260D2E
// IDA_NAME: _ZN12stComboSkillD0Ev
// DEMANGLED: stComboSkill::~stComboSkill()
// deleting destructor
void stComboSkill_deleting_destructor(stComboSkill* self) {
    if (self) {
        self->~stComboSkill();
        operator delete(self);
    }
}

// IDA_EA: 0x8231D32
// IDA_NAME: _ZN11stSkillInitC2Ev
// DEMANGLED: stSkillInit::stSkillInit(void)
stSkillInit::stSkillInit()
    : ParamBase(),
      dword_04(0),
      byte_08(0) {}

// IDA_EA: 0x82466AE
// IDA_NAME: _ZN11stSkillInitD2Ev
// DEMANGLED: stSkillInit::~stSkillInit()
stSkillInit::~stSkillInit() {}

// IDA_EA: 0x82466DE
// IDA_NAME: _ZN11stSkillInitD0Ev
// DEMANGLED: stSkillInit::~stSkillInit()
// deleting destructor
void stSkillInit_deleting_destructor(stSkillInit* self) {
    if (self) {
        self->~stSkillInit();
        operator delete(self);
    }
}
