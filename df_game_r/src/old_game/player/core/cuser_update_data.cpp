#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "core/config/env/environment.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/charac_expand_data_mgr.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"
#include "old_data/db/inventory/db_save_account_cargo.h"
#include "old_data/db/event/db_update_item_count_value.h"
#include "old_data/db/event/db_update_add_item_count_value.h"
#include "old_data/db/event/db_update_school_point.h"
#include "old_data/db/account/db_update_user_fatigue_quantity.h"
#include "old_data/db/account/db_update_user_gift_fatigue_quantity.h"
#include "old_data/db/pvp_party/db_save_pvp.h"
#include "old_data/db/quest/db_update_quest.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>


// B still owns the payload layout; an incomplete declaration is sufficient
// for C to call A's exact typed stream member without a free-function facade.
struct InformNoticeFlagData;

namespace {

struct SkillTreePersistenceSummary {
    unsigned int indexed_slots;
    unsigned int positive_levels;
    unsigned int level_sum;
};

SkillTreePersistenceSummary SummarizeSkillTree(
    const _Mastered_skill* tree,
    unsigned int count) {
    SkillTreePersistenceSummary result = {0u, 0u, 0u};
    if (!tree) return result;
    for (unsigned int index = 0; index < count; ++index) {
        if (!tree[index].skill_index) continue;
        ++result.indexed_slots;
        const unsigned int level =
            static_cast<unsigned char>(tree[index].skill_level);
        if (level) ++result.positive_levels;
        result.level_sum += level;
    }
    return result;
}

void LogSkillSaveSummary(const CUser* user, const SIG_LOAD_SKILL& skill) {
    if (!user) return;
    const SkillTreePersistenceSummary first = SummarizeSkillTree(
        skill.first_tree, sizeof(skill.first_tree) / sizeof(skill.first_tree[0]));
    const SkillTreePersistenceSummary second = SummarizeSkillTree(
        skill.second_tree,
        sizeof(skill.second_tree) / sizeof(skill.second_tree[0]));
    LogManager::logFormat(
        1, "skill_persistence_diag.cpp", "CUser::UpdateData", 0,
        "[SKILL_PERSIST][SAVE_QUEUE] job=%d raw_grow=%d first_grow=%d "
        "second_grow=%d selected_tree=%d table=%u script_version=%u "
        "tree0_slots=%u tree0_positive=%u tree0_levels=%u "
        "tree1_slots=%u tree1_positive=%u tree1_levels=%u "
        "sp0=%u sp1=%u sfp0=%u sfp1=%u",
        user->get_charac_job(), user->getCurCharacGrowType(),
        user->getCurCharFirstGrowType(),
        static_cast<int>(user->getCurCharSecondGrowType()),
        user->GetCurCharacSkillTreeIndex(),
        static_cast<unsigned int>(skill.skill_table_index),
        static_cast<unsigned int>(skill.script_version),
        first.indexed_slots, first.positive_levels, first.level_sum,
        second.indexed_slots, second.positive_levels, second.level_sum,
        skill.remain_sp, skill.remain_sp_second,
        skill.remain_sfp_first, skill.remain_sfp_second);
}

}  // namespace

// IDA_EA: 0x8653270
// IDA_NAME: _ZN5CUser10UpdateDataEv
// DEMANGLED: CUser::UpdateData(void)
char CUser::UpdateData() {
    CUser* const user = this;
    const int state = user->get_state();
    if (state <= 2) {
        return static_cast<char>(state);
    }

    if (G_GameWorld()->GetChannelType() == 7) {
        return 1;
    }

    if (!user->getCurCharacR()) {
        cMyTrace trace("void CUser::UpdateData()", 7577, 5);
        return static_cast<char>(trace(
            "CUser::UpdateData() character %s - m_selected is null",
            user->getCurCharacName()));
    }

    if (static_cast<unsigned char>(user->isLocked4DataLoad())) {
        const int charac_no = user->getCurCharacNo();
        cMyTrace trace("void CUser::UpdateData()", 7585, 5);
        return static_cast<char>(trace(
            "Can not save character until loading done(m_id: %s, charac_no: %d",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            charac_no));
    }

    CInventory* inventory = user->getCurCharacInvenW();
    if (static_cast<unsigned char>(inventory->checkToFix())) {
        inventory->CheckNFix();
    }

    if (user->getCurCharacR() &&
        user->get_charac_memberkey()) {
        const int money_tax =
            user->get_member_pay_tex_money_to_upper();
        const int fatigue_tax =
            user->get_member_pay_tex_fatigue_to_upper();
        GlobalData::s_monitor_proxy_mgr
            ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(
                user->GetServerGroup()))
            ->SendMemberPayTax(
            user->is_connect_upper_member() != 0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            money_tax,
            fatigue_tax);
        if (money_tax > 0) {
            user->set_member_pay_tex_money_to_upper(0);
        }
        if (fatigue_tax > 0) {
            user->set_member_pay_tex_fatigue_to_upper(0);
        }
    }

    user->_SaveCharacData();
    user->UpdateCharacView();
    ARAD::Singleton<HeroMissionEvent>::Get()->saveEventData(user, false);

    if (static_cast<unsigned char>(user->isSaveInven()) ||
        static_cast<unsigned char>(user->isSaveCargo())) {
        if (state == 5) {
            user->SaveInventoryOnly();
        } else {
            user->SaveInventory();
        }
        user->disableSaveInven();
        user->disableSaveCargo();
    }

    WongWork::CMailBoxHelper::ReqDBSendStoredMail(user);

    if (static_cast<unsigned char>(user->isSaveSkill())) {
        CStreamGuard guard(GlobalData::stream_pool()->Acquire("user.cpp", 7661),
                           true);
        *guard << 36;
        *guard << user->GetUID();
        SIG_LOAD_SKILL* payload = guard.GetInBuffer<SIG_LOAD_SKILL>();
        std::memset(payload, 0, sizeof(*payload));
        payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        payload->script_version =
            G_CDataManager()->get_pvp_skill_tree_parameter_script()->version;
        SkillSlot* skill = user->getCurCharacSkillR();
        skill->get_remain_sp(reinterpret_cast<int*>(&payload->remain_sp),
                             reinterpret_cast<int*>(&payload->remain_sp_second),
                             reinterpret_cast<int*>(&payload->remain_sfp_first),
                             reinterpret_cast<int*>(&payload->remain_sfp_second));
        const _Mastered_skill* const first_tree = skill->get_skillslot_buf(
            static_cast<::ENUM_SKILL_TREE_KIND>(0));
        const _Mastered_skill* const second_tree = skill->get_skillslot_buf(
            static_cast<::ENUM_SKILL_TREE_KIND>(1));
        if (first_tree && second_tree) {
            std::memcpy(payload->first_tree, first_tree,
                        sizeof(payload->first_tree));
            std::memcpy(payload->second_tree, second_tree,
                        sizeof(payload->second_tree));
        }
        void* commands = user->get_skill_command_buf();
        if (commands) {
            std::memcpy(payload->skill_commands, commands,
                        sizeof(payload->skill_commands));
        }
        LogSkillSaveSummary(user, *payload);
        std::memset(user->skill_commands_, 0,
                    sizeof(user->skill_commands_));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->updateComboSkill();
        user->disableSaveSkill();
    }

    if (static_cast<unsigned char>(user->isSavePvP())) {
        CStreamGuard guard(GlobalData::stream_pool()->Acquire("user.cpp", 7724),
                           true);
        *guard << 37;
        *guard << user->GetUID();
        SIG_SAVE_PVP* payload = guard.GetInBuffer<SIG_SAVE_PVP>();
        std::memset(payload, 0, sizeof(*payload));
        payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        payload->pvp_result.Set(*user->getPVPResultRefR());
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->disableSavePvP();
    }

    if (static_cast<unsigned char>(user->isSaveDungeonClear())) {
        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("user.cpp", 7758), true);
        *guard << 39;
        *guard << user->GetUID();
        SIG_SAVE_DUNGEON_CLEAR* payload =
            guard.GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>();
        std::memset(payload, 0, sizeof(*payload));
        payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        payload->account_id = static_cast<uint32_t>(user->get_acc_id());
        if (reinterpret_cast<const WongWork::CDungeonClear*>(
                user->getDungeonClearRefR())
                ->toString(payload->dungeon, sizeof(payload->dungeon)) <
            0) {
            const int charac_no = user->getCurCharacNo();
            cMyTrace trace("void CUser::UpdateData()", 7774, 5);
            trace("Quest buffer overflow (m_id: %s, charac_no: %d",
                  NumberToString(
                      static_cast<unsigned int>(user->get_acc_id()), 0),
                  charac_no);
        }
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->disableSaveDungeonClear();
    }

    if (static_cast<unsigned char>(user->isSaveBattleRecord())) {
        user->disableSaveBattleRecord();
    }

    if (static_cast<unsigned char>(user->isSaveQuest())) {
        CStreamGuard guard(
            GlobalData::big_stream_pool()->Acquire("user.cpp", 7820), true);
        *guard << 41;
        *guard << user->GetUID();
        SIG_UPDATE_QUEST* payload = guard.GetInBuffer<SIG_UPDATE_QUEST>();
        std::memset(payload, 0, sizeof(*payload));
        UserQuest* quest = user->getCurCharacQuestR();
        if (quest->clear_quest.checkSaveFlag()) {
            std::memcpy(payload->clear_quest,
                        quest->clear_quest.getClearedQuest(),
                        sizeof(payload->clear_quest));
            payload->update_clear_quest = 1;
        }
        payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        for (int index = 0; index <= 19; ++index) {
            payload->play[index] = quest->play_quest[index];
            payload->play_trigger[index] = quest->play_quest_trigger[index];
        }
        payload->urgent_quest_index = quest->urgent_quest_index;
        std::memcpy(payload->quest_notify, quest->quest_notify,
                    sizeof(payload->quest_notify));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->getCurCharacQuestW()->clear_quest.resetSaveFlags();
        user->disableSaveQuest();
    }

    inventory = user->getCurCharacInvenR();
    user_creature::CCreatureMgr* creature_manager = inventory->GetCreatureMgrW();
    if (static_cast<unsigned char>(creature_manager->IsSaveCreatureItem())) {
        creature_manager->SaveCreatureItem();
    }

    const unsigned int event_item_count = static_cast<unsigned int>(
        static_cast<unsigned char>(
            user->GetCurCharacEventItemCount()));
    if (event_item_count) {
        DB_UpdateItemCountValue::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()), event_item_count);
        user->ResetCurCharacEventItemCount();
    }

    if (static_cast<unsigned char>(user->isSaveInformNoticeFlag())) {
        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("user.cpp", 7979), true);
        *guard << 757;
        *guard << -1;
        unsigned char* payload = reinterpret_cast<unsigned char*>(
            guard.GetInBuffer<InformNoticeFlagData>());
        std::memset(payload, 0, 0x14u);
        *reinterpret_cast<uint32_t*>(payload) =
            static_cast<uint32_t>(user->get_acc_id());
        for (int index = 0; index <= 15; ++index) {
            payload[index + 4] = static_cast<unsigned char>(
                user->getInformNoticeFlag(index));
        }
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->disableSaveInformNoticeFlag();
    }

    const unsigned int used_fatigue = static_cast<unsigned int>(
        user->GetCurCharacUsedFatigueQuantity());
    if (used_fatigue &&
        DB_UpdateUserFatigueQuantity::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()), used_fatigue)) {
        user->ResetCurCharacUsedFatigueQuantity();
    }

    if (static_cast<unsigned char>(user->GetChangedGiftFatigueQuantity())) {
        DB_UpdateUserGiftFatigueQuantity::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            static_cast<unsigned int>(user->getCurCharacNo()),
            static_cast<unsigned int>(static_cast<unsigned char>(
                user->GetCurCharacUsedGiftFatigueQuantity())));
        user->SetChangedGiftFatigueQuantity(false);
    }

    EventClassify::CEventScriptMng* event_script_mng =
        G_CDataManager()->get_event_script_mng();
    if (event_script_mng->is_eventing(411) &&
        static_cast<unsigned char>(
            user->getCurUseBringUpMaterialCount())) {
        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("user.cpp", 8051), true);
        *guard << 714;
        *guard << -1;
        SIG_OBJECT_BRING_UP* payload =
            guard.GetInBuffer<SIG_OBJECT_BRING_UP>();
        payload->m_id = static_cast<uint32_t>(user->get_acc_id());
        payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        payload->server_no = static_cast<unsigned char>(
            G_CEnvironment()->get_server_group());
        payload->use_material_count = static_cast<unsigned char>(
            user->getCurUseBringUpMaterialCount());
        payload->give_reward_flag = 0;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        user->resetCurUseBringUpMaterialCount();
    }

    const unsigned int event_add_item_count = static_cast<unsigned int>(
        static_cast<unsigned char>(
            user->GetCurCharacEventAddItemCount()));
    if (event_add_item_count) {
        DB_UpdateAddItemCountValue::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            event_add_item_count);
        user->ResetCurCharacEventAddItemCount();
    }

    for (int hack_type = 101; hack_type <= 4000; ++hack_type) {
        WongWork::CHackAnalyzer* const analyzer = user->getHackAnalyzer();
        const WongWork::ENUM_HACKTYPE typed_hack_type =
            static_cast<WongWork::ENUM_HACKTYPE>(hack_type);
        if (analyzer->getServerHackCnt(typed_hack_type)) {
            const int server_count =
                analyzer->getServerHackCnt(typed_hack_type);
            reinterpret_cast<WongWork::CHackAnalyzer*>(
                static_cast<uintptr_t>(user->get_acc_id()))->ReqDBSaveHackInfo(
                static_cast<unsigned int>(user->get_charac_no(-1)),
                static_cast<unsigned short>(hack_type),
                user->GetETC(static_cast<short>(hack_type)),
                server_count);
            analyzer->resetServerHackCnt(typed_hack_type);
        }
    }

    const unsigned short mcap_point =
        static_cast<unsigned short>(mcap_manager_.getPoint());
    if (mcap_point > 0x1F3u) {
        reinterpret_cast<WongWork::CHackAnalyzer*>(
            static_cast<uintptr_t>(user->get_acc_id()))->ReqDBSaveHackInfo(
            static_cast<unsigned int>(user->get_charac_no(-1)),
            0x329u,
            mcap_point,
            mcap_manager_.getExposedCount());
        mcap_manager_.resetExposedCount();
    }

    const int school_point = user->getCurCharacSchoolPoint();
    if (school_point) {
        DB_UpdateSchoolPoint::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            school_point,
            user->GetUID());
        user->resetCurCharacSchoolPoint();
    }

    if (static_cast<unsigned char>(
            user->isBloodBestRecordUpdate())) {
        user->UpdateBloodRecord();
    }
    if (dungeon_runtime::IsInCountDirty(user) ||
        (user->get_charac_level() > 59 &&
         (static_cast<unsigned char>(
              user->IsSaveDemensionInoutValue()) ||
          static_cast<unsigned char>(
              user->IsSaveBloodInoutValue())))) {
        user->UpdateDimensionInout();
    }
    if (static_cast<unsigned char>(user->isUsedBreakAwayLuckPoint())) {
        user->UpdateBreakAwayLuckPoint();
    }
    if (static_cast<unsigned char>(user->isBreakAwayDungeonClear())) {
        user->UpdateBreakAwayDungeonClearCnt();
    }

    const unsigned int link_exp = static_cast<unsigned int>(
        user->getCharacLinkBonusExp());
    const unsigned int link_gold = static_cast<unsigned int>(
        user->getCharacLinkBonusGold());
    if (link_exp || link_gold) {
        const char link_slot = static_cast<char>(
            user->getLinkCharacSlotIndex());
        user->UpdateCharacLinkBonus(link_exp,
                                    link_gold,
                                    user->getLinkCharacNo(link_slot),
                                    static_cast<ENUM_CHARAC_LINK_BONUS_STATE>(2));
    }

    user->_updateNPCRelationship();
    user->charac_expand_data_.update(user);
    user->_saveConditionEventInfo(
        static_cast<unsigned int>(user->getCurCharacNo()),
        static_cast<short>(user->GetCurConditionEventStep()),
        static_cast<short>(
            user->GetCurConditionEventRewardStep()));
    user->_saveProperDungeonClearCount(
        static_cast<unsigned int>(user->getCurCharacNo()),
        static_cast<short>(user->GetProperDungeonClearCount()));
    if (static_cast<unsigned char>(user->isSaveCharacOption())) {
        user->_updateSaveOption();
    }

    if (static_cast<unsigned char>(user->IsExistAccountCargo())) {
        CAccountCargo* account_cargo = static_cast<CAccountCargo*>(
            user->GetAccountCargo());
        if (static_cast<unsigned char>(account_cargo->IsAlter())) {
            DB_SaveAccountCargo::makeRequest(
                user->GetUID(),
                static_cast<unsigned int>(user->get_acc_id()),
                static_cast<CAccountCargo*>(account_cargo),
                user->GetServerGroup(),
                static_cast<unsigned int>(user->getCurCharacNo()));
        }
    }

    reinterpret_cast<Secu_GoldControl*>((user->GetGoldControl()))->SavetoDB(true, true, false);
    user->SaveMercenaryData();
    user->updatePcRoomPlayTime();
    user->updateBlueMarbleEnterCount();
    return static_cast<char>(reinterpret_cast<uintptr_t>(
        user->updateBingoData()));
}
