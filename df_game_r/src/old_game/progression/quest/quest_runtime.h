#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_RUNTIME_H_

// The stable legacy C08 units already contain the one-to-one UserQuest,
// quest-dispatcher and ComboSkill bodies.  Import their sole declarations
// while C migration proceeds; never recreate a parallel quest object layout.
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/skill/combo_skill.h"
#include "old_data/PVF/n_quest/quest_skill_script.h"
#include "old_game/progression/skill/item_making_skill.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/player/core/charac_expand_cdata.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "infra/network/packet/PacketHeader.h"

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

class CUser;
class CUserCharacInfo;
class CItem;
class CStackableItem;
class PacketGuard;
class CSkill;
struct stCompoundRate;
struct stSPReturnInfo_t;
struct STGuildDBInfoOnly;
enum ENUM_AGREE_ENCHANT_TYPE : int;
enum ENUM_EXPERT_JOB_TYPE : int;
enum ENUM_PEER_REQUEST_TYPE : int;
enum ENUM_TRY_ENCHANT_TYPE : int;

#pragma pack(push, 1)
// IDA packet payload used by CEnchanter::_PutItemForEnchant/_TryEnchant.
// The old command carries one byte of item-space followed immediately by the
// two-byte slot, so the public ABI type is exactly three bytes.
struct stItemPos_t {
    uint8_t item_space;
    uint16_t slot;
};

// Common physical prefix of the static-command family.  Individual commands
// extend it with their own typed body; the C++ identity remains the original
// MSG_STATIC_COMMAND used by the exported member signatures.
struct MSG_STATIC_COMMAND : CS_Header {
    uint8_t action_type;  // IDA +0x0d
};
#pragma pack(pop)

static_assert(sizeof(stItemPos_t) == 0x03u,
              "stItemPos_t old packet ABI must stay 0x03");
static_assert(sizeof(MSG_STATIC_COMMAND) == 0x0eu,
              "MSG_STATIC_COMMAND prefix must stay 14 bytes");
static_assert(offsetof(MSG_STATIC_COMMAND, action_type) == 0x0du,
              "MSG_STATIC_COMMAND action byte must stay at +0x0d");

// IDA exposes this named enum in Quest::CheckKillMonster, but the recovered
// old source has no enumerator list yet.  Its 32-bit by-value ABI is fixed by
// the old mangled signature; retain the named opaque enum instead of replacing
// it with an int or a raw layout surrogate.
enum ENUM_QUEST_ENEMY_TYPE : int;

// IDA unit: 08_quest_skill_growth/Quest.
// The full object layout is fixed by Quest::Quest @0x837AA28,
// Quest::~Quest @0x837A6E0, and QuestList::load_list's 0x150 allocation.
// The fields below are real old-IDB fields, not a substitute layout.
class Quest {
public:
    // IDA_EA: 0x837AA28.
    Quest();
    // IDA_EA: 0x837A6E0.
    ~Quest();

    // IDA_EA: 0x835233E.
    void set_quest(QuestScript script);
    // IDA_EA: 0x822B5E2.
    int exposeQuest() const;
    // IDA_EA: 0x8352A5C.
    int checkPossibleJob(ENUM_CHARACTERJOB charac_job) const;
    // IDA_EA: 0x8352AFC.
    int checkMatchedGrowType(ENUM_CHARACTERJOB charac_job,
                             int first_grow_type,
                             int second_grow_type) const;
    // IDA_EA: 0x8352BEE.
    bool isRepeatableQuest() const;
    // IDA_EA: 0x8352C10.
    int _isNeededCheckGrowType(ENUM_CHARACTERJOB charac_job) const;
    // IDA_EA: 0x8352CC2.
    // IDA_NAME: _ZN5Quest16check_power_sideEc
    int check_power_side(char power_side);
    // IDA_EA: 0x8352D86.
    bool check_possible(const stSelectQuestParam& param) const;
    // IDA_EA: 0x8352FB4.
    int get_init_achievement_trigger(unsigned short& first,
                                     unsigned short& second,
                                     unsigned short& third) const;
    // IDA_EA: 0x8353082.
    int get_title_reward() const;
    // IDA_EA: 0x83530D6.
    int get_init_trigger() const;
    // IDA_EA: 0x8353342.
    int check_clear_item(int dungeon_index, int difficulty) const;
    // IDA_EA: 0x8353404.
    std::vector<std::pair<int, int> > get_clear_itemlist(
        int dungeon_index) const;
    // IDA_EA: 0x83535D6.
    bool CheckKillMonster(int dungeon_index,
                          int difficulty,
                          int monster_index,
                          ENUM_QUEST_ENEMY_TYPE enemy_type,
                          std::vector<MonsterKillItem>& matched) const;
    // IDA_EA: 0x83536F2.
    int get_appearmap(int index, int compare_index) const;
    // IDA_EA: 0x8353776.
    bool IsOpenScheduleQuest() const;
    // IDA_EA: 0x83537C2.
    int ConvertRewardSelectIndex(const CUser* user, int select_index);
    // IDA_EA: 0x84E923C.
    int check_clear_send() const;

    std::string name;                                      // +0x000
    int quest_index;                                       // +0x004
    int quest_grade;                                       // +0x008
    int value_0c;                                          // +0x00c
    std::vector<stQuestTargetCharacter_t> target_characters;  // +0x010
    int check_grow_type_mode;                              // +0x01c
    int minimum_level;                                    // +0x020
    int maximum_level;                                    // +0x024
    int level_limit_value;                                // +0x028
    std::vector<std::vector<int> > conditions;            // +0x02c
    std::vector<int> condition_values;                     // +0x038
    bool clear_send_enabled;                              // +0x044
    uint8_t pad_45[3];
    std::vector<DungeonClearItem> dungeon_clear_items;    // +0x048
    std::vector<MonsterKillItem> monster_kill_items;      // +0x054
    AppearMap appear_map;                                  // +0x060
    std::vector<std::pair<int, int> > depend_give_items;  // +0x070
    int trigger_type;                                     // +0x07c
    int trigger_subtype;                                  // +0x080
    std::vector<int> clear_condition_values;              // +0x084
    std::vector<int> achievement_trigger_values;          // +0x090
    PostalReward postal_reward;                            // +0x09c
    std::vector<std::pair<int, int> > reward_select_pairs; // +0x0ac
    int reward_type;                                      // +0x0b8
    int value_bc;                                         // +0x0bc
    int value_c0;                                         // +0x0c0
    std::vector<RewardItemInfo> reward_items;             // +0x0c4
    std::vector<RewardItemInfo> light_reward_items;       // +0x0d0
    std::vector<int> clear_reward_items;                  // +0x0dc
    int value_e8;                                         // +0x0e8
    std::vector<RewardItemInfo> reward_select_items;      // +0x0ec
    int value_f8;                                         // +0x0f8
    int value_fc;                                         // +0x0fc
    bool exposed_by_npc;                                  // +0x100
    bool event;                                           // +0x101
    uint8_t difficulty;                                   // +0x102
    uint8_t pad_103;
    int gold_multiple;                                    // +0x104
    int8_t mail_time;                                     // +0x108
    uint8_t pad_109[3];
    std::string mail_title;                               // +0x10c
    std::string mail_contents;                            // +0x110
    std::vector<int> mail_enclose;                        // +0x114
    bool cant_giveup;                                     // +0x120
    uint8_t pad_121[3];
    std::vector<dungeonEventHanaseStruct> dungeon_event_dialogs; // +0x124
    bool value_130;                                       // +0x130
    uint8_t belong_to_winning_power;                       // +0x131
    uint8_t pad_132[2];
    int quest_type;                                       // +0x134
    int16_t quest_point;                                  // +0x138
    int16_t quest_piece;                                  // +0x13a
    int expert_job_level;                                 // +0x13c
    int expert_job_level_max;                             // +0x140
    std::vector<ENUM_QUEST_EXCEPTION_TYPE> exception_quest_types; // +0x144
};

// IDA_EA: 0x8352203.
// IDA_NAME: _Z18makeRewardItemInfoRKSt6vectorIiSaIiEERS_I14RewardItemInfoSaIS4_EE
// DEMANGLED: makeRewardItemInfo(std::vector<int> const&,std::vector<RewardItemInfo>&)
void makeRewardItemInfo(const std::vector<int>& values,
                        std::vector<RewardItemInfo>& rewards);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Quest) == 0x150u,
              "Quest old ABI size must stay 0x150");
static_assert(offsetof(Quest, name) == 0x000u &&
                  offsetof(Quest, quest_index) == 0x004u &&
                  offsetof(Quest, quest_grade) == 0x008u &&
                  offsetof(Quest, target_characters) == 0x010u &&
                  offsetof(Quest, minimum_level) == 0x020u &&
                  offsetof(Quest, dungeon_clear_items) == 0x048u &&
                  offsetof(Quest, monster_kill_items) == 0x054u &&
                  offsetof(Quest, appear_map) == 0x060u &&
                  offsetof(Quest, trigger_type) == 0x07cu &&
                  offsetof(Quest, clear_condition_values) == 0x084u &&
                  offsetof(Quest, achievement_trigger_values) == 0x090u &&
                  offsetof(Quest, postal_reward) == 0x09cu &&
                  offsetof(Quest, reward_items) == 0x0c4u &&
                  offsetof(Quest, light_reward_items) == 0x0d0u &&
                  offsetof(Quest, exposed_by_npc) == 0x100u &&
                  offsetof(Quest, difficulty) == 0x102u &&
                  offsetof(Quest, mail_title) == 0x10cu &&
                  offsetof(Quest, mail_enclose) == 0x114u &&
                  offsetof(Quest, dungeon_event_dialogs) == 0x124u &&
                  offsetof(Quest, belong_to_winning_power) == 0x131u &&
                  offsetof(Quest, quest_type) == 0x134u &&
                  offsetof(Quest, quest_point) == 0x138u &&
                  offsetof(Quest, quest_piece) == 0x13au &&
                  offsetof(Quest, exception_quest_types) == 0x144u,
              "Quest old ABI member offsets must stay exact");
#endif

namespace expert_job {

enum ENUM_CLOSE_MISSION_TYPE : int;

class CExpertJob;
class CExpertJobFactory;

class CExpertJobMgr {
public:
    // IDA_EA: 0x849E102 / 0x849E1AC
    CExpertJobMgr();
    ~CExpertJobMgr();

    // IDA_EA: 0x849E1DC / 0x849E1F8
    CExpertJob* AcquireExpertJob(ENUM_EXPERT_JOB_TYPE expert_job_type);
    CExpertJob* FreeExpertJob(CExpertJob* expert_job);

    // IDA_EA: 0x849E214
    // IDA_NAME: _ZN10expert_job13CExpertJobMgr11OnLeaveUserEP5CUser
    // DEMANGLED: expert_job::CExpertJobMgr::OnLeaveUser(CUser *)
    char OnLeaveUser(CUser* user);

    // IDA_EA: 0x849E256 / 0x849E55E
    // CMD010's expert-job peer-request handoff.  The target availability
    // check has exactly the two CUser inputs used by the recovered body.
    int OnRequestPeer(CUser* requester,
                      CUser* target,
                      ENUM_PEER_REQUEST_TYPE request_type,
                      PacketGuard& packet);
    // IDA_EA: 0x849E362.  CMD011 confirms the peer request, starts the
    // shared enchanter mission, and emits both accepted-notification frames.
    int OnResponsePeer(CUser* requester,
                       CUser* target,
                       CUser* mission_user,
                       CUser* peer_user,
                       int payload,
                       ENUM_PEER_REQUEST_TYPE request_type,
                       PacketGuard& packet);
    int IsAvailableRequestPeer(CUser* requester, CUser* target);

    // IDA_EA: 0x849E60C
    // IDA_NAME: _ZN10expert_job13CExpertJobMgr8OnGiveupEP5CUser
    // DEMANGLED: expert_job::CExpertJobMgr::OnGiveup(CUser *)
    void OnGiveup(CUser* user);

    // IDA_EA: 0x849ED1A
    // IDA_NAME: _ZN10expert_job13CExpertJobMgr15OnEnchantByBeadEP5CUseriiii
    // DEMANGLED: expert_job::CExpertJobMgr::OnEnchantByBead(CUser *,int,int,int,int)
    int OnEnchantByBead(CUser* user,
                        int source_item_space,
                        int bead_slot,
                        int target_item_space,
                        int target_slot);

private:
    // IDA CExpertJobMgr is one pointer wide; C2 allocates the exact 0x60-byte
    // factory and D2 releases it.
    CExpertJobFactory* factory_;
};

class CExpertJob {
public:
    CExpertJob();

    // Vtable order proven by the CEnchanter/CDisjointer tables:
    // +0x00 start, +0x04 close, +0x08 available, +0x0c reset,
    // +0x10 complete destructor, +0x14 deleting destructor.
    virtual int OnStartMission(CUser* user, CUser* peer_user) = 0;
    virtual void OnCloseMission(CUser* user,
                                ENUM_CLOSE_MISSION_TYPE close_type) = 0;
    virtual int IsAvailableJob() = 0;
    virtual void Reset() = 0;
    virtual ~CExpertJob();

    // IDA_EA: 0x849AE40
    // IDA_NAME: _ZN10expert_job10CExpertJob20IncreaseExpertJobExpEP5CUseri
    // DEMANGLED: expert_job::CExpertJob::IncreaseExpertJobExp(CUser *,int)
    // The old call sites pass exactly these two machine arguments.  The
    // member-qualified symbol is therefore the stateless/static helper, not
    // an instance call with an additional hidden receiver.
    static void IncreaseExpertJobExp(CUser* user, int expert_job_exp);

    static stCompoundRate* GetSuccessRate(CUser* user,
                                          const CStackableItem* recipe,
                                          int& rate_variation);
    static bool IsSuccess(CUser* user,
                          const CStackableItem* recipe,
                          int& expert_job_exp);
    static bool IsLearnRecipe(CUser* user,
                              const CStackableItem* recipe);

    CExpertJob* SetType(ENUM_EXPERT_JOB_TYPE expert_job_type);
    int GetType();

protected:
    // The compiler-owned vptr is +0x00; GetType/SetType access this dword.
    int type_;  // +0x04
};

// These old CAlchemist item-policy entries are stateless expert-job helpers.
// Their Itanium names encode every explicit argument; no CAlchemist receiver
// is present at their call sites.
class CAlchemist {
public:
    // IDA_EA: 0x85D121E
    // IDA_NAME: _ZN10expert_job10CAlchemist27IsExistContinuousEffectItemEP5CUseri
    // DEMANGLED: expert_job::CAlchemist::IsExistContinuousEffectItem(
    //             CUser *,int)
    static int IsExistContinuousEffectItem(CUser* user, int item_id);

    // IDA_EA: 0x85D12EE
    // IDA_NAME: _ZN10expert_job10CAlchemist20GetQuestItemDropRateEP5CUseri
    // DEMANGLED: expert_job::CAlchemist::GetQuestItemDropRate(CUser *,int)
    static int GetQuestItemDropRate(CUser* user, int base_rate);

    // IDA_EA: 0x85D1360
    // IDA_NAME: _ZN10expert_job10CAlchemist25GetItemUpgradeSuccessRateEv
    // DEMANGLED: expert_job::CAlchemist::GetItemUpgradeSuccessRate(void)
    static long double GetItemUpgradeSuccessRate();

    // IDA_EA: 0x85D137C
    // IDA_NAME: _ZN10expert_job10CAlchemist17GetAssaultPreventEP5CUserPb
    static int GetAssaultPrevent(CUser* user, bool* party_member_prevent);

    // IDA_EA: 0x85D1424
    // IDA_NAME: _ZN10expert_job10CAlchemist19OnTeleportCharacterEP5CUseriiiib
    // DEMANGLED: expert_job::CAlchemist::OnTeleportCharacter(
    //     CUser *,int,int,int,int,bool)
    //
    // This is the target-village movement transaction used by CMD241 after
    // the teleport potion is removed.  It is a stateless helper: the
    // Itanium name has no CAlchemist receiver.
    static int OnTeleportCharacter(CUser* user,
                                   int village_index,
                                   int area_index,
                                   int position_x,
                                   int position_y,
                                   bool use_gate_center);

    // IDA_EA: 0x85D1694
    // IDA_NAME: _ZN10expert_job10CAlchemist14UseRevivalItemEP5CUser
    // DEMANGLED: expert_job::CAlchemist::UseRevivalItem(CUser *)
    static int UseRevivalItem(CUser* user);

    // IDA_EA: 0x85D1712
    // IDA_NAME: _ZN10expert_job10CAlchemist18OnUseStackableItemEP5CUsermPK5CItem
    // DEMANGLED: expert_job::CAlchemist::OnUseStackableItem(
    //             CUser *,unsigned long,CItem const *)
    static int OnUseStackableItem(CUser* user,
                                  unsigned long item_index,
                                  const CItem* item);

    // IDA_EA: 0x85D18AE
    // IDA_NAME: _ZN10expert_job10CAlchemist10UseDelilahEP5CUser
    // DEMANGLED: expert_job::CAlchemist::UseDelilah(CUser *)
    static int UseDelilah(CUser* user);

    // IDA_EA: 0x85D18E2
    // IDA_NAME: _ZN10expert_job10CAlchemist24CheckUsableStackableItemEP5CUserPK5CItem
    // DEMANGLED: expert_job::CAlchemist::CheckUsableStackableItem(
    //             CUser *,CItem const *)
    static int CheckUsableStackableItem(CUser* user,
                                        const CItem* item);
};

// IDA unit: 08_quest_skill_growth/expert_job/ExpertJobMaterial.
struct ExpertJobMaterial {
    ExpertJobMaterial();

    ExpertJobMaterial* Reset();
    bool Empty();
    int SetData(CUser* user, unsigned short item_space, unsigned short slot);
    int CheckType(bool stackable, int item_type);
    int IsRight();
    int IsRecipe();
    int IsExistMaterialOfRecipe();
    bool DelMaterialOfRecipe(
        CUser* user,
        const std::vector<std::pair<int, int> >& materials,
        int recipe_slot,
        PacketGuard* packet,
        bool& equipment_changed);
    int UseMaterialOfRecipe(PacketGuard* packet);
    static CItem* FindMonsterCard(
        const std::vector<std::pair<int, int> >& materials);
    CItem* GetEnchantItem();

    CUser* user_;             // +0x00
    CItem* item_;             // +0x04
    int item_id_;             // +0x08
    uint16_t slot_;           // +0x0c
    uint16_t item_space_;     // +0x0e
};

// IDA unit: 99_misc_global/expert_job/CEnchanter.
class CEnchanter : public CExpertJob {
public:
    friend class CExpertJobMgr;

    CEnchanter();
    virtual ~CEnchanter();

    virtual int OnStartMission(CUser* user, CUser* peer_user);
    virtual void OnCloseMission(CUser* user,
                                ENUM_CLOSE_MISSION_TYPE close_type);
    virtual int IsAvailableJob();
    virtual void Reset();

    // IDA_EA: 0x849C7B6
    // IDA_NAME: _ZN10expert_job10CEnchanter14OnStartMissionEP5CUserS2_
    // DEMANGLED: expert_job::CEnchanter::OnStartMission(CUser *,CUser *)
    // IDA_EA: 0x849DF5E
    // IDA_NAME: _ZN10expert_job10CEnchanter19OnPutItemForEnchantEP5CUserP18MSG_STATIC_COMMAND
    // DEMANGLED: expert_job::CEnchanter::OnPutItemForEnchant(
    //             CUser *,MSG_STATIC_COMMAND *)
    char OnPutItemForEnchant(CUser* user, MSG_STATIC_COMMAND* command);

    // IDA_EA: 0x849DFCC
    // IDA_NAME: _ZN10expert_job10CEnchanter12OnTryEnchantEP5CUserP18MSG_STATIC_COMMAND
    // DEMANGLED: expert_job::CEnchanter::OnTryEnchant(
    //             CUser *,MSG_STATIC_COMMAND *)
    void OnTryEnchant(CUser* user, MSG_STATIC_COMMAND* command);

    // IDA_EA: 0x849E07E
    // IDA_NAME: _ZN10expert_job10CEnchanter14OnAgreeEnchantEP5CUserP18MSG_STATIC_COMMAND
    // DEMANGLED: expert_job::CEnchanter::OnAgreeEnchant(
    //             CUser *,MSG_STATIC_COMMAND *)
    char OnAgreeEnchant(CUser* user, MSG_STATIC_COMMAND* command);

private:
    int IsRightMaterial();
    bool IsRecipeRight();
    bool IsEqualMonsterCad();
    bool _DrawEnchant();
    int _DoEnchant(ENUM_TRY_ENCHANT_TYPE try_type);
    int _PutItemForEnchant(CUser* user,
                           int put_type,
                           stItemPos_t* item_position,
                           int item_id);
    int _TryEnchantMine(CUser* user, stItemPos_t* item_position);
    int _TryEnchantOther(CUser* user, stItemPos_t* item_position);
    int _TryEnchant(CUser* user,
                    stItemPos_t* item_position,
                    ENUM_TRY_ENCHANT_TYPE try_type);
    int _AgreeEnchant(CUser* user,
                      ENUM_AGREE_ENCHANT_TYPE agree_type);
    void SetPeerRequestType(ENUM_PEER_REQUEST_TYPE request_type);
    bool IsUseOtherUserMonsterCard();

    ExpertJobMaterial materials_[2];       // +0x08
    CUser* mission_user_;                  // +0x28
    CUser* peer_user_;                     // +0x2c
    uint32_t mission_unique_id_;           // +0x30
    uint32_t peer_unique_id_;              // +0x34
    int fee_;                              // +0x38
    CItem* enchant_item_;                  // +0x3c
    CStackableItem* recipe_item_;          // +0x40
    ENUM_PEER_REQUEST_TYPE peer_request_type_;  // +0x44
};

#if defined(__i386__)
static_assert(sizeof(CExpertJobMgr) == 0x04u,
              "CExpertJobMgr old ABI must stay one pointer wide");
static_assert(sizeof(CExpertJob) == 0x08u,
              "CExpertJob base ABI must stay 0x08");
static_assert(sizeof(ExpertJobMaterial) == 0x10u,
              "ExpertJobMaterial old ABI must stay 0x10");
static_assert(sizeof(CEnchanter) == 0x48u,
              "CEnchanter session ABI must stay 0x48");
#endif

}  // namespace expert_job

// IDA unit: 04_character_select_create/SkillSlot.  The object is embedded in
// the selected-character state; C owns this one declaration because character
// creation, USERINFO persistence and combo-skill packets all operate on the
// same live layout.  Its one-to-one bodies remain in skill_slot.cpp.
class SkillSlot {
public:
    SkillSlot();
    ~SkillSlot();
    SkillSlot(const SkillSlot& other);
    SkillSlot& operator=(const SkillSlot& other);

    SkillSlot* set_parent(CUser* user);
    void* set_skill_slot(char* src, char* src_1, int size);
    void* get_skill_slot(char* dest, char* dest_1, int size) const;
    void* set_skill_slot_at_index(char* src, int size,
                                  ENUM_SKILL_TREE_KIND kind);
    void* get_skill_slot_at_index(char* dest, int size,
                                  ENUM_SKILL_TREE_KIND kind) const;
    int set_remain_sp_at_index(int value, ENUM_SKILL_TREE_KIND kind);
    int get_remain_sp_at_index(ENUM_SKILL_TREE_KIND kind) const;
    int set_remain_sfp_at_index(int value, ENUM_SKILL_TREE_KIND kind);
    int get_remain_sfp_at_index(ENUM_SKILL_TREE_KIND kind) const;
    int get_skill_count() const;
    int FindSkill(int skill_index, ENUM_SKILL_TREE_KIND kind) const;
    int FindSkill(int skill_index) const;
    // IDA_EA: 0x86037C6
    // IDA_NAME: _ZN9SkillSlot22master_new_guild_skillEiiiiRK17STGuildDBInfoOnlyiRiS3_Rsi
    int master_new_guild_skill(int skill_index,
                               int charac_job,
                               int first_grow_type,
                               int second_grow_type,
                               const STGuildDBInfoOnly& guild_info,
                               int charac_money,
                               int& need_gold,
                               int& learned_level,
                               short& need_gsp,
                               int guild_skill_point);
    // IDA_EA: 0x8603A74
    // IDA_NAME: _ZN9SkillSlot16master_new_skillEiiiiii
    int master_new_skill(int skill_index,
                         int charac_level,
                         int charac_job,
                         int first_grow_type,
                         int second_grow_type,
                         int requested_level);
    int insert_skill(int skill_index, int group, bool active,
                     ENUM_SKILL_TREE_KIND kind, int level);
    // IDA_EA: 0x86043B4
    // IDA_NAME: _ZN9SkillSlot28insert_skill_with_slot_indexEiibi
    // DEMANGLED: SkillSlot::insert_skill_with_slot_index(int,int,bool,int)
    int insert_skill_with_slot_index(int slot_index,
                                     int unused_group,
                                     bool unused_active,
                                     int unused_level);
    // IDA_EA: 0x86040BC
    // IDA_NAME: _ZN9SkillSlot14growtype_skillEiii20ENUM_SKILL_TREE_KIND
    // DEMANGLED: SkillSlot::growtype_skill(
    //             int,int,int,ENUM_SKILL_TREE_KIND)
    int growtype_skill(int charac_job,
                       int skill_index,
                       int skill_level,
                       ENUM_SKILL_TREE_KIND kind);
    // IDA_EA: 0x86047D8
    // IDA_NAME: _ZN9SkillSlot12delete_skillEii20ENUM_SKILL_TREE_KIND
    // DEMANGLED: SkillSlot::delete_skill(int,int,ENUM_SKILL_TREE_KIND)
    // This exact named provider remains a link-debt boundary.  Its old body
    // writes the embedded skill-slot buffer, whose complete typed owner is
    // not restored in this C slice.
    int delete_skill(int charac_job,
                     int skill_index,
                     ENUM_SKILL_TREE_KIND kind);
    bool check_skill_mastered(int skill_index, int level, int group) const;
    bool check_skill_mastered(int skill_index,
                              int level,
                              int group,
                              ENUM_SKILL_TREE_KIND kind) const;
    // IDA_EA: 0x8604C02
    // IDA_NAME: _ZNK9SkillSlot16check_skill_listEiSt6vectorISt4pairIiiESaIS2_EEi
    // DEMANGLED: SkillSlot::check_skill_list(
    //             int,std::vector<std::pair<int,int>>,int)const
    int check_skill_list(int charac_job,
                         std::vector<std::pair<int, int> > required_skills,
                         int level_offset) const;
    int get_skillslot_group(int slot_index) const;
    int get_skillslot_no(int skill_index,
                         int group,
                         ENUM_SKILL_TREE_KIND kind,
                         bool quick_slot) const;
    // IDA_EA: 0x8607DBA
    // IDA_NAME: _ZNK9SkillSlot16get_skillslot_noEP15_Mastered_skilliib
    // DEMANGLED: SkillSlot::get_skillslot_no(_Mastered_skill *,int,int,bool)const
    int get_skillslot_no(_Mastered_skill* skill_slot,
                         int skill_index,
                         int group,
                         bool quick_slot) const;
    int clear_all_skills(ENUM_SKILL_TREE_KIND kind);
    int clear_all_skills_both();
    // IDA_EA: 0x8608120
    // IDA_NAME: _ZN9SkillSlot9InitSkillEb20ENUM_SKILL_TREE_KIND
    // DEMANGLED: SkillSlot::InitSkill(bool,ENUM_SKILL_TREE_KIND)
    int InitSkill(bool send_skill_info, ENUM_SKILL_TREE_KIND kind);
    // IDA_EA: 0x86085FA
    // IDA_NAME: _ZN9SkillSlot19InitSkillHistoryLogE20ENUM_SKILL_TREE_KINDi
    // DEMANGLED: SkillSlot::InitSkillHistoryLog(ENUM_SKILL_TREE_KIND,int)
    CUserCharacInfo* InitSkillHistoryLog(ENUM_SKILL_TREE_KIND kind,
                                         int reason);
    // IDA_EA: 0x8604E78
    // IDA_NAME: _ZN9SkillSlot16clear_sfp_skillsEc20ENUM_SKILL_TREE_KIND
    // DEMANGLED: SkillSlot::clear_sfp_skills(char,ENUM_SKILL_TREE_KIND)
    int clear_sfp_skills(char charac_job, ENUM_SKILL_TREE_KIND kind);
    int GetSkillTreeIndex() const;
    // Return type is not part of the Itanium C++ mangled identity.  IDA
    // proves that the old EAX result addresses one of the two embedded
    // _Mastered_skill[204] arrays, so expose that record type directly.
    const _Mastered_skill* get_skillslot_buf(
        ENUM_SKILL_TREE_KIND kind) const;
    // Owner-local typed projection of the two IDA-proven 0x198-byte
    // mastered-skill arrays at +0x46/+0x1DE.  This is not an old exported
    // identity; it keeps callers from reproducing SkillSlot offsets.
    _Mastered_skill* get_mastered_skill_tree(ENUM_SKILL_TREE_KIND kind);
    int* get_cur_remain_sp(ENUM_SKILL_TREE_KIND kind) const;
    int get_skill_index(int slot_index) const;
    int get_skill_level(int slot_index) const;
    // IDA_EA: 0x8604428
    // IDA_NAME: _ZN9SkillSlot10skill_moveEhhi
    // DEMANGLED: SkillSlot::skill_move(unsigned char,unsigned char,int)
    int skill_move(unsigned char source_slot,
                   unsigned char destination_slot,
                   int charac_job);
    int IsEmptySkillTree(ENUM_SKILL_TREE_KIND kind) const;
    int* get_temp_remain_sp();
    int* get_temp_remain_sfp();
    _Mastered_skill* get_temp_skill_slot();
    int backupSkill(ENUM_SKILL_TREE_KIND kind);
    // IDA_EA: 0x86069BA
    // IDA_NAME: _ZN9SkillSlot12refund_skillEiiiiiiRi
    // DEMANGLED: SkillSlot::refund_skill(int,int,int,int,int,int,int &)
    int refund_skill(int skill_index,
                     int charac_level,
                     int charac_job,
                     int first_grow_type,
                     int second_grow_type,
                     int requested_level,
                     int& reason);
    int try_master_new_skills(int skill_index,
                              int charac_level,
                              int charac_job,
                              int first_grow_type,
                              int second_grow_type,
                              int requested_level);
    int try_refund_skill(int skill_index,
                         int charac_level,
                         int charac_job,
                         int first_grow_type,
                         int second_grow_type,
                         int requested_level,
                         int& reason);
    bool check_skill_mastered(_Mastered_skill* skill_slot,
                              int skill_index,
                              int level,
                              int group) const;
    // IDA_EA: 0x86080A4 / 0x860714A.  These operate solely on the existing
    // temporary _Mastered_skill array used by the multi-buy preflight.
    int insert_skill(_Mastered_skill* skill_slot,
                     int skill_index,
                     int group,
                     bool active,
                     int level);
    int get_give_skill(int charac_job,
                       int first_grow_type,
                       int second_grow_type,
                       std::vector<std::pair<int, int> >& skills);
    int IsLoadStrikerSkill();
    SkillSlot* SetLoadStrikerSkill(bool loaded);
    void SendSkillInfo(int charac_idx, ENUM_SKILL_TREE_KIND kind);
    int checkskillSlotEmptyCount(int& kind);
    void loadComboSkill(SIG_COMBO_SKILL* data);
    CUser* getComboSkillSaveData(SIG_COMBO_SKILL* data) const;
    int getComboSkillSendInfo(PacketGuard& packet) const;
    int setComboSkillInfo(ENUM_SKILL_TREE_KIND kind,
                          ComboSkillMap& combo_map);
    bool checkMoveComboSkillSlot(unsigned char src_slot,
                                 unsigned char dst_slot);
    int resetComboSkill(ENUM_SKILL_TREE_KIND kind);
    bool checkComboSkillInsertQuickSlot(int skill_index) const;
    char pullComboSkillExtensionQuickSlot(ENUM_SKILL_TREE_KIND kind);
    bool moveComboSkillExtensionQuickSlot(int skill_index,
                                          int level,
                                          ENUM_SKILL_TREE_KIND kind);
    int eraseDuplicationComboSKill();
    char eraseDuplicationComboSKillTree(ENUM_SKILL_TREE_KIND kind);
    int getSkillSlotIndex(ENUM_SKILL_TREE_KIND kind, int slot_index) const;
    int verifyComboSkill();
    char verifyComboSkillTree(ENUM_SKILL_TREE_KIND kind);
    SkillSlot* set_remain_sp(int sp0, int sp1, int sfp0, int sfp1);
    int* get_remain_sp(int* sp0, int* sp1, int* sfp0, int* sfp1) const;

    // IDA_EA: 0x8605D2A
    // IDA_NAME: _ZN9SkillSlot19checkOverLevelSkillERSt6vectorI16stSPReturnInfo_tSaIS1_EE20ENUM_SKILL_TREE_KINDi
    // DEMANGLED: SkillSlot::checkOverLevelSkill(
    //     std::vector<stSPReturnInfo_t>&,ENUM_SKILL_TREE_KIND,int)
    int checkOverLevelSkill(std::vector<stSPReturnInfo_t>& return_info,
                            ENUM_SKILL_TREE_KIND kind,
                            int charac_level);

    // IDA_EA: 0x860658E
    // IDA_NAME: _ZNK9SkillSlot26calcReturnSpSfpPerOneSkillERiS0_iPK15_Mastered_skilliPK6CSkillb
    // DEMANGLED: SkillSlot::calcReturnSpSfpPerOneSkill(
    //     int&,int&,int,_Mastered_skill const*,int,CSkill const*,bool)const
    char calcReturnSpSfpPerOneSkill(int& returned_sp,
                                    int& returned_sfp,
                                    int maximum_level,
                                    const _Mastered_skill* skill_tree,
                                    int slot_index,
                                    const CSkill* skill,
                                    bool stealable) const;

    // IDA_EA: 0x86055EC
    // IDA_NAME: _ZNK9SkillSlot10calcUsedSPEPK15_Mastered_skillRiS3_
    // DEMANGLED: SkillSlot::calcUsedSP(
    //     _Mastered_skill const*,int&,int&)const
    void calcUsedSP(const _Mastered_skill* skill_tree,
                    int& used_sp,
                    int& used_sfp) const;

    // Verified dependencies of the CUser exact-96 business closure.
    int makeCheckSum(int seed) const;
    int debugCommandResetSkill();
    // IDA_EA: 0x86053C6
    // IDA_NAME: _ZN9SkillSlot23debugCheckGrowTypeSkillEiiii
    // DEMANGLED: SkillSlot::debugCheckGrowTypeSkill(int,int,int,int)
    void debugCheckGrowTypeSkill(int character_level,
                                 int character_job,
                                 int first_grow_type,
                                 int second_grow_type);
    int initialize_sfp_skill();

    // IDA physical object shared by C2 @0x8694D6A, copy C2 @0x86996F6 and
    // operator= @0x869D91E.  ComboSkill now owns its two real old-ABI rb trees
    // at +0/+0x15, so the complete 0x518 SkillSlot can be expressed with its
    // named subobjects instead of placement construction and offset views.
    CUser* parent_;                    // IDA +0x00
    int remain_sp_tree_0_;             // IDA +0x04
    bool load_striker_skill_;          // IDA +0x08
    int remain_sp_tree_1_;             // IDA +0x0c
    int selected_skill_tree_index_;    // IDA +0x10; C2 initializes to -1
    int remain_sfp_tree_0_;            // IDA +0x14
    int remain_sfp_tree_1_;            // IDA +0x18
    ComboSkill combo_skill_;            // IDA +0x1c, size 0x2a
    _Mastered_skill skill_tree_0_[204]; // IDA +0x46
    _Mastered_skill skill_tree_1_[204]; // IDA +0x1de
    _Mastered_skill temporary_tree_[204]; // IDA +0x376
    int temporary_remain_sp_;           // IDA +0x510
    int temporary_remain_sfp_;          // IDA +0x514
};

#if defined(__i386__)
static_assert(sizeof(CUser*) == 4u,
              "SkillSlot 0627 ABI requires 32-bit pointers");
static_assert(offsetof(SkillSlot, parent_) == 0x00u,
              "SkillSlot parent must stay at +0x00");
static_assert(offsetof(SkillSlot, remain_sp_tree_0_) == 0x04u,
              "SkillSlot tree-0 SP must stay at +0x04");
static_assert(offsetof(SkillSlot, load_striker_skill_) == 0x08u,
              "SkillSlot striker-load flag must stay at +0x08");
static_assert(offsetof(SkillSlot, remain_sp_tree_1_) == 0x0cu,
              "SkillSlot tree-1 SP must stay at +0x0c");
static_assert(offsetof(SkillSlot, selected_skill_tree_index_) == 0x10u,
              "SkillSlot selected-tree state must stay at +0x10");
static_assert(offsetof(SkillSlot, remain_sfp_tree_0_) == 0x14u,
              "SkillSlot tree-0 SFP must stay at +0x14");
static_assert(offsetof(SkillSlot, remain_sfp_tree_1_) == 0x18u,
              "SkillSlot tree-1 SFP must stay at +0x18");
static_assert(offsetof(SkillSlot, remain_sfp_tree_1_) +
                  sizeof(((SkillSlot*)0)->remain_sfp_tree_1_) == 0x1cu &&
                  offsetof(SkillSlot, combo_skill_) == 0x1cu,
              "SkillSlot ComboSkill member must start at +0x1c");
static_assert(offsetof(SkillSlot, skill_tree_0_) == 0x46u &&
                  offsetof(SkillSlot, skill_tree_1_) == 0x1deu &&
                  offsetof(SkillSlot, temporary_tree_) == 0x376u,
              "SkillSlot mastered-skill trees moved from old offsets");
static_assert(offsetof(SkillSlot, temporary_remain_sp_) == 0x510u &&
                  offsetof(SkillSlot, temporary_remain_sfp_) == 0x514u &&
                  sizeof(SkillSlot) == 0x518u,
              "SkillSlot complete 0627 ABI must stay 0x518");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_RUNTIME_H_
