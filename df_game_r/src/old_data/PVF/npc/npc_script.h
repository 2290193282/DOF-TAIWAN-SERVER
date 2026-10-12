#ifndef OLDSRV_OLD_DATA_PVF_NPC_NPC_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_NPC_NPC_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <ext/hash_map>

#include "common/config.h"
#include "old_data/PVF/npc/interactive_dialog_script.h"
#include "old_data/PVF/event/present_response_script.h"
#include "old_data/PVF/event/present_response_script_exception.h"
#include "old_data/PVF/event/present_response_script_event.h"
#include "old_data/PVF/common/script_file_list.h"
#include "old_data/PVF/npc/npc_common_data.h"

class CItem;
class PacketGuard;
struct Inven_Item;

// These opaque i386 enums are named in the original member ABI. Their values
// are parsed from NPC script data, so callers convert to an index only at the
// proven array boundary.
enum ENUM_NPC_MOOD : int;
enum ENUM_NPC_FAVOR_CHANGE_CONST : int;
enum ENUM_NPC_FAVOR_GIFT_RESULT_CONST : int;
enum ENUM_NPC_ROLE : int;

// B-owned typed PVF/RDAR NPC configuration and parser ABI. Runtime list
// bodies that still require untyped CDataManager slots remain outside this
// header owner until their shared layout is recovered.
struct ClickMarkInfo {
    int32_t type;
    int32_t value;
};

struct randomMessageBalloonInfo {
    randomMessageBalloonInfo();
    ~randomMessageBalloonInfo();

    void clear();

    std::string animation;
    int32_t probability;
    std::vector<std::string> messages;
};

struct NpcMovieData {
    NpcMovieData();
    ~NpcMovieData();

    NpcMovieData* clear();

    // IDA_EA: 0x89FB7EE
    // IDA_NAME: _ZN12NpcMovieData16getNextMoviePathEv
    // DEMANGLED: NpcMovieData::getNextMoviePath(void)
    std::string getNextMoviePath();

    std::deque<std::string> movie_paths;
    int32_t next_movie_index;
};

struct stFavorableRelationShip_t {
    struct stGiftItemInfo_t {
        uint16_t required_count;
        int16_t favor_value;
        int32_t value_04;
    };

    struct stRewardItemElement_t {
        // IDA_EA: 0x8A06F28
        // IDA_NAME: _ZN25stFavorableRelationShip_t19stRewardItemElement_tD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stRewardItemElement_t::~stRewardItemElement_t()
        ~stRewardItemElement_t();

        int32_t item_index;
        int32_t item_count;
        int32_t accumulated_rate;
    };

    struct stRewardItemInfo_t {
        // IDA_EA: 0x8A00D58
        // IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_tC2Ev
        // DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::stRewardItemInfo_t(void)
        stRewardItemInfo_t();

        // IDA_EA: 0x8581A80
        // IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_tD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::~stRewardItemInfo_t()
        ~stRewardItemInfo_t();

        // IDA_EA: 0x89FB4C0
        // IDA_NAME: _ZN25stFavorableRelationShip_t18stRewardItemInfo_t5clearEv
        // DEMANGLED: stFavorableRelationShip_t::stRewardItemInfo_t::clear(void)
        void clear();

        std::vector<stRewardItemElement_t> reward_items[3];
        int32_t total_rates[3];
    };

    // The original stFavorableRelationShip_t stores this 5 x 3 aggregate at
    // +0x208.  Keep it as a typed wrapper rather than a bare array so its
    // C2/D2/clear lifecycle is addressable exactly as in the old ELF.  The
    // subscript helpers preserve the existing reward_items[mood][favor]
    // access syntax without changing the physical 0x2d0 layout.
    struct stRewardItem_t {
        // IDA_EA: 0x8A00DCC
        // IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_tC2Ev
        // DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::stRewardItem_t(void)
        stRewardItem_t();

        // IDA_EA: 0x8581AAE
        // IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_tD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::~stRewardItem_t()
        ~stRewardItem_t();

        // IDA_EA: 0x89FB508
        // IDA_NAME: _ZN25stFavorableRelationShip_t14stRewardItem_t5clearEv
        // DEMANGLED: stFavorableRelationShip_t::stRewardItem_t::clear(void)
        void clear();

        stRewardItemInfo_t* operator[](int mood) {
            return reward_items[mood];
        }
        const stRewardItemInfo_t* operator[](int mood) const {
            return reward_items[mood];
        }

        stRewardItemInfo_t reward_items[5][3];
    };

    struct stLevelRewardInfo {
        // IDA_EA: 0x858188A
        // IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoC2Ev
        // DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo(void)
        stLevelRewardInfo();

        // IDA_EA: 0x85823C6
        // IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::~stLevelRewardInfo()
        ~stLevelRewardInfo();

        // IDA_EA: 0x8A04160
        // IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoC2ERKS0_
        // DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::stLevelRewardInfo(stFavorableRelationShip_t::stLevelRewardInfo const&)
        stLevelRewardInfo(const stLevelRewardInfo& rhs);

        // IDA_EA: 0x8A04268
        // IDA_NAME: _ZN25stFavorableRelationShip_t17stLevelRewardInfoaSERKS0_
        // DEMANGLED: stFavorableRelationShip_t::stLevelRewardInfo::operator=(stFavorableRelationShip_t::stLevelRewardInfo const&)
        stLevelRewardInfo& operator=(const stLevelRewardInfo& rhs);

        uint16_t favor_level;
        uint16_t reserved_02;
        int32_t item_index;
        int32_t item_count;
        std::string title;
        std::string text;
    };

    struct stBuffRewardInfo {
        // IDA_EA: 0x85818F0
        // IDA_NAME: _ZN25stFavorableRelationShip_t16stBuffRewardInfoC2Ev
        // DEMANGLED: stFavorableRelationShip_t::stBuffRewardInfo::stBuffRewardInfo(void)
        stBuffRewardInfo();

        // IDA_EA: 0x8A073D4
        // IDA_NAME: _ZN25stFavorableRelationShip_t16stBuffRewardInfoD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stBuffRewardInfo::~stBuffRewardInfo()
        ~stBuffRewardInfo();

        uint16_t favor_level;
        uint16_t buff_type;
        int32_t probability;
    };

    struct stPeriodRewardInfo {
        // IDA_EA: 0x8A00C70
        // IDA_NAME: _ZN25stFavorableRelationShip_t18stPeriodRewardInfoC2Ev
        // DEMANGLED: stFavorableRelationShip_t::stPeriodRewardInfo::stPeriodRewardInfo(void)
        stPeriodRewardInfo();

        // IDA_EA: 0x85819E4
        // IDA_NAME: _ZN25stFavorableRelationShip_t18stPeriodRewardInfoD2Ev
        // DEMANGLED: stFavorableRelationShip_t::stPeriodRewardInfo::~stPeriodRewardInfo()
        ~stPeriodRewardInfo();

        uint16_t reward_type;
        uint16_t reserved_02;
        std::string item_or_quest;
        int32_t value_08;
        int32_t value_0c;
        std::string title;
        std::string text;
    };

    stFavorableRelationShip_t();
    ~stFavorableRelationShip_t();

    void clear();

    uint8_t enabled;
    uint8_t reserved_01[3];
    int32_t default_favor;
    uint16_t max_gift_per_day;
    uint16_t dialog_day_not_continuously;
    uint16_t value_0c;
    uint16_t reserved_0e;
    std::vector<std::pair<int, uint32_t> > mood_changes;
    std::vector<std::pair<uint32_t, uint32_t> > key_items;
    std::map<uint32_t, stGiftItemInfo_t> prefer_items[5];
    std::map<uint32_t, stGiftItemInfo_t> unprefer_items[5];
    std::map<uint32_t, stGiftItemInfo_t> prefer_item_groups[5];
    std::map<uint32_t, stGiftItemInfo_t> unprefer_item_groups[5];
    stRewardItem_t reward_items;
    float favor_rate_per_mood[3];
    std::vector<stLevelRewardInfo> level_rewards;
    std::vector<stBuffRewardInfo> buff_rewards;
    stPeriodRewardInfo period_reward;
};

struct NpcScript {
    NpcScript();
    ~NpcScript();

    NpcScript* clear();

    int32_t index;
    std::string name;
    std::string field_name;
    std::string small_face;
    int32_t small_face_index;
    std::string big_face;
    int32_t big_face_index;
    std::string popup_face;
    int32_t popup_face_index;
    std::string field_animation;
    std::string field_wav;
    std::string popup_wavs[2];
    uint8_t look_around;
    uint8_t reserved_35[3];
    int32_t value_38;
    std::string value_3c;
    std::vector<int> value_40;
    std::map<int, int> roles;
    std::string quest_speech;
    std::vector<int>
        skills_by_job[oldsrv::config::kCharacterJobWithCommonCount];
    std::string values_f8[3];
    std::vector<std::string> string_data;
    std::vector<int> int_data;
    std::vector<std::string> balloon_messages;
    std::vector<int> balloon_message_values;
    std::vector<InteractiveDialogScript> dialogs;
    std::vector<PresentResponceScript> favor_dialogs;
    std::vector<std::string> first_favor_dialogs;
    std::vector<PresentResponceScriptException> favor_dialog_exceptions;
    std::vector<PresentResponceScriptEvent> event_favor_dialogs;
    stFavorableRelationShip_t favorable_relation;
    std::string favor_duration_reward_buff_dialog;
    uint8_t value_688;
    uint8_t reserved_689[3];
    std::vector<std::string> web_pages;
    std::vector<ClickMarkInfo> click_marks;
    randomMessageBalloonInfo random_message_balloon;
    uint8_t hidden;
    uint8_t reserved_6b9[3];
    NpcMovieData movie_data;
};

// The 0828 client PVF adds presentation/direct-action records which are not
// part of the recovered 0x6f4 native NpcScript layout.  Keep the exact typed
// values in an owner-keyed sidecar so the ABI remains unchanged while
// runtime/UI bridges can query the imported data without rescanning PVF.
namespace npc_script_0828 {

struct FieldRole {
    FieldRole();

    bool has_name;
    std::string name;
    bool has_image;
    std::string image;
    int32_t image_index;
};

struct ClickArea {
    ClickArea();

    int32_t values[8];
};

struct ClickAnimationAction {
    std::string action;
    int32_t value;
};

struct ClickAnimation {
    std::string animation;
    std::vector<ClickAnimationAction> actions;
};

struct Extension {
    Extension();

    bool has_field_role;
    FieldRole field_role;
    bool has_click_area;
    ClickArea click_area;
    bool has_role_direct_action;
    int32_t role_direct_action;
    bool has_click_animation;
    ClickAnimation click_animation;
    bool has_sub_field_animation;
    std::string sub_field_animation;
};

bool Find(const NpcScript* script, Extension* output);

}  // namespace npc_script_0828

class CNPCScript {
public:
    CNPCScript();
    ~CNPCScript();

    int load(const char* filename);
    unsigned int getIndex() const;
    int isFavorableNPC() const;
    bool get0828Extension(npc_script_0828::Extension* output) const;

    // IDA_EA: 0x85809E4
    // IDA_NAME: _ZNK10CNPCScript12giveGiftItemEjPK5CItemjR32ENUM_NPC_FAVOR_GIFT_RESULT_CONST
    // DEMANGLED: CNPCScript::giveGiftItem(unsigned int,CItem const*,unsigned int,ENUM_NPC_FAVOR_GIFT_RESULT_CONST &)const
    int giveGiftItem(unsigned int mood,
                     const CItem* item,
                     unsigned int item_count,
                     ENUM_NPC_FAVOR_GIFT_RESULT_CONST& result_const) const;

    // IDA_EA: 0x8580D30
    // IDA_NAME: _ZNK10CNPCScript17getGiftRewardItemE13ENUM_NPC_MOOD27ENUM_NPC_FAVOR_CHANGE_CONSTj
    // DEMANGLED: CNPCScript::getGiftRewardItem(ENUM_NPC_MOOD,ENUM_NPC_FAVOR_CHANGE_CONST,unsigned int)const
    Inven_Item getGiftRewardItem(ENUM_NPC_MOOD mood,
                                 ENUM_NPC_FAVOR_CHANGE_CONST favor_change,
                                 unsigned int favor_level) const;

    // IDA_EA: 0x8580E6A
    // IDA_NAME: _ZNK10CNPCScript9isKeyItemEmj
    // DEMANGLED: CNPCScript::isKeyItem(unsigned long,unsigned int)const
    int isKeyItem(unsigned long item_index, unsigned int item_count) const;

    // IDA_EA: 0x8580F28
    // IDA_NAME: _ZNK10CNPCScript11getNextMoodER13ENUM_NPC_MOODRj
    // DEMANGLED: CNPCScript::getNextMood(ENUM_NPC_MOOD &,unsigned int &)const
    int getNextMood(ENUM_NPC_MOOD& mood, unsigned int& mood_position) const;

    // IDA_EA: 0x8580FD0
    // IDA_NAME: _ZNK10CNPCScript13isRewardLevelEt
    // DEMANGLED: CNPCScript::isRewardLevel(unsigned short)const
    int isRewardLevel(unsigned short favor_level) const;

    // IDA_EA: 0x8581076
    // IDA_NAME: _ZNK10CNPCScript18getLevelRewardInfoEt
    // DEMANGLED: CNPCScript::getLevelRewardInfo(unsigned short)const
    stFavorableRelationShip_t::stLevelRewardInfo
    getLevelRewardInfo(unsigned short favor_level) const;

    // IDA_EA: 0x85811A4
    // IDA_NAME: _ZNK10CNPCScript19getPeriodRewardTypeEv
    // DEMANGLED: CNPCScript::getPeriodRewardType(void)const
    int getPeriodRewardType() const;

    // IDA_EA: 0x85811F8
    // IDA_NAME: _ZNK10CNPCScript17getBuffRewardInfoEi
    // DEMANGLED: CNPCScript::getBuffRewardInfo(int)const
    stFavorableRelationShip_t::stBuffRewardInfo
    getBuffRewardInfo(int favor_value) const;

    // IDA_EA: 0x85819C6
    // IDA_NAME: _ZNK10CNPCScript19getFavorRatePerMoodE13ENUM_NPC_MOOD
    // DEMANGLED: CNPCScript::getFavorRatePerMood(ENUM_NPC_MOOD)const
    float getFavorRatePerMood(ENUM_NPC_MOOD mood) const;

    // IDA_EA: 0x86D2736
    // IDA_NAME: _ZNK10CNPCScript11isExistRoleE13ENUM_NPC_ROLE
    // DEMANGLED: CNPCScript::isExistRole(ENUM_NPC_ROLE)const
    int isExistRole(ENUM_NPC_ROLE role) const;

    int32_t index;
    NpcScript script;
};

class CNPCDynamicInfo {
public:
    explicit CNPCDynamicInfo(const CNPCScript* script);

    // IDA_EA: 0x85804B2
    // IDA_NAME: _ZN15CNPCDynamicInfo10changeMoodEb
    // DEMANGLED: CNPCDynamicInfo::changeMood(bool)
    int changeMood(bool force_change);

    // IDA_EA: 0x858053E
    // IDA_NAME: _ZN15CNPCDynamicInfo15forceChangeMoodE13ENUM_NPC_MOOD
    // DEMANGLED: CNPCDynamicInfo::forceChangeMood(ENUM_NPC_MOOD)
    int forceChangeMood(ENUM_NPC_MOOD target_mood);

    // IDA_EA: 0x8580584
    // IDA_NAME: _ZNK15CNPCDynamicInfo9isKeyItemEmj
    // DEMANGLED: CNPCDynamicInfo::isKeyItem(unsigned long,unsigned int)const
    int isKeyItem(unsigned long item_index, unsigned int item_count) const;

    // IDA_EA: 0x85805A8
    // IDA_NAME: _ZNK15CNPCDynamicInfo12giveGiftItemEjmjPK5CItemR32ENUM_NPC_FAVOR_GIFT_RESULT_CONST
    // DEMANGLED: CNPCDynamicInfo::giveGiftItem(unsigned int,unsigned long,unsigned int,CItem const*,ENUM_NPC_FAVOR_GIFT_RESULT_CONST &)const
    int giveGiftItem(unsigned int mood,
                     unsigned long item_index,
                     unsigned int item_count,
                     const CItem* item,
                     ENUM_NPC_FAVOR_GIFT_RESULT_CONST& result_const) const;

    // IDA_EA: 0x8580624
    // IDA_NAME: _ZNK15CNPCDynamicInfo21makeNotiPacketCurMoodER11PacketGuard
    // DEMANGLED: CNPCDynamicInfo::makeNotiPacketCurMood(PacketGuard &)const
    int makeNotiPacketCurMood(PacketGuard& packet) const;

    // IDA_EA: 0x8581910
    // IDA_NAME: _ZNK15CNPCDynamicInfo7getMoodEv
    // DEMANGLED: CNPCDynamicInfo::getMood(void)const
    int getMood() const;

    const CNPCScript* script;
    int32_t mood;
    int32_t mood_position;
    int32_t next_change_second;
};

class CNPCScriptList {
public:
    typedef __gnu_cxx::hash_map<unsigned int, CNPCScript*> ScriptMap;

    CNPCScriptList();
    ~CNPCScriptList();

    int initScript(char* src, char* list_file, char* common_file);
    CNPCScript* find(unsigned int index) const;
    int getFavorLevel(unsigned short value) const;
    int getFavorLevelValue(unsigned int level) const;
    int _destroy();
    int getMaxFavorValue() const;

private:
    ScriptMap scripts_;
    stNPCCommonData_t common_;
};

class CNPCDynamicInfoManager {
public:
    typedef std::map<unsigned int, CNPCDynamicInfo*> InfoMap;

    CNPCDynamicInfoManager();
    ~CNPCDynamicInfoManager();

    int add(const CNPCScript* script);
    CNPCDynamicInfo* getNPCInfo(unsigned int index);

    // IDA_EA: 0x8580722
    // IDA_NAME: _ZN22CNPCDynamicInfoManager7onTimerEv
    // DEMANGLED: CNPCDynamicInfoManager::onTimer(void)
    void onTimer();

    // IDA_EA: 0x85808A6
    // IDA_NAME: _ZNK22CNPCDynamicInfoManager21makeNotiPacketNPCMoodER11PacketGuard
    // DEMANGLED: CNPCDynamicInfoManager::makeNotiPacketNPCMood(PacketGuard &)const
    int makeNotiPacketNPCMood(PacketGuard& packet) const;

private:
    InfoMap infos_;
};

extern STScriptFileList g_npcScriptFileList;

int initNpcScript(const char* src, const char* filename);
int isExistNPCScript(const char* filename);
int getNpcMoodFromString(const char* text);
int importNpcScript(NpcScript* script, const char* filename)
    __asm__("_Z15importNpcScriptP9NpcScriptPKc");
int getNpcRoleByString(std::string text)
    __asm__("_Z18getNpcRoleByStringSs");

static_assert(sizeof(ClickMarkInfo) == 0x08u,
              "ClickMarkInfo IDA ABI size must stay 0x08");
static_assert(sizeof(InteractiveDialogScript) == 0x34u,
              "InteractiveDialogScript IDA ABI size must stay 0x34");
static_assert(sizeof(PresentResponceScript) == 0x10u,
              "PresentResponceScript IDA ABI size must stay 0x10");
static_assert(sizeof(PresentResponceScriptException) == 0x08u,
              "PresentResponceScriptException IDA ABI size must stay 0x08");
static_assert(sizeof(PresentResponceScriptEvent) == 0x08u,
              "PresentResponceScriptEvent IDA ABI size must stay 0x08");
static_assert(sizeof(randomMessageBalloonInfo) == 0x14u,
              "randomMessageBalloonInfo IDA ABI size must stay 0x14");
static_assert(sizeof(NpcMovieData) == 0x2Cu,
              "NpcMovieData IDA ABI size must stay 0x2C");
static_assert(sizeof(stFavorableRelationShip_t::stBuffRewardInfo) == 0x08u,
              "stBuffRewardInfo IDA ABI size must stay 0x08");
static_assert(sizeof(stFavorableRelationShip_t::stRewardItemElement_t) ==
                  0x0Cu,
              "stRewardItemElement_t IDA ABI size must stay 0x0c");
static_assert(sizeof(stFavorableRelationShip_t::stRewardItemInfo_t) ==
                  0x30u,
              "stRewardItemInfo_t IDA ABI size must stay 0x30");
static_assert(offsetof(stFavorableRelationShip_t::stRewardItemInfo_t,
                       total_rates) == 0x24u,
              "stRewardItemInfo_t rates must stay at +0x24");
static_assert(sizeof(stFavorableRelationShip_t::stRewardItem_t) == 0x2D0u,
              "stRewardItem_t IDA ABI size must stay 0x2d0");
static_assert(sizeof(stFavorableRelationShip_t::stLevelRewardInfo) ==
                  0x14u,
              "stLevelRewardInfo IDA ABI size must stay 0x14");
static_assert(offsetof(stFavorableRelationShip_t,
                       reward_items) == 0x208u,
              "stRewardItem_t must remain at stFavorableRelationShip_t +0x208");
static_assert(offsetof(stFavorableRelationShip_t,
                       level_rewards) == 0x4E4u,
              "level rewards must remain at stFavorableRelationShip_t +0x4e4");
static_assert(sizeof(stFavorableRelationShip_t::stPeriodRewardInfo) == 0x18u,
              "stPeriodRewardInfo IDA ABI size must stay 0x18");
static_assert(sizeof(stFavorableRelationShip_t) == 0x514u,
              "stFavorableRelationShip_t IDA ABI size must stay 0x514");
// PVF_EVIDENCE: the target PVF has concrete job 11 and still uses the
// distinct [common] skill bucket at sentinel 12.  The thirteenth i386 vector
// expands both native owner layouts by 0x0c from their restored 0627 sizes.
static_assert(sizeof(NpcScript) == 0x6F4u,
              "NpcScript 0828 native size must stay 0x6F4");
static_assert(sizeof(CNPCScript) == 0x6F8u,
              "CNPCScript 0828 native size must stay 0x6F8");
static_assert(sizeof(CNPCDynamicInfo) == 0x10u,
              "CNPCDynamicInfo IDA ABI size must stay 0x10");
static_assert(sizeof(stNPCCommonData_t) == 0x50u,
              "stNPCCommonData_t IDA ABI size must stay 0x50");
static_assert(sizeof(CNPCScriptList) == 0x64u,
              "CNPCScriptList IDA ABI size must stay 0x64");
static_assert(sizeof(CNPCDynamicInfoManager) == 0x18u,
              "CNPCDynamicInfoManager IDA ABI size must stay 0x18");

#endif  // OLDSRV_OLD_DATA_PVF_NPC_NPC_SCRIPT_H_
