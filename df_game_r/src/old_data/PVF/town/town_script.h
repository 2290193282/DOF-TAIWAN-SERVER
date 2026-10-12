#ifndef OLDSRV_OLD_DATA_PVF_TOWN_TOWN_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_TOWN_TOWN_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/common/script_file_list.h"
#include "common/types/t_rect.h"

// IDA unit: 05_world_enter_village/TownAreaScript.
// IDA `importTownScript @0x8AADAA8` maps these values directly from the
// `[normal]` through `[safe zone against b]` area tags.
enum TownAreaType : int32_t {
    TOWN_AREA_NORMAL = 0,
    TOWN_AREA_GATE = 1,
    TOWN_AREA_FISHING_PLACE = 2,
    TOWN_AREA_PUB = 3,
    TOWN_AREA_DUNGEON_GATE = 4,
    TOWN_AREA_SAFE_ZONE_AGAINST_A = 5,
    TOWN_AREA_SAFE_ZONE_AGAINST_B = 6,
};

struct TownAreaScript {
    TownAreaScript();
    TownAreaScript(const TownAreaScript& rhs);
    ~TownAreaScript();

    TownAreaScript* clear();
    TownAreaScript& operator=(const TownAreaScript& rhs);

    int32_t area_index;
    std::string area_name;
    TownAreaType area_type;
    std::vector<T_Rect> minimap_rects;
    std::vector<int> linked_area_indices;
    uint8_t private_store_enabled;
};

// 0828 `[permission]` is richer than the recovered 0627 TownScript ABI:
// one default requirement can be replaced by a job-specific exceptional
// requirement, and `[need quest]` is an ordered list.  Keep this data in a
// town-index side table rather than widening the verified TownScript/Village
// prefixes used by the original i386 server.
struct TownPermissionRequirement {
    TownPermissionRequirement() : required_level(0), required_quest_ids() {}

    int32_t required_level;
    std::vector<int32_t> required_quest_ids;
};

struct TownPermissionSet {
    TownPermissionRequirement default_requirement;
    std::map<int, TownPermissionRequirement> exceptional_requirements;

    const TownPermissionRequirement* select(int character_job) const {
        std::map<int, TownPermissionRequirement>::const_iterator found =
            exceptional_requirements.find(character_job);
        return found != exceptional_requirements.end()
                   ? &found->second
                   : &default_requirement;
    }
};

// IDA unit: 05_world_enter_village/TownScript.  Although the original
// lifecycle callsites reside in the world unit, this is the B-owned RDAR
// record populated by initTownScript/importTownScript.
struct TownScript {
    TownScript();
    ~TownScript();

    TownScript* clear();

    int32_t town_index;
    std::string town_name;
    std::string entering_title;
    std::string cutscene_image;
    std::string entering_title_next;
    int32_t cutscene_image_value;
    int32_t dungeon_what_must_be_cleared;
    int32_t server_dungeon_what_must_be_cleared;
    std::map<int, TownAreaScript> areas;
    std::string town_script_path;
    int32_t limit_level;
    int32_t need_quest;
};

static_assert(sizeof(TownAreaScript) == 0x28u,
              "TownAreaScript IDA ABI size must stay 0x28");
static_assert(offsetof(TownAreaScript, area_index) == 0x00u &&
                  offsetof(TownAreaScript, area_name) == 0x04u &&
                  offsetof(TownAreaScript, area_type) == 0x08u &&
                  offsetof(TownAreaScript, minimap_rects) == 0x0cu &&
                  offsetof(TownAreaScript, linked_area_indices) == 0x18u &&
                  offsetof(TownAreaScript, private_store_enabled) == 0x24u,
              "TownAreaScript IDA member offsets must stay exact");
static_assert(sizeof(TownScript) == 0x44u,
              "TownScript IDA ABI size must stay 0x44");
static_assert(offsetof(TownScript, town_index) == 0x00u &&
                  offsetof(TownScript, town_name) == 0x04u &&
                  offsetof(TownScript, cutscene_image_value) == 0x14u &&
                  offsetof(TownScript, dungeon_what_must_be_cleared) == 0x18u &&
                  offsetof(TownScript, server_dungeon_what_must_be_cleared) == 0x1cu &&
                  offsetof(TownScript, areas) == 0x20u &&
                  offsetof(TownScript, town_script_path) == 0x38u &&
                  offsetof(TownScript, limit_level) == 0x3cu &&
                  offsetof(TownScript, need_quest) == 0x40u,
              "TownScript IDA member offsets must stay exact");

// IDA_EA: 0x93FA4E0 / 0x93FA5E0.  Direct Town.lst loader state.
extern char g_townScriptBaseDirectory[256];
extern STScriptFileList g_townScriptFileList;

// IDA_EA: 0x8AAD9EC / 0x8AADAA8.
// IDA_NAME: _Z14initTownScriptPcS_ / _Z16importTownScriptP10TownScriptPKc
// DEMANGLED: initTownScript(char *,char *) / importTownScript(TownScript *,char const*)
int initTownScript(char* src, char* filename);
int importTownScript(TownScript* script, const char* filename);

// Returns the 0828 default/job-specific requirement for a loaded town, or 0
// when the town used only the legacy scalar fields.
const TownPermissionRequirement* GetTownPermissionRequirement(
    int town_index,
    int character_job);

#endif  // OLDSRV_OLD_DATA_PVF_TOWN_TOWN_SCRIPT_H_
