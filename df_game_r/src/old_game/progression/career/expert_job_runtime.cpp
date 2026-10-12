#include "old_game/progression/quest/quest_runtime.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

namespace expert_job {

namespace {

#pragma pack(push, 1)
struct EnchantItemReference {
    stItemPos_t position;
    uint8_t item_id_high_byte;

    int item_id() const {
        return static_cast<int>(
            static_cast<uint32_t>(position.item_space) |
            (static_cast<uint32_t>(position.slot) << 8u) |
            (static_cast<uint32_t>(item_id_high_byte) << 24u));
    }
};

struct MSG_PUT_ITEM_FOR_ENCHANT : MSG_STATIC_COMMAND {
    EnchantItemReference item;
};

struct MSG_TRY_ENCHANT : MSG_STATIC_COMMAND {
    uint32_t recipe_item_id;
    uint8_t reserved_12;
    stItemPos_t item_position;
};
#pragma pack(pop)

static_assert(sizeof(EnchantItemReference) == 0x04u,
              "enchant item reference must stay four bytes");
static_assert(sizeof(MSG_PUT_ITEM_FOR_ENCHANT) == 0x12u,
              "put-enchant request layout drift");
static_assert(sizeof(MSG_TRY_ENCHANT) == 0x16u,
              "try-enchant request layout drift");

}  // namespace

// IDA_EA: 0x849AE40
// IDA_NAME: _ZN10expert_job10CExpertJob20IncreaseExpertJobExpEP5CUseri
// DEMANGLED: expert_job::CExpertJob::IncreaseExpertJobExp(CUser *,int)
void CExpertJob::IncreaseExpertJobExp(CUser* user, int expert_job_exp) {
    if (!user->getCurCharacR()) {
        return;
    }

    STExpertJobScript* const script =
        G_CDataManager()->GetExpertJobScript(
            user->GetCurCharacExpertJobType());
    if (!script) {
        return;
    }

    const int max_exp =
        static_cast<int>(script->expert_job_exp.back().exp);
    if (max_exp < expert_job_exp) {
        expert_job_exp = max_exp;
    }

    bool level_up = false;
    const int current_level =
        user->GetCurExpertJobLevel(user->GetCurCharacExpertJobExp());
    const int requested_level =
        user->GetCurExpertJobLevel(expert_job_exp);
    STExpertJobEtcScript* const etc_script =
        G_CDataManager()->GetExpertJobEtcScript();

    if (current_level != requested_level) {
        std::map<unsigned char, short>::iterator limit =
            etc_script->expert_job_limit_levels.find(
                static_cast<unsigned char>(requested_level));
        if (limit != etc_script->expert_job_limit_levels.end() &&
            limit->second <= user->get_charac_level()) {
            level_up = true;
        }
    } else if (script->isBoundaryExpValue(expert_job_exp) &&
               etc_script->isExpertJobLimitLevel(
                   user->get_charac_level()) &&
               current_level != 11) {
        level_up = true;
    }

    const int boundary_exp = static_cast<int>(
        script->expert_job_exp[current_level - 1].exp);
    if (!level_up && expert_job_exp > boundary_exp) {
        expert_job_exp = boundary_exp;
    }

    if (!level_up &&
        user->GetCurCharacExpertJobExp() == expert_job_exp) {
        return;
    }

    user->SetCurCharacExpertJobExp(expert_job_exp);

    PacketGuard basic_packet;
    basic_packet.clear();
    basic_packet.put_header(0, 2);
    basic_packet.put_byte(0);
    basic_packet.put_short(1);
    user->make_basic_info(basic_packet, 0);
    basic_packet.finalize(true);

    if (!level_up) {
        user->Send(basic_packet);
        return;
    }

    const int level =
        user->GetCurExpertJobLevel(user->GetCurCharacExpertJobExp());
    user->LogHistory("ExpertJobLv+,%d,%d,%d",
                     user->get_charac_level(),
                     user->GetCurCharacExpertJobType(),
                     level);
    G_GameWorld()->send_all(basic_packet, user);

    for (unsigned int index = 0;
         index < script->unknown_exp_pairs.size();
         ++index) {
        const int first_level =
            user->GetCurExpertJobLevel(
                user->GetCurCharacExpertJobExp());
        const int first_skill_index =
            script->unknown_exp_pairs[index].first;
        user->getCurCharacSkillW()->growtype_skill(
            user->get_charac_job(),
            first_skill_index,
            first_level,
            static_cast<ENUM_SKILL_TREE_KIND>(0));

        const int second_level =
            user->GetCurExpertJobLevel(
                user->GetCurCharacExpertJobExp());
        const int second_skill_index =
            script->unknown_exp_pairs[index].first;
        user->getCurCharacSkillW()->growtype_skill(
            user->get_charac_job(),
            second_skill_index,
            second_level,
            static_cast<ENUM_SKILL_TREE_KIND>(1));
    }

    user->send_skill_info();
    PacketGuard quest_packet;
    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&quest_packet));
    user->Send(quest_packet);
}

// IDA_EA: 0x8234796
// IDA_NAME: _ZN10expert_job10CExpertJob7GetTypeEv
// DEMANGLED: expert_job::CExpertJob::GetType(void)
int CExpertJob::GetType() {
    return type_;
}

// IDA_EA: 0x849B2E8
// IDA_NAME: _ZN10expert_job10CExpertJob14GetSuccessRateEP5CUserPK14CStackableItemRi
// DEMANGLED: expert_job::CExpertJob::GetSuccessRate(CUser *,CStackableItem const*,int &)
stCompoundRate* CExpertJob::GetSuccessRate(
    CUser* user,
    const CStackableItem* recipe,
    int& rate_variation) {
    rate_variation = 0;
    STExpertJobScript* const script =
        G_CDataManager()->GetExpertJobScript(
            user->GetCurCharacExpertJobType());
    if (!script) {
        return 0;
    }

    const std::map<int, int>::const_iterator required_level =
        script->int_map_230.find(recipe->get_index());
    if (required_level == script->int_map_230.end()) {
        return 0;
    }

    const int level_difference =
        required_level->second -
        user->GetCurExpertJobLevel(
            user->GetCurCharacExpertJobExp());
    stCompoundRate* const compound_rate =
        script->GetCompoundRate(level_difference);
    if (!compound_rate) {
        return 0;
    }

    rate_variation = static_cast<int>(
        user->getCurCharacInvenR()
            ->GetExpertJobCompoundRateVariation(
                static_cast<ENUM_EXPERT_JOB_TYPE>(
                    user->GetCurCharacExpertJobType()),
                user->GetCurExpertJobLevel(
                    user->GetCurCharacExpertJobExp())));
    return compound_rate;
}

// IDA_EA: 0x849B462
// IDA_NAME: _ZN10expert_job10CExpertJob9IsSuccessEP5CUserPK14CStackableItemRi
// DEMANGLED: expert_job::CExpertJob::IsSuccess(CUser *,CStackableItem const*,int &)
bool CExpertJob::IsSuccess(CUser* user,
                           const CStackableItem* recipe,
                           int& expert_job_exp) {
    int rate_variation = 0;
    stCompoundRate* const success_rate =
        GetSuccessRate(user, recipe, rate_variation);
    if (!success_rate) {
        return false;
    }

    if (success_rate->rates[0] + rate_variation <=
        static_cast<int>(get_rand_int(100))) {
        return false;
    }

    expert_job_exp =
        static_cast<int>(get_rand_int(
            success_rate->rates[2] -
            success_rate->rates[1] + 1)) +
        success_rate->rates[1];

    STItemScript::SEXPERTJOB::stExpertJobAdditionalExp additional_exp;
    const int expert_job_level =
        user->GetCurExpertJobLevel(
            user->GetCurCharacExpertJobExp());
    user->getCurCharacInvenR()->GetExpertJobAddtionalExp(
        static_cast<ENUM_EXPERT_JOB_TYPE>(
            user->GetCurCharacExpertJobType()),
        expert_job_level,
        additional_exp);
    if (additional_exp.value_00 >
        static_cast<float>(get_rand_int(100))) {
        expert_job_exp += additional_exp.value_04;
    }
    return true;
}

// IDA_EA: 0x849B590
// IDA_NAME: _ZN10expert_job10CExpertJob13IsLearnRecipeEP5CUserPK14CStackableItem
// DEMANGLED: expert_job::CExpertJob::IsLearnRecipe(CUser *,CStackableItem const*)
bool CExpertJob::IsLearnRecipe(
    CUser* user,
    const CStackableItem* recipe) {
    STExpertJobScript* const script =
        G_CDataManager()->GetExpertJobScript(
            user->GetCurCharacExpertJobType());
    if (!script) {
        cMyTrace trace(
            "static bool expert_job::CExpertJob::IsLearnRecipe(CUser*, const CStackableItem*)",
            319,
            0);
        trace("EXPERT_JOB_ERROR : GetExpertJobScript() charac_no(%d)",
              user->getCurCharacNo());
        return false;
    }

    const int recipe_index =
        script->GetRecipeInfoIndex(recipe->get_index());
    if (recipe_index < 0) {
        cMyTrace trace(
            "static bool expert_job::CExpertJob::IsLearnRecipe(CUser*, const CStackableItem*)",
            326,
            0);
        trace("EXPERT_JOB_ERROR : GetRecipeInfoIndex() charac_no(%d)",
              user->getCurCharacNo());
        return false;
    }

    CCharacExpertJob* const expert_job =
        static_cast<CCharacExpertJob*>(
            user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    if (expert_job->get_recipe(recipe_index) == 1) {
        return true;
    }
    if (GlobalData::s_pGMAccounts_->isGM(
            static_cast<uint32_t>(user->get_acc_id()))) {
        return true;
    }

    cMyTrace trace(
        "static bool expert_job::CExpertJob::IsLearnRecipe(CUser*, const CStackableItem*)",
        334,
        0);
    trace("EXPERT_JOB_ERROR : get_recipe() charac_no(%d)",
          user->getCurCharacNo());
    return false;
}

// IDA_EA: 0x849C7B6
// IDA_NAME: _ZN10expert_job10CEnchanter14OnStartMissionEP5CUserS2_
// DEMANGLED: expert_job::CEnchanter::OnStartMission(CUser *,CUser *)
int CEnchanter::OnStartMission(CUser* user, CUser* peer_user) {
    mission_user_ = user;
    mission_unique_id_ = static_cast<unsigned short>(user->get_unique_id());
    user->SetCurCharacExpertJob(this);

    peer_user_ = peer_user;
    peer_unique_id_ =
        static_cast<unsigned short>(peer_user->get_unique_id());
    return peer_user->SetCurCharacExpertJob(this);
}

// IDA_EA: 0x849DF5E
// IDA_NAME: _ZN10expert_job10CEnchanter19OnPutItemForEnchantEP5CUserP18MSG_STATIC_COMMAND
// DEMANGLED: expert_job::CEnchanter::OnPutItemForEnchant(
//             CUser *,MSG_STATIC_COMMAND *)
char CEnchanter::OnPutItemForEnchant(CUser* user,
                                     MSG_STATIC_COMMAND* command) {
    MSG_PUT_ITEM_FOR_ENCHANT* const request =
        reinterpret_cast<MSG_PUT_ITEM_FOR_ENCHANT*>(command);
    const int result = _PutItemForEnchant(
        user,
        static_cast<int>(request->action_type),
        &request->item.position,
        request->item.item_id());
    if (result) {
        return static_cast<char>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_PUT_ITEM_FOR_ENCHANT,
            static_cast<unsigned char>(result)));
    }
    return 0;
}

// IDA_EA: 0x849DFCC
// IDA_NAME: _ZN10expert_job10CEnchanter12OnTryEnchantEP5CUserP18MSG_STATIC_COMMAND
// DEMANGLED: expert_job::CEnchanter::OnTryEnchant(
//             CUser *,MSG_STATIC_COMMAND *)
void CEnchanter::OnTryEnchant(CUser* user,
                              MSG_STATIC_COMMAND* command) {
    MSG_TRY_ENCHANT* const request =
        reinterpret_cast<MSG_TRY_ENCHANT*>(command);
    const int item_id = static_cast<int>(request->recipe_item_id);
    recipe_item_ = static_cast<CStackableItem*>(
        G_CDataManager()->find_item(item_id));

    const int result = _TryEnchant(
        user,
        &request->item_position,
        static_cast<ENUM_TRY_ENCHANT_TYPE>(request->action_type));
    if (result) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_TRY_ENCHANT,
            static_cast<unsigned char>(result)));
        return;
    }

    APSystem::CUserProc::ClearActionAndSendtoUser(
        user,
        static_cast<APSystem::_ActionGroupIndex>(31),
        0,
        static_cast<ENUM_PACKETCLASS>(0));
}

// IDA_EA: 0x849E07E
// IDA_NAME: _ZN10expert_job10CEnchanter14OnAgreeEnchantEP5CUserP18MSG_STATIC_COMMAND
// DEMANGLED: expert_job::CEnchanter::OnAgreeEnchant(
//             CUser *,MSG_STATIC_COMMAND *)
char CEnchanter::OnAgreeEnchant(CUser* user,
                                MSG_STATIC_COMMAND* command) {
    const int result = _AgreeEnchant(
        user, static_cast<ENUM_AGREE_ENCHANT_TYPE>(command->action_type));
    if (!result) {
        return 0;
    }

    static_cast<void>(user->SendCmdErrorPacket(
        ENUM_CMDPACKET_AGREE_ENCHANT,
        static_cast<unsigned char>(result)));
    if (!mission_user_) {
        return 0;
    }
    return static_cast<char>(mission_user_->SendCmdErrorPacket(
        ENUM_CMDPACKET_AGREE_ENCHANT,
        static_cast<unsigned char>(result)));
}

// IDA_EA: 0x849E214
// IDA_NAME: _ZN10expert_job13CExpertJobMgr11OnLeaveUserEP5CUser
// DEMANGLED: expert_job::CExpertJobMgr::OnLeaveUser(CUser *)
char CExpertJobMgr::OnLeaveUser(CUser* user) {
    CExpertJob* const expert_job = user->GetCurCharacExpertJob();
    const char has_expert_job = static_cast<char>(expert_job != 0);
    if (!has_expert_job) {
        return has_expert_job;
    }

    expert_job->OnCloseMission(
        user, static_cast<ENUM_CLOSE_MISSION_TYPE>(1));
    return has_expert_job;
}

}  // namespace expert_job

// IDA_EA: 0x868BA58
// IDA_NAME: _ZN5CUser27CalculateExpertJobMaterialsERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::CalculateExpertJobMaterials(std::vector<std::pair<int,int> >&)
void CUser::CalculateExpertJobMaterials(
    std::vector<std::pair<int, int> >& materials) {
    const int expert_job_level =
        GetCurExpertJobLevel(GetCurCharacExpertJobExp());
    const ENUM_EXPERT_JOB_TYPE expert_job_type =
        static_cast<ENUM_EXPERT_JOB_TYPE>(
            GetCurCharacExpertJobType());
    const CInventory* const inventory = getCurCharacInvenR();
    const float variation =
        inventory->GetExpertJobCompoundMaterialVariation(
            expert_job_type, expert_job_level) /
        100.0f;

    std::vector<std::pair<int, int> > additions(materials);
    std::vector<std::pair<int, int> >::iterator addition =
        additions.begin();
    std::vector<std::pair<int, int> >::iterator material =
        materials.begin();
    while (addition != additions.end() &&
           material != materials.end()) {
        addition->second =
            static_cast<int>(addition->second * variation);
        material->second += addition->second;
        if (material->second == 0) {
            material->second = 1;
        }
        ++addition;
        ++material;
    }
}
