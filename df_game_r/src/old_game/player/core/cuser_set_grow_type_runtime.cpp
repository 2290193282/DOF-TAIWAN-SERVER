#include "old_game/player/core/cuser.h"

#include "old_game/progression/skill/create_character_skill_abi.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_changer_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <utility>
#include <vector>




// IDA_EA: 0x86787FC
// IDA_NAME: _ZN5CUser13set_grow_typeEhhPc21eChangeGrowTypeReason
// DEMANGLED: CUser::set_grow_type(unsigned char,unsigned char,char *,eChangeGrowTypeReason)
void CUser::set_grow_type(unsigned char first_grow_type,
                          unsigned char second_grow_type,
                          char* packet,
                          eChangeGrowTypeReason reason) {
    if (first_grow_type > 5u || second_grow_type > 2u) {
        LogManager::logFormat(
            1,
            "user.cpp",
            "void CUser::set_grow_type(unsigned char, unsigned char, char*, eChangeGrowTypeReason)",
            0x713b,
            "User %s - grow type < 1 || grow type >= GROW_TYPE_NUMBER",
            getCurCharacName());
        return;
    }

    if (second_grow_type == 0u) {
        CMissionList_Charac* const mission_list =
            static_cast<CMissionList_Charac*>(GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        mission_list->Remove_Mission(31);
        mission_list->MakeMissionList_JustKind(*this, 0x20u);
    }

    const int reason_value = static_cast<int>(reason);
    if (reason_value == 1 || second_grow_type == 0u) {
        CQuestShop* const quest_shop = static_cast<CQuestShop*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
        if (quest_shop) {
            quest_shop->clearQP(this);
            quest_shop->sendCharacQp(this, 1);
        }
    }

    const int current_grow_type = getCurCharacGrowType();
    const int current_village = getCurCharacVill();
    const int character_job = get_charac_job();
    if (second_grow_type != 0u) {
        history_log().ChangeGrowType(character_job,
                                     current_village,
                                     second_grow_type,
                                     current_grow_type,
                                     2,
                                     reason);
    } else {
        history_log().ChangeGrowType(character_job,
                                     current_village,
                                     first_grow_type,
                                     current_grow_type,
                                     1,
                                     reason);
    }

    InterfacePacketBuf* const output =
        reinterpret_cast<InterfacePacketBuf*>(packet);
    setCurCharGrowthType(static_cast<char>(first_grow_type),
                          static_cast<char>(second_grow_type));
    character_view_.enableSaveCharacView();

    if (second_grow_type == 0u) {
        adjust_charac_stat();
        if (state_ == 3u) {
            PacketGuard basic_info_packet;
            basic_info_packet.clear();
            basic_info_packet.put_header(0, 2);
            basic_info_packet.put_byte(1);
            basic_info_packet.put_short(1);
            make_basic_info(basic_info_packet, 1);
            basic_info_packet.finalize(true);
            Send(basic_info_packet);
        }
    }

    std::vector<std::pair<int, int> > give_skills;
    int give_skill_result = 0;
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        give_skill_result = 1;
    } else if (reason_value != 2 && reason_value != 0) {
        if (reason_value == 1) {
            CCharacList* const character_list =
                G_CDataManager()->get_character_list();
            CCharacter* const character =
                &character_list->characters_[get_charac_job()];
            give_skill_result = character->get_give_skill(
                first_grow_type,
                second_grow_type,
                give_skills,
                second_grow_type != 0u ? 0u : 1u);
        }
    } else if (second_grow_type != 0u) {
        CCharacList* const character_list =
            G_CDataManager()->get_character_list();
        CCharacter* const character =
            &character_list->characters_[get_charac_job()];
        give_skill_result = character->get_give_skill(first_grow_type,
                                                       second_grow_type,
                                                       give_skills,
                                                       2u);
    } else {
        getCurCharacSkillW()->clear_all_skills_both();
        getCurCharacSkillW()->set_parent(this);
        addSkillOnCreateCharacter(
            getCurCharacSkillW()->get_mastered_skill_tree(
                ENUM_SKILL_TREE_0),
            get_charac_job());
        addSkillOnCreateCharacter(
            getCurCharacSkillW()->get_mastered_skill_tree(
                ENUM_SKILL_TREE_1),
            get_charac_job());

        WongWork::CSkillChanger skill_changer;
        skill_changer.SkillInitialize(this, 0, false);
        send_skill_info();

        CCharacList* const character_list =
            G_CDataManager()->get_character_list();
        CCharacter* const character =
            &character_list->characters_[get_charac_job()];
        give_skill_result = character->get_give_skill(first_grow_type,
                                                       0,
                                                       give_skills,
                                                       1u);
    }

    if (give_skill_result != 1) {
        LogManager::logFormat(
            1,
            "user.cpp",
            "void CUser::set_grow_type(unsigned char, unsigned char, char*, eChangeGrowTypeReason)",
            0x720c,
            "User %s - CUser::set_grow_type first_grow_type %d, second_grow_type %d",
            getCurCharacName(),
            first_grow_type,
            second_grow_type);
        return;
    }

    NormalizeAutomaticGiveSkillLevels(
        this, get_charac_job(), first_grow_type, second_grow_type,
        give_skills);

    if (output) {
        int count_index = output->get_index();
        int count = 0;
        output->put_byte(0);
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 give_skills.begin();
             iter != give_skills.end();
             ++iter) {
            const int loop_job = get_charac_job();
            SkillSlot* const loop_skill_slot = getCurCharacSkillW();
            const int level = loop_skill_slot->growtype_skill(
                loop_job, iter->first, iter->second, ENUM_SKILL_TREE_0);
            if (level > 0) {
                output->put_byte(level);
                output->put_byte(iter->first);
                output->put_byte(iter->second);
                ++count;
            }
        }
        output->put_byte(count_index, count);

        count_index = output->get_index();
        count = 0;
        output->put_byte(0);
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 give_skills.begin();
             iter != give_skills.end();
             ++iter) {
            const int loop_job = get_charac_job();
            SkillSlot* const loop_skill_slot = getCurCharacSkillW();
            const int level = loop_skill_slot->growtype_skill(
                loop_job, iter->first, iter->second, ENUM_SKILL_TREE_1);
            if (level > 0) {
                output->put_byte(level);
                output->put_byte(iter->first);
                output->put_byte(iter->second);
                ++count;
            }
        }
        output->put_byte(count_index, count);
    } else {
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 give_skills.begin();
             iter != give_skills.end();
             ++iter) {
            const int first_tree_job = get_charac_job();
            getCurCharacSkillW()->growtype_skill(first_tree_job,
                                                  iter->first,
                                                  iter->second,
                                                  ENUM_SKILL_TREE_0);
            const int second_tree_job = get_charac_job();
            getCurCharacSkillW()->growtype_skill(second_tree_job,
                                                  iter->first,
                                                  iter->second,
                                                  ENUM_SKILL_TREE_1);
        }
    }

    // A second-awakening transition publishes only its stage-2 delta above.
    // Reconcile the already-attained stage-1 fixed active as a separate
    // no-SP invariant so legacy level 0 cannot survive the transition.
    (void)ReconcileAutomaticGiveSkillLevels(this, true);

    _UpdateUserInfo4Guild();
    const short guild_grow_type = static_cast<short>(
        static_cast<signed char>(getCurCharacGrowType()));
    const short guild_character_level =
        static_cast<short>(get_charac_level());
    const unsigned int guild_character_no =
        static_cast<unsigned int>(get_charac_no(-1));
    const unsigned int guild_account_id =
        static_cast<unsigned int>(get_acc_id());
    const ENUM_SERVER_GROUP guild_server_group =
        static_cast<ENUM_SERVER_GROUP>(GetServerGroup());
    GlobalData::s_guild_proxy_mgr
        ->GetServerProxy(guild_server_group)
        ->SendCharLevelGrowType(guild_account_id,
                                guild_character_no,
                                guild_character_level,
                                guild_grow_type);

    const short monitor_grow_type = static_cast<short>(
        static_cast<signed char>(getCurCharacGrowType()));
    const short monitor_character_level =
        static_cast<short>(get_charac_level());
    const unsigned int monitor_character_no =
        static_cast<unsigned int>(get_charac_no(-1));
    const unsigned int monitor_account_id =
        static_cast<unsigned int>(get_acc_id());
    const ENUM_SERVER_GROUP monitor_server_group =
        static_cast<ENUM_SERVER_GROUP>(GetServerGroup());
    GlobalData::s_monitor_proxy_mgr
        ->GetServerProxy(monitor_server_group)
        ->SendCharLevelGrowType(monitor_account_id,
                                monitor_character_no,
                                monitor_character_level,
                                monitor_grow_type);
    processGrowTypeEvent(first_grow_type, second_grow_type, reason);
}
