#include "old_game/progression/career/character_expert_job.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/quest/expert_job_payload.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_expert_job.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


namespace expert_job {

typedef char CCharacExpertJob_Size_Must_Be_0x30[
    (sizeof(CCharacExpertJob) == 0x30) ? 1 : -1];

// IDA_EA: 0x832AF52
// IDA_NAME: _ZN10expert_job16CCharacExpertJobC2Ev
// DEMANGLED: expert_job::CCharacExpertJob::CCharacExpertJob(void)
CCharacExpertJob::CCharacExpertJob()
    : charac_expand::CData() {}

// IDA_EA: 0x832B064
// IDA_NAME: _ZN10expert_job16CCharacExpertJob6_resetEv
// DEMANGLED: expert_job::CCharacExpertJob::_reset(void)
int CCharacExpertJob::_reset() {
    giveup_count_ = 0;
    disjoint_machine_endurance_ = 0;
    disjoint_machine_grade_ = 0;
    for (int index = 0; index <= 29; ++index) {
        recipe_flags_[index] = 0;
    }
    return 0;
}

// IDA_EA: 0x832B0AC
// IDA_NAME: _ZN10expert_job16CCharacExpertJob9_saveDataEP5CUser
// DEMANGLED: expert_job::CCharacExpertJob::_saveData(CUser *)
int CCharacExpertJob::_saveData(CUser* user) {
    Stream* stream =
        GlobalData::s_stream_pool->Acquire("CharacExpertJob.cpp", 22);
    CStreamGuard guard(stream, true);
    *guard << 0x181;
    *guard << user->GetUID();

    SIG_EXPERT_JOB* payload = guard->GetInBuffer<SIG_EXPERT_JOB>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = user->getCurCharacNo();
    payload->expert_job_giveup_cnt = giveup_count_;
    payload->expert_job_info = disjoint_machine_endurance_;
    payload->expert_job_info_ex = disjoint_machine_grade_;
    std::memcpy(payload->recipe, recipe_flags_, sizeof(recipe_flags_));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x832B1FE
// IDA_NAME: _ZN10expert_job16CCharacExpertJob8loadDataEP5CUserPc
// DEMANGLED: expert_job::CCharacExpertJob::loadData(CUser *,char *)
bool CCharacExpertJob::loadData(CUser* user, char* data) {
    (void)user;
    const SIG_EXPERT_JOB* payload =
        reinterpret_cast<const SIG_EXPERT_JOB*>(data);
    giveup_count_ = payload->expert_job_giveup_cnt;
    disjoint_machine_endurance_ = payload->expert_job_info;
    disjoint_machine_grade_ = payload->expert_job_info_ex;
    for (int index = 0; index <= 29; ++index) {
        recipe_flags_[index] = payload->recipe[index];
    }
    return true;
}

// IDA_EA: 0x832B264
// IDA_NAME: _ZN10expert_job16CCharacExpertJob10set_recipeEi
// DEMANGLED: expert_job::CCharacExpertJob::set_recipe(int)
int CCharacExpertJob::set_recipe(int recipe_index) {
    const int byte_index = recipe_index / 8;
    const int bit_index = recipe_index % 8;
    if (byte_index < 0 || byte_index > 29) {
        return 0;
    }
    if (((recipe_flags_[byte_index] >> bit_index) & 1) != 0) {
        return 0;
    }
    recipe_flags_[byte_index] |= static_cast<uint8_t>(1 << bit_index);
    return 1;
}

// IDA_EA: 0x832B306
// IDA_NAME: _ZNK10expert_job16CCharacExpertJob10get_recipeEi
// DEMANGLED: expert_job::CCharacExpertJob::get_recipe(int)const
int CCharacExpertJob::get_recipe(int recipe_index) const {
    const int byte_index = recipe_index / 8;
    if (byte_index >= 0 && byte_index <= 29) {
        return (recipe_flags_[byte_index] >> (recipe_index % 8)) & 1;
    }
    return 0;
}

// IDA_EA: 0x832B36C
// IDA_NAME: _ZN10expert_job16CCharacExpertJob10InitScriptEP17STExpertJobScript
// DEMANGLED: expert_job::CCharacExpertJob::InitScript(STExpertJobScript *)
int CCharacExpertJob::InitScript(STExpertJobScript* script) {
    disjoint_machine_endurance_ = script->dword_274;
    disjoint_machine_grade_ = 0;
    for (int index = 0; index <= 29; ++index) {
        recipe_flags_[index] = 0;
    }
    alter();
    return 1;
}

// IDA_EA: 0x832B3C0
// IDA_NAME: _ZN10expert_job16CCharacExpertJob11learnRecipeEmP17STExpertJobScripti
// DEMANGLED: expert_job::CCharacExpertJob::learnRecipe(unsigned long,STExpertJobScript *,int)
int CCharacExpertJob::learnRecipe(unsigned long recipe_result_item_index,
                                  STExpertJobScript* script,
                                  int expert_job_level) {
    const int recipe_index =
        script->GetRecipeInfoIndex(recipe_result_item_index);
    if (recipe_index < 0) {
        return 0;
    }
    stRecipeInfo* recipe_info = script->GetRecipeInfo(recipe_index);
    if (recipe_info == 0) {
        return 0;
    }
    if (static_cast<uint16_t>(recipe_info->required_level) >
        expert_job_level + 2) {
        return 0;
    }
    set_recipe(recipe_index);
    alter();
    return 1;
}

// IDA_EA: 0x832B454
// IDA_NAME: _ZN10expert_job16CCharacExpertJob17SendExpertJobInfoEP5CUser
// DEMANGLED: expert_job::CCharacExpertJob::SendExpertJobInfo(CUser *)
int CCharacExpertJob::SendExpertJobInfo(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 205);
    packet.put_byte(giveup_count_);
    packet.put_byte(user->GetCurCharacExpertJobType());

    const int expert_job_type = user->GetCurCharacExpertJobType();
    if (expert_job_type == 3) {
        packet.put_int(disjoint_machine_grade_ + 1);
        packet.put_int(disjoint_machine_endurance_);
    } else if (expert_job_type == 1 || expert_job_type == 2 ||
               expert_job_type == 4) {
        int recipe_count = 0;
        int recipe_count_index = packet.get_index();
        packet.put_byte(recipe_count);
        STExpertJobScript* script = G_CDataManager()->GetExpertJobScript(
            user->GetCurCharacExpertJobType());
        if (script != 0) {
            for (int recipe_index = 0; recipe_index <= 239;
                 ++recipe_index) {
                if (get_recipe(recipe_index) == 1) {
                    stRecipeInfo* recipe_info =
                        script->GetRecipeInfo(recipe_index);
                    if (recipe_info != 0 &&
                        recipe_info->recipe_item_index != 0) {
                        // The old packet reads the first dword of stRecipeInfo.
                        packet.put_int(recipe_info->recipe_item_index);
                        ++recipe_count;
                    }
                }
            }
        }
        packet.put_byte(recipe_count_index, recipe_count);
    }

    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x832BD22
// IDA_NAME: _ZN10expert_job16CCharacExpertJobD2Ev
// DEMANGLED: expert_job::CCharacExpertJob::~CCharacExpertJob()
// IDA_EA: 0x832BD52
// IDA_NAME: _ZN10expert_job16CCharacExpertJobD0Ev
// DEMANGLED: expert_job::CCharacExpertJob::~CCharacExpertJob()
CCharacExpertJob::~CCharacExpertJob() {}

// IDA_EA: 0x849F5A0
// IDA_NAME: _ZNK10expert_job16CCharacExpertJob16get_giveup_countEv
// DEMANGLED: expert_job::CCharacExpertJob::get_giveup_count(void)const
int CCharacExpertJob::get_giveup_count() const {
    return giveup_count_;
}

// IDA_EA: 0x849F5AC
// IDA_NAME: _ZN10expert_job16CCharacExpertJob16set_giveup_countEt
// DEMANGLED: expert_job::CCharacExpertJob::set_giveup_count(unsigned short)
CCharacExpertJob* CCharacExpertJob::set_giveup_count(uint16_t giveup_count) {
    alter();
    giveup_count_ = giveup_count;
    return this;
}

// IDA_EA: 0x85D3E66
// IDA_NAME: _ZNK10expert_job16CCharacExpertJob30get_disjoint_machine_enduranceEv
// DEMANGLED: expert_job::CCharacExpertJob::get_disjoint_machine_endurance(void)const
int CCharacExpertJob::get_disjoint_machine_endurance() const {
    return disjoint_machine_endurance_;
}

// IDA_EA: 0x85D3E72
// IDA_NAME: _ZN10expert_job16CCharacExpertJob30set_disjoint_machine_enduranceEi
// DEMANGLED: expert_job::CCharacExpertJob::set_disjoint_machine_endurance(int)
CCharacExpertJob* CCharacExpertJob::set_disjoint_machine_endurance(
    int endurance) {
    alter();
    disjoint_machine_endurance_ = endurance;
    return this;
}

// IDA_EA: 0x85D3E8E
// IDA_NAME: _ZNK10expert_job16CCharacExpertJob26get_disjoint_machine_gradeEv
// DEMANGLED: expert_job::CCharacExpertJob::get_disjoint_machine_grade(void)const
int CCharacExpertJob::get_disjoint_machine_grade() const {
    return disjoint_machine_grade_;
}

// IDA_EA: 0x85D3E9A
// IDA_NAME: _ZN10expert_job16CCharacExpertJob26set_disjoint_machine_gradeEi
// DEMANGLED: expert_job::CCharacExpertJob::set_disjoint_machine_grade(int)
CCharacExpertJob* CCharacExpertJob::set_disjoint_machine_grade(int grade) {
    alter();
    disjoint_machine_grade_ = grade;
    return this;
}

}  // namespace expert_job
