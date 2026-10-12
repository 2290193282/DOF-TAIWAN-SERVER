#include "old_game/progression/career/expert_job_disjointer.h"
#include "infra/network/packet/PacketEnum.h"

#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/find_price_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

bool SelectDisjointAdditional(const std::vector<stItemSelectRate>& entries,
                              int random_value,
                              int item_grade,
                              stItemSelectRate& selected) {
    selected.item_index = -1;
    for (std::vector<stItemSelectRate>::const_iterator it = entries.begin();
         it != entries.end();
         ++it) {
        if (it->min_grade > item_grade || it->max_grade < item_grade) {
            continue;
        }
        if (it->select_rate >= random_value) {
            selected = *it;
            return true;
        }
        random_value -= it->select_rate;
    }
    return false;
}

}  // namespace

namespace expert_job {

// IDA_EA: 0x85D1E0A
// IDA_NAME: _ZN10expert_job11CDisjointer9GetScriptEv
// DEMANGLED: expert_job::CDisjointer::GetScript(void)
STExpertJobScript* CDisjointer::GetScript() {
    static STExpertJobScript* script = 0;
    if (!script) {
        script = G_CDataManager()->GetExpertJobScript(3);
    }
    return script;
}

// IDA_EA: 0x85D1E3A
// IDA_NAME: _ZN10expert_job11CDisjointer12GetEtcScriptEv
// DEMANGLED: expert_job::CDisjointer::GetEtcScript(void)
STExpertJobEtcScript* CDisjointer::GetEtcScript() {
    static STExpertJobEtcScript* script = 0;
    if (!script) {
        script = G_CDataManager()->GetExpertJobEtcScript();
    }
    return script;
}

// IDA_EA: 0x85D35CE
// IDA_NAME: _ZN10expert_job11CDisjointer23OnRepairDisjointMachineEP5CUser
// DEMANGLED: expert_job::CDisjointer::OnRepairDisjointMachine(CUser *)
int CDisjointer::OnRepairDisjointMachine(CUser* user) {
    CCharacExpertJob* const expert_job =
        static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    const int endurance = expert_job->get_disjoint_machine_endurance();
    const int grade = expert_job->get_disjoint_machine_grade();
    STExpertJobScript* const script = GetScript();

    int usable_money = script->GetDisjointMachineRepairCost(grade + 1,
                                                              endurance);
    int repaired_endurance = 0;
    if (user->getCurCharacMoney() >= usable_money) {
        const stEnduranceRepairCost* const full_repair =
            script->_GetEnduranceRepairCost(static_cast<unsigned int>(grade + 1));
        if (full_repair) {
            repaired_endurance = full_repair->max_endurance;
        }
    } else {
        repaired_endurance = endurance + script->GetDisjointMachineRepair(
                                           grade + 1,
                                           user->getCurCharacMoney(),
                                           usable_money);
    }

    if (repaired_endurance <= 0) {
        return 22;
    }

    user->getCurCharacInvenW()->use_money(
        usable_money, static_cast<eMoneySubReason>(0x20), true);
    expert_job->set_disjoint_machine_endurance(repaired_endurance);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REPAIR_DISJOINT_MACHINE);
    packet.put_byte(1);
    packet.put_int(user->getCurCharacMoney());
    packet.put_int(expert_job->get_disjoint_machine_endurance());
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x85D37E8
// IDA_NAME: _ZN10expert_job11CDisjointer24OnUpgradeDisjointMachineEP5CUser
// DEMANGLED: expert_job::CDisjointer::OnUpgradeDisjointMachine(CUser *)
int CDisjointer::OnUpgradeDisjointMachine(CUser* user) {
    CCharacExpertJob* const expert_job =
        static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    int grade = expert_job->get_disjoint_machine_grade();
    STExpertJobScript* const script = GetScript();
    if (grade + 1 >= script->GetDisjointMachineMaxGrade()) {
        return 19;
    }

    const int expert_job_level = user->GetCurExpertJobLevel(
        user->GetCurCharacExpertJobExp());
    if (grade + 1 >= expert_job_level) {
        return 19;
    }

    STExpertJobEtcScript* const etc_script = GetEtcScript();
    const unsigned char next_grade = static_cast<unsigned char>(grade + 2);
    std::map<unsigned char, short>::const_iterator level_limit =
        etc_script->expert_job_limit_levels.find(next_grade);
    if (level_limit == etc_script->expert_job_limit_levels.end()) {
        return 19;
    }
    if (level_limit->second > user->get_charac_level()) {
        return 14;
    }

    ++grade;
    const int upgrade_cost =
        script->GetUpgradeCost(static_cast<unsigned int>(grade));
    if (upgrade_cost <= 0 || user->getCurCharacMoney() < upgrade_cost) {
        return 22;
    }

    user->getCurCharacInvenW()->use_money(
        upgrade_cost, static_cast<eMoneySubReason>(0x20), true);
    expert_job->set_disjoint_machine_grade(grade);

    const stEnduranceRepairCost* const full_repair =
        script->_GetEnduranceRepairCost(static_cast<unsigned int>(grade + 1));
    if (full_repair) {
        expert_job->set_disjoint_machine_endurance(full_repair->max_endurance);
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_UPGRADE_DISJOINT_MACHINE);
    packet.put_byte(1);
    packet.put_int(user->getCurCharacMoney());
    packet.put_int(grade + 1);
    packet.put_int(expert_job->get_disjoint_machine_endurance());
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x85D1F0A
// IDA_NAME: _ZN10expert_job11CDisjointer26get_disjoint_machine_gradeEv
// DEMANGLED: expert_job::CDisjointer::get_disjoint_machine_grade(void)
int CDisjointer::get_disjoint_machine_grade() {
    return static_cast<char>(machine_.get_grade());
}

// IDA_EA: 0x85D3E50
// IDA_NAME: _ZNK10expert_job11CDisjointer17get_disjoint_costEv
// DEMANGLED: expert_job::CDisjointer::get_disjoint_cost(void)const
int CDisjointer::get_disjoint_cost() const {
    return machine_.get_cost();
}

// IDA_EA: 0x85D2D08
// IDA_NAME: _ZN10expert_job11CDisjointer21OnCreateDisjointStoreEP5CUserPKciii
// DEMANGLED: expert_job::CDisjointer::OnCreateDisjointStore(CUser *,char const *,int,int,int)
int CDisjointer::OnCreateDisjointStore(CUser* user,
                                       const char* title,
                                       int cost,
                                       int pos_x,
                                       int pos_y) {
    if (cost < 0 ||
        static_cast<int>(GetScript()->max_disjoint_machine_cost) < cost) {
        return 10;
    }

    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        return 190;
    }
    if (user->getCurCharacVill() == 7) {
        return 190;
    }

    GameWorld* const world = G_GameWorld();
    const int village = user->getCurCharacVill();
    const int area = user->get_area(false);
    if (!world->is_available_point(village, area, pos_x, pos_y)) {
        return 190;
    }
    const int zone_error = world->check_restrictive_commercial_transaction_zone(
        village, area, pos_x, pos_y);
    if (zone_error) {
        return zone_error;
    }

    CCharacExpertJob* const expert_job =
        static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    if (expert_job->get_disjoint_machine_endurance() <= 0) {
        return 22;
    }

    CDisjointMachine machine(user,
                             title,
                             cost,
                             static_cast<char>(
                                 expert_job->get_disjoint_machine_grade()),
                             pos_x,
                             pos_y);
    machine_ = machine;

    user->set_position(static_cast<uint16_t>(pos_x),
                       static_cast<uint16_t>(pos_y),
                       0);
    const village_object::Zone zone(
        static_cast<uint8_t>(user->getCurCharacVill()),
        static_cast<uint8_t>(user->get_area(false)));
    if (GlobalData::s_villageObjectMgr->register_object(zone, &machine_) != 1) {
        return 20;
    }

    PacketGuard packet;
    packet.put_header(0, 206);
    machine_.put_disjoint_machine_info(packet);
    packet.finalize(true);
    world->send_to_area(user->getCurCharacVill(), user->get_area(false), packet);

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_CREATE_DISJOINT_STORE);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x85D309A
// IDA_NAME: _ZN10expert_job11CDisjointer20OnEnterDisjointStoreEP5CUser
// DEMANGLED: expert_job::CDisjointer::OnEnterDisjointStore(CUser *)
int CDisjointer::OnEnterDisjointStore(CUser* user) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_ENTER_DISJOINT_STORE);
    packet.put_byte(1);
    packet.put_byte(get_disjoint_machine_grade() + 1);
    packet.put_int(get_disjoint_cost());

    CCharacExpertJob* const owner_expert_job =
        static_cast<CCharacExpertJob*>(machine_.get_user()->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    packet.put_int(owner_expert_job->get_disjoint_machine_endurance());
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x8234854
// IDA_NAME: _ZN10expert_job11CDisjointer6isMineEP5CUser
bool CDisjointer::isMine(CUser* user) const {
    return machine_.get_user() == user;
}

// IDA_EA: 0x82347EC
// IDA_NAME: _ZN10expert_job11CDisjointer19get_disjoint_resultEP5CItemP10Inven_ItemR14DisJointResultP5CUser
int CDisjointer::get_disjoint_result(CItem* item,
                                     Inven_Item* inventory_item,
                                     DisJointResult& result,
                                     CUser* requester) {
    return get_disjoint_result_(item,
                                inventory_item,
                                get_disjoint_machine_grade(),
                                result,
                                isMine(requester),
                                machine_.get_user());
}

// IDA_EA: 0x85D234A
// IDA_NAME: _ZN10expert_job11CDisjointer20get_disjoint_result_EP5CItemP10Inven_ItemiR14DisJointResultbP5CUser
int CDisjointer::get_disjoint_result_(CItem* item,
                                      Inven_Item* inventory_item,
                                      int machine_grade,
                                      DisJointResult& result,
                                      bool owner_request,
                                      CUser* owner) {
    if (!item) return 17;

    const bool unidentified = inventory_item &&
                              inventory_item->amplify_option_.isIdentified() != 1;
    STExpertJobScript* const script = GetScript();
    if (!script) return unidentified ? 211 : 17;

    const DisjointMachineGrade key(
        static_cast<uint8_t>(item->get_rarity()),
        static_cast<uint8_t>(machine_grade),
        unidentified ? 1u : 0u);
    const std::map<DisjointMachineGrade, stDisjointResult>::const_iterator found =
        script->disjoint_results.find(key);
    if (found == script->disjoint_results.end()) {
        return unidentified ? 211 : 17;
    }

    const stDisjointResult& rule = found->second;
    const int sell_price = static_cast<int>(
        static_cast<long double>(item->GetSellPrice()) * 1.1L);
    const bool special_price_type =
        item->vtable_ && item->vtable_->is_random_option &&
        item->vtable_->is_random_option(item) != 0;
    const int cube_creation_const = static_cast<int>(script->dword_1D4);
    if (cube_creation_const == 0) return 17;

    const int base_price = 1000 * getItemSellPrice(
        sell_price, 0, 0, special_price_type) / cube_creation_const;
    result.values[0] = static_cast<int>(
        static_cast<long double>(base_price) * rule.base_cost_rate / 1000.0L);
    if (result.values[0] <= 0) result.values[0] = 1;
    result.values[5] = rule.result_item_index;

    int big_win_rate = 100 * rule.big_win_rate;
    if (owner_request) {
        STExpertJobEtcScript* const etc = GetEtcScript();
        if (etc) {
            const std::map<unsigned char, unsigned char>::const_iterator bonus =
                etc->expert_job_unknown_flags.find(
                    static_cast<unsigned char>(machine_grade + 1));
            if (bonus != etc->expert_job_unknown_flags.end()) {
                big_win_rate += 100 * static_cast<int>(bonus->second);
            }
        }
        if (owner && owner->get_state() > 2 && owner->getCurCharacR()) {
            const int level = owner->GetCurExpertJobLevel(
                owner->GetCurCharacExpertJobExp());
            big_win_rate += static_cast<int>(
                owner->getCurCharacInvenR()->GetExpertJobSelfDisjointBigWinRate(
                    static_cast<ENUM_EXPERT_JOB_TYPE>(3), level) * 100.0f);
        }
    }

    stItemSelectRate selected;
    const std::vector<stItemSelectRate>& selected_rules =
        static_cast<int>(get_rand_int(10000)) >= big_win_rate
            ? rule.additional_items
            : rule.big_win_items;
    if (SelectDisjointAdditional(selected_rules,
                                 static_cast<int>(get_rand_int(10000)),
                                 item->get_grade(),
                                 selected)) {
        result.values[6] = selected.item_index;
        result.values[1] = static_cast<int>(
            static_cast<float>(item->get_grade()) / selected.grade_divisor);
        if (result.values[1] <= 0) result.values[1] = 1;
    }

    if (owner_request && item->get_rarity() > 1 &&
        static_cast<int>(get_rand_int(100)) <
            script->self_disjoin_material_info.item_index) {
        result.values[7] = script->self_disjoin_material_info.item_count;
        result.values[2] = script->self_disjoin_material_info.gold;
    }

    if (owner && owner->get_state() > 2 && owner->getCurCharacR()) {
        const int level = owner->GetCurExpertJobLevel(
            owner->GetCurCharacExpertJobExp());
        const float variation = owner->getCurCharacInvenR()
            ->GetExpertJobSelfDisjointResultVariation(
                static_cast<ENUM_EXPERT_JOB_TYPE>(3), level) / 100.0f;
        for (int index = 0; index <= 4; ++index) {
            result.values[index] += static_cast<int>(
                static_cast<float>(result.values[index]) * variation);
        }
    }
    return 0;
}

// IDA_EA: 0x85D31A0
// IDA_NAME: _ZN10expert_job11CDisjointer17complete_disjointEP5CUserR11PacketGuard
int CDisjointer::complete_disjoint(CUser* user, PacketGuard& packet) {
    CUser* const owner = machine_.get_user();
    STExpertJobScript* const script = GetScript();
    if (!owner || !user || !script) return 0;

    CCharacExpertJob* const owner_data = static_cast<CCharacExpertJob*>(
        owner->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    if (!owner_data) return 0;

    const int endurance_min = static_cast<int>(script->dword_26C);
    const int endurance_max = static_cast<int>(script->dword_270);
    int endurance_loss = endurance_min + static_cast<int>(get_rand_int(
        endurance_max - endurance_min + 1));
    const int endurance = owner_data->get_disjoint_machine_endurance();
    if (endurance_loss > endurance) endurance_loss = endurance;
    owner_data->set_disjoint_machine_endurance(endurance - endurance_loss);

    const int exp_min = static_cast<int>(script->dword_284);
    const int exp_max = static_cast<int>(script->dword_288);
    const int expert_job_exp = exp_min + static_cast<int>(get_rand_int(
        exp_max - exp_min + 1));
    if (expert_job_exp > 0) {
        // IncreaseExpertJobExp consumes an absolute total, not a delta.  The
        // 0627 CDisjointer path adds the earned amount to the character's
        // current expert-job experience before entering the shared level-cap
        // state machine.  Passing the delta directly resets accumulated
        // experience to the small per-disjoint reward on every completion.
        CExpertJob::IncreaseExpertJobExp(
            owner,
            owner->GetCurCharacExpertJobExp() + expert_job_exp);
    }

    if (user == owner) {
        packet.put_int(user->getCurCharacMoney());
        packet.put_int(owner_data->get_disjoint_machine_endurance());
    } else {
        const int cost = get_disjoint_cost();
        user->getCurCharacInvenW()->use_money(
            cost, static_cast<eMoneySubReason>(0x1d), true);
        packet.put_int(user->getCurCharacMoney());
        packet.put_int(owner_data->get_disjoint_machine_endurance());

        owner->getCurCharacInvenW()->gain_money(
            cost, static_cast<eMoneyAddReason>(0x16), true, 0);
        PacketGuard owner_packet;
        owner_packet.put_header(0, 203);
        owner_packet.put_byte(1);
        owner_packet.put_int(owner->getCurCharacMoney());
        owner_packet.put_int(owner_data->get_disjoint_machine_endurance());
        owner_packet.finalize(true);
        owner->Send(owner_packet);
    }

    if (owner_data->get_disjoint_machine_endurance() <= 0) {
        OnCloseMission(owner, static_cast<ENUM_CLOSE_MISSION_TYPE>(0));
    }
    return 1;
}

}  // namespace expert_job
