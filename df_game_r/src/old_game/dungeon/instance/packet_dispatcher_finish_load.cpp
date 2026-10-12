#include "old_game/dungeon/instance/packet_dispatcher_finish_load.h"

#include <cstring>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// CMD040 owns this concrete message/parameter template specialization.
template class IPacketDispatcher<MSG_FINISH_LOADING,
                                 stFinishLoadParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595A6C
// IDA_NAME: _ZN21DisPatcher_FinishLoadC2Ev
// DEMANGLED: DisPatcher_FinishLoad::DisPatcher_FinishLoad(void)
DisPatcher_FinishLoad::DisPatcher_FinishLoad()
    : IPacketDispatcher<MSG_FINISH_LOADING,
                        stFinishLoadParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8265540
// IDA_NAME: _ZN21DisPatcher_FinishLoadD2Ev
// DEMANGLED: DisPatcher_FinishLoad::~DisPatcher_FinishLoad()
// IDA_EA: 0x8265570
// IDA_NAME: _ZN21DisPatcher_FinishLoadD0Ev
// DEMANGLED: DisPatcher_FinishLoad::~DisPatcher_FinishLoad()
DisPatcher_FinishLoad::~DisPatcher_FinishLoad() {}

// IDA_EA: 0x8264B66
// IDA_NAME: _ZN21DisPatcher_FinishLoad4readER9PacketBufR8MSG_BASE
int DisPatcher_FinishLoad::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_FINISH_LOADING& request = static_cast<MSG_FINISH_LOADING&>(message);
    if (packet.get_int(request.inventory_checksum) != 1) {
        return LineFunc(
            1527,
            "virtual int DisPatcher_FinishLoad::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(request.skill_checksum) == 1) {
        return 0;
    }
    return LineFunc(
        1528,
        "virtual int DisPatcher_FinishLoad::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x8264BFE
// IDA_NAME: _ZN21DisPatcher_FinishLoad11check_errorEP5CUserR8MSG_BASE
int DisPatcher_FinishLoad::check_error(CUser* user, MSG_BASE& message) {
    static_cast<void>(message);
    return user->GetDebugCommand() == 1 ? 0x7fffffff : 0;
}

// IDA_EA: 0x8264C28
// IDA_NAME: _ZN21DisPatcher_FinishLoad16Party_hack_checkEP5CUser
char DisPatcher_FinishLoad::Party_hack_check(CUser* user) {
    char identical_skills = 1;
    char same_peer_job_and_skills = 1;
    char peer_prefix[16] = {};
    int baseline_job = 0;
    int baseline_grow_type = 0;
    int baseline_skills[3] = {};

    CParty* const party = user->GetParty();
    if (!party || party->get_member_count() <= 2) {
        return 0;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (party->checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = party->get_user(seat);
        if (!member) {
            continue;
        }

        if (seat == 0) {
            baseline_job = member->get_charac_job();
            baseline_grow_type = member->getCurCharFirstGrowType();

            char peer_ip[16] = {};
            member->network_.GetPeerIP2(peer_ip, sizeof(peer_ip));
            if (std::strlen(peer_ip) <= 8u) {
                identical_skills = 0;
                same_peer_job_and_skills = 0;
                break;
            }
            const char* const final_dot = std::strrchr(peer_ip, '.');
            if (!final_dot) {
                identical_skills = 0;
                same_peer_job_and_skills = 0;
                break;
            }
            std::strncpy(peer_prefix,
                         peer_ip,
                         static_cast<size_t>(final_dot - peer_ip));

            SkillSlot* const skills = member->getCurCharacSkillR();
            if (!skills) {
                identical_skills = 0;
                same_peer_job_and_skills = 0;
                break;
            }
            baseline_skills[0] = skills->get_skill_index(0);
            baseline_skills[1] = skills->get_skill_index(1);
            baseline_skills[2] = skills->get_skill_index(2);
            continue;
        }

        if (baseline_job != member->get_charac_job() ||
            baseline_grow_type != member->getCurCharFirstGrowType()) {
            identical_skills = 0;
            same_peer_job_and_skills = 0;
            break;
        }

        char peer_ip[16] = {};
        member->network_.GetPeerIP2(peer_ip, sizeof(peer_ip));
        if (std::strncmp(peer_prefix, peer_ip, std::strlen(peer_prefix)) !=
            0) {
            identical_skills = 0;
            same_peer_job_and_skills = 0;
            break;
        }

        SkillSlot* const skills = member->getCurCharacSkillR();
        if (!skills) {
            identical_skills = 0;
            same_peer_job_and_skills = 0;
            break;
        }
        if (baseline_skills[0] != skills->get_skill_index(0) ||
            baseline_skills[1] != skills->get_skill_index(1) ||
            baseline_skills[2] != skills->get_skill_index(2)) {
            identical_skills = 0;
        }
    }

    if (same_peer_job_and_skills) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x330), 1u, 0u, 0u);
        if (identical_skills) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x32f),
                1u,
                0u,
                0u);
        }
    }
    return 0;
}

// IDA_EA: 0x826503C
// IDA_NAME: _ZN21DisPatcher_FinishLoad7processEP5CUserR8MSG_BASER9ParamBase
int DisPatcher_FinishLoad::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    const MSG_FINISH_LOADING& request =
        static_cast<const MSG_FINISH_LOADING&>(message);
    stFinishLoadParam& result = static_cast<stFinishLoadParam&>(param);
    result.result_code = check_error(user, message);
    if (result.result_code > 0) {
        return 0;
    }
    if (result.result_code < 0) {
        return LineFunc(
            1738,
            "virtual int DisPatcher_FinishLoad::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result_code,
            0);
    }

    if (Global::CBossTowerFunc::FinishLoad(user) == 1) {
        return 0;
    }
    if (advancealtar::Manager::finishLoad(user) == 1) {
        return 0;
    }

    CParty* const party = user->GetParty();
    if (party && channel_zombie_matching_authority::
                     ConsumeMapOwnerFinishLoading(
                         party, user, request.inventory_checksum,
                         request.skill_checksum)) {
        return 0;
    }

    Party_hack_check(user);
    if (!party) {
        return 0;
    }
    party->finish_loading(user);
    if (party->checkStartRoom()) {
        if (user->getCurCharacInvenCheckSum() != request.inventory_checksum) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x25b),
                1u,
                0u,
                0u);
        }
        if (user->getCurCharacSkillCheckSum() != request.skill_checksum) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x25c),
                1u,
                0u,
                0u);
        }
    }
    return 0;
}

// IDA_EA: 0x8265492
// IDA_NAME: _ZN21DisPatcher_FinishLoad11check_stateEP5CUserR9PacketBuf
int DisPatcher_FinishLoad::check_state(CUser* user, PacketBuf& packet) {
    (void)packet;
    return Global::CGlobalFunc::Get_State_Check(user->get_state()) ? 0
                                                                     : 0x7fffffff;
}
