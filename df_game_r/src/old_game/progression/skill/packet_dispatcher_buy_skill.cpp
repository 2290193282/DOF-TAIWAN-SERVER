#include "old_game/progression/skill/packet_dispatcher_buy_skill.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/history_log_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

buySkillInfo* BuyInfos(stOnBuySkillParam& param) {
    return param.buy_skill_infos;
}

bool IsGuildSkill(unsigned char skill_index) {
    return skill_index > 0xc7u && skill_index <= 0xd0u;
}

bool SkillTreeMatches(const CUser* user, unsigned char requested_tree) {
    const int tree = user->GetCurCharacSkillTreeIndex();
    return (tree != -1 && tree != 0) ? requested_tree == 1u
                                     : requested_tree != 1u;
}

unsigned char ErrorCodeForBuySkillResult(int result) {
    switch (result) {
        case -6: return 92;
        case -5: return 91;
        case -4: return 1;
        case -3: return 18;
        case -2: return 22;
        case -1: return 21;
        default: return 0;
    }
}

const char* BuySkillResultReason(int result) {
    switch (result) {
        case -6: return "guild_level_rejected";
        case -5: return "guild_fund_rejected";
        case -4: return "invalid_user_or_skill_cost";
        case -3: return "growth_level_or_prerequisite_rejected";
        case -2: return "insufficient_sp_or_sfp";
        case -1: return "skill_or_slot_not_found";
        default: return result >= 0 ? "accepted" : "unknown_reject";
    }
}

void LogBuySkillDecision(const CUser* user,
                         unsigned char skill_index,
                         unsigned char requested_level,
                         unsigned char requested_tree,
                         int result,
                         const char* reason) {
    if (!user) return;
    LogManager::logFormat(
        1,
        "skill_buy_diag.cpp",
        "DisPatcher_BuySkill::master_new_skill_process",
        0,
        "[SKILL_BUY][CMD41] skill=%u requested_level=%u requested_tree=%u "
        "current_tree=%d job=%d raw_grow=%d first_grow=%d second_grow=%d "
        "result=%d reason=%s",
        static_cast<unsigned int>(skill_index),
        static_cast<unsigned int>(requested_level),
        static_cast<unsigned int>(requested_tree),
        user->GetCurCharacSkillTreeIndex(),
        user->get_charac_job(),
        static_cast<int>(user->getCurCharacGrowType()),
        user->getCurCharFirstGrowType(),
        static_cast<int>(user->getCurCharSecondGrowType()),
        result,
        reason ? reason : "unknown");
}

}  // namespace

// IDA_EA: 0x85950C4
// IDA_NAME: _ZN19DisPatcher_BuySkillC2Ev
// DEMANGLED: DisPatcher_BuySkill::DisPatcher_BuySkill(void)
DisPatcher_BuySkill::DisPatcher_BuySkill()
    : IPacketDispatcher<MSG_BUY_SKILL, stOnBuySkillParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8231F96
// IDA_NAME: _ZN19DisPatcher_BuySkillD2Ev
// DEMANGLED: DisPatcher_BuySkill::~DisPatcher_BuySkill()
// IDA_EA: 0x8231FC6
// IDA_NAME: _ZN19DisPatcher_BuySkillD0Ev
// DEMANGLED: DisPatcher_BuySkill::~DisPatcher_BuySkill()
DisPatcher_BuySkill::~DisPatcher_BuySkill() {}

// IDA_EA: 0x8231FE4
// IDA_NAME: _ZN19DisPatcher_BuySkill11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_BuySkill::check_state(CUser *,PacketBuf &)
int DisPatcher_BuySkill::check_state(CUser* user, PacketBuf& packet) {
    (void)packet;
    return user && user->get_state() == 3
               ? 0
               : LineFunc(265,
                          "virtual int DisPatcher_BuySkill::check_state(CUser*, PacketBuf&)",
                          0, 0);
}

// IDA_EA: 0x81BED4A
// IDA_NAME: _ZN19DisPatcher_BuySkill4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_BuySkill::read(PacketBuf &,MSG_BASE &)
int DisPatcher_BuySkill::read(PacketBuf& packet, MSG_BASE& base) {
    MSG_BUY_SKILL& message = static_cast<MSG_BUY_SKILL&>(base);
    if (packet.get_byte(message.skill_tree_index) != 1)
        return LineFunc(891, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
    if (packet.get_byte(message.request_count) != 1)
        return LineFunc(894, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
    if (!message.request_count)
        return LineFunc(897, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
    if (message.request_count > 70u)
        return LineFunc(899, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
    for (unsigned int index = 0; index < message.request_count; ++index) {
        MSG_BUY_SKILL::SkillRequest& request = message.requests[index];
        if (packet.get_byte(request.skill_index) != 1)
            return LineFunc(904, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
        if (packet.get_byte(request.skill_kind) != 1)
            return LineFunc(906, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
        if (request.skill_kind >= 2u)
            return LineFunc(909, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
        if (packet.get_byte(request.skill_level) != 1)
            return LineFunc(911, "virtual int DisPatcher_BuySkill::read(PacketBuf&, MSG_BASE&)", 0, 0);
        if (!request.skill_level) request.skill_level = 1;
    }
    return 0;
}

// IDA_EA: 0x81BF8BE
// IDA_NAME: _ZN19DisPatcher_BuySkill11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_BuySkill::check_error(CUser *,MSG_BASE &)
int DisPatcher_BuySkill::check_error(CUser* user,
                                     const MSG_BUY_SKILL& message) const {
    if (!user) return -1;
    for (unsigned int index = 0; index < message.request_count; ++index) {
        if (IsGuildSkill(message.requests[index].skill_index) &&
            ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                user, RestrictCategory::AuthMobile, 30) != 0) {
            return 209;
        }
    }
    return 0;
}

// IDA_EA: 0x81BF952
// IDA_NAME: _ZN19DisPatcher_BuySkill24master_new_skill_processEP5CUsercR12buySkillInfo
// DEMANGLED: DisPatcher_BuySkill::master_new_skill_process(CUser *,char,buySkillInfo &)
// buySkillInfo's first two bytes are the requested
// skill/index level; its dwords at +4/+8 retain result slot and result level.
int DisPatcher_BuySkill::master_new_skill_process(CUser* user,
                                                   unsigned char skill_tree,
                                                   buySkillInfo& info) const {
    const unsigned char requested_skill = info.job;
    const unsigned char requested_level = info.tree;
    if (!IsGuildSkill(requested_skill)) {
        if (!SkillTreeMatches(user, skill_tree)) {
            const int current_tree = user->GetCurCharacSkillTreeIndex();
            LogBuySkillDecision(user, requested_skill, requested_level,
                                skill_tree, current_tree,
                                "skill_tree_mismatch");
            return current_tree;
        }
        info.skill_id = user->getCurCharacSkillW()->master_new_skill(
            requested_skill, user->get_charac_level(), user->get_charac_job(),
            user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
            requested_level);
        if (info.skill_id >= 0)
            info.skill_level = user->getCurCharacSkillR()->get_skill_level(
                info.skill_id);
    } else if (user->IsGuildMaster() || user->IsSubGuildMaster()) {
        STGuildDBInfoOnly* const guild = user->GetGuildDBInfo();
        if (!guild) {
            info.skill_id = -1;
            return info.skill_id;
        }
        int gold = 0;
        int level = 0;
        short gsp = 0;
        info.skill_id = user->getCurCharacSkillR()->master_new_guild_skill(
            requested_skill, user->get_charac_job(),
            user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
            *guild, user->getCurCharacMoney(), gold, level, gsp, guild->raw_42);
        info.skill_level = level;
        if (info.skill_id >= 0 && GlobalData::s_guild_proxy_mgr) {
            CGuildServerProxy* const proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            if (proxy) {
                proxy->SendBuyGuildSkill(
                    static_cast<unsigned int>(user->get_charac_no(-1)),
                    static_cast<unsigned int>(user->get_charac_guildkey()),
                    requested_skill, level, gsp, static_cast<unsigned int>(gold));
            }
        }
    }
    LogBuySkillDecision(user, requested_skill, requested_level, skill_tree,
                        info.skill_id, BuySkillResultReason(info.skill_id));
    if (info.skill_id >= 0)
        return HistoryLog::WriteLearnSkill(
            user->history_stream(), requested_skill,
            user->getCurCharacSkillR()->get_skill_level(info.skill_id));
    return info.skill_id;
}

// IDA_EA: 0x81BFC5C
// IDA_NAME: _ZN19DisPatcher_BuySkill20refund_skill_processEP5CUsercR12buySkillInfo
// DEMANGLED: DisPatcher_BuySkill::refund_skill_process(CUser *,char,buySkillInfo &)
int DisPatcher_BuySkill::refund_skill_process(CUser* user,
                                               unsigned char skill_tree,
                                               buySkillInfo& info) const {
    int reason = 0;
    info.skill_id = user->getCurCharacSkillW()->refund_skill(
        info.job, user->get_charac_level(), user->get_charac_job(),
        user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
        info.tree, reason);
    if (info.skill_id >= 0)
        info.skill_level = user->getCurCharacSkillR()->get_skill_level(info.skill_id);
    (void)skill_tree;
    return info.skill_id;
}

// IDA_EA: 0x81BFDF0
// IDA_NAME: _ZN19DisPatcher_BuySkill29try_master_new_skills_processEP5CUsercR12buySkillInfo
// DEMANGLED: DisPatcher_BuySkill::try_master_new_skills_process(CUser *,char,buySkillInfo &)
int DisPatcher_BuySkill::try_master_new_skills_process(
    CUser* user, unsigned char skill_tree, buySkillInfo& info) const {
    if (IsGuildSkill(info.job) || !SkillTreeMatches(user, skill_tree))
        return user->GetCurCharacSkillTreeIndex();
    info.skill_id = user->getCurCharacSkillW()->try_master_new_skills(
        info.job, user->get_charac_level(), user->get_charac_job(),
        user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
        info.tree);
    return info.skill_id;
}

// IDA_EA: 0x81BFD3C
// IDA_NAME: _ZN19DisPatcher_BuySkill25try_refund_skills_processEP5CUsercR12buySkillInfo
// DEMANGLED: DisPatcher_BuySkill::try_refund_skills_process(CUser *,char,buySkillInfo &)
int DisPatcher_BuySkill::try_refund_skills_process(
    CUser* user, unsigned char skill_tree, buySkillInfo& info) const {
    int reason = 0;
    info.skill_id = user->getCurCharacSkillW()->try_refund_skill(
        info.job, user->get_charac_level(), user->get_charac_job(),
        user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
        info.tree, reason);
    (void)skill_tree;
    return info.skill_id;
}

// IDA_EA: 0x81BEFE8
// IDA_NAME: _ZN19DisPatcher_BuySkill7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_BuySkill::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_BuySkill::process(CUser* user, MSG_BASE& base, ParamBase& raw) {
    MSG_BUY_SKILL& message = static_cast<MSG_BUY_SKILL&>(base);
    stOnBuySkillParam& param = static_cast<stOnBuySkillParam&>(raw);
    const int error = check_error(user, message);
    if (error > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BUY_SKILL,
                                 static_cast<unsigned char>(error));
        return -1;
    }
    if (error < 0)
        return LineFunc(1007,
                        "virtual int DisPatcher_BuySkill::process(CUser*, MSG_BASE&, ParamBase&)",
                        error, static_cast<unsigned int>(user->get_acc_id()));

    buySkillInfo* const infos = BuyInfos(param);
    param.field_355 = message.request_count;
    if (message.request_count == 1u) {
        infos[0].job = message.requests[0].skill_index;
        infos[0].tree = message.requests[0].skill_level;
        if (message.requests[0].skill_kind)
            refund_skill_process(user, message.skill_tree_index, infos[0]);
        else
            master_new_skill_process(user, message.skill_tree_index, infos[0]);
        param.field_354 = infos[0].job;
        param.field_350 = infos[0].skill_id;
        return error;
    }

    SkillSlot* const skills = user->getCurCharacSkillW();
    skills->set_parent(user);
    if (skills->backupSkill(static_cast<ENUM_SKILL_TREE_KIND>(
            message.skill_tree_index)) != 1) {
        param.field_350 = -1;
        param.field_354 = infos[0].job;
        return 0;
    }
    for (unsigned int index = 0; index < message.request_count; ++index) {
        infos[index].job = message.requests[index].skill_index;
        infos[index].tree = message.requests[index].skill_level;
        if (message.requests[index].skill_kind)
            try_refund_skills_process(user, message.skill_tree_index, infos[index]);
        else
            try_master_new_skills_process(user, message.skill_tree_index, infos[index]);
        param.field_354 = infos[index].job;
        param.field_350 = infos[index].skill_id;
        if (param.field_350 < 0) return 0;
    }
    for (unsigned int index = 0; index < message.request_count; ++index) {
        infos[index].job = message.requests[index].skill_index;
        infos[index].tree = message.requests[index].skill_level;
        if (message.requests[index].skill_kind)
            refund_skill_process(user, message.skill_tree_index, infos[index]);
        else
            master_new_skill_process(user, message.skill_tree_index, infos[index]);
        param.field_354 = infos[index].job;
        param.field_350 = infos[index].skill_id;
        if (param.field_350 < 0) return 0;
    }
    return error;
}

// IDA_EA: 0x81BF468
// IDA_NAME: _ZN19DisPatcher_BuySkill4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_BuySkill::send(CUser *,ParamBase &)
void DisPatcher_BuySkill::send(CUser* user, ParamBase& raw) {
    stOnBuySkillParam& param = static_cast<stOnBuySkillParam&>(raw);
    buySkillInfo* const infos = BuyInfos(param);
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_BUY_SKILL);
    if (param.field_350 >= 0) {
        const int tree = user->GetCurCharacSkillTreeIndex();
        SkillSlot* const skills = user->getCurCharacSkillR();
        response.put_byte(1);
        response.put_byte(tree);
        response.put_short(skills->get_remain_sp_at_index(
            static_cast<ENUM_SKILL_TREE_KIND>(tree)));
        response.put_short(skills->get_remain_sfp_at_index(
            tree == -1 || tree == 0 ? ENUM_SKILL_TREE_SFP_0
                                    : ENUM_SKILL_TREE_SFP_1));
        response.put_byte(param.field_355);
        for (unsigned int index = 0; index < param.field_355; ++index) {
            response.put_byte(IsGuildSkill(infos[index].job) ? 0xff
                                                              : infos[index].skill_id);
            response.put_byte(infos[index].job);
            response.put_byte(infos[index].skill_level);
            STSkillCommandData command;
            if (user->findSkillCommandData(infos[index].job, command, true)) {
                response.put_byte(1);
                response.put_byte(command.byte_02);
                for (unsigned int command_index = 0;
                     command_index < command.byte_02;
                     ++command_index)
                    response.put_byte(command.command_storage[command_index]);
            } else {
                response.put_byte(0);
            }
        }
    } else {
        response.put_byte(0);
        response.put_byte(ErrorCodeForBuySkillResult(param.field_350));
        response.put_byte(param.field_354);
    }
    response.finalize(true);
    user->Send(response);
}

// This state-3 specialization owns its base vtable slots in the original
// CMD41 translation unit.  Keep the generic implementation here so the
// generated ABI symbols are emitted once, alongside the derived dispatcher.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
template class IPacketDispatcher<MSG_BUY_SKILL,
                                 stOnBuySkillParam,
                                 static_cast<ch_state>(3)>;
