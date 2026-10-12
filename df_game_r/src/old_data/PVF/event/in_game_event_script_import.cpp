#include "old_data/PVF/event/in_game_event_script_import.h"
#include "old_data/PVF/event/action_mail_script.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <utility>

// IDA_EA: 0x8981539
// IDA_NAME: _Z22importAttendanceScriptP16AttendanceScriptPKc
// DEMANGLED: importAttendanceScript(AttendanceScript *,char const*)
int importAttendanceScript(AttendanceScript* script,
                           const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        }
        if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->db_table_name);
        } else if (tag == "[fatigue]") {
            script->fatigue_condition = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[reward explain]") {
            ScanStr(&script->attend_mail_text);
        } else if (tag == "[final reward explain]") {
            ScanStr(&script->final_mail_text);
        } else if (tag == "[reward]") {
            while (true) {
                const int day = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->reward_items_by_day.push_back(
                    std::make_pair(day, item_id));
            }
        } else if (tag == "[final day condition]") {
            script->final_day_condition = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[final reward]") {
            script->final_reward_item.first = ScanInt(static_cast<bool*>(0));
            script->final_reward_item.second = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[send mail]") {
            (void)importActionMailScript(&script->mail_data);
        } else if (tag == "[msg overbtn]") {
            ScanStr(&script->mail_title);
        }
    }

    return success ? 1 : 0;
}

// IDA_EA: 0x8981E0F
// IDA_NAME: _Z36importExchangeRandomItemRewardScriptP30ExchangeRandomItemRewardScriptPKc
// DEMANGLED: importExchangeRandomItemRewardScript(ExchangeRandomItemRewardScript *,char const*)
int importExchangeRandomItemRewardScript(
    ExchangeRandomItemRewardScript* script,
    const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        }
        if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[material]") {
            if (ScanType(tag, true) != 1) {
                return 0;
            }
            bool scan_pairs = true;
            if (tag == "[exchange rule]") {
                script->enabled = static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    scan_pairs = false;
                }
            }
            if (scan_pairs) {
                while (true) {
                    const int first = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int second = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script->exchange_items.push_back(
                        std::make_pair(first, second));
                }
            }
        } else if (tag == "[random reward item]") {
            while (true) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->reward_items.push_back(item_id);
            }
        } else if (tag == "[send mail]") {
            (void)importActionMailScript(&script->send_mail);
        }
    }

    return success ? 1 : 0;
}
