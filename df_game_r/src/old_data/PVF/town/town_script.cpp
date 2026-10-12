#include "old_data/PVF/town/town_script.h"

#include <cstring>

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/common/rdar_script.h"

char g_townScriptBaseDirectory[256];
STScriptFileList g_townScriptFileList;

namespace {

typedef std::map<int, TownPermissionSet> TownPermissionTable;

TownPermissionTable g_townPermissionTable;

bool ScanTownPermissionQuestList(
    std::vector<int32_t>* required_quest_ids) {
    if (!required_quest_ids) {
        return false;
    }
    required_quest_ids->clear();

    for (;;) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int value = ScanTypeOrInt(is_type, token, success, false);
        if (!success) {
            return false;
        }
        if (is_type) {
            return token == "[/need quest]" &&
                   !required_quest_ids->empty();
        }
        if (value <= 0) {
            return false;
        }
        required_quest_ids->push_back(value);
    }
}

bool ScanTownPermissionLevel(int32_t* required_level) {
    if (!required_level) {
        return false;
    }
    bool success = false;
    const int level = ScanInt(&success);
    if (!success || level < 0) {
        return false;
    }

    *required_level = level;
    return true;
}

bool ScanTownPermissionRequirement(
    TownPermissionRequirement* requirement,
    const char* closing_tag) {
    if (!requirement || !closing_tag) {
        return false;
    }

    std::string tag;
    while (ScanType(tag, false) == 1) {
        if (tag == closing_tag) {
            return true;
        }
        if (tag == "[need quest]") {
            if (!ScanTownPermissionQuestList(
                    &requirement->required_quest_ids)) {
                return false;
            }
        } else if (tag == "[need level]") {
            if (!ScanTownPermissionLevel(&requirement->required_level)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return false;
}

bool ScanTownPermissionBlock(TownPermissionSet* permission_set) {
    if (!permission_set) {
        return false;
    }

    std::string tag;
    while (ScanType(tag, false) == 1) {
        if (tag == "[/permission]") {
            return true;
        }
        if (tag == "[need quest]") {
            if (!ScanTownPermissionQuestList(
                    &permission_set->default_requirement
                         .required_quest_ids)) {
                return false;
            }
        } else if (tag == "[need level]") {
            if (!ScanTownPermissionLevel(
                    &permission_set->default_requirement.required_level)) {
                return false;
            }
        } else if (tag == "[exceptional character]") {
            std::string job_name;
            if (ScanStr(&job_name) != 1) {
                return false;
            }
            // Values inside the official town `[exceptional character]`
            // block are bare PVF strings such as `at swordman`.  The legacy
            // getCharacterJob() helper accepts bracketed type names (for
            // example `[at swordman]`) and therefore maps every valid town
            // exception to the synthetic common-job slot.  Use the bare-name
            // resolver that matches the actual town grammar.
            const int character_job = getCharacterJobToEnumValue(job_name);
            if (character_job < 0 ||
                character_job >= oldsrv::config::kCharacterJobCount ||
                permission_set->exceptional_requirements.find(character_job) !=
                    permission_set->exceptional_requirements.end()) {
                return false;
            }

            TownPermissionRequirement exceptional_requirement;
            if (!ScanTownPermissionRequirement(
                    &exceptional_requirement,
                    "[/exceptional character]")) {
                return false;
            }
            permission_set->exceptional_requirements.insert(
                std::make_pair(character_job, exceptional_requirement));
        } else if (tag == "[/exceptional character]") {
            // The official 0828 chn_shonan_holy.twn contains one orphaned
            // closing tag after repeating its default level.  The client
            // accepts that asset; retain the already parsed default rule.
            continue;
        } else {
            return false;
        }
    }
    return false;
}

}  // namespace

// IDA_EA: 0x86D1D46
// IDA_NAME: _ZN14TownAreaScript5clearEv
// DEMANGLED: TownAreaScript::clear(void)
TownAreaScript* TownAreaScript::clear() {
    area_index = -1;
    area_name = "";
    area_type = TOWN_AREA_NORMAL;
    minimap_rects.clear();
    linked_area_indices.clear();
    private_store_enabled = 1;
    return this;
}

// IDA_EA: 0x86D1D9A
// IDA_NAME: _ZN14TownAreaScriptC2Ev
// DEMANGLED: TownAreaScript::TownAreaScript(void)
TownAreaScript::TownAreaScript()
    : area_name(), minimap_rects(), linked_area_indices() {
    clear();
}

// IDA_EA: 0x86D28F6
// IDA_NAME: _ZN14TownAreaScriptD2Ev
// DEMANGLED: TownAreaScript::~TownAreaScript()
TownAreaScript::~TownAreaScript() {}

// IDA_EA: 0x86D296A
// IDA_NAME: _ZN14TownAreaScriptC2ERKS_
// DEMANGLED: TownAreaScript::TownAreaScript(TownAreaScript const&)
TownAreaScript::TownAreaScript(const TownAreaScript& rhs)
    : area_index(rhs.area_index),
      area_name(rhs.area_name),
      area_type(rhs.area_type),
      minimap_rects(rhs.minimap_rects),
      linked_area_indices(rhs.linked_area_indices),
      private_store_enabled(rhs.private_store_enabled) {}

// IDA_EA: 0x86D2A1A
// IDA_NAME: _ZN14TownAreaScriptaSERKS_
// DEMANGLED: TownAreaScript::operator=(TownAreaScript const&)
TownAreaScript& TownAreaScript::operator=(const TownAreaScript& rhs) {
    area_index = rhs.area_index;
    area_name = rhs.area_name;
    area_type = rhs.area_type;
    minimap_rects = rhs.minimap_rects;
    linked_area_indices = rhs.linked_area_indices;
    private_store_enabled = rhs.private_store_enabled;
    return *this;
}

// IDA_EA: 0x86D1E2E
// IDA_NAME: _ZN10TownScript5clearEv
// DEMANGLED: TownScript::clear(void)
TownScript* TownScript::clear() {
    town_index = 0;
    town_name = "";
    entering_title = "";
    cutscene_image = "";
    entering_title_next = "";
    cutscene_image_value = 0;
    dungeon_what_must_be_cleared = -1;
    server_dungeon_what_must_be_cleared = -1;
    areas.clear();
    town_script_path = "";
    need_quest = 0;
    limit_level = need_quest;
    return this;
}

// IDA_EA: 0x86D1F04
// IDA_NAME: _ZN10TownScriptC2Ev
// DEMANGLED: TownScript::TownScript(void)
TownScript::TownScript()
    : town_name(),
      entering_title(),
      cutscene_image(),
      entering_title_next(),
      areas(),
      town_script_path() {
    clear();
}

// IDA_EA: 0x86D2AB8
// IDA_NAME: _ZN10TownScriptD2Ev
// DEMANGLED: TownScript::~TownScript()
TownScript::~TownScript() {}

// IDA_EA: 0x8AAD9EC
// IDA_NAME: _Z14initTownScriptPcS_
// DEMANGLED: initTownScript(char *,char *)
int initTownScript(char* src, char* filename) {
    g_townPermissionTable.clear();
    std::strncpy(g_townScriptBaseDirectory, src, 0x100u);
    if (g_townScriptBaseDirectory[
            std::strlen(g_townScriptBaseDirectory) - 1] != '\\' &&
        g_townScriptBaseDirectory[
            std::strlen(g_townScriptBaseDirectory) - 1] != '/') {
        const std::size_t length = std::strlen(g_townScriptBaseDirectory);
        std::memcpy(g_townScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_townScriptFileList,
                                  g_townScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x8AADAA8
// IDA_NAME: _Z16importTownScriptP10TownScriptPKc
// DEMANGLED: importTownScript(TownScript *,char const*)
int importTownScript(TownScript* script, const char* filename) {
    if (loadRDARScriptFile(g_townScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->clear();
    getDirectoryPath(script->town_script_path,
                     "%s%s",
                     g_townScriptBaseDirectory,
                     filename);

    std::string tag;
    std::string area_tag;
    TownAreaScript area;
    bool success = false;
    bool has_permission_block = false;
    TownPermissionSet permission_set;
    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->town_name);
        } else if (tag == "[entering title]") {
            ScanStr(&script->entering_title);
        } else if (tag == "[entering title next]") {
            ScanStr(&script->entering_title_next);
        } else if (tag == "[cutscene image]") {
            ScanStr(&script->cutscene_image);
            script->cutscene_image_value = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dungeon what must be cleared]") {
            script->dungeon_what_must_be_cleared =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[only server parsing dungeon what must be cleared]") {
            script->server_dungeon_what_must_be_cleared =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[area]") {
            area.clear();
            area.area_index = ScanInt(static_cast<bool*>(0));
            ScanStr(&area.area_name);
            ScanStr(&area_tag);

            if (area_tag == "[minimap rect]") {
                T_Rect rect;
                int value = ScanInt(&success);
                while (success) {
                    for (int index = 0; index <= 3 && success; ++index) {
                        if (index == 0) {
                            rect.left = value;
                        } else if (index == 1) {
                            rect.right = value;
                        } else if (index == 2) {
                            rect.top = value;
                        } else {
                            rect.bottom = value;
                        }
                        value = ScanInt(&success);
                    }
                    area.minimap_rects.push_back(rect);
                }
                ScanStr(&area_tag);
            }

            if (area_tag == "[use private store]") {
                const int use_private_store = ScanInt(&success);
                area.private_store_enabled = use_private_store != 0;
                ScanStr(&area_tag);
            }

            if (area_tag == "[normal]") {
                area.area_type = TOWN_AREA_NORMAL;
            } else if (area_tag == "[gate]") {
                area.area_type = TOWN_AREA_GATE;
            } else if (area_tag == "[fishing place]") {
                area.area_type = TOWN_AREA_FISHING_PLACE;
            } else if (area_tag == "[pub]") {
                area.area_type = TOWN_AREA_PUB;
            } else if (area_tag == "[dungeon gate]") {
                area.area_type = TOWN_AREA_DUNGEON_GATE;
            } else if (area_tag == "[safe zone against a]") {
                area.area_type = TOWN_AREA_SAFE_ZONE_AGAINST_A;
            } else if (area_tag == "[safe zone against b]") {
                area.area_type = TOWN_AREA_SAFE_ZONE_AGAINST_B;
            }

            int linked_area_index = ScanInt(&success);
            while (success) {
                area.linked_area_indices.push_back(linked_area_index);
                linked_area_index = ScanInt(&success);
            }
            script->areas[area.area_index] = area;
        } else if (tag == "[permission]") {
            if (has_permission_block ||
                !ScanTownPermissionBlock(&permission_set)) {
                return 0;
            }
            has_permission_block = true;
        } else if (tag == "[limit level]") {
            script->limit_level = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[need quest]") {
            script->need_quest = ScanInt(&success);
            if (!success) {
                return 0;
            }
        }
    }

    script->town_index = g_townScriptFileList.FindIndexByFullScan(filename);
    if (has_permission_block) {
        script->limit_level =
            permission_set.default_requirement.required_level;
        script->need_quest =
            permission_set.default_requirement.required_quest_ids.empty()
                ? 0
                : permission_set.default_requirement.required_quest_ids[0];
        g_townPermissionTable[script->town_index] = permission_set;
    } else {
        g_townPermissionTable.erase(script->town_index);
    }
    return 1;
}

const TownPermissionRequirement* GetTownPermissionRequirement(
    int town_index,
    int character_job) {
    TownPermissionTable::const_iterator found =
        g_townPermissionTable.find(town_index);
    if (found == g_townPermissionTable.end()) {
        return 0;
    }
    return found->second.select(character_job);
}
