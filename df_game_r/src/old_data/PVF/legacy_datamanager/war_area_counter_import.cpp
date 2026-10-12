#include "old_game/pvp/battlefield/war_area_counter.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>
#include <utility>

// IDA_EA: 0x89026F3
// IDA_NAME: _Z20ImportWarAreaCounterP14WarAreaCounter
// DEMANGLED: ImportWarAreaCounter(WarAreaCounter *)
int ImportWarAreaCounter(WarAreaCounter* counter) {
    if (!loadRDARScriptFile("", "Etc/WarArea.etc")) {
        LogManager::logFormat(0,
                                      "../RDARScriptEtcParameter.cpp",
                                      "ImportWarAreaCounter",
                                      0xDEB,
                                      "CRITICAL ERROR : ");
        return 0;
    }

    std::string tag;
    int time_table_index = 0;
    bool success = false;
    int max_counter = 0;
    int peak_count_index = -1;

    while (ScanType(tag, true) == 1) {
        if (tag == "[time table]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                counter->time_table[time_table_index++] = value;
            }
        } else if (tag == "[door fee]") {
            for (int index = 0;; ++index) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                counter->door_fee[index] = value;
            }
        } else if (tag == "[ban interval]") {
            counter->ban_interval = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[kick point]") {
            counter->kick_point = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[ready count]") {
            counter->ready_count = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[wp penalty]") {
            counter->wp_penalty = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[monster exp const]") {
            counter->monster_exp_const = ScanFloat(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[wararea dungeon]") {
            for (int index = 2;; ++index) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                counter->dungeon_ids[index] = value;
            }
        } else if (tag == "[wp per monster hp]") {
            while (1) {
                stWarPoint war_point;
                const int hp = ScanInt(&success);
                if (!success) {
                    break;
                }

                for (int index = 0; index <= 3; ++index) {
                    war_point.values[index] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                }

                counter->hp_to_wp_points.insert(
                    std::make_pair(hp, war_point));
            }
        } else if (tag == "[counter]") {
            counter->time_table_count = time_table_index;
            time_table_index = 0;
            do {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }

                if (value > max_counter) {
                    max_counter = value;
                    peak_count_index = time_table_index;
                }

                counter->counter[time_table_index++] = value;
            } while (counter->time_table_count != time_table_index);

            counter->peak_count_index = peak_count_index;
        }
    }

    return 1;
}
