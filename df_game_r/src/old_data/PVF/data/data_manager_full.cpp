// Canonical B-owned CDataManager full provider snapshot.
// Source is retained at src/global/data_manager.cpp because that file has pre-existing user modifications.
// Do not add the legacy source to a manifest; this canonical unit is the sole B build provider.

#include "old_data/PVF/data/data_manager.h"

#include "common/config.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/channel/pvp_channel_info_t.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/character/datamanager/character_create_creature_gift_script.h"
#include "old_data/PVF/data/datamanager/create_chrac_script.h"
#include "old_data/PVF/data/datamanager/db_log_item.h"
#include "core/config/env/environment.h"
#include "old_data/PVF/equipment/growth_equipment_script.h"
#include "old_data/PVF/genuine_upgrade_table.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_data/PVF/etc/title_book_info.h"
#include "old_data/PVF/etc/striker_script.h"
#include "old_data/PVF/itemshop/secret_shop_script.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/PVF/event/event_avatar_list_script.h"
#include "old_data/PVF/event/event_reward_script.h"
#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_data/PVF/n_quest/training_quest_script.h"
#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_data/PVF/event/datamanager/seria_room_deco_animation_event_script.h"
#include "old_data/PVF/event/seria_room_deco_event_script.h"
#include "old_data/PVF/character/datamanager/use_character_job_fatigue_script.h"
#include "old_data/Login/account_cargo/account_cargo_script.h"
#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "old_game/activity/reward/advance_altar_reward_parameter_abi.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"
#include "common/manager/bingo_broadcast_script_abi.h"
#include "common/manager/chatting_emoticon.h"
#include "common/manager/create_chrac_script_abi.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "old_game/appearance/avatar/creature_script_public_abi.h"
#include "old_game/activity/reward/event_reward_abi.h"
#include "old_game/activity/reward/event_character_parameter_script_abi.h"
#include "old_data/PVF/etc/equipment_grade_sample.h"
#include "old_game/progression/career/expert_job_script_abi.h"
#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_power_parameter_script_abi.h"
#include "old_game/social/party/hellparty_characlink_abi.h"
#include "old_data/PVF/etc/independent_drop_types.h"
#include "common/manager/invalid_char_data.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/image_communication_info_abi.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "common/manager/in_game_advertisement_script_abi.h"
#include "old_game/activity/reward/in_game_event_runtime_abi.h"
#include "old_data/PVF/etc/item_making_skill_script.h"
#include "old_game/activity/reward/level_reward_script_abi.h"
#include "old_game/economy/inventory/limit_item_usage_info_abi.h"
#include "common/manager/live_server_info_t.h"
#include "common/manager/npc_relation_condition_lifecycle_abi.h"
#include "old_game/progression/quest/quest_training_lifecycle_abi.h"
#include "old_game/progression/quest/quest_shop_script_abi.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "common/manager/reseal_table.h"
#include "common/manager/secret_shop_script_abi.h"
#include "common/manager/server_parameter_script_abi.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "common/manager/seria_blessing_script_abi.h"
#include "old_game/activity/reward/seria_room_deco_animation_event_script_abi.h"
#include "old_game/activity/reward/seria_room_deco_event_script_abi.h"
#include "common/manager/slang_filter.h"
#include "common/manager/slang_name_data.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_game/economy/trade/vending_machine_public_abi.h"
#include "old_game/world/region/village_attacked_abi.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"

#include <map>
#include <new>
#include <set>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <vector>

// IDA_EA: 0x837390A
// IDA_NAME: _ZN14ChoiceItemInfoC2Ev
// DEMANGLED: ChoiceItemInfo::ChoiceItemInfo(void)
ChoiceItemInfo::ChoiceItemInfo() { clear(); }

// IDA_EA: 0x837391E
// IDA_NAME: _ZN14ChoiceItemInfo5clearEv
// DEMANGLED: ChoiceItemInfo::clear(void)
ChoiceItemInfo* ChoiceItemInfo::clear() {
    enabled = 0;
    item_id = -1;
    return this;
}


reseal_table_t* CDataManager::get_reseal_table() {
    return &reseal_table_;
}

const reseal_table_t* CDataManager::get_reseal_table() const {
    return &reseal_table_;
}

const std::map<int, int>&
CDataManager::get_item_dictionary_item_to_flag_map() const {
    return item_dictionary_item_to_flag_;
}

const std::map<int, int>&
CDataManager::get_item_dictionary_flag_to_item_map() const {
    return item_dictionary_flag_to_item_;
}

stQuestShop* CDataManager::get_quest_shop_script() {
    return &quest_shop_script_;
}

const stQuestShop* CDataManager::get_quest_shop_script() const {
    return &quest_shop_script_;
}

stBingoScript* CDataManager::get_bingo_script() {
    return &bingo_script_;
}

const stBingoScript* CDataManager::get_bingo_script() const {
    return &bingo_script_;
}

FairPvPStatScript* CDataManager::get_fair_pvp_stat_script() {
    return &fair_pvp_stat_script_;
}

const FairPvPStatScript* CDataManager::get_fair_pvp_stat_script() const {
    return &fair_pvp_stat_script_;
}

RandomOptionScript* CDataManager::get_random_option_script() {
    return &random_option_script_;
}

const RandomOptionScript* CDataManager::get_random_option_script() const {
    return &random_option_script_;
}

RegenerationROI* CDataManager::get_regeneration_roi() {
    return &regeneration_roi_;
}

const RegenerationROI* CDataManager::get_regeneration_roi() const {
    return &regeneration_roi_;
}

BoosterGageData* CDataManager::get_booster_gage_data() {
    return &booster_gage_data_;
}

const BoosterGageData* CDataManager::get_booster_gage_data() const {
    return &booster_gage_data_;
}

PcBangItemRentarData* CDataManager::get_pc_bang_item_rental_data() {
    return &pc_bang_item_rental_data_;
}

const PcBangItemRentarData*
CDataManager::get_pc_bang_item_rental_data() const {
    return &pc_bang_item_rental_data_;
}

DimensionActivationData* CDataManager::get_dimension_activation_data() {
    return &dimension_activation_data_;
}

const DimensionActivationData*
CDataManager::get_dimension_activation_data() const {
    return &dimension_activation_data_;
}

const SlangNameData* CDataManager::get_slang_name_data() const {
    return &slang_name_data_;
}

TrainingQuestScript* CDataManager::get_training_quest_script() {
    return &training_quest_script_;
}

const TrainingQuestScript* CDataManager::get_training_quest_script() const {
    return &training_quest_script_;
}

stTitleBookInfo* CDataManager::get_title_book_info() {
    return &title_book_info_;
}

const stTitleBookInfo* CDataManager::get_title_book_info() const {
    return &title_book_info_;
}

DeleteInvalidItemScript* CDataManager::get_delete_invalid_item_script() {
    return &delete_invalid_item_script_;
}

const DeleteInvalidItemScript*
CDataManager::get_delete_invalid_item_script() const {
    return &delete_invalid_item_script_;
}

WarAreaCounter* CDataManager::get_war_area_counter() {
    return &war_area_counter_;
}

const WarAreaCounter* CDataManager::get_war_area_counter() const {
    return &war_area_counter_;
}

AvatarVariation::AvatarColorInfo* CDataManager::get_avatar_color_info() {
    return &avatar_color_info_;
}

const AvatarVariation::AvatarColorInfo*
CDataManager::get_avatar_color_info() const {
    return &avatar_color_info_;
}

StageMapList* CDataManager::get_stage_map_list() {
    return &stage_map_list_;
}

const StageMapList* CDataManager::get_stage_map_list() const {
    return &stage_map_list_;
}

GuildParameterScript* CDataManager::GetGuildParameterScriptForBuild() {
    return &guild_parameter_script_;
}

const GuildParameterScript*
CDataManager::GetGuildParameterScriptForBuild() const {
    return &guild_parameter_script_;
}

int CDataManager::GetNormalFatigueLuckPointRateForBuild() const {
    return server_parameter_script_.get_normal_fatigue_luck_point_rate();
}

int CDataManager::GetPremiumFatigueLuckPointRateForBuild() const {
    return server_parameter_script_.get_premium_fatigue_luck_point_rate();
}

namespace {

const unsigned int kMobRewardCopySize = 0x324u;
const unsigned int kMobRewardCount = 201u;
const char kServerNameCain[] = "\xC4\xAB\xC0\xCE";
const char kServerNameDiregie[] = "\xB5\xF0\xB7\xB9\xC1\xF6\xBF\xA1";
const char kServerNameSiroco[] = "\xBD\xC3\xB7\xCE\xC4\xDA";
const char kServerNamePrey[] = "\xC7\xC1\xB7\xB9\xC0\xCC";
const char kServerNameCasillas[] = "\xC4\xAB\xBD\xC3\xBE\xDF\xBD\xBA";
const char kServerNameHilder[] = "\xC8\xFA\xB4\xF5";
const char kServerNameLuke[] = "\xB7\xE7\xC5\xA9";
const char kServerNameSeria[] = "\xBC\xBC\xB8\xAE\xBE\xC6";
const char kServerNameAnton[] = "\xBE\xC8\xC5\xE6";
const char kServerNameRoxy[] = "\xB7\xCF\xBD\xC3";
const char kServerNameBakal[] = "\xB9\xD9\xC4\xAE";
const char kServerNameOzma[] = "\xBF\xC0\xC1\xEE\xB8\xB6";
const char kServerNameKazan[] = "\xC4\xAB\xC0\xDC";
const char kServerNameEvent1[] = "\xC0\xCC\xBA\xA5\xC6\xAE\x31";
const char kServerNameEvent2[] = "\xC0\xCC\xBA\xA5\xC6\xAE\x32";
const char kServerNameEvent3[] = "\xC0\xCC\xBA\xA5\xC6\xAE\x33";
const char kServerNameStart[] = "\xBD\xBA\xC5\xB8\xC6\xAE";
const char kServerNamePuppet1[] = "\xC6\xDB\xBC\xB7\x31";
const char kServerNamePuppet2[] = "\xC6\xDB\xBC\xB7\x32";
const char kErrorServer[] = "ERROR_SERVER";

template <typename RangeVector, typename Value>
bool ValueInRanges(const RangeVector& ranges, Value value, bool inclusive_end) {
    for (typename RangeVector::const_iterator it = ranges.begin();
         it != ranges.end(); ++it) {
        if (it->begin <= value &&
            (inclusive_end ? it->end >= value : it->end > value)) {
            return true;
        }
    }
    return false;
}

template <typename CharVector, typename Value>
bool ValueInChars(const CharVector& chars, Value value) {
    for (typename CharVector::const_iterator it = chars.begin();
         it != chars.end(); ++it) {
        if (*it == value) {
            return true;
        }
    }
    return false;
}

}  // namespace

advancealtar::RewardParameter*
CDataManager::get_advance_altar_reward_parameter() {
    return &advance_altar_reward_parameter_;
}

const advancealtar::RewardParameter*
CDataManager::get_advance_altar_reward_parameter() const {
    return &advance_altar_reward_parameter_;
}

advancealtar::AdvanceAltarShopParameter*
CDataManager::get_advance_altar_shop_parameter() {
    return &advance_altar_shop_parameter_;
}

const advancealtar::AdvanceAltarShopParameter*
CDataManager::get_advance_altar_shop_parameter() const {
    return &advance_altar_shop_parameter_;
}

advancealtar::ConfigParameter*
CDataManager::get_advance_altar_config_parameter() {
    return &advance_altar_config_parameter_;
}

const advancealtar::ConfigParameter*
CDataManager::get_advance_altar_config_parameter() const {
    return &advance_altar_config_parameter_;
}

class CItemShop;
// IDA_EA: 0x8355B30
// IDA_NAME: _ZN12CDataManagerC2Ev
// DEMANGLED: CDataManager::CDataManager(void)
CDataManager::CDataManager() : sp_at_level_up_() {
    disjoint_ = 0;
    character_list_ = 0;
    quest_list_ = 0;
    item_list_ = 0;
    skill_list_ = 0;
    monster_manager_ = 0;
    map_list_ = 0;
    dungeon_index_list_ = 0;
    item_upgrade_ = 0;
    item_upgrade_sep_ = 0;
    chatting_emoticon_list_ = 0;
    item_generator_ = 0;
    slang_filter_ = 0;
    ai_character_list_ = 0;
    auto_market_control_ = 0;
    npc_script_list_ = 0;
    npc_dynamic_info_manager_ = 0;
    item_limit_edition_manager_ = 0;
    experience_rate_ = 1.0f;
    drop_rate_ = 1.0f;
    striker_script_ = 0;
    character_manage_script_ = 0;
    blue_marble_script_ = 0;
}

// IDA_EA: 0x8356B68
// IDA_NAME: _ZN12CDataManagerD2Ev
// DEMANGLED: CDataManager::~CDataManager()
CDataManager::~CDataManager() {
    destroy();
}

// B-owned internal layout boundaries.  They deliberately have no old-server
// asm identity: the old binary accesses these physical members directly.
CCharacList* CDataManager::get_character_list() const {
    return character_list_;
}

const std::vector<std::pair<int, int> >&
CDataManager::get_initial_character_skills(int job) const {
    // IDA 0x8604FE2: vector<pair<int,int>> @
    // *(CDataManager + 0x14) + 0x7dc * job + 0x240.
    // CCharacList::characters_ and CCharacter::first_grow_skills_ encode
    // those two physical offsets with their actual owners and constructors.
    return character_list_->characters_[job].first_grow_skills_[0];
}

stConditionEventInfo* CDataManager::get_condition_event_info() {
    return &condition_event_info_;
}

const stConditionEventInfo* CDataManager::get_condition_event_info() const {
    return &condition_event_info_;
}

ConditionLevelChkDungeon* CDataManager::get_condition_level_chk_dungeon() {
    return &condition_level_dungeon_;
}

const ConditionLevelChkDungeon*
CDataManager::get_condition_level_chk_dungeon() const {
    return &condition_level_dungeon_;
}

bool CDataManager::is_combo_skill_index(short skill_index) const {
    // IDA 0x825F292:
    //   vector<pair<int,int> > @ *(&this->character_list_) + 0x48FC
    // is CCharacList::characters_[9].first_grow_skills_[0] after the
    // CCharacter/CCharacList layouts are expanded.  The original loop reads
    // exactly the first six script records; it intentionally assumes that
    // startup has populated all six before a state-3 packet can arrive.
    const std::vector<std::pair<int, int> >& skill_indexes =
        character_list_->characters_[9].first_grow_skills_[0];
    for (int index = 0; index <= 5; ++index) {
        if (skill_indexes[index].first == skill_index) {
            return true;
        }
    }
    return false;
}

PvPSkillTreeParameterScript*
CDataManager::get_pvp_skill_tree_parameter_script() {
    return &pvp_skill_tree_parameter_script_;
}

const PvPSkillTreeParameterScript*
CDataManager::get_pvp_skill_tree_parameter_script() const {
    return &pvp_skill_tree_parameter_script_;
}

EventCharacterParameterScript*
CDataManager::get_event_character_parameter_script() {
    return &event_character_parameter_script_;
}

const EventCharacterParameterScript*
CDataManager::get_event_character_parameter_script() const {
    return &event_character_parameter_script_;
}

QuestParameterScript* CDataManager::get_quest_parameter_script() {
    return &quest_parameter_script_;
}

const QuestParameterScript*
CDataManager::get_quest_parameter_script() const {
    return &quest_parameter_script_;
}

StrikerScript* CDataManager::get_striker_script() {
    return striker_script_;
}

int CDataManager::get_striker_skill_minimum_level() const {
    return charac_link_system_.striker_skill_minimum_level;
}

CAICharacterList* CDataManager::get_ai_character_list() {
    return ai_character_list_;
}

void CDataManager::set_ai_character_list(CAICharacterList* list) {
    ai_character_list_ = list;
}

IndependentDropParameterScript*
CDataManager::get_independent_drop_parameter_script() {
    return &independent_drop_parameter_script_;
}

const IndependentDropParameterScript*
CDataManager::get_independent_drop_parameter_script() const {
    return &independent_drop_parameter_script_;
}

stMercenarySystemInfo* CDataManager::get_mercenary_system_info() {
    return &mercenary_system_info_;
}

const stMercenarySystemInfo*
CDataManager::get_mercenary_system_info() const {
    return &mercenary_system_info_;
}

const std::map<int, advancealtar::ActionDefine>&
CDataManager::get_advance_altar_action_defines() const {
    return advance_altar_stage_timeline_.action_defines;
}

int CDataManager::GetSpecialLevelExpItemId() const {
    // CUser::increase_status @0x86657FC compares item ids against
    // CDataManager+0x50CC.  C2 @0x8355B30 constructs the enclosing script
    // at +0x50C4 and ImportGrowthCapsuleScript fills capsule_id from
    // Event/GrowthCapsule.evt's [capsule id] tag.
    return growth_capsule_script_.capsule_id;
}

uint16_t CDataManager::GetWeekendBonusFatigue() const {
    // CUser::RecoverFatigue @0x8657ADA consumes CDataManager+0x51C after
    // the weekend event gate.  The embedded ServerParameterScript owns the
    // field and its RDAR importer supplies the actual value.
    return server_parameter_script_.get_weekend_bonus_fatigue();
}

uint8_t CDataManager::GetCeraShopEventMultiplier() const {
    // premium_helper_function::cal_premium_term @0x827CFF0 multiplies the
    // base table term by this exact CDataManager byte when the request's
    // third_flag is set.  It is not a synthetic default: init owns the
    // backing PVF state and a zero value remains the old-server result.
    return booster_gage_data_.reward_multiple;
}

int CDataManager::get_secret_shop_eplp_wait_time() const {
    // See the declaration for the C2/SecretShopScript evidence.  This is a
    // typed member load from the already-constructed embedded script object,
    // not a second CDataManager layout or a packet-facing facade.
    return secret_shop_script_.eplp_wait_time;
}

SecretShopScript* CDataManager::get_secret_shop_script() {
    return &secret_shop_script_;
}

const SecretShopScript* CDataManager::get_secret_shop_script() const {
    return &secret_shop_script_;
}

// IDA_EA: 0x8357A66
// IDA_NAME: _ZN12CDataManager7destroyEv
// DEMANGLED: CDataManager::destroy(void)
CDataManager* CDataManager::destroy() {
    DisJoint* disjoint = disjoint_;
    if (disjoint != 0) {
        disjoint->~DisJoint();
        ::operator delete(disjoint);
    }
    disjoint_ = 0;
    character_list_ = 0;
    quest_list_ = 0;
    item_list_ = 0;
    skill_list_ = 0;
    monster_manager_ = 0;
    map_list_ = 0;
    dungeon_index_list_ = 0;
    if (item_upgrade_ != 0) {
        WongWork::CItemUpgrade* item_upgrade = item_upgrade_;
        item_upgrade->~CItemUpgrade();
        ::operator delete(item_upgrade);
        item_upgrade_ = 0;
    }
    if (item_upgrade_sep_ != 0) {
        WongWork::CItemUpgrade_Separate* item_upgrade_separate =
            item_upgrade_sep_;
        item_upgrade_separate->~CItemUpgrade_Separate();
        ::operator delete(item_upgrade_separate);
        item_upgrade_sep_ = 0;
    }
    chatting_emoticon_list_ = 0;
    item_generator_ = 0;
    slang_filter_ = 0;
    CAICharacterList* ai_character_list = get_ai_character_list();
    if (ai_character_list != 0) {
        ai_character_list->~CAICharacterList();
        ::operator delete(ai_character_list);
    }
    set_ai_character_list(0);
    auto_market_control_ = 0;
    npc_script_list_ = 0;
    npc_dynamic_info_manager_ = 0;
    CItemLimitEditionMgr* item_limit_edition_mgr =
        item_limit_edition_manager_;
    if (item_limit_edition_mgr != 0) {
        item_limit_edition_mgr->~CItemLimitEditionMgr();
        ::operator delete(item_limit_edition_mgr);
    }
    item_limit_edition_manager_ = 0;
    striker_script_ = 0;
    character_manage_script_ = 0;
    blue_marble_script_ = 0;

    return this;
}

// IDA_EA: 0x8365010
// IDA_NAME: _ZN12CDataManager33loadAdvanceAltarTimeLineParameterERN12advancealtar22StageTimeLineParameterE
// DEMANGLED: CDataManager::loadAdvanceAltarTimeLineParameter(advancealtar::StageTimeLineParameter &)
int CDataManager::loadAdvanceAltarTimeLineParameter(
    advancealtar::StageTimeLineParameter& parameter) {
    parameter.reset();
    const int result = parameter.importStageTimeLineParameter(
        "Etc/AdvanceAltar/AdvanceAltarSummonData.etc");
    if (result) {
        printf("importStageTimeLineParameter Fail(%d)\n", result);
        return result;
    }
    parameter.debugLog();
    return 0;
}

// IDA_EA: 0x80EED82
// IDA_NAME: _ZN12CDataManager18set_dimensionInoutEic
// DEMANGLED: CDataManager::set_dimensionInout(int,char)
int CDataManager::set_dimensionInout(int dimension, char value) {
    int result = value;
    if (dimension <= 5 && dimension >= 0) {
        result = dimension;
        dimension_inout_[dimension] = static_cast<unsigned char>(value);
    }
    return result;
}

// IDA_EA: 0x8110B62
// IDA_NAME: _ZN12CDataManager20get_event_script_mngEv
// DEMANGLED: CDataManager::get_event_script_mng(void)
EventClassify::CEventScriptMng* CDataManager::get_event_script_mng() {
    return event_script_manager_;
}

EventAvatarListScript* CDataManager::get_event_avatar_list_script() {
    return &event_avatar_list_script_;
}

// Non-old-symbol B owner boundary.  CDataManager::C2 @0x8355B30,
// D2 @0x8356B68, and init @0x8358332 respectively construct, destroy, and
// import the one embedded eventReward object at +0xB628.
eventReward* CDataManager::get_event_reward() {
    return &event_reward_;
}

// IDA_EA: 0x814A6EA

// IDA_NAME: _ZN12CDataManager16getItemGeneratorEv
// DEMANGLED: CDataManager::getItemGenerator(void)
WongWork::CItemGeneratorMgr* CDataManager::getItemGenerator() {
    return item_generator_;
}

// IDA_EA: 0x814A6F8
// IDA_NAME: _ZNK12CDataManager16GetChannelScriptEv
// DEMANGLED: CDataManager::GetChannelScript(void)const
channel_script_t* CDataManager::GetChannelScript() const {
    return const_cast<channel_script_t*>(&channel_script_);
}

// IDA_EA: 0x8179FA6
// IDA_NAME: _ZN12CDataManager22getItemLimitEditionMgrEv
// DEMANGLED: CDataManager::getItemLimitEditionMgr(void)
CItemLimitEditionMgr* CDataManager::getItemLimitEditionMgr() {
    return item_limit_edition_manager_;
}

// IDA_EA: 0x8357E9E
// IDA_NAME: _ZN12CDataManager18GetPvPChannelGradeEv
// DEMANGLED: CDataManager::GetPvPChannelGrade(void)
int CDataManager::GetPvPChannelGrade() {
    pvp_channel_info_t* active_channel = &pvp_channel_info_;
    // Original GetPvPChannelGrade @ 0x8357E9E reads CEnvironment+888
    // (Server_Envir::server_group) before indexing live_server_info_.
    const int server_group = G_CEnvironment()->get_server_group();

    if (!G_GameWorld()->IsPVPChannel()) {
        return 0;
    }

    live_server_info_t* live_server_info = &live_server_info_;
    live_server_info_t::iterator found = live_server_info->find(server_group);
    if (found != live_server_info->end()) {
        for (std::vector<pvp_channel_info_t>::iterator it =
                 found->second.begin();
             it != found->second.end(); ++it) {
            if (G_CEnvironment()->get_channel_no() == it->channel_no) {
                *active_channel = *it;
                break;
            }
        }
    }

    cMyTrace trace("bool CDataManager::GetPvPChannelGrade()", 6387, 0);
    trace("PVP Channel Information  serverGroup(%d), pvpChannelNum(%d), "
          "minGrade(%d), maxGrade(%d), upgradeRevision(%d), type(%d), "
          "minLevel(%d), maxLevel(%d)",
          server_group,
          0,
          active_channel->min_grade,
          active_channel->max_grade,
          active_channel->upgrade_revision,
          active_channel->channel_type,
          active_channel->min_level,
          active_channel->max_level);
    return 1;
}

// IDA_EA: 0x819EDE0
// IDA_NAME: _ZN12CDataManager21IsAvailablePvPChannelEi
// DEMANGLED: CDataManager::IsAvailablePvPChannel(int)
int CDataManager::IsAvailablePvPChannel(int pvp_grade) {
    const pvp_channel_info_t* pvp_channel = &pvp_channel_info_;
    if (!pvp_channel->min_grade && !pvp_channel->max_grade) {
        return 1;
    }
    return pvp_channel->min_grade <= pvp_grade &&
           pvp_channel->max_grade >= pvp_grade;
}

// IDA_EA: 0x819EE2E
// IDA_NAME: _ZN12CDataManager21GetMinLevelPvPChannelEv
// DEMANGLED: CDataManager::GetMinLevelPvPChannel(void)
int CDataManager::GetMinLevelPvPChannel() {
    return pvp_channel_info_.min_level;
}

// IDA_EA: 0x819EE3C
// IDA_NAME: _ZN12CDataManager21GetMaxLevelPvPChannelEv
// DEMANGLED: CDataManager::GetMaxLevelPvPChannel(void)
int CDataManager::GetMaxLevelPvPChannel() {
    return pvp_channel_info_.max_level;
}

// IDA_EA: 0x822B612
// IDA_NAME: _ZN12CDataManager18get_dimensionInoutEi
// DEMANGLED: CDataManager::get_dimensionInout(int)
int CDataManager::get_dimensionInout(int dimension) {
    if (dimension >= 0 && dimension < 6) {
        return dimension_inout_[dimension];
    }
    return 0;
}

// IDA_EA: 0x822B638
// IDA_NAME: _ZN12CDataManager21get_limit_inout_countE15ENUM_BLOOD_TYPE
// DEMANGLED: CDataManager::get_limit_inout_count(ENUM_BLOOD_TYPE)
int CDataManager::get_limit_inout_count(int blood_type) {
    return limit_inout_count_[blood_type];
}

// IDA_EA: 0x8374C82
// IDA_NAME: _ZN12CDataManager21set_limit_inout_countEc15ENUM_BLOOD_TYPE
// DEMANGLED: CDataManager::set_limit_inout_count(char,ENUM_BLOOD_TYPE)
int CDataManager::set_limit_inout_count(char value, int blood_type) {
    limit_inout_count_[blood_type] = static_cast<unsigned char>(value);
    return blood_type;
}

// B-owned typed boundary. C2 @0x8355B30 constructs the actual embedded
// stBloodDungeon_t at +0x5E30 and D2 @0x8356B68 destroys it; this helper has
// no separate old-server member ABI.
const stBloodDungeon_t& CDataManager::get_blood_dungeon() const {
    return blood_dungeon_;
}

// IDA_EA: 0x822B64C
// IDA_NAME: _ZNK12CDataManager22getVendingMachineTableEv
// DEMANGLED: CDataManager::getVendingMachineTable(void)const
const std::vector<stVendingMachine_t>* CDataManager::getVendingMachineTable() const {
    return &vending_machine_table_;
}

// IDA_EA: 0x822B65A
// IDA_NAME: _ZN12CDataManager21GetMaxGradePvPChannelEv
// DEMANGLED: CDataManager::GetMaxGradePvPChannel(void)
int CDataManager::GetMaxGradePvPChannel() {
    return pvp_channel_info_.max_grade;
}

// IDA_EA: 0x836544A
// IDA_NAME: _ZN12CDataManager18getMatchEventPointEi
// DEMANGLED: CDataManager::getMatchEventPoint(int)
int CDataManager::getMatchEventPoint(int event_id) {
    seriaRoomDecoEventScript* script = &seria_room_deco_event_script_;
    std::map<int, int>::iterator found = script->reward_items.find(event_id);
    if (found != script->reward_items.end()) {
        return found->second;
    }
    return 0;
}

// IDA_EA: 0x83654BA
// IDA_NAME: _ZN12CDataManager17getVaildEventItemEi
// DEMANGLED: CDataManager::getVaildEventItem(int)
int CDataManager::getVaildEventItem(int item_id) {
    (void)item_id;
    seriaRoomDecoEventScript* script = &seria_room_deco_event_script_;
    return script->using_items.begin() != script->using_items.end();

}

// IDA_EA: 0x836551A
// IDA_NAME: _ZN12CDataManager20getMaxEventItemCountEv
// DEMANGLED: CDataManager::getMaxEventItemCount(void)
int CDataManager::getMaxEventItemCount() {
    return seria_room_deco_event_script_.object_max_count;
}

// IDA_EA: 0x8365528
// IDA_NAME: _ZN12CDataManager20getFatigue_contidionEv
// DEMANGLED: CDataManager::getFatigue_contidion(void)
int CDataManager::getFatigue_contidion() {
    return seria_room_deco_event_script_.fatigue;
}

// IDA_EA: 0x8365536
// IDA_NAME: _ZN12CDataManager28getRewardItem_MailExpireDateEv
// DEMANGLED: CDataManager::getRewardItem_MailExpireDate(void)
int CDataManager::getRewardItem_MailExpireDate() {
    return seria_room_deco_event_script_.reward_item_mail_expire;
}

// IDA_EA: 0x8365544
// IDA_NAME: _ZN12CDataManager19getDecoEventItemIdxEv
// DEMANGLED: CDataManager::getDecoEventItemIdx(void)
int CDataManager::getDecoEventItemIdx() {
    seriaRoomDecoEventScript* script = &seria_room_deco_event_script_;
    if (script->using_items.begin() != script->using_items.end()) {
        return *script->using_items.begin();
    }
    return -1;
}

// IDA_EA: 0x822B5F2
// IDA_NAME: _ZN12CDataManager18GetExpertJobScriptEi
// DEMANGLED: CDataManager::GetExpertJobScript(int)
STExpertJobScript* CDataManager::GetExpertJobScript(int expert_job_index) {
    return expert_job_list_->get(expert_job_index);
}

// IDA_EA: 0x8363716
// IDA_NAME: _ZN12CDataManager20find_hellparty_groupEi
// DEMANGLED: CDataManager::find_hellparty_group(int)
stHellPartyMonsterGroupList* CDataManager::find_hellparty_group(
    int group_index) {
    stHellParty_t* script = Get_instance_stHellParty_t();
    std::map<uint16_t, stHellPartyMonsterGroupList>::iterator it =
        script->monster_groups.find(static_cast<uint16_t>(group_index));
    if (it != script->monster_groups.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x836378C
// IDA_NAME: _ZN12CDataManager27get_hellparty_script_valuesEv

// DEMANGLED: CDataManager::get_hellparty_script_values(void)
stHellParty_t* CDataManager::get_hellparty_script_values() {
    return Get_instance_stHellParty_t();
}

// IDA_EA: 0x8363838
// IDA_NAME: _ZNK12CDataManager17getRankBonusIndexEi
// DEMANGLED: CDataManager::getRankBonusIndex(int)const
int CDataManager::getRankBonusIndex(int point) const {
    const uint16_t* rank_grade =
        rank_system_info_.dungeon_rank_system.rank_grade;
    if (rank_grade[0] <= point) {
        return 4;
    }
    if (rank_grade[1] <= point) {
        return 3;
    }
    if (rank_grade[2] <= point) {
        return 2;
    }
    if (rank_grade[3] <= point) {
        return 1;
    }
    if (rank_grade[4] > point) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x83638C0
// IDA_NAME: _ZN12CDataManager12getRankIndexEi
// DEMANGLED: CDataManager::getRankIndex(int)
int CDataManager::getRankIndex(int point) {
    const uint16_t* rank_grade =
        rank_system_info_.dungeon_rank_system.rank_grade;
    if (rank_grade[0] <= point) {
        return rank_grade[0];
    }
    if (rank_grade[1] <= point) {
        return rank_grade[1];
    }
    if (rank_grade[2] <= point) {
        return rank_grade[2];
    }
    if (rank_grade[3] <= point) {
        return rank_grade[3];
    }
    if (rank_grade[4] <= point) {
        return rank_grade[4];
    }
    if (rank_grade[5] <= point) {
        return rank_grade[5];
    }
    if (rank_grade[6] <= point) {
        return rank_grade[6];
    }
    if (rank_grade[7] > point) {
        return 0;
    }
    return rank_grade[7];
}

// IDA_EA: 0x83639C4
// IDA_NAME: _ZN12CDataManager12calRankPointEii
// DEMANGLED: CDataManager::calRankPoint(int,int)
int CDataManager::calRankPoint(int clear_time_bonus, int rank_point) {
    return clear_time_bonus + rank_point;
}

// IDA_EA: 0x83639D2
// IDA_NAME: _ZN12CDataManager26getDungeonClearAverageTimeEii23ENUM_DUNGEON_DIFFICULTY
// DEMANGLED: CDataManager::getDungeonClearAverageTime(int,int,ENUM_DUNGEON_DIFFICULTY)
int CDataManager::getDungeonClearAverageTime(int dungeon_index,
                                             int party_count,
                                             int dungeon_difficulty) {
    stClearTimeAverageMapKey key;
    key.dungeon_index = static_cast<uint32_t>(dungeon_index);
    key.difficulty = static_cast<uint32_t>(party_count);

    stRankSystemInfo* info = &rank_system_info_;
    std::map<stClearTimeAverageMapKey, stClearTimeAverage>::iterator it =
        info->dungeon_rank_system.clear_time_averages.find(key);
    if (it == info->dungeon_rank_system.clear_time_averages.end()) {
        return 0;
    }

    if (dungeon_difficulty == 1) {
        return it->second.values[1];
    }
    if (dungeon_difficulty > 1) {
        if (dungeon_difficulty == 2) {
            return it->second.values[2];
        }
        if (dungeon_difficulty != 3) {
            return 0;
        }
        return it->second.values[3];
    }
    if (dungeon_difficulty != 0) {
        return 0;
    }
    return it->second.values[0];
}

// IDA_EA: 0x8363AC8
// IDA_NAME: _ZN12CDataManager22getClearTimeBonusPointEii
// DEMANGLED: CDataManager::getClearTimeBonusPoint(int,int)
int CDataManager::getClearTimeBonusPoint(int average_time, int clear_time) {
    int result = 0;
    int previous_bonus = 0;
    if (average_time == 0 || clear_time == 0) {
        return result;
    }

    const int delta = average_time - clear_time;
    stRankSystemInfo* info = &rank_system_info_;
    std::vector<stBonusPointValues>* values =
        &info->dungeon_rank_system.bonus_point_values;
    for (std::vector<stBonusPointValues>::iterator it = values->begin();
         it != values->end(); ++it) {
        if (it->threshold > delta) {
            if (it == values->begin()) {
                result = 0;
                return 0;
            }
            result = previous_bonus;
            return previous_bonus;
        }
        previous_bonus = it->bonus;
    }
    return previous_bonus;
}

// IDA_EA: 0x8363C04
// IDA_NAME: _ZNK12CDataManager15getNewRankIndexEi
// DEMANGLED: CDataManager::getNewRankIndex(int)const
int CDataManager::getNewRankIndex(int point) const {
    const uint16_t* rank_grade =
        rank_system_info_.dungeon_rank_system.rank_grade;
    if (rank_grade[0] <= point) {
        return 0;
    }
    if (rank_grade[1] <= point) {
        return 1;
    }
    if (rank_grade[2] <= point) {
        return 2;
    }
    if (rank_grade[3] <= point) {
        return 3;
    }
    if (rank_grade[4] <= point) {
        return 4;
    }
    if (rank_grade[5] <= point) {

        return 5;
    }
    if (rank_grade[6] <= point) {
        return 6;
    }
    if (rank_grade[7] > point) {
        return 8;
    }
    return 7;
}

// IDA_EA: 0x830E6D2
// IDA_NAME: _ZNK12CDataManager11getDropRateEv
// DEMANGLED: CDataManager::getDropRate(void)const
long double CDataManager::getDropRate() const {
    return drop_rate_;
}

// IDA_EA: 0x83607D8
// IDA_NAME: _ZN12CDataManager15SetItemDropRateEf
// DEMANGLED: CDataManager::SetItemDropRate(float)
void CDataManager::SetItemDropRate(float rate) {
    if (drop_rate_ != rate) {
        const float ratio = rate / drop_rate_;
        drop_rate_ = rate;
        WongWork::CItemGeneratorMgr* item_generator =
            item_generator_;
        item_generator->setDropRatio(rate);

        ItemGenRefVector* refs1 =
            server_parameter_script_.get_item_gen_ref_vector_1();
        for (ItemGenRefVector::iterator it = refs1->begin();
             it != refs1->end(); ++it) {
            it->rate = static_cast<int>(static_cast<long double>(it->rate) *
                                        ratio);
        }

        ItemGenRefVector* refs0 =
            server_parameter_script_.get_item_gen_ref_vector_0();
        for (ItemGenRefVector::iterator it = refs0->begin();
             it != refs0->end(); ++it) {
            it->rate = static_cast<int>(static_cast<long double>(it->rate) *
                                        ratio);
        }
    }
}

// IDA_EA: 0x8365E78
// IDA_NAME: _ZN12CDataManager27set_original_dimensionInoutEic
// DEMANGLED: CDataManager::set_original_dimensionInout(int,char)
int CDataManager::set_original_dimensionInout(int dimension, char value) {
    int result = value;
    if (dimension <= 5) {
        result = dimension;
        original_dimension_inout_[dimension] =
            static_cast<unsigned char>(value);
    }
    return result;
}

// IDA_EA: 0x8365EA0
// IDA_NAME: _ZN12CDataManager20reset_dimensionInoutEv
// DEMANGLED: CDataManager::reset_dimensionInout(void)
void* CDataManager::reset_dimensionInout() {
    return memcpy(dimension_inout_, original_dimension_inout_, 6u);
}

// IDA_EA: 0x8365EDC
// IDA_NAME: _ZNK12CDataManager22getUltimateDungeonListEv
// DEMANGLED: CDataManager::getUltimateDungeonList(void)const
void* CDataManager::getUltimateDungeonList() const {
    return const_cast<std::set<int>*>(&ultimate_dungeon_list_);
}

// IDA_EA: 0x8365EEA
// IDA_NAME: _ZN12CDataManager19getBlueMarbleScriptEv
// DEMANGLED: CDataManager::getBlueMarbleScript(void)
BlueMarbleScriptManager* CDataManager::getBlueMarbleScript() {
    return blue_marble_script_;
}

// IDA_EA: 0x8365EF8
// IDA_NAME: _ZN12CDataManager9testBingoEv
// DEMANGLED: CDataManager::testBingo(void)
void CDataManager::testBingo() {}

// IDA_EA: 0x81455D8
// IDA_NAME: _ZN12CDataManager14GetTowerScriptEj
// DEMANGLED: CDataManager::GetTowerScript(unsigned int)
stDeathTower_t* CDataManager::GetTowerScript(unsigned int tower_index) {
    std::map<unsigned int, stDeathTower_t>* tower_info = &tower_scripts_;
    std::map<unsigned int, stDeathTower_t>::iterator it =
        tower_info->find(tower_index);
    if (it == tower_info->end()) {
        cMyTrace trace("GetTowerScript", 3396, 5);
        trace("not finnd tower : %u", tower_index);
    }
    return &it->second;
}

// IDA_EA: 0x83640FE
// IDA_NAME: _ZN12CDataManager21getDailyTrainingQuestEi
// DEMANGLED: CDataManager::getDailyTrainingQuest(int)
std::vector<int>* CDataManager::getDailyTrainingQuest(int n70) {
    return &daily_training_quests_[n70 - 1];
}

// IDA_EA: 0x836411E
// IDA_NAME: _ZN12CDataManager29isThereDailyTrainingQuestListEii
// DEMANGLED: CDataManager::isThereDailyTrainingQuestList(int,int)
int CDataManager::isThereDailyTrainingQuestList(int n70, int quest_index) {
    if (n70 < oldsrv::config::kCharacterLevelMinimum ||
        n70 > oldsrv::config::kTrainingQuestLevelMaximum) {
        return 0;
    }

    std::vector<int>* quests = getDailyTrainingQuest(n70);
    for (std::vector<int>::iterator it = quests->begin();
         it != quests->end(); ++it) {
        if (*it == quest_index) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8360442
// IDA_NAME: _ZNK12CDataManager13get_level_expEi
// DEMANGLED: CDataManager::get_level_exp(int)const
int CDataManager::get_level_exp(int level) const {
    if (level > 1 && level <= 200) {
        return level_exp_[level - 2];
    }
    return 0;
}

// IDA_EA: 0x8360470
// IDA_NAME: _ZNK12CDataManager13get_exp_levelEi
// DEMANGLED: CDataManager::get_exp_level(int)const
int CDataManager::get_exp_level(int exp) const {
    int level = 1;
    while (level <= 99 && get_level_exp(level) <= exp) {
        ++level;
    }
    return level - 1;

}

// IDA_EA: 0x83604B6
// IDA_NAME: _ZNK12CDataManager21get_level_section_expEi
// DEMANGLED: CDataManager::get_level_section_exp(int)const
int CDataManager::get_level_section_exp(int level) const {
    if (level > 1 && level <= 200) {
        return level_exp_[level - 1] - level_exp_[level - 2];
    }
    return 0;
}

// IDA_EA: 0x83604FE
// IDA_NAME: _ZNK12CDataManager21get_level_exp_percentEii
// DEMANGLED: CDataManager::get_level_exp_percent(int,int)const
int CDataManager::get_level_exp_percent(int level, int percent) const {
    return percent * get_level_section_exp(level) / 100;
}

// IDA_EA: 0x836053C
// IDA_NAME: _ZNK12CDataManager22get_creature_level_expEi
// DEMANGLED: CDataManager::get_creature_level_exp(int)const
int CDataManager::get_creature_level_exp(int level) const {
    if (level > 1 && level <= 99) {
        return CreatureScript::ExpTable_[level - 2];
    }
    return 0;
}

// IDA_EA: 0x8360562
// IDA_NAME: _ZNK12CDataManager22get_creature_exp_levelEi
// DEMANGLED: CDataManager::get_creature_exp_level(int)const
int CDataManager::get_creature_exp_level(int exp) const {
    int level = 1;
    while (level <= 98 && get_creature_level_exp(level) <= exp) {
        ++level;
    }
    return level - 1;
}

// IDA_EA: 0x8360716
// IDA_NAME: _ZN12CDataManager10SetExpRateEf
// DEMANGLED: CDataManager::SetExpRate(float)
void CDataManager::SetExpRate(float rate) {
    if (experience_rate_ != rate) {
        ::memcpy(mob_rewards_,
                 original_mob_rewards_,
                 kMobRewardCopySize);
        experience_rate_ = rate;
        unsigned int* rewards = mob_rewards_;
        for (unsigned int index = 0; index < kMobRewardCount; ++index) {
            rewards[index] = static_cast<unsigned int>(
                static_cast<long double>(rewards[index]) * rate);
        }
    }
}

// IDA_EA: 0x8360914
// IDA_NAME: _ZN12CDataManager14BaseExpPenaltyEii
// DEMANGLED: CDataManager::BaseExpPenalty(int,int)
long double CDataManager::BaseExpPenalty(int charac_level,
                                         int standard_level) {
    const int delta = standard_level - charac_level;
    if (delta < -6) {
        return 0.050000001f;
    }
    if (delta < -5) {
        return 0.2f;
    }
    switch (delta) {
    case -5:
        return 0.5f;
    case -4:
        return 0.75f;
    case -3:
    case -2:
    case -1:
    case 0:
        return 1.0f;
    case 1:
    case 2:
    case 3:
        return 1.12f;
    case 4:
    case 5:
        return 1.0f;
    case 6:
        return 0.75f;
    case 7:
        return 0.69999999f;
    case 8:
        return 0.60000002f;
    case 9:
        return 0.5f;
    default:
        return 0.050000001f;
    }
}

// IDA_EA: 0x8360A6E
// IDA_NAME: _ZNK12CDataManager14get_mob_rewardEiRj
// DEMANGLED: CDataManager::get_mob_reward(int,unsigned int &)const
unsigned int* CDataManager::get_mob_reward(int level,
                                           unsigned int& reward) const {
    if (level > 0 && level <= 199) {
        reward = mob_rewards_[level];
    } else {
        reward = 0;
    }
    return &reward;
}

// IDA_EA: 0x8360CB8
// IDA_NAME: _ZNK12CDataManager14GetSpAtLevelUpEi
// DEMANGLED: CDataManager::GetSpAtLevelUp(int)const
int CDataManager::GetSpAtLevelUp(int level) const {
    if (level >= oldsrv::config::kCharacterLevelMinimum &&
        level <= oldsrv::config::kCharacterLevelMaximum) {
        return sp_at_level_up_.values_[level];
    }
    return 0;
}

// IDA_EA: 0x8360CE0
// IDA_NAME: _ZNK12CDataManager21GetMoneyLimitPerLevelEiPKc
// DEMANGLED: CDataManager::GetMoneyLimitPerLevel(int,char const*)const
int CDataManager::GetMoneyLimitPerLevel(int level, const char* token) const {
    if (level <= 0 || level > 200) {
        LogManager::logFormat(
            1,
            "data_manager.cpp",
            "dword CDataManager::GetMoneyLimitPerLevel(int, const char*) "
            "const",
            0x2705,
            "Invalid Character Level %d",
            level);
        if (level > 200) {
            level = 200;
        }
        if (level < 1) {
            level = 1;
        }
    }
    if (token && !::strcmp(token, "rksekrlsek")) {
        return 0x7FFFFFFF;
    }
    return quest_shop_script_.money_limit_per_level[level];
}

// IDA_EA: 0x83620EE
// IDA_NAME: _ZNK12CDataManager9VerifyMapERK4CMapi
// DEMANGLED: CDataManager::VerifyMap(CMap const&,int) const
int CDataManager::VerifyMap(const CMap& map, int dungeon_index) const {
    // The original first calls get_map_type(), walks the copied monster/item
    // lists and emits diagnostics for unknown APC/item references, but none
    // of those paths change its return value.  Its only failure gate is a
    // missing dungeon in the selected map association: all map dungeons for
    // -1, otherwise exactly the requested dungeon.
    std::list<int> dungeon_indices;
    if (dungeon_index == -1) {
        dungeon_indices = map.getDungeonLists();
    } else {
        dungeon_indices.push_back(dungeon_index);
    }

    for (std::list<int>::const_iterator it = dungeon_indices.begin();
         it != dungeon_indices.end(); ++it) {
        if (!find_dungeon(*it)) {
            return 0;
        }
    }
    return 1;
}


// IDA_EA: 0x835FA68
// IDA_NAME: _ZNK12CDataManager9find_itemEPKc
// DEMANGLED: CDataManager::find_item(char const*)const
CItem* CDataManager::find_item(const char* item_name) const {
    return item_list_->find_item(item_name);
}

// IDA_EA: 0x835FDE4
// IDA_NAME: _ZNK12CDataManager9FindGoodsEi
// DEMANGLED: CDataManager::FindGoods(int)const
CCeraShopGoods* CDataManager::FindGoods(int goods_no) const {
    return item_list_->FindGoods(goods_no);
}

// IDA_EA: 0x835FA86
// IDA_NAME: _ZNK12CDataManager12find_missionEi
// DEMANGLED: CDataManager::find_mission(int)const
CMission* CDataManager::find_mission(int mission_index) const {
    return pvp_mission_system_->find_mission(
        mission_index);
}

// IDA_EA: 0x835FAA6
// IDA_NAME: _ZNK12CDataManager21get_kind_mission_listEi
// DEMANGLED: CDataManager::get_kind_mission_list(int)const
std::list<CMission*>& CDataManager::get_kind_mission_list(
    int mission_kind) const {
    return pvp_mission_system_->get_kind_mission_list(
        static_cast<unsigned int>(mission_kind));
}

// IDA_EA: 0x835FAC6
// IDA_NAME: _ZN12CDataManager20get_New_DailyMissionERSt6vectorISt4pairIjjESaIS2_EE
// DEMANGLED: CDataManager::get_New_DailyMission(std::vector<std::pair<unsigned int,unsigned int> >&)
void CDataManager::get_New_DailyMission(
    std::vector<std::pair<unsigned int, unsigned int> >& missions) {
    pvp_mission_system_->get_New_DailyMission(
        missions);
}

// IDA_EA: 0x835FAE6
// IDA_NAME: _ZNK12CDataManager22get_daily_mission_kindEv
// DEMANGLED: CDataManager::get_daily_mission_kind(void)const
std::bitset<32> CDataManager::get_daily_mission_kind() const {
    return pvp_mission_system_->get_daily_mission_kind();
}

// IDA_EA: 0x835FB12
// IDA_NAME: _ZNK12CDataManager25get_BaseMissionExp_byRankEt
// DEMANGLED: CDataManager::get_BaseMissionExp_byRank(unsigned short)const
unsigned int CDataManager::get_BaseMissionExp_byRank(unsigned short rank) const {
    return pvp_mission_system_->get_BaseMissionExp_byRank(
        rank);
}

// IDA_EA: 0x835FB3A
// IDA_NAME: _ZNK12CDataManager22get_WithinMissionIndexEi
// DEMANGLED: CDataManager::get_WithinMissionIndex(int)const
int CDataManager::get_WithinMissionIndex(int pvp_grade) const {
    return pvp_mission_system_->get_WithinMissionIndex(
        pvp_grade);
}

// IDA_EA: 0x835FB5A
// IDA_NAME: _ZNK12CDataManager21find_lottery_use_costEj
// DEMANGLED: CDataManager::find_lottery_use_cost(unsigned int)const
int CDataManager::find_lottery_use_cost(unsigned int item_index) const {
    return lottery_need_money_.isNeedMoney(item_index);
}

const LotteryUpgradePolicy& CDataManager::get_lottery_upgrade_policy() const {
    return server_parameter_script_.get_lottery_upgrade_policy();
}

// IDA_EA: 0x835FB7A
// IDA_NAME: _ZN12CDataManager20set_lottery_use_costEjj
// DEMANGLED: CDataManager::set_lottery_use_cost(unsigned int,unsigned int)
void CDataManager::set_lottery_use_cost(unsigned int item_index,
                                        unsigned int need_money) {
    lottery_need_money_.add_lottery_needMoney(item_index, need_money);
}

// IDA_EA: 0x835FC76
// IDA_NAME: _ZN12CDataManager18ReCalcEmoticonConfER23CChattingEmoticonConfigRK5CUser
// DEMANGLED: CDataManager::ReCalcEmoticonConf(CChattingEmoticonConfig &,CUser const&)
int CDataManager::ReCalcEmoticonConf(CChattingEmoticonConfig& config,
                                     const CUser& user) {
    return chatting_emoticon_list_->ReCalc(config, user);
}

// IDA_EA: 0x849F516
// IDA_NAME: _ZN12CDataManager21GetExpertJobEtcScriptEv
// DEMANGLED: CDataManager::GetExpertJobEtcScript(void)
STExpertJobEtcScript* CDataManager::GetExpertJobEtcScript() {
    return expert_job_list_->get_etc();
}

// IDA_EA: 0x84E92FC
// IDA_NAME: _ZN12CDataManager30GetAutoMarketContitionsControlEv
// DEMANGLED: CDataManager::GetAutoMarketContitionsControl(void)
CAutoMarketConditionsControl*
CDataManager::GetAutoMarketContitionsControl() {
    return auto_market_control_;
}

// IDA_EA: 0x844D4AE
// IDA_NAME: _ZN12CDataManager10GetPvPModeEv
// DEMANGLED: CDataManager::GetPvPMode(void)
int CDataManager::GetPvPMode() {
    return pvp_channel_info_.pvp_mode;
}

// IDA_EA: 0x84E930A
// IDA_NAME: _ZN12CDataManager28GetUpgradeRevisionPvPChannelEv
// DEMANGLED: CDataManager::GetUpgradeRevisionPvPChannel(void)
int CDataManager::GetUpgradeRevisionPvPChannel() {
    return pvp_channel_info_.upgrade_revision;
}

// IDA_EA: 0x8568CE0
// IDA_NAME: _ZN12CDataManager15GetPvPTeamCountEv
// DEMANGLED: CDataManager::GetPvPTeamCount(void)
int CDataManager::GetPvPTeamCount() {
    return pvp_channel_info_.team_count;
}

// IDA_EA: 0x85E68A8
// IDA_NAME: _ZN12CDataManager17GetPvPChannelTypeEv
// DEMANGLED: CDataManager::GetPvPChannelType(void)
int CDataManager::GetPvPChannelType() {
    return pvp_channel_info_.channel_type;
}

// IDA_EA: 0x83641C2
// IDA_NAME: _ZN12CDataManager14SetScriptPacksEPKc
// DEMANGLED: CDataManager::SetScriptPacks(char const*)
int CDataManager::SetScriptPacks(const char* script_packs) {
    std::string* script_packs_slot = &script_packs_;
    if (script_packs != 0) {
        *script_packs_slot = script_packs;
    } else {
        *script_packs_slot = "Script.pvf";
    }


    return static_cast<int>(reinterpret_cast<intptr_t>(script_packs_slot));
}

// IDA_EA: 0x8364202
// IDA_NAME: _ZNK12CDataManager14GetScriptPacksEv
// DEMANGLED: CDataManager::GetScriptPacks(void)const
const char* CDataManager::GetScriptPacks() const {
    return script_packs_.c_str();
}

// IDA_EA: 0x83653B0
// IDA_NAME: _ZN12CDataManager13setChoiceItemEbj
// DEMANGLED: CDataManager::setChoiceItem(bool,unsigned int)
CDataManager* CDataManager::setChoiceItem(bool enabled,
                                          unsigned int item_id) {
    choice_item_info_.enabled = static_cast<unsigned char>(enabled);
    choice_item_info_.item_id = static_cast<int32_t>(item_id);
    return this;
}

// IDA_EA: 0x83653D8
// IDA_NAME: _ZN12CDataManager15checkChoiceItemEv
// DEMANGLED: CDataManager::checkChoiceItem(void)
int CDataManager::checkChoiceItem() {
    return choice_item_info_.enabled;
}

// IDA_EA: 0x83653E8
// IDA_NAME: _ZN12CDataManager13getChoiceItemEv
// DEMANGLED: CDataManager::getChoiceItem(void)
unsigned int CDataManager::getChoiceItem() {
    return static_cast<unsigned int>(choice_item_info_.item_id);
}

// IDA_EA: 0x8362BE6
// IDA_NAME: _ZN12CDataManager18checkValidShopItemEjm
// DEMANGLED: CDataManager::checkValidShopItem(unsigned int,unsigned long)
int CDataManager::checkValidShopItem(unsigned int npc_id,
                                     unsigned long item_id) {
    return item_shop_->checkValidShopItem(
        npc_id,
        item_id);
}

// IDA_EA: 0x83653F6
// IDA_NAME: _ZN12CDataManager24checkLimitNpcBuyItemListEjm
// DEMANGLED: CDataManager::checkLimitNpcBuyItemList(unsigned int,unsigned long)
int CDataManager::checkLimitNpcBuyItemList(unsigned int npc_id,
                                           unsigned long item_id) {
    return item_shop_->checkLimitNpcBuyItemList(
        npc_id,
        item_id);
}

int CDataManager::is_one_a_day_shop(int npc_id) const {
    return item_shop_->IsOneADayShop(
        npc_id);
}

// IDA_EA: 0x83655AC
// IDA_NAME: _ZNK12CDataManager27get_GrowthWeaponEventRewardEi
// DEMANGLED: CDataManager::get_GrowthWeaponEventReward(int)const
int CDataManager::get_GrowthWeaponEventReward(int event_id) const {
    const growthEquipmentScript* script = &growth_equipment_script_;
    std::map<int, int>::const_iterator found =
        script->reward_items.find(event_id);
    if (found != script->reward_items.end()) {
        return found->second;
    }
    return 0;
}

// IDA_EA: 0x836561C
// IDA_NAME: _ZN12CDataManager23get_GiftGrowthWeaponBoxEv
// DEMANGLED: CDataManager::get_GiftGrowthWeaponBox(void)
int CDataManager::get_GiftGrowthWeaponBox() {
    return growth_equipment_script_.gift_growth_weapon_box;
}

// IDA_EA: 0x836562A
// IDA_NAME: _ZN12CDataManager28get_GrowthWeaponMaterialItemEv
// DEMANGLED: CDataManager::get_GrowthWeaponMaterialItem(void)
int CDataManager::get_GrowthWeaponMaterialItem() {
    return growth_equipment_script_.growth_material_item;
}

// IDA_EA: 0x8365638
// IDA_NAME: _ZN12CDataManager27get_GrowthWeaponMaterialMaxEv
// DEMANGLED: CDataManager::get_GrowthWeaponMaterialMax(void)
int CDataManager::get_GrowthWeaponMaterialMax() {
    return growth_equipment_script_.growth_material_item_max;
}

// IDA_EA: 0x8365648
// IDA_NAME: _ZN12CDataManager30get_GrowthWeaponEventBubbleMaxEv
// DEMANGLED: CDataManager::get_GrowthWeaponEventBubbleMax(void)
int CDataManager::get_GrowthWeaponEventBubbleMax() {
    return growth_equipment_script_.event_bubble_max;
}

// IDA_EA: 0x8365658
// IDA_NAME: _ZN12CDataManager28get_GrowthWeaponEventSandMaxEv
// DEMANGLED: CDataManager::get_GrowthWeaponEventSandMax(void)
int CDataManager::get_GrowthWeaponEventSandMax() {
    return growth_equipment_script_.event_sand_max;
}

// IDA_EA: 0x8365668
// IDA_NAME: _ZNK12CDataManager23get_createCharacterGiftEiRN17createChracScript14rewardInfoListE
// DEMANGLED: CDataManager::get_createCharacterGift(int,createChracScript::rewardInfoList &)const
void CDataManager::get_createCharacterGift(
    int job,
    createChracScript::rewardInfoList& rewards) const {
    const createChracScript* script = &create_character_script_;
    createChracScript::rewardMap::const_iterator found =
        script->rewards_by_job.find(job);
    if (found != script->rewards_by_job.end()) {
        rewards.rewards.insert(rewards.rewards.end(),
                               found->second.rewards.begin(),
                               found->second.rewards.end());
    }

    found = script->rewards_by_job.find(
        oldsrv::config::kCharacterJobCommon);
    if (found != script->rewards_by_job.end()) {
        rewards.rewards.insert(rewards.rewards.end(),
                               found->second.rewards.begin(),
                               found->second.rewards.end());
    }
}

// IDA_EA: 0x836580E
// IDA_NAME: _ZN12CDataManager26get_useJobFatigueMatchGiftEcii
// DEMANGLED: CDataManager::get_useJobFatigueMatchGift(char,int,int)
int CDataManager::get_useJobFatigueMatchGift(char job,
                                             int used_fatigue,
                                             int use_fatigue) {
    const int before_use_fatigue = used_fatigue - use_fatigue;
    useCharacterJobFatigue* script = &use_character_job_fatigue_;
    std::map<int, useCharacterJobFatigue::fatigueMatchGift>::iterator found =
        script->gifts_by_job.find(static_cast<int>(job));
    if (found == script->gifts_by_job.end()) {
        return 0;
    }

    std::map<int, int>& reward_items = found->second.reward_items;
    std::map<int, int>::iterator before =
        reward_items.upper_bound(before_use_fatigue);
    std::map<int, int>::iterator after =
        reward_items.upper_bound(before_use_fatigue + use_fatigue);
    if (before == reward_items.end() && after == reward_items.end()) {
        return 0;
    }

    if (before != after) {
        return before->second;
    }
    return 0;
}

// IDA_EA: 0x836598E
// IDA_NAME: _ZN12CDataManager29get_useJobFatigueMatchGiftMsgEiRSsS0_
// DEMANGLED: CDataManager::get_useJobFatigueMatchGiftMsg(int,std::string &,std::string &)
void CDataManager::get_useJobFatigueMatchGiftMsg(int job,
                                                 std::string& title,
                                                 std::string& text) {
    useCharacterJobFatigue* script = &use_character_job_fatigue_;
    std::map<int, useCharacterJobFatigue::fatigueMatchGift>::iterator found =
        script->gifts_by_job.find(job);
    if (found != script->gifts_by_job.end()) {
        title = found->second.fatigue_title;
        text = found->second.fatigue_text;
    }
}

// IDA_EA: 0x8365A24
// IDA_NAME: _ZN12CDataManager32getGiveGrowCreatureEventJobCountEi
// DEMANGLED: CDataManager::getGiveGrowCreatureEventJobCount(int)
int CDataManager::getGiveGrowCreatureEventJobCount(int cur_sec) {
    CharacterCreateCreatureGiftScript* script =
        &character_create_creature_gift_script_;
    if (script->event_jobs_by_job.empty()) {
        return 0;
    }

    int count = 0;
    for (std::map<int, CharacterCreateCreatureGiftScript::eventJob>::iterator
             it = script->event_jobs_by_job.begin();
         it != script->event_jobs_by_job.end(); ++it) {
        if (it->second.start < cur_sec && it->second.end > cur_sec) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x8365AFA
// IDA_NAME: _ZN12CDataManager35isAvailableGiveGrowCreatureEventJobEii
// DEMANGLED: CDataManager::isAvailableGiveGrowCreatureEventJob(int,int)
int CDataManager::isAvailableGiveGrowCreatureEventJob(int job, int cur_sec) {
    CharacterCreateCreatureGiftScript* script =
        &character_create_creature_gift_script_;
    std::map<int, CharacterCreateCreatureGiftScript::eventJob>::iterator found =
        script->event_jobs_by_job.find(job);
    if (found != script->event_jobs_by_job.end() &&
        found->second.start < cur_sec &&
        found->second.end > cur_sec) {
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8365B98
// IDA_NAME: _ZN12CDataManager24getChangeAbleCreatureLevEv
// DEMANGLED: CDataManager::getChangeAbleCreatureLev(void)
int CDataManager::getChangeAbleCreatureLev() {
    return character_create_creature_gift_script_.change_able_creature_level;
}

// IDA_EA: 0x8365BA6
// IDA_NAME: _ZN12CDataManager30getInfinityChangeCreatureIndexEi
// DEMANGLED: CDataManager::getInfinityChangeCreatureIndex(int)
int CDataManager::getInfinityChangeCreatureIndex(int job) {
    CharacterCreateCreatureGiftScript* script =
        &character_create_creature_gift_script_;
    std::map<int, int>::iterator found =
        script->change_infinity_creatures.find(job);
    if (found != script->change_infinity_creatures.end()) {
        return found->second;
    }
    return -1;
}

// IDA_EA: 0x8365C16
// IDA_NAME: _ZN12CDataManager28getClientChangeCreatureIndexEi
// DEMANGLED: CDataManager::getClientChangeCreatureIndex(int)
int CDataManager::getClientChangeCreatureIndex(int job) {
    CharacterCreateCreatureGiftScript* script =
        &character_create_creature_gift_script_;
    std::map<int, int>::iterator found =
        script->client_infinity_creatures.find(job);
    if (found != script->client_infinity_creatures.end()) {
        return found->second;
    }
    return -1;
}

const CharacterCreateCreatureGiftScript*
CDataManager::get_character_create_creature_gift_script() const {
    return &character_create_creature_gift_script_;
}

// IDA_EA: 0x8365C86
// IDA_NAME: _ZN12CDataManager18invalidAniDecoItemEii
// DEMANGLED: CDataManager::invalidAniDecoItem(int,int)
int CDataManager::invalidAniDecoItem(int item_id, int deco_index) {
    seriaRoomDecoAniEventScript* script =
        &seria_room_deco_animation_event_script_;
    std::map<int, int>::iterator found =
        script->using_items.find(item_id + 1);
    if (found != script->using_items.end() && found->second == deco_index) {
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8365D0E
// IDA_NAME: _ZN12CDataManager14maxAniDecoItemEv
// DEMANGLED: CDataManager::maxAniDecoItem(void)
int CDataManager::maxAniDecoItem() {
    return seria_room_deco_animation_event_script_.using_items.size();
}

// IDA_EA: 0x8365D26
// IDA_NAME: _ZN12CDataManager29getSeriaRoomAniDecoRewardItemEi
// DEMANGLED: CDataManager::getSeriaRoomAniDecoRewardItem(int)
int CDataManager::getSeriaRoomAniDecoRewardItem(int item_id) {
    seriaRoomDecoAniEventScript* script =
        &seria_room_deco_animation_event_script_;
    if (script->reward_items.empty()) {
        return 0;
    }
    for (std::vector<seriaRoomDecoRewardInfo>::iterator it =
             script->reward_items.begin();
         it != script->reward_items.end(); ++it) {
        if (it->item_id == item_id) {
            return it->item_count;
        }
    }
    return 0;
}

// IDA_EA: 0x8365DE4
// IDA_NAME: _ZN12CDataManager23getSeriaRoomAniDecoItemEi
// DEMANGLED: CDataManager::getSeriaRoomAniDecoItem(int)
int CDataManager::getSeriaRoomAniDecoItem(int deco_index) {
    seriaRoomDecoAniEventScript* script =
        &seria_room_deco_animation_event_script_;
    if (script->using_items.empty()) {
        return 0;
    }
    std::map<int, int>::iterator found =
        script->using_items.find(deco_index + 1);
    if (found != script->using_items.end()) {
        return found->second;
    }

    return 0;
}

// IDA_EA: 0x8365F00
// IDA_NAME: _ZNK12CDataManager15get_server_nameEi
// DEMANGLED: CDataManager::get_server_name(int)const
const char* CDataManager::get_server_name(int server_type) const {
    if (server_type < 0 || server_type > 0x12) {
        return kErrorServer;
    }

    switch (server_type) {
    case 1:
        return kServerNameCain;
    case 2:
        return kServerNameDiregie;
    case 3:
        return kServerNameSiroco;
    case 4:
        return kServerNamePrey;
    case 5:
        return kServerNameCasillas;
    case 6:
        return kServerNameHilder;
    case 7:
        return kServerNameLuke;
    case 8:
        return kServerNameSeria;
    case 9:
        return kServerNameAnton;
    case 10:
        return kServerNameRoxy;
    case 11:
        return kServerNameBakal;
    case 12:
        return kServerNameOzma;
    case 13:
        return kServerNameKazan;
    case 14:
        return kServerNameEvent1;
    case 15:
        return kServerNameEvent2;
    case 16:
        return kServerNameEvent3;
    case 17:
        return kServerNameStart;
    case 99:
        return kServerNamePuppet1;
    case 100:
        return kServerNamePuppet2;
    default:
        return kErrorServer;
    }
}

// IDA_EA: 0x8365FC0
// IDA_NAME: _ZN12CDataManager14testActionTypeEv
// DEMANGLED: CDataManager::testActionType(void)
void CDataManager::testActionType() {}

// IDA_EA: 0x836541E
// IDA_NAME: _ZN12CDataManager24GetImageCommnicationDataEv
// DEMANGLED: CDataManager::GetImageCommnicationData(void)
ImageCommunicationData* CDataManager::GetImageCommnicationData() {
    return &image_communication_data_;
}

// IDA_EA: 0x8693F16
// IDA_NAME: _ZN12CDataManager21GetLimitItemUsageInfoEv
// DEMANGLED: CDataManager::GetLimitItemUsageInfo(void)
stLimitItemUsageInfo* CDataManager::GetLimitItemUsageInfo() {
    return &limit_item_usage_info_.limit_item_usage_info;
}

// IDA_EA: 0x8693F24
// IDA_NAME: _ZNK12CDataManager16GetResetItemInfoEv
// DEMANGLED: CDataManager::GetResetItemInfo(void)const
stResetItemInfo* CDataManager::GetResetItemInfo() const {
    return const_cast<stResetItemInfo*>(
        &limit_item_usage_info_.reset_item_info);
}

// IDA_EA: 0x8693F32
// IDA_NAME: _ZNK12CDataManager17GetRefillItemInfoEv
// DEMANGLED: CDataManager::GetRefillItemInfo(void)const
stRefillItemInfo* CDataManager::GetRefillItemInfo() const {
    return const_cast<stRefillItemInfo*>(
        &limit_item_usage_info_.refill_item_info);
}

// IDA_EA: 0x8364F2E
// IDA_NAME: _ZN12CDataManager11IsDBLogItemEj
// DEMANGLED: CDataManager::IsDBLogItem(unsigned int)
int CDataManager::IsDBLogItem(unsigned int item_id) {
    const int cur_sec = GlobalData::system_time()->getCurSec();
    const std::vector<stDBLogItem>* items = &db_log_items_;

    for (std::vector<stDBLogItem>::const_iterator it = items->begin();
         it != items->end(); ++it) {
        if (static_cast<unsigned int>(it->item_id) == item_id &&
            it->expire_time >= cur_sec) {
            return it->log_type;
        }
    }

    return -1;
}

// IDA_EA: 0x84E92A4
// IDA_NAME: _ZNK12CDataManager25is_AuctionRegFreeCeraItemEj
// DEMANGLED: CDataManager::is_AuctionRegFreeCeraItem(unsigned int)const
bool CDataManager::is_AuctionRegFreeCeraItem(unsigned int item_id) const {
    const std::set<unsigned int>* item_set = &auction_reg_free_cera_items_;
    return item_set->find(item_id) != item_set->end();
}

// IDA_EA: 0x8513FBC
// IDA_NAME: _ZN12CDataManager26set_AuctionRegFreeCeraItemEj
// DEMANGLED: CDataManager::set_AuctionRegFreeCeraItem(unsigned int)
int CDataManager::set_AuctionRegFreeCeraItem(unsigned int item_id) {
    std::set<unsigned int>* item_set = &auction_reg_free_cera_items_;
    std::pair<std::set<unsigned int>::iterator, bool> inserted =
        item_set->insert(item_id);
    return inserted.second ? 1 : 0;
}

// IDA_EA: 0x8513FE6
// IDA_NAME: _ZN12CDataManager20GetCeraShopGoodsNameEv
// DEMANGLED: CDataManager::GetCeraShopGoodsName(void)
const char* CDataManager::GetCeraShopGoodsName() {
    return cera_shop_goods_name_.c_str();
}

// IDA_EA: 0x8513FFE
// IDA_NAME: _ZN12CDataManager19SuddenShopFileName1Ev
// DEMANGLED: CDataManager::SuddenShopFileName1(void)
const char* CDataManager::SuddenShopFileName1() {
    return sudden_shop_file_name_1_.c_str();
}

// IDA_EA: 0x8514016
// IDA_NAME: _ZN12CDataManager19SuddenShopFileName2Ev
// DEMANGLED: CDataManager::SuddenShopFileName2(void)
const char* CDataManager::SuddenShopFileName2() {
    return sudden_shop_file_name_2_.c_str();
}

// IDA_EA: 0x851402E
// IDA_NAME: _ZN12CDataManager19SuddenShopFileName3Ev
// DEMANGLED: CDataManager::SuddenShopFileName3(void)

const char* CDataManager::SuddenShopFileName3() {
    return sudden_shop_file_name_3_.c_str();
}

// CParty::clear_reward @ 0x85AE0D4 reads the script-selected replacement
// card directly from the canonical CDataManager instance.
int CDataManager::getEventDungeonClearRewardItemId() const {
    return server_parameter_script_.get_event_dungeon_clear_reward_item_id();
}

// CParty::clear_reward @ 0x85AE0D4, repeat event 65 commission multiplier.
float CDataManager::getPremiumGoldCardCommissionMultiplier() const {
    return server_parameter_script_.get_premium_gold_card_commission_multiplier();
}

// CParty::addDungeonClear @ 0x85BDDFC and CParty::DungeonPermission
// @ 0x85B12F8 read these three adjacent uint16_t policy values.
unsigned short CDataManager::getDungeonPermissionNormalRankRequirement() const {
    return rank_system_info_.dungeon_rank_system.rank_grade[0];
}

unsigned short CDataManager::getDungeonPermissionUltimateRankRequirement() const {
    return rank_system_info_.dungeon_rank_system.rank_grade[1];
}

unsigned short CDataManager::getDungeonPermissionHardRankRequirement() const {
    return rank_system_info_.dungeon_rank_system.rank_grade[2];
}
