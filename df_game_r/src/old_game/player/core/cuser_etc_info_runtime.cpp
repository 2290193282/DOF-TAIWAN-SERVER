#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/progression/quest/quest_runtime.h"

#include <cstring>
#include <map>
#include <utility>


namespace {

// IDA_EA: CUser::GetEtcInfo @0x8677A79..0x8677DC9.  These are the
// character-expand manager's fixed old slot identities, not arbitrary
// payload selectors.
const ENUM_CHARAC_EXPAND_TYPE kExpandRedeemItem =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(6);
const ENUM_CHARAC_EXPAND_TYPE kExpandFairPvp =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(7);
const ENUM_CHARAC_EXPAND_TYPE kExpandPvpMission =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(8);
const ENUM_CHARAC_EXPAND_TYPE kExpandItemMakingSkill =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(10);
const ENUM_CHARAC_EXPAND_TYPE kExpandQuestShop =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(11);
const ENUM_CHARAC_EXPAND_TYPE kExpandTodUserState =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(13);
const ENUM_CHARAC_EXPAND_TYPE kExpandTitleBook =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(14);
const ENUM_CHARAC_EXPAND_TYPE kExpandAchievement =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(15);
const ENUM_CHARAC_EXPAND_TYPE kExpandItemDictionary =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(16);
const ENUM_CHARAC_EXPAND_TYPE kExpandBoosterGage =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(17);
const ENUM_CHARAC_EXPAND_TYPE kExpandBestClearTime =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(18);
const ENUM_CHARAC_EXPAND_TYPE kExpandCharacterInformNotice =
    static_cast<ENUM_CHARAC_EXPAND_TYPE>(19);

typedef std::map<int, STSkillCommandData> SkillCommandMap;

}  // namespace

// IDA_EA: 0x8654768
// IDA_NAME: _ZNK5CUser21get_skill_command_bufEPN18STSkillCommandData9STCommandE
// DEMANGLED: CUser::get_skill_command_buf(STSkillCommandData::STCommand *)const
STSkillCommandData::STCommand* CUser::get_skill_command_buf(
    STSkillCommandData::STCommand* output) const {
    SkillCommandMap* const commands = getSkillCommand();
    STSkillCommandData::STCommand* result =
        reinterpret_cast<STSkillCommandData::STCommand*>(commands);

    if (commands) {
        SkillCommandMap::iterator it = commands->begin();
        const SkillCommandMap::iterator end = commands->end();
        int slot = 0;

        while (it != end) {
            STSkillCommandData::STCommand command;
            uint8_t* const packed = command.raw;
            const STSkillCommandData& value = it->second;

            *reinterpret_cast<uint16_t*>(packed) = static_cast<uint16_t>(
                (value.word_00 & 0x3fffu) |
                (*reinterpret_cast<uint16_t*>(packed) & 0xc000u));
            packed[3] = static_cast<uint8_t>(
                ((value.byte_02 & 7u) << 5) | (packed[3] & 0x1fu));
            *reinterpret_cast<uint32_t*>(packed) =
                ((static_cast<uint32_t>(value.raw_03[0] & 7u)) << 14) |
                (*reinterpret_cast<uint32_t*>(packed) & 0xfffe3fffu);
            packed[2] = static_cast<uint8_t>(
                ((value.raw_03[1] & 7u) << 1) | (packed[2] & 0xf1u));
            packed[2] = static_cast<uint8_t>(
                ((value.raw_03[2] & 7u) << 4) | (packed[2] & 0x8fu));
            *reinterpret_cast<uint16_t*>(packed + 2) = static_cast<uint16_t>(
                ((value.raw_03[3] & 7u) << 7) |
                (*reinterpret_cast<uint16_t*>(packed + 2) & 0xfc7fu));
            packed[3] = static_cast<uint8_t>(
                ((value.raw_03[4] & 7u) << 2) | (packed[3] & 0xe3u));

            output[slot] = command;
            result = output + slot;
            if (++slot > 49) {
                break;
            }
            ++it;
        }

    }

    return result;
}

// IDA_EA: 0x864DCF2
// IDA_NAME: _ZNK5CUser8GetSkillEP14SIG_LOAD_SKILLP15SIG_COMBO_SKILL
// DEMANGLED: CUser::GetSkill(SIG_LOAD_SKILL *,SIG_COMBO_SKILL *)const
int CUser::GetSkill(SIG_LOAD_SKILL* skill,
                    SIG_COMBO_SKILL* combo_skill) const {
    skill->result = 0;

    SkillSlot* const current_skill = getCurCharacSkillR();
    int remain_sp = 0;
    int remain_sp_second = 0;
    int remain_sfp_first = 0;
    int remain_sfp_second = 0;
    current_skill->get_remain_sp(&remain_sp,
                                  &remain_sp_second,
                                  &remain_sfp_first,
                                  &remain_sfp_second);
    skill->remain_sp = static_cast<uint32_t>(remain_sp);
    skill->remain_sp_second = static_cast<uint32_t>(remain_sp_second);
    skill->remain_sfp_first = static_cast<uint32_t>(remain_sfp_first);
    skill->remain_sfp_second = static_cast<uint32_t>(remain_sfp_second);

    getCurCharacSkillR()->get_skill_slot(
        reinterpret_cast<char*>(skill->first_tree),
        reinterpret_cast<char*>(skill->second_tree),
        sizeof(skill->first_tree));

    STSkillCommandData::STCommand commands[50];
    std::memset(commands, 0, sizeof(commands));
    get_skill_command_buf(commands);
    std::memcpy(skill->skill_commands, commands, sizeof(commands));

    skill->script_version =
        G_CDataManager()->get_pvp_skill_tree_parameter_script()->version;
    getCurCharacSkillR()->getComboSkillSaveData(combo_skill);
    return 1;
}

// IDA_EA: 0x864DB20
// IDA_NAME: _ZN5CUser8SetSkillEP14SIG_LOAD_SKILLP15SIG_COMBO_SKILL
// DEMANGLED: CUser::SetSkill(SIG_LOAD_SKILL *,SIG_COMBO_SKILL *)
int CUser::SetSkill(SIG_LOAD_SKILL* skill, SIG_COMBO_SKILL* combo_skill) {
    const int remain_sp = static_cast<int>(skill->remain_sp);
    char* const first_tree = reinterpret_cast<char*>(skill->first_tree);
    const int remain_sp_second = static_cast<int>(skill->remain_sp_second);
    char* const second_tree = reinterpret_cast<char*>(skill->second_tree);
    const int remain_sfp_first = static_cast<int>(skill->remain_sfp_first);
    const int remain_sfp_second = static_cast<int>(skill->remain_sfp_second);

    if (getCurCharacR()) {
        getCurCharacSkillW()->set_remain_sp(remain_sp,
                                             remain_sp_second,
                                             remain_sfp_first,
                                             remain_sfp_second);
        getCurCharacSkillW()->set_skill_slot(first_tree,
                                             second_tree,
                                             sizeof(skill->first_tree));
        setSkillCommandData(
            reinterpret_cast<char*>(skill->skill_commands));
        getCurCharacSkillW()->SetLoadStrikerSkill(false);
        if (get_charac_job() ==
            oldsrv::config::kCharacterJobDemonicSwordman) {
            getCurCharacSkillW()->loadComboSkill(combo_skill);
            getCurCharacSkillW()->verifyComboSkill();
        }
        return 1;
    }

    cMyTrace trace("bool CUser::SetSkill(SIG_LOAD_SKILL*, SIG_COMBO_SKILL*)",
                   5037,
                   5);
    trace("CUser::SetSkill User  m_selected is null");
    return 0;
}

// IDA_EA: 0x8654924
// IDA_NAME: _ZN5CUser19setSkillCommandDataEPc
// DEMANGLED: CUser::setSkillCommandData(char *)
void* CUser::setSkillCommandData(char* source) {
    std::memcpy(skill_commands_, source, sizeof(skill_commands_));

    SkillCommandMap* const commands = getSkillCommand();
    if (!commands) {
        return commands;
    }

    for (int slot = 0; slot <= 49; ++slot) {
        const STSkillCommandData::STCommand& packed = skill_commands_[slot];
        const uint16_t skill_index = static_cast<uint16_t>(
            static_cast<uint16_t>(packed.raw[0]) |
            (static_cast<uint16_t>(packed.raw[1] & 0x3fu) << 8));
        if (!skill_index) {
            break;
        }

        STSkillCommandData command;
        command.word_00 = skill_index;
        command.byte_02 = static_cast<uint8_t>(packed.raw[2] >> 5);
        command.raw_03[0] = static_cast<uint8_t>(
            ((static_cast<uint16_t>(packed.raw[1]) |
              (static_cast<uint16_t>(packed.raw[2]) << 8)) >> 6) &
            7u);
        command.raw_03[1] = static_cast<uint8_t>((packed.raw[1] >> 1) & 7u);
        command.raw_03[2] = static_cast<uint8_t>((packed.raw[1] >> 4) & 7u);
        command.raw_03[3] = static_cast<uint8_t>(
            ((static_cast<uint32_t>(packed.raw[1]) |
              (static_cast<uint32_t>(packed.raw[2]) << 8) |
              (static_cast<uint32_t>(packed.raw[3]) << 16)) >>
             15) &
            7u);
        command.raw_03[4] = static_cast<uint8_t>((packed.raw[2] >> 2) & 7u);

        SkillCommandMap::iterator found = commands->find(command.word_00);
        if (found == commands->end()) {
            commands->insert(std::make_pair(static_cast<int>(command.word_00),
                                            command));
        } else {
            found->second.word_00 = command.word_00;
            found->second.byte_02 = command.byte_02;
            for (int index = 0; index < command.byte_02; ++index) {
                found->second.raw_03[index] = command.raw_03[index];
            }
        }
    }

    return std::memset(skill_commands_, 0, sizeof(skill_commands_));
}

// IDA_EA: 0x8677DD8
// IDA_NAME: _ZNK5CUser9GetRecordEP12SIG_LOAD_ETC
// DEMANGLED: CUser::GetRecord(SIG_LOAD_ETC *)const
SIG_LOAD_ETC* CUser::GetRecord(SIG_LOAD_ETC* payload) const {
    const std::map<int, GameResultType>* const records =
        getBattleRecordRefR();
    size_t index = 0;

    for (std::map<int, GameResultType>::const_iterator it = records->begin();
         it != records->end() && index <= 99u;
         ++it, ++index) {
        payload->battle_records[index] = it->second;
    }

    payload->battle_record_count = static_cast<uint32_t>(records->size());
    return payload;
}

// IDA_EA: 0x8688DC4
// IDA_NAME: _ZNK5CUser17GetLimitItemUsageEP12SIG_LOAD_ETC
// DEMANGLED: CUser::GetLimitItemUsage(SIG_LOAD_ETC *)const
void CUser::GetLimitItemUsage(SIG_LOAD_ETC* payload) const {
    if (!payload) {
        return;
    }

    payload->limit_item_info.count = 0;
    uint32_t index = 0;
    for (std::vector<LimitItemUsage>::const_iterator it =
             limit_item_usage_.begin();
         it != limit_item_usage_.end();
        ++it, ++index) {
        LimitItemUsagePayload& output =
            payload->limit_item_info.entries[index];
        // 0x8688DC4 copies LimitItemUsage+0x0c (CommonTime's current_time)
        // into the record's +0x04 field.  It does not serialize the usage
        // limit at +0x11; that value is reconstructed from the PVF script
        // by SetLimitInfo during character load.
        std::memcpy(&output.last_usage_time,
                    &it->common_time.current_time,
                    sizeof(output.last_usage_time));
        output.item_id = static_cast<uint32_t>(it->item_id);
        output.used_count = static_cast<uint32_t>(it->used_count);
        output.limit_type = it->limit_type;
    }
    payload->limit_item_info.count = index;
}

// IDA_EA: 0x867794C
// IDA_NAME: _ZNK5CUser10GetEtcInfoEP12SIG_LOAD_ETC
// DEMANGLED: CUser::GetEtcInfo(SIG_LOAD_ETC *)const
int CUser::GetEtcInfo(SIG_LOAD_ETC* payload) const {
    payload->result = 0;
    getPVPResultR()->Get(payload->pvp_result);
    GetRecord(payload);
    GetContinuousItemInfo(payload);
    GetLimitItemUsage(payload);

    payload->one_day_lethe_state =
        static_cast<uint8_t>(GetOneDayLetheFlag());
    std::memcpy(&payload->premium_lethe,
                GetCurCharacBackupSkill(),
                sizeof(payload->premium_lethe));
    payload->one_day_lethe_second_state =
        static_cast<uint8_t>(GetOneDayLetheFlag2ND());
    std::memcpy(&payload->premium_lethe_second,
                GetCurCharacBackupSkill2ND(),
                sizeof(payload->premium_lethe_second));
    payload->condition_event_step =
        static_cast<uint16_t>(GetCurConditionEventStep());
    payload->condition_event_reward_step =
        static_cast<uint16_t>(GetCurConditionEventRewardStep());
    payload->proper_dungeon_clear_count =
        static_cast<uint16_t>(GetProperDungeonClearCount());
    GetCharacExpandDataR(kExpandRedeemItem)
        ->getData(reinterpret_cast<char*>(&payload->redeem_items));

    payload->village_attack_count =
        static_cast<uint8_t>(GetCurVillageAttackCount());
    payload->revenge_dungeon_count =
        static_cast<uint8_t>(GetCurRevengeDungeonCount());
    payload->avenger_title_received =
        static_cast<uint8_t>(getFlagPresentAvengerTitle());
    payload->pc_room_play_time.play_time = getTotalPcRoomPlayTime();
    payload->pc_room_play_time.reward_index = getPcRoomPlayTimeRewardCnt();
    payload->used_gift_fatigue = GetCurCharacUsedGiftFatigueQuantity();
    payload->user_confirm_flags.set();
    GetSkill(&payload->skill, &payload->combo_skill);

    GetCharacExpandDataR(kExpandFairPvp)
        ->getData(reinterpret_cast<char*>(&payload->fair_pvp));
    GetCharacExpandDataR(kExpandTodUserState)
        ->getData(reinterpret_cast<char*>(&payload->tod_user_state));
    GetCharacExpandDataR(kExpandPvpMission)
        ->getData(reinterpret_cast<char*>(&payload->pvp_missions));
    GetCharacExpandDataR(kExpandQuestShop)
        ->getData(reinterpret_cast<char*>(&payload->quest_shop));

    if (getTagCharacR()) {
        getTagCharacSkillR()->get_skill_slot(
            reinterpret_cast<char*>(payload->tag_charac_etc.skill_trees[0]),
            reinterpret_cast<char*>(payload->tag_charac_etc.skill_trees[1]),
            sizeof(payload->tag_charac_etc.skill_trees[0]));
    }

    GetCharacExpandDataR(kExpandItemMakingSkill)
        ->getData(reinterpret_cast<char*>(&payload->item_making_skill));
    GetCharacExpandDataR(kExpandItemDictionary)
        ->getData(reinterpret_cast<char*>(&payload->item_dictionary));
    GetCharacExpandDataR(kExpandTitleBook)
        ->getData(reinterpret_cast<char*>(&payload->title_book));
    GetCharacExpandDataR(kExpandAchievement)
        ->getData(reinterpret_cast<char*>(&payload->achievement));
    GetCharacExpandDataR(kExpandBoosterGage)
        ->getData(reinterpret_cast<char*>(&payload->booster_gage));
    GetCharacExpandDataR(kExpandBestClearTime)
        ->getData(reinterpret_cast<char*>(&payload->best_clear_time));
    GetAuraAvatarOption(payload->aura_avatar_option);
    GetCurCharacOption(payload->character_option);
    GetCharacCashAttendanceInfo(payload->attendance);
    payload->blue_marble_enter_count =
        static_cast<uint8_t>(getBlueMarbleEnterCount());
    GetCharacExpandDataR(kExpandCharacterInformNotice)
        ->getData(payload->charac_inform_notice);
    return 1;
}
