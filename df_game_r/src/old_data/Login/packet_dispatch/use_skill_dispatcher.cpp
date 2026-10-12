#include "old_data/Login/packet_dispatch/use_skill_dispatcher.h"

#include <limits.h>

#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/log/format/log_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int DisPatcher_UseSkill::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int DisPatcher_UseSkill::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x82864FC
// IDA_NAME: _ZN26CHackLog_InvalidUsingSkill17IsValidUsingSkillEPK25SKILL_USING_VALIDATE_INFO
// DEMANGLED: CHackLog_InvalidUsingSkill::IsValidUsingSkill(SKILL_USING_VALIDATE_INFO const*)
bool CHackLog_InvalidUsingSkill::IsValidUsingSkill(
    const SKILL_USING_VALIDATE_INFO* info) {
    CSkill* const skill = G_CDataManager()->find_skill(info->charac_job,
                                                        info->level);
    if (!skill) {
        return false;
    }
    if (IsSkillMatchFirstGrowType(skill, info->first_grow_type)) {
        return true;
    }
    if (IsSkillMatchSecondGrowType(skill, info->first_grow_type,
                                   info->second_grow_type)) {
        return true;
    }
    return skill->IsStealable(info->first_grow_type, info->second_grow_type) !=
           0;
}

// IDA_EA: 0x82865CA
// IDA_NAME: _ZN26CHackLog_InvalidUsingSkill25IsSkillMatchFirstGrowTypeEPK6CSkillc
// DEMANGLED: CHackLog_InvalidUsingSkill::IsSkillMatchFirstGrowType(CSkill const*,char)
bool CHackLog_InvalidUsingSkill::IsSkillMatchFirstGrowType(
    const CSkill* skill, char first_grow_type) {
    for (int index = 0; index <= 5; ++index) {
        if (skill->fit_grow_type_[index] == first_grow_type) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x8286612
// IDA_NAME: _ZN26CHackLog_InvalidUsingSkill26IsSkillMatchSecondGrowTypeEPK6CSkillcc
// DEMANGLED: CHackLog_InvalidUsingSkill::IsSkillMatchSecondGrowType(CSkill const*,char,char)
bool CHackLog_InvalidUsingSkill::IsSkillMatchSecondGrowType(
    const CSkill* skill, char first_grow_type, char second_grow_type) {
    return skill->get_second_growtype_max_level(first_grow_type,
                                                second_grow_type) > 0;
}

// IDA_EA: 0x8286656
// IDA_NAME: _ZN26CHackLog_InvalidUsingSkill11GetHackTypeEv
// DEMANGLED: CHackLog_InvalidUsingSkill::GetHackType(void)
int CHackLog_InvalidUsingSkill::GetHackType() {
    return 825;
}

// IDA_EA: 0x85A77D8
// IDA_NAME: _ZN6CParty9use_skillEP5CUserii
// DEMANGLED: CParty::use_skill(CUser *,int,int)
int CParty::use_skill(CUser* user, int skill_slot, int skill_index) {
    return OnUsedSkill(user, skill_slot, skill_index);
}

// IDA_EA: 0x85B5F8E
// IDA_NAME: _ZN6CParty11OnUsedSkillEP5CUserii
// DEMANGLED: CParty::OnUsedSkill(CUser *,int,int)
int CParty::OnUsedSkill(CUser* user, int skill_slot, int) {
    IsCollectDungeonStatistic();
    for (int seat = 0; seat <= 3; ++seat) {
        if (members_[seat].user == user) {
            ++battle_data_.skill_use_count(seat);
            if (isCheckUseSkill(user->get_charac_job(), skill_slot)) {
                ++skill_quest_use_counts_[seat];
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8257BD8
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8257D28
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8257D32
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x823696C / 0x823699C
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EED2Ev /
//           _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EED0Ev
// IDA_EA: 0x825ACF4
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x859820A
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_USE_SKILL15stUseSkillParamL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_USE_SKILL, stUseSkillParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595310
// IDA_NAME: _ZN19DisPatcher_UseSkillC2Ev
// DEMANGLED: DisPatcher_UseSkill::DisPatcher_UseSkill(void)
DisPatcher_UseSkill::DisPatcher_UseSkill()
    : IPacketDispatcher<MSG_USE_SKILL, stUseSkillParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82327B8
// IDA_NAME: _ZN19DisPatcher_UseSkillD2Ev
// IDA_EA: 0x82327E8
// IDA_NAME: _ZN19DisPatcher_UseSkillD0Ev
// DEMANGLED: DisPatcher_UseSkill::~DisPatcher_UseSkill()
DisPatcher_UseSkill::~DisPatcher_UseSkill() {}

// IDA_EA: 0x81CAC5E
// IDA_NAME: _ZN19DisPatcher_UseSkill4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_UseSkill::read(PacketBuf &,MSG_BASE &)
int DisPatcher_UseSkill::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USE_SKILL& request = static_cast<MSG_USE_SKILL&>(message);
    if (packet.get_byte(request.request_kind) != 1) {
        return LineFunc(8142, kReadSignature, 0, 0);
    }
    if (packet.get_byte(request.skill_slot) != 1) {
        return LineFunc(8144, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.skill_index) != 1) {
        return LineFunc(8146, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.request_value) == 1) {
        return 0;
    }
    return LineFunc(8153, kReadSignature, 0, 0);
}

// IDA_EA: 0x81CAD80
// IDA_NAME: _ZN19DisPatcher_UseSkill11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseSkill::check_error(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_UseSkill::check_error(CUser* user, MSG_BASE&, ParamBase&) {
    return user->get_state() == 5 ? 0 : INT_MAX;
}

// IDA_EA: 0x81CADAA
// IDA_NAME: _ZN19DisPatcher_UseSkill7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseSkill::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_UseSkill::process(CUser* user, MSG_BASE& message,
                                 ParamBase& param) {
    stUseSkillParam& response = static_cast<stUseSkillParam&>(param);
    response.result = check_error(user, message, param);
    if (response.result > 0) {
        return 0;
    }
    if (response.result < 0) {
        return LineFunc(8178, kProcessSignature, response.result,
                        user->get_acc_id());
    }

    const MSG_USE_SKILL& request =
        static_cast<const MSG_USE_SKILL&>(message);
    const SKILL_USING_VALIDATE_INFO info(
        user->get_charac_job(), request.skill_slot,
        user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType());
    if (!CHackLog_InvalidUsingSkill::IsValidUsingSkill(&info)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(
                CHackLog_InvalidUsingSkill::GetHackType()),
            1, request.skill_slot, 0);
    }
    CParty* const party = user->GetParty();
    if (party) {
        party->use_skill(user, request.skill_slot, request.skill_index);
    }
    return 0;
}

// IDA_EA: 0x81CAF28
// IDA_NAME: _ZN19DisPatcher_UseSkill4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_UseSkill::send(CUser *,ParamBase &)
void DisPatcher_UseSkill::send(CUser*, ParamBase& param) {
    (void)static_cast<stUseSkillParam&>(param).result;
}
