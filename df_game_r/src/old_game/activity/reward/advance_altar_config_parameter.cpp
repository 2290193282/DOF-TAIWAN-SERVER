#include "old_game/activity/reward/advance_altar_config_parameter.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string.h>
#include <string>

namespace advancealtar {

// IDA_EA: 0x889B662
// IDA_NAME: _ZN12advancealtar15ConfigParameterC2Ev
// DEMANGLED: advancealtar::ConfigParameter::ConfigParameter(void)
ConfigParameter::ConfigParameter() {
    daily_refill_ticket = 0;
    gage_upgrade_cost_percent = 0.0f;
    gage_up_rates_ptr =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(gage_up_rates));
    memset(gage_up_rates, 0, sizeof(gage_up_rates));
    memset(gage_upgrade_costs, 0, sizeof(gage_upgrade_costs));
}

// IDA_EA: 0x889B6C6
// IDA_NAME: _ZN12advancealtar15ConfigParameterD2Ev
// DEMANGLED: advancealtar::ConfigParameter::~ConfigParameter()
ConfigParameter::~ConfigParameter() {}

// IDA_EA: 0x889B6CC
// IDA_NAME: _ZN12advancealtar15ConfigParameter21importConfigParameterEPKc
// DEMANGLED: advancealtar::ConfigParameter::importConfigParameter(char const*)
int ConfigParameter::importConfigParameter(const char* filename) {
    if (loadRDARScriptFile("", filename) != 1) {
        // CDataManager::init treats a non-zero result as a startup failure.
        // Returning success here used to publish the constructor defaults when
        // the official file could not be opened.
        return 1024;
    }

    int result = 0;
    int parsed_daily_refill_ticket = 0;
    float parsed_gage_upgrade_cost_percent = 0.0f;
    int32_t parsed_gage_up_rates[4] = {};
    int32_t parsed_gage_upgrade_costs[4] = {};
    bool parsed_gage_indexes[4] = {};
    bool saw_daily_refill_ticket = false;
    bool saw_gage_upgrade_cost_percent = false;
    bool saw_gage_up_rate = false;
    std::string tag;

    while (ScanType(tag, true) == 1) {
        if (tag == "[gage up rate]") {
            if (saw_gage_up_rate) {
                result = 1025;
                break;
            }
            saw_gage_up_rate = true;
            for (;;) {
                bool is_type = false;
                bool success = false;
                std::string value_or_type;
                const int index = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success) {
                    result = 1025;
                    break;
                }
                if (is_type) {
                    if (value_or_type != "[/gage up rate]") {
                        result = 1025;
                    }
                    break;
                }
                if (index < 0 || index >= 4 ||
                    parsed_gage_indexes[index]) {
                    result = 1025;
                    break;
                }

                const int cost = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success || is_type) {
                    result = 1001;
                    break;
                }
                const int rate = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success || is_type) {
                    result = 1004;
                    break;
                }
                if (cost < 0 || rate < 0) {
                    result = 1025;
                    break;
                }

                parsed_gage_upgrade_costs[index] = cost;
                parsed_gage_up_rates[index] = rate;
                parsed_gage_indexes[index] = true;
            }
            if (result != 0) {
                break;
            }
        } else if (tag == "[gage upgrade cost percent]") {
            if (saw_gage_upgrade_cost_percent) {
                result = 1025;
                break;
            }
            saw_gage_upgrade_cost_percent = true;
            bool success = false;
            parsed_gage_upgrade_cost_percent = ScanFloat(&success);
            if (!success || parsed_gage_upgrade_cost_percent < 0.0f) {
                result = 1016;
                break;
            }
            std::string closing_tag;
            if (ScanType(closing_tag, true) != 1 ||
                closing_tag != "[/gage upgrade cost percent]") {
                result = 1025;
                break;
            }
        } else if (tag == "[daily refill ticket]") {
            if (saw_daily_refill_ticket) {
                result = 1025;
                break;
            }
            saw_daily_refill_ticket = true;
            bool success = false;
            parsed_daily_refill_ticket = ScanInt(&success);
            if (!success || parsed_daily_refill_ticket < 0) {
                result = 1023;
                break;
            }
            std::string closing_tag;
            if (ScanType(closing_tag, true) != 1 ||
                // The 0828 official file uses this shorter terminator.  The
                // second spelling is the literal checked by the 0627 parser.
                (closing_tag != "[/refill ticket]" &&
                 closing_tag != "[/daily refill ticket]")) {
                result = 1025;
                break;
            }
        } else {
            result = 1025;
            break;
        }
    }

    unloadRDARScriptFile();
    if (result == 0) {
        if (!saw_daily_refill_ticket ||
            !saw_gage_upgrade_cost_percent ||
            !saw_gage_up_rate ||
            !parsed_gage_indexes[0] ||
            !parsed_gage_indexes[1] ||
            !parsed_gage_indexes[2] ||
            !parsed_gage_indexes[3] ||
            parsed_gage_up_rates[0] <= 0) {
            return 1025;
        }

        // Publish only after the complete official shape has validated.  The
        // embedded arrays and legacy pointer keep their original ABI offsets.
        daily_refill_ticket = parsed_daily_refill_ticket;
        gage_upgrade_cost_percent = parsed_gage_upgrade_cost_percent;
        memcpy(gage_up_rates, parsed_gage_up_rates,
               sizeof(parsed_gage_up_rates));
        memcpy(gage_upgrade_costs, parsed_gage_upgrade_costs,
               sizeof(parsed_gage_upgrade_costs));
    }
    return result;
}

}  // namespace advancealtar
