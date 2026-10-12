#include "old_data/PVF/npc/datamanager/npc_script.h"
#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "common/manager/change_status_data.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mall/npc_shop_timer_runtime.h"

#include <algorithm>
#include <map>
#include <new>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

STScriptFileList g_npcScriptFileList __asm__("g_npcScriptFileList");

namespace npc_script_0828 {
namespace {

typedef std::map<const NpcScript*, Extension> ExtensionMap;

ExtensionMap& Extensions() {
    static ExtensionMap extensions;
    return extensions;
}

Extension& Mutable(const NpcScript* script) {
    return Extensions()[script];
}

void Erase(const NpcScript* script) {
    Extensions().erase(script);
}

}  // namespace

FieldRole::FieldRole()
    : has_name(false),
      name(),
      has_image(false),
      image(),
      image_index(0) {}

ClickArea::ClickArea() : values() {}

Extension::Extension()
    : has_field_role(false),
      field_role(),
      has_click_area(false),
      click_area(),
      has_role_direct_action(false),
      role_direct_action(0),
      has_click_animation(false),
      click_animation(),
      has_sub_field_animation(false),
      sub_field_animation() {}

bool Find(const NpcScript* script, Extension* output) {
    if (!script || !output) {
        return false;
    }
    const ExtensionMap::const_iterator it = Extensions().find(script);
    if (it == Extensions().end()) {
        return false;
    }
    *output = it->second;
    return true;
}

}  // namespace npc_script_0828

// IDA_EA: 0x89FAE70
// IDA_NAME: _Z16isExistNPCScriptPKc
// DEMANGLED: isExistNPCScript(char const*)
int isExistNPCScript(const char* filename) {
    return loadRDARScriptFile(g_npcScriptBaseDirectory, filename) ? 1
                                                                         : 0;
}

// IDA_EA: 0x89FAF46
// IDA_NAME: _Z20getNpcMoodFromStringPKc
// DEMANGLED: getNpcMoodFromString(char const*)
int getNpcMoodFromString(const char* text) {
    if (strncmp("good", text, 10u) == 0) {
        return 2;
    }
    if (strncmp("normal", text, 10u) == 0) {
        return 1;
    }
    if (strncmp("bad", text, 10u) == 0) {
        return 0;
    }
    return 3;
}

// IDA_EA: 0x89FECEE
// IDA_NAME: _Z18getNpcRoleByStringSs
// DEMANGLED: getNpcRoleByString(std::string)
int getNpcRoleByString(std::string text) {
    if (text == "[item shop]") return 0;
    if (text == "[recover stamina]") return 1;
    if (text == "[sera shop]") return 2;
    if (text == "[guild]") return 3;
    if (text == "[upgrade item]") return 4;
    if (text == "[upgrade item event]") return 5;
    if (text == "[disjoint item]") return 6;
    if (text == "[product item]") return 7;
    if (text == "[power war]") return 8;
    if (text == "[move town]") return 9;
    if (text == "[item storage]") return 10;
    if (text == "[post box]") return 11;
    if (text == "[coke vending machine]") return 12;
    if (text == "[mouse register]") return 13;
    if (text == "[security card make]") return 14;
    if (text == "[security card remake]") return 15;
    if (text == "[security card delete]") return 16;
    if (text == "[automat]") return 17;
    if (text == "[just a cake]") return 18;
    if (text == "[convert avatar]") return 19;
    if (text == "[compound emblem]") return 20;
    if (text == "[amplify item]") return 21;
    if (text == "[coin vending machine]") return 22;
    if (text == "[event message board]") return 23;
    if (text == "[donate power war point]") return 24;
    if (text == "[create guild agit]") return 25;
    if (text == "[delete guild agit]") return 26;
    if (text == "[enter guild agit]") return 27;
    if (text == "[exit guild agit]") return 28;
    if (text == "[upgrade guild agit]") return 29;
    if (text == "[housing guild agit]") return 30;
    if (text == "[guild storage]") return 31;
    if (text == "[powerwar statue]") return 32;
    if (text == "[disjoint repair]") return 33;
    if (text == "[disjoint upgrade]") return 34;
    if (text == "[job change]") return 35;
    if (text == "[cera shop inventory]") return 36;
    if (text == "[random option operate]") return 37;
    if (text == "[sudden shop]") return 38;
    if (text == "[repair item]") return 39;
    if (text == "[SANSENBAKO]") return 40;
    if (text == "[contest reception]") return 41;
    if (text == "[genuine damage upgrade]") return 42;
    if (text == "[event halloween witch]") return 43;
    if (text == "[event halloween caldron]") return 44;
    if (text == "[image communication]") return 45;
    if (text == "[tutorial dungeon lvup]") return 46;
    if (text == "[crane minigame]") return 47;
    if (text == "[time sand]") return 48;
    if (text == "[play quest movie]") return 49;
    if (text == "[blue marble]") return 50;
    if (text == "[play bead movie]") return 51;
    if (text == "[secret shop]") return 52;
    if (text == "[convert avatar jpn]") return 53;
    if (text == "[compound emblem jpn]") return 54;
    if (text == "[one a day shop]") return 55;
    return -1;
}

// IDA_EA: 0x89FC1A8
// IDA_NAME: _Z13initNpcScriptPKcS0_
// DEMANGLED: initNpcScript(char const*, char const*)
int initNpcScript(const char* src, const char* filename) {
    strncpy(g_npcScriptBaseDirectory, src, 0x100u);
    g_npcScriptBaseDirectory[0x100] = '\0';
    const size_t length = strlen(g_npcScriptBaseDirectory);
    if (length != 0 && g_npcScriptBaseDirectory[length - 1] != '\\' &&
        g_npcScriptBaseDirectory[length - 1] != '/') {
        strncat(g_npcScriptBaseDirectory, "\\", 0x100u - length);
    }
    return loadRDARScriptListFile(
               &g_npcScriptFileList,
               g_npcScriptBaseDirectory,
               filename) == 1
               ? 1
               : 0;
}

// IDA_EA: 0x8376262
// IDA_NAME: _ZN14CNPCScriptListC2Ev
// DEMANGLED: CNPCScriptList::CNPCScriptList(void)
CNPCScriptList::CNPCScriptList() : scripts_(), common_() {}

// IDA_EA: 0x83762A8
// IDA_NAME: _ZN14CNPCScriptListD2Ev
// DEMANGLED: CNPCScriptList::~CNPCScriptList()
CNPCScriptList::~CNPCScriptList() {
    _destroy();
}

// IDA_EA: 0x8581314
// IDA_NAME: _ZN14CNPCScriptList10initScriptEPcS0_S0_
// DEMANGLED: CNPCScriptList::initScript(char *, char *, char *)
int CNPCScriptList::initScript(char* src,
                               char* list_file,
                               char* common_file) {
    if (initNpcScript(src, list_file) != 1) {
        return 417;
    }

    for (STScriptFileList::iterator it = g_npcScriptFileList.begin();
         it != g_npcScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (isExistNPCScript(filename) != 1) {
            cMyTrace trace(
                "int CNPCScriptList::initScript(NCHAR*, NCHAR*, NCHAR*)",
                425,
                5);
            trace("%s file not exists\n", filename);
            continue;
        }

        CNPCScript* script =
            static_cast<CNPCScript*>(::operator new(sizeof(CNPCScript)));
        new (script) CNPCScript;
        if (script->load(filename) != 1) {
            cMyTrace trace(
                "int CNPCScriptList::initScript(NCHAR*, NCHAR*, NCHAR*)",
                432,
                5);
            trace("%s load failed\n", filename);
            script->~CNPCScript();
            ::operator delete(script);
            return 436;
        }

        scripts_.insert(std::make_pair(script->getIndex(), script));
        if (script->isFavorableNPC()) {
            if (CNPCDynamicInfoManager* manager =
                    G_CDataManager()->get_npc_dynamic_info_manager()) {
                manager->add(script);
            }
        }
    }

    if (loadNPCCommonData(common_, common_file) != 0) {
        cMyTrace trace(
            "int CNPCScriptList::initScript(NCHAR*, NCHAR*, NCHAR*)",
            446,
            5);
        trace("%s load failed\n", common_file);
        return 447;
    }

    if (common_.favorable_npc_indices.size() <= 0x32u) {
        TimerNPCMoodChange::registNextTimer(0);
        return 0;
    }

    cMyTrace trace(
        "int CNPCScriptList::initScript(NCHAR*, NCHAR*, NCHAR*)",
        451,
        5);
    trace("%s Favorable npc count over", common_file);
    return 452;
}

// IDA_EA: 0x8581680
// IDA_NAME: _ZNK14CNPCScriptList4findEj
// DEMANGLED: CNPCScriptList::find(unsigned int) const
CNPCScript* CNPCScriptList::find(unsigned int index) const {
    ScriptMap::const_iterator it = scripts_.find(index);
    if (it == scripts_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x85816E4
// IDA_NAME: _ZNK14CNPCScriptList13getFavorLevelEt
// DEMANGLED: CNPCScriptList::getFavorLevel(unsigned short) const
int CNPCScriptList::getFavorLevel(unsigned short value) const {
    if (value >= getMaxFavorValue()) {
        return 4;
    }
    for (int level = 1; level <= 3; ++level) {
        if (common_.favor_level_values[level] >= value) {
            return level;
        }
    }
    return 4;
}

// IDA_EA: 0x858174E
// IDA_NAME: _ZNK14CNPCScriptList18getFavorLevelValueEj
// DEMANGLED: CNPCScriptList::getFavorLevelValue(unsigned int) const
int CNPCScriptList::getFavorLevelValue(unsigned int level) const {
    if (level > 3) {
        return getMaxFavorValue();
    }
    if (level == 0) {
        return 0;
    }
    return common_.favor_level_values[level];
}

// IDA_EA: 0x8581784
// IDA_NAME: _ZN14CNPCScriptList8_destroyEv
// DEMANGLED: CNPCScriptList::_destroy(void)
int CNPCScriptList::_destroy() {
    for (ScriptMap::iterator it = scripts_.begin(); it != scripts_.end();
         ++it) {
        if (it->second != 0) {
            it->second->~CNPCScript();
            ::operator delete(it->second);
        }
    }
    scripts_.clear();
    return 0;
}

// IDA_EA: 0x8582364
// IDA_NAME: _ZNK14CNPCScriptList16getMaxFavorValueEv
// DEMANGLED: CNPCScriptList::getMaxFavorValue(void) const
int CNPCScriptList::getMaxFavorValue() const {
    return common_.max_favor_value;
}

// IDA_EA: 0x83760F2
// IDA_NAME: _ZN22CNPCDynamicInfoManagerC2Ev
// DEMANGLED: CNPCDynamicInfoManager::CNPCDynamicInfoManager(void)
CNPCDynamicInfoManager::CNPCDynamicInfoManager() : infos_() {}

// IDA_EA: 0x8376106
// IDA_NAME: _ZN22CNPCDynamicInfoManagerD2Ev
// DEMANGLED: CNPCDynamicInfoManager::~CNPCDynamicInfoManager()
CNPCDynamicInfoManager::~CNPCDynamicInfoManager() {}

// IDA_EA: 0x8580668
// IDA_NAME: _ZN22CNPCDynamicInfoManager3addEPK10CNPCScript
// DEMANGLED: CNPCDynamicInfoManager::add(CNPCScript const*)
int CNPCDynamicInfoManager::add(const CNPCScript* script) {
    const unsigned int index = script->getIndex();
    if (getNPCInfo(index) != 0) {
        return 0;
    }
    CNPCDynamicInfo* info =
        static_cast<CNPCDynamicInfo*>(::operator new(0x10u));
    new (info) CNPCDynamicInfo(script);
    infos_.insert(std::make_pair(index, info));
    return 1;
}

// IDA_EA: 0x8581948
// IDA_NAME: _ZN22CNPCDynamicInfoManager10getNPCInfoEj
// DEMANGLED: CNPCDynamicInfoManager::getNPCInfo(unsigned int)
CNPCDynamicInfo* CNPCDynamicInfoManager::getNPCInfo(unsigned int index) {
    InfoMap::iterator it = infos_.find(index);
    if (it == infos_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8A00C08
// IDA_NAME: _ZN24randomMessageBalloonInfoC2Ev
// DEMANGLED: randomMessageBalloonInfo::randomMessageBalloonInfo(void)
randomMessageBalloonInfo::randomMessageBalloonInfo()
    : animation(), probability(0), messages() {
    clear();
}

// IDA_EA: 0x8581D50
// IDA_NAME: _ZN24randomMessageBalloonInfoD2Ev
// DEMANGLED: randomMessageBalloonInfo::~randomMessageBalloonInfo()
randomMessageBalloonInfo::~randomMessageBalloonInfo() {}

// IDA_EA: 0x8A00BD2
// IDA_NAME: _ZN24randomMessageBalloonInfo5clearEv
// DEMANGLED: randomMessageBalloonInfo::clear(void)
void randomMessageBalloonInfo::clear() {
    animation.clear();
    probability = 0;
    messages.clear();
    messages.reserve(10);
}

// IDA_EA: 0x89FB78E
// IDA_NAME: _ZN12NpcMovieDataC2Ev
// DEMANGLED: NpcMovieData::NpcMovieData(void)
NpcMovieData::NpcMovieData() : movie_paths(), next_movie_index(0) {
    clear();
}

// IDA_EA: 0x8581D96
// IDA_NAME: _ZN12NpcMovieDataD2Ev
// DEMANGLED: NpcMovieData::~NpcMovieData()
NpcMovieData::~NpcMovieData() {}

// IDA_EA: 0x89FB7D0
// IDA_NAME: _ZN12NpcMovieData5clearEv
// DEMANGLED: NpcMovieData::clear(void)
NpcMovieData* NpcMovieData::clear() {
    movie_paths.clear();
    next_movie_index = 0;
    return this;
}

// IDA_EA: 0x89FB7EE
// IDA_NAME: _ZN12NpcMovieData16getNextMoviePathEv
// DEMANGLED: NpcMovieData::getNextMoviePath(void)
std::string NpcMovieData::getNextMoviePath() {
    std::string path;
    if (!movie_paths.empty() &&
        static_cast<int32_t>(movie_paths.size()) > next_movie_index) {
        path = movie_paths[static_cast<uint32_t>(next_movie_index)];
        const uint32_t next =
            static_cast<uint32_t>(next_movie_index) + 1u;
        next_movie_index =
            static_cast<int32_t>(next % movie_paths.size());
    }
    return path;
}

// IDA_EA: 0x85818F0
// IDA_NAME: _ZN25stFavorableRelationShip_t16stBuffRewardInfoC2Ev
// DEMANGLED: stFavorableRelationShip_t::stBuffRewardInfo::stBuffRewardInfo(void)
stFavorableRelationShip_t::stBuffRewardInfo::stBuffRewardInfo()
    : favor_level(0), buff_type(0), probability(0) {}

// IDA_EA: 0x8A073D4
// IDA_NAME: _ZN25stFavorableRelationShip_t16stBuffRewardInfoD2Ev
// DEMANGLED: stFavorableRelationShip_t::stBuffRewardInfo::~stBuffRewardInfo()
stFavorableRelationShip_t::stBuffRewardInfo::~stBuffRewardInfo() {}

// IDA_EA: 0x8A00C70
// IDA_NAME: _ZN25stFavorableRelationShip_t18stPeriodRewardInfoC2Ev
// DEMANGLED: stFavorableRelationShip_t::stPeriodRewardInfo::stPeriodRewardInfo(void)
stFavorableRelationShip_t::stPeriodRewardInfo::stPeriodRewardInfo()
    : reward_type(0),
      item_or_quest(),
      value_08(0),
      value_0c(0),
      title(),
      text() {}

// IDA_EA: 0x85819E4
// IDA_NAME: _ZN25stFavorableRelationShip_t18stPeriodRewardInfoD2Ev
// DEMANGLED: stFavorableRelationShip_t::stPeriodRewardInfo::~stPeriodRewardInfo()
stFavorableRelationShip_t::stPeriodRewardInfo::~stPeriodRewardInfo() {}

// IDA_EA: 0x8A00EB0
// IDA_NAME: _ZN25stFavorableRelationShip_tC2Ev
// DEMANGLED: stFavorableRelationShip_t::stFavorableRelationShip_t(void)
stFavorableRelationShip_t::stFavorableRelationShip_t()
    : enabled(0),
      reserved_01(),
      default_favor(0),
      max_gift_per_day(0),
      dialog_day_not_continuously(0),
      value_0c(0),
      reserved_0e(0),
      mood_changes(),
      key_items(),
      prefer_items(),
      unprefer_items(),
      prefer_item_groups(),
      unprefer_item_groups(),
      reward_items(),
      favor_rate_per_mood(),
      level_rewards(),
      buff_rewards(),
      period_reward() {
    clear();
}

// IDA_EA: 0x8581AE0
// IDA_NAME: _ZN25stFavorableRelationShip_tD2Ev
// DEMANGLED: stFavorableRelationShip_t::~stFavorableRelationShip_t()
stFavorableRelationShip_t::~stFavorableRelationShip_t() {}

// IDA_EA: 0x89FB56C
// IDA_NAME: _ZN25stFavorableRelationShip_t5clearEv
// DEMANGLED: stFavorableRelationShip_t::clear(void)
void stFavorableRelationShip_t::clear() {
    enabled = 0;
    default_favor = 0;
    max_gift_per_day = 0;
    dialog_day_not_continuously = 0;
    value_0c = 0;
    mood_changes.clear();
    key_items.clear();
    for (int index = 0; index <= 4; ++index) {
        prefer_items[index].clear();
        unprefer_items[index].clear();
        prefer_item_groups[index].clear();
        unprefer_item_groups[index].clear();
    }
    favor_rate_per_mood[0] = 1.0f;
    favor_rate_per_mood[1] = 1.0f;
    favor_rate_per_mood[2] = 1.0f;
    // IDA 0x89FB56C stores the three 1.0f rates before forwarding to the
    // embedded stRewardItem_t::clear at favorable_relation +0x208.
    reward_items.clear();
    level_rewards.clear();
    buff_rewards.clear();
    period_reward.reward_type = 0;
    period_reward.item_or_quest.clear();
    period_reward.value_08 = 0;
    period_reward.value_0c = 0;
    period_reward.title.clear();
    period_reward.text.clear();
}

// IDA_EA: 0x89FB942
// IDA_NAME: _ZN9NpcScriptC2Ev
// DEMANGLED: NpcScript::NpcScript(void)
NpcScript::NpcScript()
    : index(0),
      name(),
      field_name(),
      small_face(),
      small_face_index(0),
      big_face(),
      big_face_index(0),
      popup_face(),
      popup_face_index(0),
      field_animation(),
      field_wav(),
      popup_wavs(),
      look_around(0),
      reserved_35(),
      value_38(-1),
      value_3c(),
      value_40(),
      roles(),
      quest_speech(),
      skills_by_job(),
      values_f8(),
      string_data(),
      int_data(),
      balloon_messages(),
      balloon_message_values(),
      dialogs(),
      favor_dialogs(),
      first_favor_dialogs(),
      favor_dialog_exceptions(),
      event_favor_dialogs(),
      favorable_relation(),
      favor_duration_reward_buff_dialog(),
      value_688(0),
      reserved_689(),
      web_pages(),
      click_marks(),
      random_message_balloon(),
      hidden(0),
      reserved_6b9(),
      movie_data() {
    clear();
}

// IDA_EA: 0x8581DAA
// IDA_NAME: _ZN9NpcScriptD2Ev
// DEMANGLED: NpcScript::~NpcScript()
NpcScript::~NpcScript() {
    npc_script_0828::Erase(this);
}

// IDA_EA: 0x89FBF22
// IDA_NAME: _ZN9NpcScript5clearEv
// DEMANGLED: NpcScript::clear(void)
NpcScript* NpcScript::clear() {
    npc_script_0828::Erase(this);
    index = 0;
    name.clear();
    field_name.clear();
    small_face.clear();
    small_face_index = 0;
    big_face.clear();
    big_face_index = 0;
    popup_face.clear();
    popup_face_index = 0;
    field_animation.clear();
    field_wav.clear();
    popup_wavs[0].clear();
    popup_wavs[1].clear();
    look_around = 1;
    roles.clear();
    quest_speech.clear();
    for (int index_value = 0;
         index_value < oldsrv::config::kCharacterJobWithCommonCount;
         ++index_value) {
        skills_by_job[index_value].clear();
    }
    values_f8[0].clear();
    values_f8[1].clear();
    values_f8[2].clear();
    value_38 = -1;
    string_data.clear();
    int_data.clear();
    balloon_messages.clear();
    balloon_message_values.clear();
    dialogs.clear();
    favor_dialogs.clear();
    first_favor_dialogs.clear();
    favor_dialog_exceptions.clear();
    event_favor_dialogs.clear();
    favorable_relation.clear();
    web_pages.clear();
    value_688 = 0;
    click_marks.clear();
    favor_duration_reward_buff_dialog.clear();
    values_f8[1].clear();
    hidden = 0;
    return this;
}

// IDA_EA: 0x8582338
// IDA_NAME: _ZN10CNPCScriptC2Ev
// DEMANGLED: CNPCScript::CNPCScript(void)
CNPCScript::CNPCScript() : index(0), script() {}

// IDA_EA: 0x858234E
// IDA_NAME: _ZN10CNPCScriptD2Ev
// DEMANGLED: CNPCScript::~CNPCScript()
CNPCScript::~CNPCScript() {}

// IDA_EA: 0x85809A6
// IDA_NAME: _ZN10CNPCScript4loadEPKc
// DEMANGLED: CNPCScript::load(char const*)
int CNPCScript::load(const char* filename) {
    if (importNpcScript(&script, filename) != 1) {
        return 0;
    }
    index = script.index;
    return 1;
}

// IDA_EA: 0x85819AC
// IDA_NAME: _ZNK10CNPCScript8getIndexEv
// DEMANGLED: CNPCScript::getIndex(void)const
unsigned int CNPCScript::getIndex() const {
    return static_cast<unsigned int>(index);
}

// IDA_EA: 0x85819B6
// IDA_NAME: _ZNK10CNPCScript14isFavorableNPCEv
// DEMANGLED: CNPCScript::isFavorableNPC(void)const
int CNPCScript::isFavorableNPC() const {
    return script.favorable_relation.enabled;
}

bool CNPCScript::get0828Extension(
    npc_script_0828::Extension* output) const {
    return npc_script_0828::Find(&script, output);
}

// IDA_EA: 0x858191C
// IDA_NAME: _ZN15CNPCDynamicInfoC2EPK10CNPCScript
// DEMANGLED: CNPCDynamicInfo::CNPCDynamicInfo(CNPCScript const*)
CNPCDynamicInfo::CNPCDynamicInfo(const CNPCScript* script_value)
    : script(script_value), mood(3), mood_position(0x7FFFFFFF),
      next_change_second(0) {}

// IDA_EA: 0x89FC264
// IDA_NAME: _Z15importNpcScriptP9NpcScriptPKc
// DEMANGLED: importNpcScript(NpcScript *,char const*)
int importNpcScript(NpcScript* script, const char* filename) {
    // IDA 0x89FC3AC: direct scalar reads call _Z7ScanIntPb with a null bool*.
    if (!loadRDARScriptFile(g_npcScriptBaseDirectory, filename)) {
        return 0;
    }

    script->clear();
    getDirectoryPath(script->values_f8[2],
                             "%s%s",
                             g_npcScriptBaseDirectory,
                             filename);

    std::string tag;
    std::string text;
    bool success = false;
    int value = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->name);
        } else if (tag == "[field name]") {
            ScanStr(&script->field_name);
        } else if (tag == "[small face]") {
            ScanStr(&script->small_face);
            script->small_face_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[big face]") {
            ScanStr(&script->big_face);
            script->big_face_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[popup face]") {
            ScanStr(&script->popup_face);
            script->popup_face_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[field animation]") {
            ScanStr(&script->field_animation);
        } else if (tag == "[field wav]") {
            ScanStr(&script->field_wav);
        } else if (tag == "[popup wav]") {
            ScanStr(&script->popup_wavs[0]);
            ScanStr(&script->popup_wavs[1]);
        } else if (tag == "[look around]") {
            script->look_around =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[field role]") {
            npc_script_0828::FieldRole field_role;
            bool field_role_closed = false;
            while (ScanType(tag, true) == 1) {
                if (tag == "[/field role]") {
                    field_role_closed = true;
                    break;
                }
                if (tag == "[name]") {
                    if (ScanStr(&field_role.name) != 1) {
                        return 0;
                    }
                    field_role.has_name = true;
                } else if (tag == "[img]") {
                    if (ScanStr(&field_role.image) != 1) {
                        return 0;
                    }
                    field_role.image_index = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    field_role.has_image = true;
                } else {
                    return 0;
                }
            }
            if (!field_role_closed) {
                return 0;
            }
            npc_script_0828::Extension& extension =
                npc_script_0828::Mutable(script);
            extension.has_field_role = true;
            extension.field_role = field_role;
        } else if (tag == "[click area]") {
            npc_script_0828::ClickArea click_area;
            for (int index = 0; index < 8; ++index) {
                click_area.values[index] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
            }
            npc_script_0828::Extension& extension =
                npc_script_0828::Mutable(script);
            extension.has_click_area = true;
            extension.click_area = click_area;
        } else if (tag == "[is role direct action]") {
            const int direct_action = ScanInt(&success);
            if (!success) {
                return 0;
            }
            npc_script_0828::Extension& extension =
                npc_script_0828::Mutable(script);
            extension.has_role_direct_action = true;
            extension.role_direct_action = direct_action;
        } else if (tag == "[click ani]") {
            npc_script_0828::ClickAnimation click_animation;
            if (ScanStr(&click_animation.animation) != 1) {
                return 0;
            }
            while (ScanStr(&text) == 1) {
                npc_script_0828::ClickAnimationAction action;
                action.action = text;
                action.value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                click_animation.actions.push_back(action);
            }
            npc_script_0828::Extension& extension =
                npc_script_0828::Mutable(script);
            extension.has_click_animation = true;
            extension.click_animation = click_animation;
        } else if (tag == "[sub field animation]") {
            npc_script_0828::Extension& extension =
                npc_script_0828::Mutable(script);
            if (ScanStr(&extension.sub_field_animation) != 1) {
                return 0;
            }
            extension.has_sub_field_animation = true;
        } else if (tag == "[role]") {
            while (ScanStr(&text) == 1) {
                const int role = getNpcRoleByString(text);
                if (role == -1) {
                    ScanInt(static_cast<bool*>(0));
                } else {
                    script->roles[role] =
                        ScanInt(static_cast<bool*>(0));
                }
            }
        } else if (tag == "[quest speech]") {
            ScanStr(&script->quest_speech);
        } else if (tag == "[skill]") {
            int job = oldsrv::config::kCharacterJobCommon;
            ScanStr(&text);
            if (text != "[common]") {
                job = ::getCharacterJob(text.c_str());
            }
            if (job < 0 ||
                job >= oldsrv::config::kCharacterJobWithCommonCount) {
                return 0;
            }
            script->skills_by_job[job].clear();
            while (true) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->skills_by_job[job].push_back(value);
            }
        } else if (tag == "[web page]") {
            const int count = ScanInt(static_cast<bool*>(0));
            for (int index = 0; index < count; ++index) {
                ScanStr(&text);
                script->web_pages.push_back(text);
            }
        } else if (tag == "[movie path]") {
            while (ScanStr(&text) == 1) {
                script->movie_data.movie_paths.push_back(text);
            }
            script->movie_data.next_movie_index = 0;
        } else if (tag == "[ingame advertisement movie path]") {
            while (ScanStr(&text) == 1) {
            }
        } else if (tag == "[speech]") {
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanStr(&text);
            ScanStr(&text);
            while (ScanInt(&success), success) {
            }
        } else if (tag == "[click mark]") {
            while (true) {
                ClickMarkInfo mark;
                mark.type = ScanInt(&success);
                if (!success) {
                    break;
                }
                mark.value = 0;
                if (mark.type == 0) {
                    mark.value = ScanInt(&success);
                }
                script->click_marks.push_back(mark);
            }
        } else if (tag == "[dialog]") {
            InteractiveDialogScript dialog;
            dialog.value_00 = ScanInt(static_cast<bool*>(0));
            dialog.value_04 = ScanInt(static_cast<bool*>(0));
            dialog.value_08 = ScanInt(static_cast<bool*>(0));
            const int condition_count = ScanInt(static_cast<bool*>(0));
            if (condition_count == -1) {
                dialog.value_18 = 1;
            } else {
                for (int index = 0; index < condition_count * 2; ++index) {
                    dialog.value_0C.push_back(
                        ScanInt(static_cast<bool*>(0)));
                }
            }
            dialog.value_1C = ScanInt(static_cast<bool*>(0));
            dialog.value_20 =
                ScanInt(static_cast<bool*>(0)) != 0;
            ScanStr(&dialog.value_24);
            while (true) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                dialog.value_28.push_back(value);
            }
            script->dialogs.push_back(dialog);
        } else if (tag == "[favor_dialog]") {
            PresentResponceScript dialog;
            dialog.value_00 = ScanInt(static_cast<bool*>(0));
            dialog.value_04 = ScanInt(static_cast<bool*>(0));
            dialog.value_08 = ScanInt(static_cast<bool*>(0));
            ScanStr(&dialog.value_0C);
            script->favor_dialogs.push_back(dialog);
        } else if (tag == "[favor_dialog_exception]") {
            PresentResponceScriptException exception;
            exception.value_00 = ScanInt(static_cast<bool*>(0));
            ScanStr(&exception.value_04);
            script->favor_dialog_exceptions.push_back(exception);
        } else if (tag == "[event_favor_dialog]") {
            PresentResponceScriptEvent event;
            event.value_00 = ScanInt(static_cast<bool*>(0));
            ScanStr(&event.text);
            script->event_favor_dialogs.push_back(event);
        } else if (tag == "[1st favor dialog]") {
            while (ScanStr(&text) == 1) {
                const char* delimiter = strchr(text.c_str(), '>');
                if (delimiter != 0) {
                    script->first_favor_dialogs.push_back(delimiter + 1);
                }
            }
        } else if (tag == "[favor_duration_reward_buff_dialog]") {
            ScanStr(&script->favor_duration_reward_buff_dialog);
            ScanInt(&success);
        } else if (tag == "[string data]") {
            script->string_data.clear();
            while (ScanStr(&text) == 1) {
                script->string_data.push_back(text);
            }
        } else if (tag == "[int data]") {
            script->int_data.clear();
            while (true) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->int_data.push_back(value);
            }
        } else if (tag == "[balloon message]") {
            script->balloon_messages.clear();
            script->balloon_message_values.clear();
            while (ScanStr(&text) == 1) {
                script->balloon_messages.push_back(text);
                script->balloon_message_values.push_back(
                    ScanInt(static_cast<bool*>(0)));
            }
        } else if (tag == "[favorable relationship]") {
            script->favorable_relation.enabled = 1;
            while (ScanType(tag, true) == 1 &&
                   tag != "[/favorable relationship]") {
                if (tag == "[default favor]") {
                    script->favorable_relation.default_favor =
                        ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (tag == "[max gift per day]") {
                    script->favorable_relation.max_gift_per_day =
                        static_cast<uint16_t>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                } else if (tag == "[dialog day not continuously]") {
                    script->favorable_relation.dialog_day_not_continuously =
                        static_cast<uint16_t>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                    script->favorable_relation.value_0c =
                        static_cast<uint16_t>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                } else if (tag == "[mood]") {
                    while (ScanType(tag, true) == 1 &&
                           tag != "[/mood]") {
                        if (tag == "[change]") {
                            while (ScanStr(&text) == 1) {
                                const int mood =
                                    getNpcMoodFromString(text.c_str());
                                if (mood == 3) {
                                    return 0;
                                }
                                const int seconds = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                script->favorable_relation.mood_changes.push_back(
                                    std::make_pair(
                                        mood,
                                        static_cast<uint32_t>(seconds)));
                            }
                        } else if (tag == "[key item]") {
                            while (true) {
                                const int item_index = ScanInt(&success);
                                if (!success) {
                                    break;
                                }
                                const int item_count = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                script->favorable_relation.key_items.push_back(
                                    std::make_pair(
                                        static_cast<uint32_t>(item_index),
                                        static_cast<uint32_t>(item_count)));
                            }
                        }
                    }
                } else if (tag == "[gift item]") {
                    while (ScanType(tag, true) == 1 &&
                           tag != "[/gift item]") {
                        if (tag == "[favor rate per mood]") {
                            script->favorable_relation.favor_rate_per_mood[2] =
                                static_cast<float>(ScanInt(&success)) /
                                100.0f;
                            if (!success) {
                                return 0;
                            }
                            script->favorable_relation.favor_rate_per_mood[1] =
                                static_cast<float>(ScanInt(&success)) /
                                100.0f;
                            if (!success) {
                                return 0;
                            }
                            script->favorable_relation.favor_rate_per_mood[0] =
                                static_cast<float>(ScanInt(&success)) /
                                100.0f;
                            if (!success) {
                                return 0;
                            }
                        } else if (tag == "[prefer item]" ||
                                   tag == "[unprefer item]") {
                            while (true) {
                                const int mood = ScanInt(&success);
                                if (!success) {
                                    break;
                                }
                                if (mood > 4 || mood < 0) {
                                    return 0;
                                }
                                const int item_index = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                const int required_count =
                                    ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                int favor_value = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                stFavorableRelationShip_t::stGiftItemInfo_t info;
                                info.required_count =
                                    static_cast<uint16_t>(required_count);
                                info.value_04 = tag == "[unprefer item]" ? 1 : 0;
                                if (tag == "[unprefer item]") {
                                    favor_value = -favor_value;
                                }
                                info.favor_value =
                                    static_cast<int16_t>(favor_value);
                                if (tag == "[prefer item]") {
                                    script->favorable_relation.prefer_items[mood]
                                        [static_cast<uint32_t>(item_index)] = info;
                                } else {
                                    script->favorable_relation.unprefer_items[mood]
                                        [static_cast<uint32_t>(item_index)] = info;
                                }
                            }
                        } else if (tag == "[prefer item group]" ||
                                   tag == "[unprefer item group]") {
                            while (true) {
                                const int mood = ScanInt(&success);
                                if (!success) {
                                    break;
                                }
                                if (mood > 4 || mood < 0 ||
                                    ScanStr(&text) != 1) {
                                    return 0;
                                }
                                std::map<std::string, int>* group_table =
                                    getItemGroupNameTable();
                                std::map<std::string, int>::const_iterator group_it =
                                    group_table->find(text);
                                if (group_it == group_table->end()) {
                                    return 0;
                                }
                                const int required_count =
                                    ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                int favor_value = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                                stFavorableRelationShip_t::stGiftItemInfo_t info;
                                info.required_count =
                                    static_cast<uint16_t>(required_count);
                                info.value_04 =
                                    tag == "[unprefer item group]" ? 1 : 0;
                                if (tag == "[unprefer item group]") {
                                    favor_value = -favor_value;
                                }
                                info.favor_value =
                                    static_cast<int16_t>(favor_value);
                                if (tag == "[prefer item group]") {
                                    script->favorable_relation
                                        .prefer_item_groups[mood]
                                            [static_cast<uint32_t>(group_it->second)] =
                                                info;
                                } else {
                                    script->favorable_relation
                                        .unprefer_item_groups[mood]
                                            [static_cast<uint32_t>(group_it->second)] =
                                                info;
                                }
                            }
                        }
                    }
                } else if (tag == "[reward item]") {
                    while (ScanType(tag, true) == 1 &&
                           tag != "[/reward item]") {
                        if (tag != "[item info]") {
                            continue;
                        }
                        int favor_level = 0;
                        while (ScanType(tag, true) == 1 &&
                               tag != "[/item info]") {
                            if (tag == "[favor level]") {
                                favor_level = ScanInt(&success);
                                if (!success) {
                                    return 0;
                                }
                            } else if (tag == "[mood]") {
                                if (ScanStr(&text) != 1 ||
                                    favor_level == 0) {
                                    return 0;
                                }
                                const int mood =
                                    getNpcMoodFromString(text.c_str());
                                if (mood == 3 || mood > 2 || favor_level > 4) {
                                    return 0;
                                }
                                while (ScanType(tag, true) == 1 &&
                                       tag != "[/mood]") {
                                    if (tag != "[favor]" ||
                                        ScanStr(&text) != 1) {
                                        continue;
                                    }
                                    int favor_change = 3;
                                    if (text == "up") {
                                        favor_change = 1;
                                    } else if (text == "down") {
                                        favor_change = 2;
                                    } else if (text == "keep") {
                                        favor_change = 0;
                                    }
                                    if (favor_change == 3) {
                                        return 0;
                                    }
                                    stFavorableRelationShip_t::stRewardItemInfo_t&
                                        reward = script->favorable_relation
                                                     .reward_items[favor_level]
                                                                  [mood];
                                    while (true) {
                                        stFavorableRelationShip_t::stRewardItemElement_t
                                            element;
                                        element.item_index =
                                            ScanInt(&success);
                                        if (!success) {
                                            break;
                                        }
                                        element.item_count =
                                            ScanInt(&success);
                                        if (!success) {
                                            return 0;
                                        }
                                        const int rate = ScanInt(&success);
                                        if (!success) {
                                            return 0;
                                        }
                                        reward.total_rates[favor_change] += rate;
                                        element.accumulated_rate =
                                            reward.total_rates[favor_change];
                                        reward.reward_items[favor_change]
                                            .push_back(element);
                                    }
                                }
                            }
                        }
                    }
                } else if (tag == "[favor level reward mail]") {
                    while (true) {
                        stFavorableRelationShip_t::stLevelRewardInfo reward;
                        reward.favor_level = static_cast<uint16_t>(
                            ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        reward.item_index = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        reward.item_count = ScanInt(&success);
                        if (!success || ScanStr(&reward.title) != 1 ||
                            ScanStr(&reward.text) != 1) {
                            break;
                        }
                        script->favorable_relation.level_rewards.push_back(reward);
                    }
                } else if (tag == "[favor_duration_reward]") {
                    stFavorableRelationShip_t::stPeriodRewardInfo& reward =
                        script->favorable_relation.period_reward;
                    reward.reward_type = static_cast<uint16_t>(
                        ScanInt(&success));
                    if (!success || ScanStr(&reward.item_or_quest) != 1) {
                        return 0;
                    }
                    reward.value_08 = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    reward.value_0c = ScanInt(&success);
                    if (!success || ScanStr(&reward.title) != 1 ||
                        ScanStr(&reward.text) != 1) {
                        return 0;
                    }
                } else if (tag == "[favor_duration_reward_buff]") {
                    while (true) {
                        stFavorableRelationShip_t::stBuffRewardInfo reward;
                        reward.favor_level = static_cast<uint16_t>(
                            ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        reward.buff_type = static_cast<uint16_t>(
                            ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        reward.probability = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        script->favorable_relation.buff_rewards.push_back(reward);
                    }
                }
            }
        } else if (tag == "[visible on dungeon clear]") {
            script->value_688 =
                ScanInt(static_cast<bool*>(0)) == 1;
        } else if (tag == "[move town limit str]") {
            ScanStr(&script->values_f8[0]);
        } else if (tag == "[move town confirm str]") {
            ScanStr(&script->values_f8[1]);
        } else if (tag == "[random message balloon]") {
            while (ScanStr(&text) == 1) {
                if (text == "[random message animation]") {
                    if (ScanStr(&script->random_message_balloon.animation) !=
                        1) {
                        return 0;
                    }
                } else if (text == "[random message probability]") {
                    const int probability = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    if (probability != 0) {
                        script->random_message_balloon.probability = probability;
                    }
                } else if (text == "[random message list]") {
                    const int count = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    std::vector<std::string> messages;
                    messages.reserve(count);
                    for (int index = 0; index < count; ++index) {
                        if (ScanStr(&text) != 1) {
                            return 0;
                        }
                        messages.push_back(text);
                    }
                    if (!messages.empty()) {
                        script->random_message_balloon.messages = messages;
                    }
                }
            }
        } else if (tag == "[hide]") {
            script->hidden = ScanInt(static_cast<bool*>(0)) == 1;
        }
    }

    script->index = g_npcScriptFileList.FindIndexByFullScan(filename);
    return 1;
}
