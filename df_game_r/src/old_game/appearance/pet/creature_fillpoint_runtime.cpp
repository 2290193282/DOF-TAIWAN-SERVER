#include "old_game/appearance/pet/creature_fillpoint_runtime.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/creature/creature_script.h"

#include <pthread.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace CreatureFillPoint {
namespace {

typedef std::vector<SkillConfig> SkillConfigList;
typedef std::map<int, SkillConfigList> SkillConfigMap;
typedef std::map<int, std::vector<int> > PassivePreloadBySkill;

struct CreaturePassivePreloads {
    PassivePreloadBySkill general;
    PassivePreloadBySkill over;
};

typedef std::map<int, CreaturePassivePreloads> PassivePreloadMap;

struct CooldownState {
    CooldownState()
        : charac_no(0),
          creature_uid(0),
          last_tick(0),
          pending_tick(0),
          has_last(false),
          pending(false) {}

    int charac_no;
    int creature_uid;
    uint32_t last_tick;
    uint32_t pending_tick;
    bool has_last;
    bool pending;
};

typedef std::map<CUser*, CooldownState> CooldownMap;

SkillConfigMap g_skill_configs;
PassivePreloadMap g_passive_preloads;
SkillConfigMap g_staged_skill_configs;
PassivePreloadMap g_staged_passive_preloads;
bool g_script_cache_load_active = false;
CooldownMap g_cooldowns;
pthread_mutex_t g_fillpoint_mutex = PTHREAD_MUTEX_INITIALIZER;

class MutexGuard {
public:
    explicit MutexGuard(pthread_mutex_t* mutex) : mutex_(mutex) {
        pthread_mutex_lock(mutex_);
    }

    ~MutexGuard() { pthread_mutex_unlock(mutex_); }

private:
    MutexGuard(const MutexGuard&);
    MutexGuard& operator=(const MutexGuard&);

    pthread_mutex_t* mutex_;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

// Returns -1 for malformed [fill point], 0 when the skill does not use the
// field, and 1 when a positive integer was imported.
int ImportFillPoint(const std::string& filename, int& fill_point) {
    fill_point = 0;
    if (filename.empty() || !loadRDARScriptFile("", filename.c_str())) {
        return 0;
    }
    RdarUnloadGuard unload;

    bool expect_value = false;
    bool found = false;
    int data_type = 3;
    int integer_value = 0;
    std::string text;
    while (ScanAnyData(data_type, text, integer_value, false) == 1) {
        if (expect_value) {
            if (data_type != 1 || found || integer_value <= 0 ||
                integer_value > 100) {
                return -1;
            }
            fill_point = integer_value;
            found = true;
            expect_value = false;
            continue;
        }

        if (data_type == 2 && text == "[fill point]") {
            if (found) {
                return -1;
            }
            expect_value = true;
        }
    }

    if (expect_value) {
        return -1;
    }
    return found ? 1 : 0;
}

bool SameIdentity(const CooldownState& state,
                  int charac_no,
                  int creature_uid) {
    return state.charac_no == charac_no &&
           state.creature_uid == creature_uid;
}

void ResetIdentity(CooldownState& state,
                   int charac_no,
                   int creature_uid) {
    state = CooldownState();
    state.charac_no = charac_no;
    state.creature_uid = creature_uid;
}

bool ImportPassivePreloads(
    const std::map<int, CreatureScriptSkillInfo>& source,
    PassivePreloadBySkill& destination) {
    destination.clear();
    for (std::map<int, CreatureScriptSkillInfo>::const_iterator it =
             source.begin();
         it != source.end();
         ++it) {
        if (it->first < 0) {
            return false;
        }
        const std::vector<int>& indices =
            it->second.preload_passive_indices;
        for (std::vector<int>::const_iterator passive = indices.begin();
             passive != indices.end();
             ++passive) {
            if (*passive <= 0) {
                return false;
            }
        }
        if (!indices.empty()) {
            destination.insert(std::make_pair(it->first, indices));
        }
    }
    return true;
}

}  // namespace

SkillConfig::SkillConfig()
    : skill_id(-1),
      fill_point(0),
      recovery_time_ms(0),
      learn_skill_level(1),
      preload_passive_indices() {}

void ResetScriptCache() {
    MutexGuard guard(&g_fillpoint_mutex);
    g_skill_configs.clear();
    g_passive_preloads.clear();
    g_staged_skill_configs.clear();
    g_staged_passive_preloads.clear();
    g_script_cache_load_active = false;
}

void BeginScriptCacheLoad() {
    MutexGuard guard(&g_fillpoint_mutex);
    g_staged_skill_configs.clear();
    g_staged_passive_preloads.clear();
    g_script_cache_load_active = true;
}

void CommitScriptCacheLoad() {
    MutexGuard guard(&g_fillpoint_mutex);
    if (!g_script_cache_load_active) return;
    g_skill_configs.swap(g_staged_skill_configs);
    g_passive_preloads.swap(g_staged_passive_preloads);
    g_staged_skill_configs.clear();
    g_staged_passive_preloads.clear();
    g_script_cache_load_active = false;
}

void AbortScriptCacheLoad() {
    MutexGuard guard(&g_fillpoint_mutex);
    g_staged_skill_configs.clear();
    g_staged_passive_preloads.clear();
    g_script_cache_load_active = false;
}

bool RegisterCreatureScript(int item_id, const CreatureScript& script) {
    if (item_id <= 0) {
        return true;
    }

    SkillConfigList imported;
    CreaturePassivePreloads passive_preloads;
    if (!ImportPassivePreloads(script.skill_infos,
                               passive_preloads.general) ||
        !ImportPassivePreloads(script.over_skill_infos,
                               passive_preloads.over)) {
        return false;
    }
    for (std::map<int, CreatureScriptSkillInfo>::const_iterator it =
             script.skill_infos.begin();
         it != script.skill_infos.end();
         ++it) {
        const CreatureScriptSkillInfo& source = it->second;
        if (source.skill_info.empty()) {
            continue;
        }

        const std::string filename = script.script_path + source.skill_info;
        int fill_point = 0;
        const int import_result = ImportFillPoint(filename, fill_point);
        if (import_result < 0) {
            return false;
        }
        if (import_result == 0) {
            continue;
        }
        if (source.skill_recovery_time <= 0 ||
            source.learn_skill_level <= 0) {
            return false;
        }

        SkillConfig config;
        config.skill_id = it->first;
        config.fill_point = fill_point;
        config.recovery_time_ms = source.skill_recovery_time;
        config.learn_skill_level = source.learn_skill_level;
        config.preload_passive_indices = source.preload_passive_indices;
        imported.push_back(config);
    }

    MutexGuard guard(&g_fillpoint_mutex);
    SkillConfigMap& skill_configs = g_script_cache_load_active
                                        ? g_staged_skill_configs
                                        : g_skill_configs;
    PassivePreloadMap& passive_cache = g_script_cache_load_active
                                           ? g_staged_passive_preloads
                                           : g_passive_preloads;
    skill_configs.erase(item_id);
    passive_cache.erase(item_id);
    if (!imported.empty()) {
        skill_configs.insert(std::make_pair(item_id, imported));
    }
    if (!passive_preloads.general.empty() ||
        !passive_preloads.over.empty()) {
        passive_cache.insert(std::make_pair(item_id, passive_preloads));
    }
    return true;
}

bool ResolveGeneralSkill(int item_id,
                         uint32_t requested_fill_point,
                         int creature_level,
                         SkillConfig& config) {
    MutexGuard guard(&g_fillpoint_mutex);
    SkillConfigMap::const_iterator found = g_skill_configs.find(item_id);
    if (found == g_skill_configs.end()) {
        return false;
    }

    const SkillConfig* match = 0;
    for (SkillConfigList::const_iterator it = found->second.begin();
         it != found->second.end();
         ++it) {
        if (it->fill_point != static_cast<int>(requested_fill_point) ||
            creature_level < it->learn_skill_level) {
            continue;
        }
        // The wire cannot distinguish two general skills with the same
        // fill-point value.  Reject such an ambiguous script projection.
        if (match != 0) {
            return false;
        }
        match = &*it;
    }
    if (!match) {
        return false;
    }
    config = *match;
    return true;
}

bool ResolveSkillPassivePreloads(int item_id,
                                 int skill_id,
                                 bool over_skill,
                                 std::vector<int>& passive_indices) {
    passive_indices.clear();
    MutexGuard guard(&g_fillpoint_mutex);
    const PassivePreloadMap::const_iterator creature =
        g_passive_preloads.find(item_id);
    if (creature == g_passive_preloads.end()) {
        return false;
    }
    const PassivePreloadBySkill& skills =
        over_skill ? creature->second.over : creature->second.general;
    const PassivePreloadBySkill::const_iterator skill = skills.find(skill_id);
    if (skill == skills.end()) {
        return false;
    }
    passive_indices = skill->second;
    return true;
}

bool BeginCooldown(CUser* user,
                   int charac_no,
                   int creature_uid,
                   int recovery_time_ms,
                   uint32_t now_tick) {
    if (!user || charac_no <= 0 || creature_uid <= 0 ||
        recovery_time_ms <= 0) {
        return false;
    }

    MutexGuard guard(&g_fillpoint_mutex);
    CooldownState& state = g_cooldowns[user];
    if (!SameIdentity(state, charac_no, creature_uid)) {
        ResetIdentity(state, charac_no, creature_uid);
    }
    if (state.pending) {
        return false;
    }
    if (state.has_last &&
        now_tick - state.last_tick <
            static_cast<uint32_t>(recovery_time_ms)) {
        return false;
    }

    state.pending = true;
    state.pending_tick = now_tick;
    return true;
}

void CommitCooldown(CUser* user,
                    int charac_no,
                    int creature_uid,
                    uint32_t now_tick) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_fillpoint_mutex);
    CooldownState& state = g_cooldowns[user];
    if (!SameIdentity(state, charac_no, creature_uid)) {
        ResetIdentity(state, charac_no, creature_uid);
    }
    state.last_tick = now_tick;
    state.pending_tick = 0;
    state.has_last = true;
    state.pending = false;
}

void CancelCooldown(CUser* user,
                    int charac_no,
                    int creature_uid,
                    uint32_t now_tick) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_fillpoint_mutex);
    CooldownMap::iterator found = g_cooldowns.find(user);
    if (found != g_cooldowns.end() &&
        SameIdentity(found->second, charac_no, creature_uid) &&
        found->second.pending &&
        found->second.pending_tick == now_tick) {
        found->second.pending = false;
        found->second.pending_tick = 0;
        if (!found->second.has_last) {
            g_cooldowns.erase(found);
        }
    }
}

void ClearCooldown(CUser* user) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_fillpoint_mutex);
    g_cooldowns.erase(user);
}

}  // namespace CreatureFillPoint
