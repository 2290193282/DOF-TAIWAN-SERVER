#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// The old body throws this opaque fixed-underlying error enum directly; the
// dispatcher template owns its eventual error boundary.
enum ENUM_ERROR : int;

namespace expert_job {

// IDA_EA: 0x849E60C
// IDA_NAME: _ZN10expert_job13CExpertJobMgr8OnGiveupEP5CUser
// DEMANGLED: expert_job::CExpertJobMgr::OnGiveup(CUser *)
void CExpertJobMgr::OnGiveup(CUser* user) {
    if (!user->getCurCharacR()) {
        throw static_cast<ENUM_ERROR>(3);
    }

    const int expert_job_type = user->GetCurCharacExpertJobType();
    if (!expert_job_type) {
        throw static_cast<ENUM_ERROR>(8);
    }

    STExpertJobScript* const script =
        G_CDataManager()->GetExpertJobScript(expert_job_type);
    if (!script) {
        throw static_cast<ENUM_ERROR>(8);
    }

    CCharacExpertJob* const character_expert_job =
        static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    unsigned int giveup_count = static_cast<unsigned int>(
        static_cast<uint16_t>(character_expert_job->get_giveup_count()));

    if (script->dword_vector_1E8.size() <= giveup_count) {
        throw static_cast<ENUM_ERROR>(1);
    }
    const int giveup_cost = script->dword_vector_1E8.at(giveup_count);
    if (user->getCurCharacMoney() < giveup_cost) {
        throw static_cast<ENUM_ERROR>(21);
    }

    for (std::vector<int>::const_iterator it =
             script->dword_vector_1DC.begin();
         it != script->dword_vector_1DC.end();
         ++it) {
        if (user->getCurCharacQuestRefR()->isClearQuest(*it) != 1) {
            throw static_cast<ENUM_ERROR>(8);
        }
    }

    for (std::vector<int>::const_iterator it =
             script->dword_vector_1DC.begin();
         it != script->dword_vector_1DC.end();
         ++it) {
        static_cast<void>(user->getCurCharacQuestW()->resetClearQuest(*it));
    }

    PacketGuard packet;
    static_cast<void>(user->getCurCharacInvenW()->use_money(
        giveup_cost, static_cast<eMoneySubReason>(30), true));

    packet.clear();
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    static_cast<void>(user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), 0, packet));
    packet.finalize(true);
    user->Send(packet);

    for (unsigned int index = 0;
         index < script->unknown_exp_pairs.size();
         ++index) {
        const int first_skill_index = script->unknown_exp_pairs[index].first;
        const int first_charac_job = user->get_charac_job();
        static_cast<void>(user->getCurCharacSkillW()->delete_skill(
            first_charac_job,
            first_skill_index,
            static_cast<ENUM_SKILL_TREE_KIND>(0)));
        const int second_skill_index = script->unknown_exp_pairs[index].first;
        const int second_charac_job = user->get_charac_job();
        static_cast<void>(user->getCurCharacSkillW()->delete_skill(
            second_charac_job,
            second_skill_index,
            static_cast<ENUM_SKILL_TREE_KIND>(1)));
    }

    user->send_skill_info();
    user->LogHistory("ExpertJob-,%d,%d,%d",
                     user->get_charac_level(),
                     user->GetCurCharacExpertJobType(),
                     giveup_count);
    user->SetCurCharacExpertJobType(0);
    user->SetCurCharacExpertJobExp(-1);

    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&packet));
    user->Send(packet);

    ++giveup_count;
    if (script->dword_vector_1E8.size() <= giveup_count) {
        giveup_count =
            static_cast<unsigned int>(script->dword_vector_1E8.size() - 1u);
    }

    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(0))->reset();
    static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)))
        ->set_giveup_count(static_cast<uint16_t>(giveup_count));

    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_GIVEUP_EXPERT_JOB);
    packet.put_byte(1);
    packet.put_int(user->getCurCharacMoney());
    packet.put_byte(giveup_count);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace expert_job
