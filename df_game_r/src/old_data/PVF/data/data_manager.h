#ifndef OLDSRV_DATAMANAGER_DATA_MANAGER_H_
#define OLDSRV_DATAMANAGER_DATA_MANAGER_H_

// Canonical B-owned CDataManager ABI declarations.

#include <stdint.h>
#include <stddef.h>

#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "old_data/db/core/db_conn_info.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/invalid_char_data.h"
#include "common/manager/live_server_info_t.h"
#include "common/manager/reseal_table.h"
#include "common/manager/slang_name_data.h"
#include "old_data/Login/account_cargo/account_cargo_script.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/channel/pvp_channel_info_t.h"
#include "old_data/PVF/character/datamanager/character_create_creature_gift_script.h"
#include "old_data/PVF/character/datamanager/use_character_job_fatigue_script.h"
#include "old_data/PVF/common/arad_script_data.h"
#include "old_data/PVF/data/datamanager/create_chrac_script.h"
#include "old_data/PVF/data/datamanager/db_log_item.h"
#include "old_data/PVF/data/datamanager/etc_parameter_script.h"
#include "old_data/PVF/data/datamanager/image_communication_info.h"
#include "old_data/PVF/data/datamanager/in_game_advertisement_script.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/equipment/datamanager/broadcast_item_script_data.h"
#include "old_data/PVF/equipment/growth_equipment_script.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/PVF/etc/delete_invalid_item_data.h"
#include "old_data/PVF/etc/datamanager_etc_drivers_0828.h"
#include "old_data/PVF/etc/equipment_grade_sample.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/PVF/etc/item_making_skill_script.h"
#include "old_data/PVF/etc/emotion_type_tags.h"
#include "old_data/PVF/etc/power_parameter_script.h"
#include "old_data/PVF/etc/title_book_info.h"
#include "old_data/PVF/equipment/datamanager/limit_item_usage_info.h"
#include "old_data/PVF/event/bingo_script.h"
#include "old_data/PVF/event/event_avatar_list_script.h"
#include "old_data/PVF/event/event_etc_script.h"
#include "old_data/PVF/event/event_reward_script.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/PVF/event/seria_blessing_script.h"
#include "old_data/PVF/event/seria_room_deco_event_script.h"
#include "old_data/PVF/event/datamanager/seria_room_deco_animation_event_script.h"
#include "old_data/PVF/event/unlimit_challenge_script.h"
#include "old_data/PVF/itemshop/datamanager/quest_shop_script.h"
#include "old_data/PVF/itemshop/secret_shop_script.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_data/PVF/n_quest/training_quest_script.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"
#include "old_game/economy/trade/vending_machine_public_abi.h"
#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_game/activity/schedule/event_character_parameter_script.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/dungeon/instance/condition_level_chk_dungeon.h"
#include "old_game/economy/shop/npc_relation_event_manager.h"
#include "old_game/player/core/character_misc_data.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_game/social/party/hellparty_characlink_abi.h"
#include "old_game/world/region/village_attacked_abi.h"
#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"

struct CItemLimitEditionMgr;
class CItemShop;
class CLevelRewardTableMgr;
class BlueMarbleScriptManager;
class CharacterCreateCreatureGiftScript;


namespace APSystem {
class CActionPointEtcParameter;
}  // namespace APSystem

namespace EventClassify {
class CEventScriptMng;
}  // namespace EventClassify

namespace WongWork {
class CMailBoxHelper;
class CItemGeneratorMgr;
class CItemUpgrade;
class CItemUpgrade_Separate;
}  // namespace WongWork

namespace advancealtar {
class ConfigParameter;
class RewardParameter;
class AdvanceAltarShopParameter;
class StageTimeLineParameter;
struct ActionDefine;
}  // namespace advancealtar

namespace ARAD {
namespace SCRIPT {
class AradJumping_Script;
}  // namespace SCRIPT
}  // namespace ARAD

namespace AvatarVariation {
struct AvatarColorInfo;
}  // namespace AvatarVariation

class CItem;
class CItemList;
struct EventAvatarListScript;
class eventReward;
class CCharacList;
class CAICharacterList;
class IndependentDropParameterScript;
class CCeraShopGoods;
class CChattingEmoticonList;
class CDungeon;
class CDungeonList;
class CMonsterManager;
class CMap;
class CMapList;
class CMission;
class CMonster;
class CNPCScript;
struct StageMapList;
class ServerParameterScript;
class SecretShopScript;
class PowerParameterScript;
class stSeriaBlessingScript;
struct STGrowthPowerData;
struct stVictoryPenalty;
struct stEmotionTypeTagsInfo;
struct stEquipmentGradeSample;
class stBroadCastItemScript;
class PvP_MissionSystem;
class PvPSkillTreeParameterScript;
class EventCharacterParameterScript;
struct QuestParameterScript;
class CAutoMarketConditionsControl;
class CExpertJobList;
class CSkill;
class CSkillList;
class CUser;
class StrikerScript;
class Area;
class Village;
class GameWorld;
class CPowerManager;
class CharacManageScript;
class CAchievement;
class DB_UpdateOneDayLetheSkill;

// IDA_EA: 0x837390A / 0x837391E.
// CDataManager owns this original two-field value at +0xB4A0.  It is a real
// subobject (C2 invokes ChoiceItemInfo::C2), not an anonymous byte/word pair.
struct ChoiceItemInfo {
    ChoiceItemInfo();
    ChoiceItemInfo* clear();

    uint8_t enabled;
    uint8_t alignment_01[3];
    int32_t item_id;
};

static_assert(sizeof(ChoiceItemInfo) == 0x08u,
              "ChoiceItemInfo i386 ABI size must stay 0x08");

struct WarAreaCounter;
class DisJoint;
class Quest;
class QuestList;
struct CChattingEmoticonConfig;
struct ImageCommunicationData;
struct PVPMAP_DATA;
struct STExpertJobEtcScript;
struct STExpertJobScript;
struct channel_script_t;
struct stLimitItemUsageInfo;
struct stRefillItemInfo;
struct stResetItemInfo;
struct stItemMakingSkill;
struct stDeathTower_t;
struct stBloodDungeon_t;
struct stHellPartyMonsterGroupList;
struct STHellPartyGroupOrder;
struct stHellParty_t;
struct stMercenarySystemInfo;
struct stLevelUpRewardItemScript;
struct stReturnUserRewardScript;
struct stConditionEventInfo;
struct ConditionLevelChkDungeon;
class GuildParameterScript;
struct reseal_table_t;

// IDA unit: 12_pvf_datamanager_scripts/CDataManager.
// IDA_EA: ctor @0x8355B30, dtor @0x8356B68, init @0x8358332,
// SetScriptPacks/GetScriptPacks @0x83641C2/0x8364202.
// This public baseline
// exposes the object ABI, script-pack string slot, and narrow offset accessors
// needed by startup and later DataManager/PVF restoration.
class CDataManager {
public:
    CDataManager();
    ~CDataManager();

    // CMD84 consumes the in-place Reseal.etc table created by C2 at +0x2c.
    // Keep that owner layout behind DataManager rather than reopening it in
    // inventory packet handlers.
    reseal_table_t* get_reseal_table();
    const reseal_table_t* get_reseal_table() const;

    CDataManager* destroy();
    int init();
    // 0627 server importers and the 0828 client independently prove these
    // four ETC owners plus the 29-value 0828 damage-formula generation.  They
    // are loaded atomically at DataManager startup;
    // queries below consume the rebuild-owned sidecar without changing the
    // ABI-locked CDataManager object layout.
    int import_datamanager_etc_0828();
    const DamageFormulaConstants0828&
    get_damage_formula_constants_0828() const;
    float get_damage_formula_constant_0828(unsigned int index) const;
    const std::vector<PremiumServiceData>&
    get_premium_service_data_0828() const;
    const PremiumServiceData* find_premium_service_data_0828(int type) const;
    const std::vector<int>&
    get_common_party_matching_dungeons_0828() const;
    bool is_common_party_matching_dungeon_0828(int dungeon_index) const;
    const std::vector<stPartyMatchingInfo>&
    get_party_matching_info_0828() const;
    const stPartyMatchingInfo* find_party_matching_info_0828(
        const std::string& channel_name,
        const std::string& region_name) const;
    const QuestMovieMap& get_quest_movie_list_0828() const;
    const QuestMovieInfo* find_quest_movie_0828(
        int quest_index,
        ENUM_QUEST_MOVIE_EVENT event) const;
    const stBloodWarGrowTable& get_war_room_wp_table_0828() const;
    bool get_war_room_wp_pair_0828(
        unsigned int row_index,
        unsigned int pair_index,
        std::pair<int, int>* output) const;
    // Etc/Labyrinth.etc and Etc/ChaosFPShop.etc are strict startup-loaded
    // typed resources.  These read-only views intentionally do not authorize
    // gameplay mutation or a shop purchase: no exact 0828 server transaction
    // owner has yet been recovered for either resource.
    const LabyrinthConfig0828& get_labyrinth_config_0828() const;
    const LabyrinthReinforceType0828*
    find_labyrinth_reinforce_type_0828(int pattern) const;
    const LabyrinthEquipmentDefinition0828*
    find_labyrinth_equipment_definition_0828(
        const std::string& category) const;
    const ChaosFPShopCatalog0828& get_chaos_fp_shop_catalog_0828() const;
    const std::vector<ChaosFPShopEntry0828>*
    get_chaos_fp_shop_section_0828(ChaosFPShopSection0828 section) const;
    // IDA_EA: 0x83580AA.
    int initRandomOptionItem();
    // IDA_EA: 0x835F1D6.
    int initFairPVPSkillPoint();
    // IDA_EA: 0x835FE02
    int load_map_files(std::vector<PVPMAP_DATA>& pvp_map_data);
    int loadAdvanceAltarTimeLineParameter(
        advancealtar::StageTimeLineParameter& parameter);
    // B-owned internal layout boundary.  CDataManager::C2 @0x8355B30
    // constructs StageTimeLineParameter at +0x62D0, whose exact
    // action_defines map is at +0x18 (+0x62E8 overall); D2 @0x8356B68
    // destroys that same object, and init @0x8358332 loads it through
    // loadAdvanceAltarTimeLineParameter.  This has no old-server ABI symbol.
    const std::map<int, advancealtar::ActionDefine>&
    get_advance_altar_action_defines() const;
    // CDataManager::C2/D2 construct and destroy this exact PVF object at
    // +0x634C.  Advance Altar reward consumers use this named B-owner
    // boundary instead of reproducing the DataManager offset.
    advancealtar::RewardParameter* get_advance_altar_reward_parameter();
    const advancealtar::RewardParameter*
    get_advance_altar_reward_parameter() const;
    advancealtar::AdvanceAltarShopParameter*
    get_advance_altar_shop_parameter();
    const advancealtar::AdvanceAltarShopParameter*
    get_advance_altar_shop_parameter() const;
    // CDataManager::C2/D2 construct and destroy this exact PVF object at
    // +0x6364.  Character-side Advance Altar state uses the named owner
    // boundary instead of deriving the DataManager layout itself.
    advancealtar::ConfigParameter*
    get_advance_altar_config_parameter();
    const advancealtar::ConfigParameter*
    get_advance_altar_config_parameter() const;
    int set_dimensionInout(int dimension, char value);
    EventClassify::CEventScriptMng* get_event_script_mng();
    // CDataManager::C2/D2 own this embedded EventAvatarListScript at
    // +0xB508.  CMD450 uses the accessor rather than reprojecting the
    // DataManager object from event gameplay code.
    EventAvatarListScript* get_event_avatar_list_script();
    // C2/D2/init own the single EventReward PVF table at +0xB628.  This
    // B-owner typed boundary returns that in-place object only: it neither
    // allocates, copies, nor initializes an alternate reward table.
    eventReward* get_event_reward();
    APSystem::CActionPointEtcParameter* GetActionPointEtcParameter(bool is_gm);
    WongWork::CItemGeneratorMgr* getItemGenerator();
    channel_script_t* GetChannelScript() const;
    ::CItemLimitEditionMgr* getItemLimitEditionMgr();
    // CDataManager::C2 owns the CItemShop pointer at +0x8CBC.  The login
    // completion flow consumes the one-a-day list through this typed owner
    // boundary rather than projecting the old CDataManager storage in a
    // dispatcher.
    CItemShop* get_item_shop() { return item_shop_; }
    const CItemShop* get_item_shop() const {
        return item_shop_;
    }
    int GetPvPChannelGrade();
    int IsAvailablePvPChannel(int pvp_grade);
    int GetMinLevelPvPChannel();
    int GetMaxLevelPvPChannel();
    int get_dimensionInout(int dimension);
    // Callers in the recovered tree use integer literals, while the original
    // member ABI carries the distinct ENUM_BLOOD_TYPE spelling.  Keep the
    // source-compatible integer parameter and lock its old i386 symbol name.
    int get_limit_inout_count(int blood_type);
    int set_limit_inout_count(char value, int blood_type);
    // B-owned typed reconstruction boundary, not an old exported member.
    // CDataManager::C2 @0x8355B30 constructs stBloodDungeon_t in place at
    // +0x5E30, and D2 @0x8356B68 destroys that same object.  Blood-dungeon
    // gameplay must consume this PVF-owned instance rather than project the
    // CDataManager layout outside its owner.
    const stBloodDungeon_t& get_blood_dungeon() const;
    // CDataManager::C2 @0x8355CF0 constructs this embedded stDeathTower_t at
    // +0x5198.  The immediately preceding +0x5180 object is a distinct
    // tower-index map, so Death Tower consumers must use this named PVF
    // member rather than GetTowerScript's lookup table.
    const stDeathTower_t& get_death_tower() const;
    // Typed Death-Tower reward-card table boundaries used by the C tower
    // clear state machine.  IDA 0x846791F/0x8467936 indexes the normal and
    // illusion tables directly; preserve its unchecked stage precondition
    // while keeping the CDataManager layout inside this B-owned provider.
    int get_death_tower_reward_card_num(unsigned int stage) const;
    int get_death_tower_illusion_reward_card_num(unsigned int stage) const;
    const std::vector<stVendingMachine_t>* getVendingMachineTable() const;
    int GetMaxGradePvPChannel();
    CDungeonList* getDungeonIndexList();
    int getMatchEventPoint(int event_id);
    int getVaildEventItem(int item_id);
    int getMaxEventItemCount();
    int getFatigue_contidion();
    int getRewardItem_MailExpireDate();
    int getDecoEventItemIdx();
    CDataManager* setCharacNameFilterPass(bool pass);
    int getCharacNameFilterPass() const;
    // CInGameAdvertisementManager::checkCondition @0x80FCD2E reads the
    // embedded script's age_condition_flags table.  Keep that physical
    // ownership in the B provider; gameplay must not project CDataManager's
    // layout just to evaluate an advertisement age gate.
    bool is_in_game_ad_age_allowed(unsigned int age) const;
    // IDA_EA: 0x8362C0E.  The old server returns one for every byte; its
    // unsigned-char parameter is part of the exported i386 ABI.
    int _isOneByteCharacter(unsigned char character);
    // IDA_EA: 0x8362C22
    // IDA_NAME: _ZN12CDataManager19_subPunctuationMarkEPKcPc
    // DEMANGLED: CDataManager::_subPunctuationMark(char const*,char *)
    char* _subPunctuationMark(const char* source, char* destination);
    STExpertJobScript* GetExpertJobScript(int expert_job_index);
    int hasSpecialCharacterExceptSpace(const char* text);
    int hasSpecialCharacter(const char* text);
    int hasPreventString(const char* text);
    int hasPreventStringName(const char* text);
    int hasPreventPrefixCharacName(const char* text, int length = 0);
    stHellPartyMonsterGroupList* find_hellparty_group(int group_index);
    // IDA_EA: 0x836379A.  The returned map is owned by the HellParty PVF
    // singleton and stays valid until that singleton is destroyed.
    std::map<uint16_t, STHellPartyGroupOrder>* get_hellparty_groupOrder_map(
        int group_index);
    stHellParty_t* get_hellparty_script_values();
    int getRankBonusIndex(int point) const;
    int getRankIndex(int point);
    int calRankPoint(int clear_time_bonus, int rank_point);
    int getDungeonClearAverageTime(int dungeon_index,
                                   int party_count,
                                   int dungeon_difficulty);
    int getClearTimeBonusPoint(int average_time, int clear_time);
    int getNewRankIndex(int point) const;
    // CParty::clear_reward reads these two script-owned values directly in
    // the original CMD049 reward tail.  Keep the storage projection in the
    // data owner rather than repeating raw CDataManager offsets in CParty.
    int getEventDungeonClearRewardItemId() const;
    float getPremiumGoldCardCommissionMultiplier() const;
    // CParty::DungeonPermission/addDungeonClear rank gates at +0x512c..30.
    unsigned short getDungeonPermissionNormalRankRequirement() const;
    unsigned short getDungeonPermissionUltimateRankRequirement() const;
    unsigned short getDungeonPermissionHardRankRequirement() const;
    long double getDropRate() const;
    float getUpgradeItemRepairCostRate(int upgrade, bool has_ability);
    // These are B-owned typed projections of the C2-established +0x20/+0x24
    // upgrade-service pair.  They deliberately have no exported old-server
    // ABI: dispatchers must not reproduce CDataManager offsets.
    WongWork::CItemUpgrade* get_item_upgrade();
    WongWork::CItemUpgrade_Separate* get_item_upgrade_separate();
    // C2/D2 own these two embedded script objects at +0x5094 and +0x8710.
    // Keep their physical placement in B so request dispatchers and CUser
    // logic never manufacture a second CDataManager layout boundary.
    stSeriaBlessingScript* get_seria_blessing_script();
    STGrowthPowerData* get_growth_power_data();
    void SetItemDropRate(float rate);
    int set_original_dimensionInout(int dimension, char value);
    void* reset_dimensionInout();
    stItemMakingSkill* GetItemMakingSkill() const;
    void* getUltimateDungeonList() const;
    BlueMarbleScriptManager* getBlueMarbleScript();
    void testBingo();
    // IDA_EA: 0x81455D8.
    stDeathTower_t* GetTowerScript(unsigned int tower_index);
    std::vector<int>* getDailyTrainingQuest(int n70);
    int isThereDailyTrainingQuestList(int n70, int quest_index);
    // IDA_EA: 0x8363CE0
    // IDA_NAME: _ZN12CDataManager26reselectDailyTrainingQuestEv
    // DEMANGLED: CDataManager::reselectDailyTrainingQuest(void)
    bool reselectDailyTrainingQuest();
    // IDA_EA: 0x8360442, 0x8360470, 0x83604B6, 0x83604FE,
    // 0x8360914, 0x8360A6E, 0x8360CB8, 0x8360CE0.
    int get_level_exp(int level) const;
    int get_exp_level(int exp) const;
    int get_level_section_exp(int level) const;
    int get_level_exp_percent(int level, int percent) const;
    int get_creature_level_exp(int level) const;
    int get_creature_exp_level(int exp) const;
    // IDA_EA: 0x8360306.  The old ABI uses server_group as the
    // login_account_N table suffix and gc_no as m_channel_no.  It resets the
    // matching login state according to the server-start mode.
    int InitLoginState(ENUM_SERVER_GROUP login_table_no, int channel_gc_no);
    // IDA_EA: 0x8360400, 0x83605A8, 0x8360AA2, 0x8360BDE.
    int set_level_exp();
    int set_mob_reward();
    int set_pvp_grade();
    int set_reward_sp();
    void SetExpRate(float rate);
    long double BaseExpPenalty(int charac_level, int standard_level);
    unsigned int* get_mob_reward(int level, unsigned int& reward) const;
    int GetSpAtLevelUp(int level) const;
    int GetMoneyLimitPerLevel(int level, const char* token) const;
    CDungeon* find_dungeon(int dungeon_index) const;
    CMap* find_map(int map_index) const;
    // IDA_EA: 0x83620EE.  QuestList::load_list validates an [appear map]
    // dungeon association through this const CDataManager ABI.
    int VerifyMap(const CMap& map, int dungeon_index) const;
    // IDA_EA: 0x8360D94
    // IDA_NAME: _ZNK12CDataManager6VerifyEv
    // DEMANGLED: CDataManager::Verify(void)const
    bool Verify() const;
    // IDA_EA: 0x8362B44
    // IDA_NAME: _ZNK12CDataManager13VerifyDungeonERK8CDungeon
    // DEMANGLED: CDataManager::VerifyDungeon(CDungeon const&)const
    bool VerifyDungeon(const CDungeon& dungeon) const;
    CItem* find_item(int item_id) const;
    CItem* find_item(const char* item_name) const;
    // C2/D2 own this exact B-side script object at +0xB5BC.  Runtime packet
    // handlers access it through its owner instead of reprojecting the large
    // CDataManager ABI in gameplay code.
    const stBroadCastItemScript* get_broadcast_item_script() const;
    // C2/D2 construct/destroy this embedded map holder at the verified
    // +0x8740 ABI slot.  This accessor does not synthesize a second store.
    AvatarVariation::AvatarColorInfo* get_avatar_color_info();
    const AvatarVariation::AvatarColorInfo* get_avatar_color_info() const;
    // IDA_EA: 0x835FBA2
    // IDA_NAME: _ZN12CDataManager21rewriteUsableEmoticonERK5CUserPc
    // DEMANGLED: CDataManager::rewriteUsableEmoticon(CUser const&,char *)
    void rewriteUsableEmoticon(const CUser& user, char* message);
    // IDA_EA: 0x835FC9A
    // IDA_NAME: _ZN12CDataManager17IsUsableEmoticonsERK5CUserRKSt6vectorIiSaIiEE
    // DEMANGLED: CDataManager::IsUsableEmoticons(
    //             CUser const&,std::vector<int> const&)
    std::vector<int> IsUsableEmoticons(
        const CUser& user, const std::vector<int>& emoticons);
    // IDA_EA: 0x835FD84.
    CMonster* find_monster(int monster_index) const;
    // IDA_EA: 0x8363818.
    CNPCScript* find_npc(unsigned int npc_index) const;
    // IDA_EA: 0x835FDA2.
    CSkill* find_skill(int job_type, int skill_index) const;
    // IDA_EA: 0x835FDE4.
    CCeraShopGoods* FindGoods(int goods_no) const;
    // IDA_EA: 0x835FDC6.
    Quest* find_quest(int quest_index) const;
    CMission* find_mission(int mission_index) const;
    std::list<CMission*>& get_kind_mission_list(int mission_kind) const;
    void get_New_DailyMission(
        std::vector<std::pair<unsigned int, unsigned int> >& missions);
    std::bitset<32> get_daily_mission_kind() const;
    unsigned int get_BaseMissionExp_byRank(unsigned short rank) const;
    int get_WithinMissionIndex(int pvp_grade) const;
    int find_lottery_use_cost(unsigned int item_index) const;
    void set_lottery_use_cost(unsigned int item_index, unsigned int need_money);
    // CUser::lottery_item @0x86733A2 is the sole C-side consumer of this
    // PVF policy.  The accessor keeps its physical CDataManager placement
    // in B and carries no legacy exported member ABI.
    const LotteryUpgradePolicy& get_lottery_upgrade_policy() const;
    int ReCalcEmoticonConf(CChattingEmoticonConfig& config,
                           const CUser& user);
    STExpertJobEtcScript* GetExpertJobEtcScript();
    CAutoMarketConditionsControl* GetAutoMarketContitionsControl();
    int GetPvPMode();
    int GetUpgradeRevisionPvPChannel();
    int GetPvPTeamCount();
    int GetPvPChannelType();
    int SetScriptPacks(const char* script_packs);
    const char* GetScriptPacks() const;
    CDataManager* setChoiceItem(bool enabled, unsigned int item_id);
    int checkChoiceItem();
    unsigned int getChoiceItem();
    // IDA_EA: 0x8362BE6, 0x83653F6.
    int checkValidShopItem(unsigned int npc_id, unsigned long item_id);
    int checkLimitNpcBuyItemList(unsigned int npc_id, unsigned long item_id);
    // C-owned boundary for the item-shop one-a-day flag.  NPC dispatchers
    // must not reach through CDataManager's private item-shop pointer.
    int is_one_a_day_shop(int npc_id) const;
    int get_GrowthWeaponEventReward(int event_id) const;
    int get_GiftGrowthWeaponBox();
    int get_GrowthWeaponMaterialItem();
    int get_GrowthWeaponMaterialMax();
    int get_GrowthWeaponEventBubbleMax();
    int get_GrowthWeaponEventSandMax();
    void get_createCharacterGift(
        int job,
        createChracScript::rewardInfoList& rewards) const;
    int get_useJobFatigueMatchGift(char job,
                                   int used_fatigue,
                                   int use_fatigue);
    void get_useJobFatigueMatchGiftMsg(int job,
                                       std::string& title,
                                       std::string& text);
    int getGiveGrowCreatureEventJobCount(int cur_sec);
    int isAvailableGiveGrowCreatureEventJob(int job, int cur_sec);
    int getChangeAbleCreatureLev();
    int getInfinityChangeCreatureIndex(int job);
    int getClientChangeCreatureIndex(int job);
    // Typed owner access for event-103 packet serialization.  The old object
    // is constructed at CDataManager+0x4c04; keeping that offset within B
    // prevents C runtime code from inventing an untyped overlay.
    const CharacterCreateCreatureGiftScript*
    get_character_create_creature_gift_script() const;
    int invalidAniDecoItem(int item_id, int deco_index);
    int maxAniDecoItem();
    int getSeriaRoomAniDecoRewardItem(int item_id);
    int getSeriaRoomAniDecoItem(int deco_index);
    const char* get_server_name(int server_type) const;
    void testActionType();
    ImageCommunicationData* GetImageCommnicationData();
    stLimitItemUsageInfo* GetLimitItemUsageInfo();
    stResetItemInfo* GetResetItemInfo() const;
    stRefillItemInfo* GetRefillItemInfo() const;
    int IsDBLogItem(unsigned int item_id);
    bool is_AuctionRegFreeCeraItem(unsigned int item_id) const;
    int set_AuctionRegFreeCeraItem(unsigned int item_id);
    const char* GetCeraShopGoodsName();
    const char* SuddenShopFileName1();
    const char* SuddenShopFileName2();
    const char* SuddenShopFileName3();
    CDungeon* get_dungeon(int dungeon_index);

    // ABI bridge: CDataManager::C2 @0x8355B30 constructs the exact embedded
    // ServerParameterScript at +0x68.  This provides a typed landing for
    // consumers whose field provenance is direct IDA evidence.
    ServerParameterScript* get_server_parameter_script();
    const ServerParameterScript* get_server_parameter_script() const;

    // CNPCScriptList::initScript @0x858166B registers favorable NPCs in the
    // manager pointer owned at +0xA8D4.  Keep that physical placement inside
    // CDataManager while exposing the already named lifecycle object to its
    // script-list consumer.
    class CNPCDynamicInfoManager* get_npc_dynamic_info_manager() {
        return npc_dynamic_info_manager_;
    }
    const class CNPCDynamicInfoManager* get_npc_dynamic_info_manager() const {
        return npc_dynamic_info_manager_;
    }

    FairPvPStatScript* get_fair_pvp_stat_script();
    const FairPvPStatScript* get_fair_pvp_stat_script() const;
    RandomOptionScript* get_random_option_script();
    const RandomOptionScript* get_random_option_script() const;
    RegenerationROI* get_regeneration_roi();
    const RegenerationROI* get_regeneration_roi() const;
    BoosterGageData* get_booster_gage_data();
    const BoosterGageData* get_booster_gage_data() const;
    PcBangItemRentarData* get_pc_bang_item_rental_data();
    const PcBangItemRentarData* get_pc_bang_item_rental_data() const;
    DimensionActivationData* get_dimension_activation_data();
    const DimensionActivationData* get_dimension_activation_data() const;
    const SlangNameData* get_slang_name_data() const;
    TrainingQuestScript* get_training_quest_script();
    const TrainingQuestScript* get_training_quest_script() const;
    stTitleBookInfo* get_title_book_info();
    const stTitleBookInfo* get_title_book_info() const;
    DeleteInvalidItemScript* get_delete_invalid_item_script();
    const DeleteInvalidItemScript* get_delete_invalid_item_script() const;

    // CDataManager::C2/D2 own the QuestList pointer at +0x18.  ResetDailyQuest
    // and ResetTrainingQuest reach the canonical quest owner through this
    // typed boundary rather than reopening DataManager storage.
    QuestList* get_quest_list();

    // C2/D2 establish the FairPVPEquipment map at +0x86F8 and init() fills
    // it through importFairPVPEquipment("Etc/FairPVPEquipment.etc", ...).
    // This is a B-owned layout boundary, not a new old-server ABI export.
    const FairPvpEquipmentMap& get_fair_pvp_equipment_map() const;

    // CDataManager::C2/D2 own the single 0627 quest-shop script at +0x6398.
    // Its map, initialization cost, appendage lookup and per-level money
    // table are real stQuestShop members; mall code must not reproject them.
    stQuestShop* get_quest_shop_script();
    const stQuestShop* get_quest_shop_script() const;

    // Typed reconstruction bridge, not an old exported CDataManager member.
    // C2/D2/init directly prove the embedded stEmotionTypeTagsInfo at +0x4D44.
    // C consumers must use this B-owned boundary rather than duplicate the
    // layout access.
    const stEmotionTypeTagsInfo* get_emotion_type_tags_info() const;

    // Typed reconstruction bridge for CItemUpgrade::ProcUpgradeChronicle.
    // C2/D2 establish the five-vector EquipmentGradeSample record at +0x4D5C
    // and init() imports Etc/EquipmentGradeSample.etc before any character
    // can execute CMD274.  Keeping this projection in the DataManager owner
    // prevents item gameplay from recreating that private object offset.
    const stEquipmentGradeSample* get_equipment_grade_sample() const;

    // Typed reconstruction bridge, not an old exported CDataManager member.
    // C2 @0x8355B30 constructs ARAD::SCRIPT::AradJumping_Script at +0xB4C8
    // (46280); D2 @0x8356B68 destroys that same embedded object.
    ARAD::SCRIPT::AradJumping_Script* get_arad_jumping_script();

    // Typed reconstruction boundaries for the CMD585 internal code-45
    // consumer; neither is an exported old-server CDataManager identity.
    //
    // `Arad_INTER_kAradJumpingCharacter::clearEpicQuest` @0x81993C4 loads
    // the QuestList pointer from CDataManager+0x18 before calling
    // QuestList::getQuestNPCList(ENUM_QUEST_GRADE(0)).  CDataManager::C2
    // @0x8355B30 zeroes the corresponding i386 storage slot.
    QuestList* get_arad_jumping_quest_list();

    // `...::sendHelperBox` @0x819973E loads the CItemList pointer from
    // CDataManager+0x0C before CItemList::create_item.  CDataManager::C2
    // @0x8355B30 zeroes the corresponding i386 storage slot.
    CItemList* get_arad_jumping_item_list();

    // Typed reconstruction boundary for gameplay consumers which create a
    // normal PVF item.  The CDataManager constructor establishes item_list_
    // at the IDA-proven +0x0C slot; this avoids having the gameplay caller
    // reproject that field as untyped storage.
    CItemList* get_item_list();

    // CDataManager::C2/D2 own the embedded 0627 world-map table at +0x8780.
    // World, party and dungeon code must reach it through the data owner.
    CWorldMapList* get_world_map_list();
    const CWorldMapList* get_world_map_list() const;

    // BingoEvent and CBingo consume the same script imported by init into the
    // C2/D2-owned +0xB568 member.  This is a typed owner edge, not a second
    // event-local layout.
    stBingoScript* get_bingo_script();
    const stBingoScript* get_bingo_script() const;

    // CDataManager::C2 @0x8356059/0x8356061 constructs the first of these
    // maps at +0xA928, and the following call at 0x8356069/0x8356071
    // constructs the second at +0xA940.  init @0x8359B3B imports
    // `(item_id, dictionary_bit)` into the first map; @0x8359BB0 imports the
    // reverse pair into the second and @0x8359C28 sends that reverse map to
    // `CSyncScript::insert_mapping_equip_info_to_db`.  These accessors are
    // B-owned layout boundaries, not invented old-server member identities.
    const std::map<int, int>&
    get_item_dictionary_item_to_flag_map() const;
    const std::map<int, int>&
    get_item_dictionary_flag_to_item_map() const;

    // B owns the embedded condition-event data at +0x4D98: C2/D2 invoke
    // stConditionEventInfo's lifecycle there.  Event gameplay reaches it
    // through this named boundary instead of projecting DataManager storage.
    stConditionEventInfo* get_condition_event_info();
    const stConditionEventInfo* get_condition_event_info() const;

    // C2/D2 construct this exact 0x10 condition table at +0x4DE8.  CMD16
    // starts use it to mark ordinary/event proper-level participation, so
    // expose the B-owned object without recreating its offset in CParty.
    ConditionLevelChkDungeon* get_condition_level_chk_dungeon();
    const ConditionLevelChkDungeon* get_condition_level_chk_dungeon() const;

    // B-owned typed reconstruction boundary for
    // CUser::processLevelUpEventReward @0x8663CC0.  CDataManager::C2/D2
    // construct and destroy the exact embedded stLevelUpRewardItemScript at
    // +0x50AC; this accessor keeps that owner offset out of C gameplay code.
    stLevelUpRewardItemScript* get_level_up_reward_item_script();

    // CDataManager::C2/D2 own this exact return-user reward script at
    // +0xB548.  Return-user gameplay consumes its package item and embedded
    // level-up reward table through this B-owned boundary.
    stReturnUserRewardScript* get_return_user_reward_script();

    // B-owned typed reconstruction boundary for
    // NewAccountLevelUpEventToJob::processLevelUpToJob @0x81B82A4.  The
    // equip-reward table is the stNewAccountLevelUpToJobScript child at
    // CDataManager+0xB5F8.
    stLevelUpRewardItemScript*
    get_new_account_level_up_equip_reward_script();

    // B-owned typed reconstruction boundary for CUser::_update_charac_stat
    // @0x86646C8.  The original body directly reads the CCharacList pointer
    // established by CDataManager::C2 at +0x14; C callers must not recreate
    // that owner layout with an untyped offset projection.
    CCharacList* get_character_list() const;

    // SkillSlot's original refund guards ask CSkillList for skills that would
    // become invalid after a downgrade.  Keep that owned pointer access in
    // CDataManager rather than duplicating the old +0x10 layout in gameplay.
    CSkillList* get_skill_list() const { return skill_list_; }

    // The original global character-skill seeder at 0x8604FE2 reads the
    // vector at character_list_->characters_[job].first_grow_skills_[0].
    // Keep that physical CCharacter ownership in CDataManager (which is the
    // declared friend of CCharacter), so callers never recreate +0x14/+0x240
    // projections of the PVF object.
    const std::vector<std::pair<int, int> >&
    get_initial_character_skills(int job) const;

    // CMD509 Dispatcher_ComboSkill::checkComboSkillIndex @0x825F292
    // directly addresses the job-9 first-grow skill vector through the
    // CDataManager-owned CCharacList (+0x14) at the proven aggregate offset
    // +0x48FC.  Keep that physical ownership here; network handlers only
    // ask for the original six-entry membership test.
    bool is_combo_skill_index(short skill_index) const;

    // B-owned typed reconstruction boundary for the embedded
    // PvPSkillTreeParameterScript at IDA-proven +0xA800.  The old callers
    // address this member directly; gameplay code must not repeat the owner
    // offset.
    PvPSkillTreeParameterScript* get_pvp_skill_tree_parameter_script();
    const PvPSkillTreeParameterScript*
    get_pvp_skill_tree_parameter_script() const;

    // B-owned typed reconstruction boundary for the embedded
    // EventCharacterParameterScript at IDA-proven +0xA86C.  C2/D2 construct
    // and destroy this same PVF object in place; event gameplay must reach
    // it through its data owner rather than reprojecting CDataManager.
    EventCharacterParameterScript*
    get_event_character_parameter_script();
    const EventCharacterParameterScript*
    get_event_character_parameter_script() const;

    // B-owned typed reconstruction boundary for the embedded
    // QuestParameterScript at IDA-proven +0x8CC8.  CDataManager::C2
    // @0x8355B30 constructs it, D2 @0x8356B68 destroys it, and init
    // imports questParameter.etc into the same physical subobject.
    // Gameplay consumers must not reproduce this owner offset.
    QuestParameterScript* get_quest_parameter_script();
    const QuestParameterScript* get_quest_parameter_script() const;

    // B-owned typed projection for the IDA-proven DisJoint pointer at +0x1C.
    // CItemUpgrade::_CheckCondition @0x8546342 consumes the concrete
    // DisJoint provider; keeping this bridge inline preserves the original
    // direct field load without reproducing CDataManager layout in the caller.
    DisJoint* get_disjoint() const { return disjoint_; }

    // B-owned typed projection for the IDA-proven CMapList pointer at +0x04.
    // PvP room start consumes the map count through the actual CMapList
    // service, without reproducing CDataManager's storage in the caller.
    CMapList* get_map_list() const { return map_list_; }

    // B-owned typed reconstruction boundaries for CBossStage::consistMap.
    // IDA 0x814BEF4 loads the already-named monster manager at +0x08.  This
    // is not an old exported CDataManager identity; it keeps the C caller out
    // of owner storage.
    CMonsterManager* get_boss_consist_monster_manager();

    // These are internal B layout boundaries, not reconstructed old-server
    // CDataManager methods.  CDataManager::C2 @0x8355B30 clears the
    // CAICharacterList pointer at +0x869C; init @0x8358332 allocates a real
    // CAICharacterList, invokes its C2 and init, then stores it there;
    // destroy @0x8357A66 invokes D2, operator delete, and clears the slot.
    // C2/D2 respectively construct/destroy IndependentDropParameterScript
    // in place at +0xA884.  Owner-side consumers use these boundaries rather
    // than reproducing offsets outside the canonical DataManager provider.
    CAICharacterList* get_ai_character_list();
    void set_ai_character_list(CAICharacterList* list);
    IndependentDropParameterScript* get_independent_drop_parameter_script();
    const IndependentDropParameterScript*
    get_independent_drop_parameter_script() const;

    // B-owned typed reconstruction boundary for the stMercenarySystemInfo
    // object constructed by CDataManager::C2 at the proven +0xA8DC slot.
    // The old gameplay methods read this member directly; the bridge keeps
    // that owner layout out of C callers and has no old exported identity.
    stMercenarySystemInfo* get_mercenary_system_info();
    const stMercenarySystemInfo* get_mercenary_system_info() const;

    // B-owned typed reconstruction boundary for the Power.etc instance
    // embedded at +0xA628.  The old server consumes +0x54 for the PowerWar
    // point multiplier and +0x58 for the straight-victories threshold.
    PowerParameterScript* get_power_war_parameter_script();
    const PowerParameterScript* get_power_war_parameter_script() const;
    int get_power_war_point_multiplier() const;
    int get_power_war_straight_victories() const;
    int get_power_war_victorious_reward_minutes() const;
    // IDA: Inter_PowerWarEndInfo @0x84DB070 and CPowerWarEvent::StartEvent
    // both consume CDataManager+0xA6A4, the Power.etc +0x7c basic-point
    // value.  Keep that physical field within the PVF owner.
    int get_power_war_basic_point() const;
    // IDA_EA: Dispatcher_CompleteLoadAfterAssault::dispatch_sig @0x8206B5C
    // reads CDataManager+0xA688, the Power.etc +0x60 member.
    int get_power_war_assault_superstate_after_duration_seconds() const;
    stVictoryPenalty get_power_war_victory_penalty(int party_victory) const;
    int get_power_war_break_bonus(int party_victory) const;

    // B-owned typed reconstruction boundary for the WarAreaCounter embedded
    // by CDataManager::C2/D2 at +0x87b4.  Its door_fee table is consumed by
    // the WarRoom state transition; C callers must not reproduce the owner
    // storage offset.
    WarAreaCounter* get_war_area_counter();
    const WarAreaCounter* get_war_area_counter() const;

    // B-owned projections for CUser::FatigueUp @0x8655C60.  The original
    // body addresses the embedded GuildParameterScript at +0xA50C and the
    // two fatigue luck-point rates at +0xE0/+0xE4.  Keep those offsets inside
    // their canonical CDataManager owner instead of repeating them in C.
    GuildParameterScript* GetGuildParameterScriptForBuild();
    const GuildParameterScript* GetGuildParameterScriptForBuild() const;
    int GetNormalFatigueLuckPointRateForBuild() const;
    int GetPremiumFatigueLuckPointRateForBuild() const;

    // B-owned typed data boundaries for CUser::increase_status and
    // CUser::RecoverFatigue.  The old binary reads these embedded PVF values
    // directly; gameplay consumers must not recreate CDataManager offsets.
    int GetSpecialLevelExpItemId() const;
    uint16_t GetWeekendBonusFatigue() const;

    // B-owned typed reconstruction boundary for
    // CUser::ValidateStrikerInfo @0x8690BE6.  The original member body loads
    // the initialized StrikerScript pointer from CDataManager+0x6390 before
    // validating each tag-character skill.  Keep this physical owner detail
    // in the DataManager provider rather than reopening it in CUser.
    StrikerScript* get_striker_script();

    // Dispatcher_SelectStriker @0x81E4066 compares the candidate character's
    // level with this scalar at CDataManager+0x629c.  The binary has no
    // exported accessor, so retain the physical boundary in its true PVF
    // owner rather than reproducing it in the packet handler.
    int get_striker_skill_minimum_level() const;

    // B-owned typed data boundary for the event multiplier consumed by
    // premium_helper_function::cal_premium_term @0x827CFF0.  The original
    // body reads the unsigned byte at CDataManager+0x877E; consumers keep
    // that physical-layout knowledge inside the canonical PVF provider.
    uint8_t GetCeraShopEventMultiplier() const;

    // CParty::dungeon_clear @0x85AB736 and TournamentDungeonClear
    // @0x85BD236 multiply their return-to-village delay by the int at
    // CDataManager+0xA784.  CDataManager::C2 @0x8355B30 constructs the
    // containing SecretShopScript at +0xA700; SecretShopScript::clear
    // @0x8372020 clears its +0x84 eplp_wait_time member.  Keep this
    // owner-side typed projection in PVF rather than exposing an offset
    // reader to party gameplay code.  It has no legacy exported ABI.
    int get_secret_shop_eplp_wait_time() const;

    // CSecretShop::LotteryNpc @0x85FABAC consumes the embedded script that
    // C2 constructs at +0xA700.  This remains a typed B-owner projection;
    // callers do not reopen CDataManager's physical layout.
    SecretShopScript* get_secret_shop_script();
    const SecretShopScript* get_secret_shop_script() const;

    // The account-cargo runtime historically reached the embedded script via
    // AccountCargoAt.  Keep that old source boundary, but have it resolve the
    // real +0xA7E0 member through its owner instead of re-projecting this.
    AccountCargoScript* get_account_cargo_script() {
        return &account_cargo_script_;
    }
    const AccountCargoScript* get_account_cargo_script() const {
        return &account_cargo_script_;
    }

    // CDataManager::C2/D2 construct and destroy the real StageMapList at
    // +0x8798.  Advance-Altar owns its stage progression but must obtain the
    // PVF object through its canonical owner rather than recreate that layout.
    StageMapList* get_stage_map_list();
    const StageMapList* get_stage_map_list() const;

    // DisPatcher_Login @0x81E9632 compares the client-supplied version with
    // CDataManager+0x3C0.  That dword is ServerParameterScript::[script
    // version] (the embedded script begins at +0x68 and owns it at +0x358),
    // not the separately derived anti-bot display version at +0x3C4.
    uint32_t get_login_client_version_gate() const {
        return server_parameter_script_.get_script_version();
    }

    // CDataManager::init @0x8358C24 derives this from the four [version]
    // bytes at +0x3BC and writes the result to +0x3C4.  It must not alias the
    // CMD1 compatibility gate above.
    uint32_t get_antibot_server_version() const {
        return server_parameter_script_.get_antibot_server_version();
    }

    // CCompoundAvatar::_ProcCompoundCore @0x8332EE8 applies the exact two
    // PVF-owned ten-thousandth modifiers at CDataManager+0x4a54/+0x4a58
    // before selecting an upper/lower compound result.  Keep the binary
    // layout inside DataManager; gameplay must not reproduce these offsets.
    int get_compound_avatar_upper_rate_bonus() const {
        return server_parameter_script_.get_compound_avatar_upper_rate_bonus();
    }
    int get_compound_avatar_lower_rate_reduction() const {
        return server_parameter_script_.get_compound_avatar_lower_rate_reduction();
    }
    int get_compound_avatar_master_rate_bonus() const {
        return server_parameter_script_.get_compound_avatar_master_rate_bonus();
    }

    // Event/NPCRelationEvent.evt is owned by the embedded +0x4d28 manager.
    // CMD222 consumes both the legacy relation table and the 140828 gift-log
    // policy through this typed owner boundary; no offset projection is used.
    CNPCRelationEventManager* get_npc_relation_event_manager() {
        return &npc_relation_event_manager_;
    }
    const CNPCRelationEventManager* get_npc_relation_event_manager() const {
        return &npc_relation_event_manager_;
    }

    // CAssaultMgr::GetRequestAssaultNeedMoney @0x82EDC86 owns these three
    // direct CDataManager values at +0x600/+0x604/+0x608.  Keep the price
    // decay curve in its PVF owner instead of reinterpreting DataManager in
    // the CMD447 dispatcher.
    int get_assault_price_time_weight_limit() const {
        return server_parameter_script_.get_assault_price_time_weight_limit();
    }
    int get_assault_price_time_limit_seconds() const {
        return server_parameter_script_.get_assault_price_time_limit_seconds();
    }
    float get_assault_price_level_gap_weight() const {
        return server_parameter_script_.get_assault_price_level_gap_weight();
    }

private:
    friend class CUser;
    // These original world/PvP members directly consume the corresponding
    // CDataManager-owned PVF children.  Friendship preserves that physical
    // ownership without creating offset/getter facade APIs.
    friend class Area;
    friend class GameWorld;
    friend class CPowerManager;
    friend class CAchievement;
    friend class DB_UpdateOneDayLetheSkill;
    friend class WongWork::CMailBoxHelper;
    // CMD454 @0x81DF42C consumes the original +0x18 QuestList child
    // directly.  Keeping this as direct friend access avoids inventing a
    // CDataManager accessor facade for the packet dispatcher.
    friend class Dispatcher_Req_Urgent_Quest;

    // Rebuild safety repair for a verified original heap overflow:
    // GlobalInstance<CDataManager>::create @ 0x80CF147 allocates exactly
    // 0xB678 bytes, while set_reward_sp @ 0x8360BDE writes 86 int values to
    // this+0xDA90..+0xDBE7.  C2/D2 prove that the original typed object ends
    // at DeleteInvalidItemScript (+0xB658, size 0x20), so the latter range
    // cannot be modeled as an undiscovered in-object field or raw tail.
    //
    // Keep the IDA-proven 0xB678-byte prefix intact and give the rebuild its
    // own typed, single-owner table.  C2 zero-initializes it, set_reward_sp
    // fills it once during init, and GetSpAtLevelUp reads it thereafter.
    // This is deliberately documented as a defect correction, not a claim
    // that +0xDA90 was a valid original CDataManager member.
    struct SpAtLevelUpTable {
        static const unsigned int kElementCount =
            oldsrv::config::kCharacterLevelTableCount;
        int32_t values_[kElementCount];
    };

    static_assert(sizeof(SpAtLevelUpTable) ==
                      sizeof(int32_t) * SpAtLevelUpTable::kElementCount,
                  "SP-at-level table must follow the configured level cap");

    // One physical 0627 layout.  Every C2/D2-owned value is a real C++
    // subobject in original construction order.  Byte arrays below cover
    // only ranges for which the current IDB proves scalar/POD state but has
    // not yet yielded stable field names; no non-trivial child is hidden in
    // them.
    CDungeonList* dungeon_index_list_;                   // +0x0000
    CMapList* map_list_;                                 // +0x0004
    CMonsterManager* monster_manager_;                   // +0x0008
    CItemList* item_list_;                               // +0x000c
    CSkillList* skill_list_;                             // +0x0010
    CCharacList* character_list_;                        // +0x0014
    QuestList* quest_list_;                              // +0x0018
    DisJoint* disjoint_;                                 // +0x001c
    WongWork::CItemUpgrade* item_upgrade_;               // +0x0020
    WongWork::CItemUpgrade_Separate* item_upgrade_sep_;  // +0x0024
    CChattingEmoticonList* chatting_emoticon_list_;      // +0x0028
    reseal_table_t reseal_table_;                        // +0x002c
    ServerParameterScript server_parameter_script_;      // +0x0068
    InGameAdvertisementScript in_game_advertisement_script_; // +0x4a78 (0828)
    uint8_t in_game_advertisement_alignment_;            // +0x4b0f (0828)
    PcBangItemRentarData pc_bang_item_rental_data_;      // +0x4b10 (0828)
    DimensionActivationData dimension_activation_data_;  // +0x4b34 (0828)
    seriaRoomDecoEventScript seria_room_deco_event_script_; // +0x4b70
    growthEquipmentScript growth_equipment_script_;      // +0x4bac
    createChracScript create_character_script_;          // +0x4bd4
    useCharacterJobFatigue use_character_job_fatigue_;   // +0x4bec
    CharacterCreateCreatureGiftScript
        character_create_creature_gift_script_;          // +0x4c04
    seriaRoomDecoAniEventScript
        seria_room_deco_animation_event_script_;         // +0x4c50
    stUnlimitChallengeInfo unlimit_challenge_info_;        // +0x4ca8
    CLevelRewardTableMgr* level_reward_table_manager_;    // +0x4cf4
    stLimitItemUsageInfoEx limit_item_usage_info_;        // +0x4cf8
    std::vector<stDBLogItem> db_log_items_;               // +0x4d1c
    CNPCRelationEventManager npc_relation_event_manager_; // +0x4d28
    CItemLimitEditionMgr* item_limit_edition_manager_;    // +0x4d40
    stEmotionTypeTagsInfo emotion_type_tags_info_;        // +0x4d44
    stEquipmentGradeSample equipment_grade_sample_;       // +0x4d5c
    stConditionEventInfo condition_event_info_;           // +0x4d98
    ConditionLevelChkDungeon condition_level_dungeon_;    // +0x4de8
    FairPvPStatScript fair_pvp_stat_script_;               // +0x4df8
    RandomOptionScript random_option_script_;             // +0x4e10
    RegenerationROI regeneration_roi_;                     // +0x5040
    CExpertJobList* expert_job_list_;                      // +0x5090
    stSeriaBlessingScript seria_blessing_script_;         // +0x5094
    stLevelUpRewardItemScript level_up_reward_item_script_; // +0x50ac
    stGrowthCapsuleScript growth_capsule_script_;          // +0x50c4
    SecurityReward security_reward_;                       // +0x50d8
    stRankSystemInfo rank_system_info_;                    // +0x5108
    std::map<unsigned int, stDeathTower_t> tower_scripts_; // +0x5180
    stDeathTower_t death_tower_;                           // +0x5198
    stBloodDungeon_t blood_dungeon_;                       // +0x5e30
    stVillageAttacked_t village_attacked_;                 // +0x61a0
    APSystem::CActionPointEtcParameter action_point_parameter_; // +0x61ac
    APSystem::CActionPointEtcParameter gm_action_point_parameter_; // +0x6224
    stCharacLinkSystem_t charac_link_system_;              // +0x629c
    advancealtar::StageTimeLineParameter
        advance_altar_stage_timeline_;                     // +0x62d0
    advancealtar::AdvanceAltarShopParameter
        advance_altar_shop_parameter_;                    // +0x631c
    advancealtar::RewardParameter advance_altar_reward_parameter_; // +0x634c
    advancealtar::ConfigParameter advance_altar_config_parameter_; // +0x6364
    StrikerScript* striker_script_;                        // +0x6390
    CharacManageScript* character_manage_script_;          // +0x6394
    stQuestShop quest_shop_script_;                        // +0x6398
    WorldDropInfo world_drop_info_[201];                   // +0x6734
    WorldDropInfo original_world_drop_info_[201];          // +0x76e8
    CAICharacterList* ai_character_list_;                  // +0x869c
    SlangNameData slang_name_data_;                        // +0x86a0
    InvalidCharData invalid_char_data_;                    // +0x86c4
    class SlangFilter* slang_filter_;                       // +0x86f4
    GrowthPowerPackageRewardMap fair_pvp_equipment_;       // +0x86f8
    STGrowthPowerData growth_power_data_;                  // +0x8710
    AvatarVariation::AvatarColorInfo avatar_color_info_;   // +0x8740
    BoosterGageData booster_gage_data_;                    // +0x8770
    CWorldMapList world_map_list_;                         // +0x8780
    StageMapList stage_map_list_;                          // +0x8798
    WarAreaCounter war_area_counter_;                      // +0x87b4
    std::string cera_shop_goods_name_;                     // +0x8cb8
    CItemShop* item_shop_;                                 // +0x8cbc
    WongWork::CItemGeneratorMgr* item_generator_;          // +0x8cc0
    CAutoMarketConditionsControl* auto_market_control_;    // +0x8cc4
    QuestParameterScript quest_parameter_script_;          // +0x8cc8
    TrainingQuestScript training_quest_script_;            // +0x8d34
    std::vector<int> daily_training_quests_[
        oldsrv::config::kTrainingQuestLevelCount];         // +0xa1c4
    GuildParameterScript guild_parameter_script_;          // +0xa50c
    PowerParameterScript power_parameter_script_;          // +0xa628
    SecretShopScript secret_shop_script_;                  // +0xa700
    stTitleBookInfo title_book_info_;                      // +0xa788
    AccountCargoScript account_cargo_script_;              // +0xa7e0
    PvPSkillTreeParameterScript pvp_skill_tree_parameter_script_; // +0xa800
    EventCharacterParameterScript event_character_parameter_script_; // +0xa86c
    IndependentDropParameterScript independent_drop_parameter_script_; // +0xa884
    std::string cash_shop_file_name_;                      // +0xa8c0
    std::string sudden_shop_file_name_1_;                  // +0xa8c4
    std::string sudden_shop_file_name_2_;                  // +0xa8c8
    std::string sudden_shop_file_name_3_;                  // +0xa8cc
    class CNPCScriptList* npc_script_list_;                // +0xa8d0
    class CNPCDynamicInfoManager* npc_dynamic_info_manager_; // +0xa8d4
    uint8_t charac_name_filter_pass_;                      // +0xa8d8
    uint8_t charac_name_filter_alignment_[3];              // +0xa8d9
    stMercenarySystemInfo mercenary_system_info_;          // +0xa8dc
    std::map<int, int> item_dictionary_item_to_flag_;      // +0xa928
    std::map<int, int> item_dictionary_flag_to_item_;      // +0xa940
    uint8_t scalar_tables_a958_aa74_[0x11c];               // +0xa958
    uint8_t dimension_inout_[6];                           // +0xaa74
    uint8_t original_dimension_inout_[6];                  // +0xaa7a
    uint8_t limit_inout_count_[4];                         // +0xaa80
    int32_t level_exp_[200];                               // +0xaa84
    uint32_t mob_rewards_[201];                            // +0xada4
    uint32_t original_mob_rewards_[201];                   // +0xb0c8
    float experience_rate_;                               // +0xb3ec
    float drop_rate_;                                     // +0xb3f0
    channel_script_t channel_script_;                     // +0xb3f4
    std::vector<stVendingMachine_t> vending_machine_table_; // +0xb424
    pvp_channel_info_t pvp_channel_info_;                  // +0xb430
    PvP_MissionSystem* pvp_mission_system_;                // +0xb454
    stItemMakingSkill item_making_skill_;                  // +0xb458
    std::set<int> ultimate_dungeon_list_;                  // +0xb488
    ChoiceItemInfo choice_item_info_;                      // +0xb4a0
    BlueMarbleScriptManager* blue_marble_script_;          // +0xb4a8
    std::set<unsigned int> auction_reg_free_cera_items_;   // +0xb4ac
    ImageCommunicationData image_communication_data_;      // +0xb4c4
    ARAD::SCRIPT::AradJumping_Script arad_jumping_script_; // +0xb4c8
    CLottery_NeedMoney lottery_need_money_;                // +0xb4d4
    live_server_info_t live_server_info_;                  // +0xb4ec
    EventClassify::CEventScriptMng* event_script_manager_; // +0xb504
    EventAvatarListScript event_avatar_list_script_;       // +0xb508
    EventEtcScript event_etc_script_;                      // +0xb520
    std::string script_packs_;                             // +0xb544
    stReturnUserRewardScript return_user_reward_script_;   // +0xb548
    stBingoScript bingo_script_;                           // +0xb568
    stBroadCastItemScript broadcast_item_script_;          // +0xb5bc
    stNewAccountLevelUpToJobScript
        new_account_level_up_to_job_script_;               // +0xb5e0
    eventReward event_reward_;                             // +0xb628
    DeleteInvalidItemScript delete_invalid_item_script_;   // +0xb658

    // This follows (rather than overlays) the entire original ABI prefix.
    // It has trivial construction/destruction and is owned by this singleton.
    SpAtLevelUpTable sp_at_level_up_;

    friend struct CDataManagerLayoutAssert;
    CDataManager(const CDataManager&);
    CDataManager& operator=(const CDataManager&);
};

struct CDataManagerLayoutAssert {
#if defined(__i386__)
    static_assert(offsetof(CDataManager, dungeon_index_list_) == 0x00u,
                  "dungeon-list pointer offset drift");
    static_assert(offsetof(CDataManager, map_list_) == 0x04u,
                  "map-list pointer offset drift");
    static_assert(offsetof(CDataManager, monster_manager_) == 0x08u,
                  "monster-manager pointer offset drift");
    static_assert(offsetof(CDataManager, item_list_) == 0x0Cu,
                  "item-list pointer offset drift");
    static_assert(offsetof(CDataManager, skill_list_) == 0x10u,
                  "skill-list pointer offset drift");
    static_assert(offsetof(CDataManager, character_list_) == 0x14u,
                  "character-list pointer offset drift");
    static_assert(offsetof(CDataManager, quest_list_) == 0x18u,
                  "quest-list pointer offset drift");
    static_assert(offsetof(CDataManager, disjoint_) == 0x1Cu,
                  "disjoint pointer offset drift");
    static_assert(offsetof(CDataManager, item_upgrade_) == 0x20u,
                  "item-upgrade pointer offset drift");
    static_assert(offsetof(CDataManager, item_upgrade_sep_) == 0x24u,
                  "separate-upgrade pointer offset drift");
    static_assert(offsetof(CDataManager, chatting_emoticon_list_) == 0x28u,
                  "chatting-emoticon pointer offset drift");
    static_assert(offsetof(CDataManager, reseal_table_) == 0x2Cu,
                  "reseal-table offset drift");
    static_assert(offsetof(CDataManager, server_parameter_script_) == 0x68u,
                  "server-parameter script offset drift");
    static_assert(offsetof(CDataManager, in_game_advertisement_script_) ==
                      0x4A78u,
                  "in-game advertisement offset drift");
    static_assert(offsetof(CDataManager, pc_bang_item_rental_data_) == 0x4B10u,
                  "PC-bang rental data offset drift");
    static_assert(offsetof(CDataManager, dimension_activation_data_) ==
                      0x4B34u,
                  "dimension activation data offset drift");
    static_assert(offsetof(CDataManager, item_limit_edition_manager_) ==
                      0x4D5Cu,
                  "item-limit manager pointer offset drift");
    static_assert(offsetof(CDataManager, emotion_type_tags_info_) == 0x4D60u,
                  "emotion-type tags offset drift");
    static_assert(offsetof(CDataManager, equipment_grade_sample_) == 0x4D78u,
                  "equipment-grade sample offset drift");
    static_assert(offsetof(CDataManager, fair_pvp_stat_script_) == 0x4E14u,
                  "fair-PvP stat script offset drift");
    static_assert(offsetof(CDataManager, random_option_script_) == 0x4E2Cu,
                  "random-option script offset drift");
    static_assert(offsetof(CDataManager, regeneration_roi_) == 0x505Cu,
                  "regeneration ROI offset drift");
    static_assert(offsetof(CDataManager, seria_blessing_script_) == 0x50B0u,
                  "Seria blessing script offset drift");
    static_assert(offsetof(CDataManager, rank_system_info_) == 0x5124u,
                  "rank-system info offset drift");
    static_assert(offsetof(CDataManager, tower_scripts_) == 0x519Cu,
                  "tower-script map offset drift");
    static_assert(offsetof(CDataManager, death_tower_) == 0x51B4u,
                  "death-tower script offset drift");
    static_assert(offsetof(CDataManager, blood_dungeon_) == 0x5E4Cu,
                  "blood-dungeon script offset drift");
    static_assert(offsetof(CDataManager, village_attacked_) == 0x61BCu,
                  "village-attacked PVF child offset drift");
    static_assert(offsetof(CDataManager, action_point_parameter_) == 0x61C8u,
                  "action-point parameter offset drift");
    static_assert(offsetof(CDataManager, gm_action_point_parameter_) == 0x6240u,
                  "GM action-point parameter offset drift");
    static_assert(offsetof(CDataManager, charac_link_system_) == 0x62B8u,
                  "character-link system offset drift");
    static_assert(offsetof(CDataManager, advance_altar_stage_timeline_) ==
                      0x62ECu,
                  "Advance Altar timeline offset drift");
    static_assert(offsetof(CDataManager, advance_altar_shop_parameter_) ==
                      0x6338u,
                  "Advance Altar shop parameter offset drift");
    static_assert(offsetof(CDataManager, advance_altar_reward_parameter_) ==
                      0x6368u,
                  "Advance Altar reward parameter offset drift");
    static_assert(offsetof(CDataManager, advance_altar_config_parameter_) ==
                      0x6380u,
                  "Advance Altar config parameter offset drift");
    static_assert(offsetof(CDataManager, character_manage_script_) == 0x63B0u,
                  "character-management script pointer offset drift");
    static_assert(offsetof(CDataManager, quest_shop_script_) == 0x63B4u,
                  "quest-shop script offset drift");
    static_assert(offsetof(CDataManager, slang_name_data_) == 0x86BCu,
                  "slang-name data offset drift");
    static_assert(offsetof(CDataManager, invalid_char_data_) == 0x86E0u,
                  "invalid-character data offset drift");
    static_assert(offsetof(CDataManager, fair_pvp_equipment_) == 0x8714u,
                  "fair-PvP equipment map offset drift");
    static_assert(offsetof(CDataManager, growth_power_data_) == 0x872Cu,
                  "growth-power data offset drift");
    static_assert(offsetof(CDataManager, avatar_color_info_) == 0x875Cu,
                  "avatar-color info offset drift");
    static_assert(offsetof(CDataManager, booster_gage_data_) == 0x878Cu,
                  "booster-gage data offset drift");
    static_assert(offsetof(CDataManager, world_map_list_) == 0x879Cu,
                  "world-map list offset drift");
    static_assert(offsetof(CDataManager, stage_map_list_) == 0x87B4u,
                  "stage-map list offset drift");
    static_assert(offsetof(CDataManager, war_area_counter_) == 0x87D0u,
                  "war-area counter offset drift");
    static_assert(offsetof(CDataManager, item_shop_) == 0x8CD8u,
                  "item-shop pointer offset drift");
    static_assert(offsetof(CDataManager, quest_parameter_script_) == 0x8CE4u,
                  "quest-parameter script offset drift");
    static_assert(offsetof(CDataManager, training_quest_script_) == 0x8D50u,
                  "training-quest script offset drift");
    static_assert(offsetof(CDataManager, guild_parameter_script_) == 0xA528u,
                  "guild-parameter script offset drift");
    static_assert(offsetof(CDataManager, power_parameter_script_) == 0xA644u,
                  "Power.etc PVF child offset drift");
    static_assert(offsetof(CDataManager, secret_shop_script_) == 0xA71Cu,
                  "secret-shop script offset drift");
    static_assert(offsetof(CDataManager, title_book_info_) == 0xA7A4u,
                  "title-book info offset drift");
    static_assert(offsetof(CDataManager, account_cargo_script_) == 0xA7FCu,
                  "account-cargo script offset drift");
    static_assert(offsetof(CDataManager, pvp_skill_tree_parameter_script_) ==
                      0xA81Cu,
                  "PvP skill-tree parameter offset drift");
    static_assert(offsetof(CDataManager, mercenary_system_info_) == 0xA8F8u,
                  "mercenary-system info offset drift");
    static_assert(offsetof(CDataManager, channel_script_) == 0xB410u,
                  "channel-script offset drift");
    static_assert(offsetof(CDataManager, vending_machine_table_) == 0xB440u,
                  "vending-machine table offset drift");
    static_assert(offsetof(CDataManager, arad_jumping_script_) == 0xB4E4u,
                  "Arad Jumping script offset drift");
    static_assert(offsetof(CDataManager, event_script_manager_) == 0xB520u,
                  "event-script manager pointer offset drift");
    static_assert(offsetof(CDataManager, event_avatar_list_script_) == 0xB524u,
                  "event-avatar list offset drift");
    static_assert(offsetof(CDataManager, return_user_reward_script_) == 0xB564u,
                  "return-user reward script offset drift");
    static_assert(offsetof(CDataManager, bingo_script_) == 0xB584u,
                  "Bingo script offset drift");
    static_assert(offsetof(CDataManager, broadcast_item_script_) == 0xB5D8u,
                  "broadcast-item script offset drift");
    static_assert(offsetof(CDataManager, new_account_level_up_to_job_script_) ==
                      0xB5FCu,
                  "new-account level-up script offset drift");
    static_assert(offsetof(CDataManager, event_reward_) == 0xB644u,
                  "event-reward script offset drift");
    static_assert(offsetof(CDataManager, delete_invalid_item_script_) ==
                      0xB674u,
                  "delete-invalid-item script offset drift");
    static_assert(offsetof(CDataManager, sp_at_level_up_) == 0xB694u,
                  "SP table must follow the exact original ABI prefix");
    static_assert(sizeof(CDataManager) == 0xB7ECu,
                  "typed SP extension size drift");
#endif
};

// IDA_EA: 0x80CC19B
// IDA_NAME: _Z14G_CDataManagerv
// DEMANGLED: G_CDataManager(void)
// This is the original process-wide PVF owner accessor.  It deliberately
// routes through GlobalInstance so it retains the old lazy-create semantics.
CDataManager* G_CDataManager();


#endif  // OLDSRV_DATAMANAGER_DATA_MANAGER_H_
