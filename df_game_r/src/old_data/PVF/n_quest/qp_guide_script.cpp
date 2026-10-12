#include "old_data/PVF/n_quest/qp_guide_recommend_data.h"

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>

namespace {

bool importRecommendations(stQPGuideScript& script) {
    stQPGuideScript::stRecommendQPData recommendation;
    while (true) {
        recommendation.clear();

        std::string job_name;
        if (ScanStr(&job_name) != 1) {
            return !script.recommendations.at(
                oldsrv::config::kCharacterJobCount - 1).empty();
        }

        const int character_job = getCharacterJob(job_name.c_str());
        if (character_job < 0 ||
            character_job >= oldsrv::config::kCharacterJobCount) {
            return false;
        }
        recommendation.value_00 = character_job;

        std::string grow_type_name;
        if (ScanStr(&grow_type_name) != 1) {
            return false;
        }
        const int grow_type = getGrowTypeIndex(grow_type_name.c_str());
        if (static_cast<unsigned short>(grow_type) == 0xffffu) {
            return false;
        }
        recommendation.value_04 = grow_type;

        int qp_count = 0;
        if (!ScanInt(&qp_count) || qp_count <= 0) {
            return false;
        }
        for (int index = 0; index < qp_count; ++index) {
            int qp = -1;
            if (!ScanInt(&qp)) {
                return false;
            }
            recommendation.values.push_back(qp);
        }

        if (script.recommendations.size() !=
            static_cast<size_t>(oldsrv::config::kCharacterJobCount)) {
            return false;
        }
        script.recommendations[character_job].push_back(recommendation);
    }
}

bool importTooltipMappings(stQPGuideScript& script) {
    std::string tooltip;
    while (true) {
        int qp = -1;
        if (!ScanInt(&qp)) {
            return !script.tooltip_mappings.empty();
        }
        if (qp < 0 || ScanStr(&tooltip) != 1) {
            return false;
        }
        if (tooltip.empty()) {
            continue;
        }
        if (script.tooltip_mappings.find(qp) == script.tooltip_mappings.end()) {
            script.tooltip_mappings.insert(std::make_pair(qp, tooltip));
        }
    }
}

}  // namespace

// IDA_EA: 0x890FF93
// IDA_NAME: _Z19importQPGuideScriptPKcR15stQPGuideScript
// DEMANGLED: importQPGuideScript(char const*,stQPGuideScript &)
int importQPGuideScript(const char* filename, stQPGuideScript& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 10422;
    }

    stQPGuideScript parsed;
    parsed.recommendations.resize(oldsrv::config::kCharacterJobCount);

    bool found_outer = false;
    bool found_recommendations = false;
    bool found_tooltips = false;
    bool found_end = false;
    bool malformed = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[qp guide]") {
            if (found_outer) {
                malformed = true;
                break;
            }
            found_outer = true;
        } else if (tag == "[qp recommendation]") {
            if (!found_outer || found_recommendations ||
                !importRecommendations(parsed)) {
                malformed = true;
                break;
            }
            found_recommendations = true;
        } else if (tag == "[qp tooltip mapping]") {
            if (!found_outer || !found_recommendations || found_tooltips ||
                !importTooltipMappings(parsed)) {
                malformed = true;
                break;
            }
            found_tooltips = true;
        } else if (tag == "[/qp guide]") {
            found_end = found_outer && found_recommendations && found_tooltips;
            break;
        }
    }

    unloadRDARScriptFile();
    if (malformed || !found_end) {
        return 0;
    }
    script.recommendations.swap(parsed.recommendations);
    script.tooltip_mappings.swap(parsed.tooltip_mappings);
    return 1;
}
