#include "old_game/activity/reward/advance_altar_timeline_parameter.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <stdio.h>

#include <string>
#include <utility>

namespace advancealtar {

// IDA_EA: 0x8899F26
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter28importStageTimeLineParameterEPKc
// DEMANGLED: advancealtar::StageTimeLineParameter::importStageTimeLineParameter(char const*)
int StageTimeLineParameter::importStageTimeLineParameter(
    const char* filename) {
    std::string tag;
    reset();

    if (loadRDARScriptFile("", filename) != 1) {
        return 0;
    }

    while (ScanType(tag, true) == 1) {
        if (tag == "[summon object]") {
            const int result = importSummonObject();
            if (result) {
                return result;
            }
        } else if (tag == "[action define]") {
            const int result = importActionDefine();
            if (result) {
                return result;
            }
        }
    }

    return 0;
}

// IDA_EA: 0x889A21E
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter18importSummonObjectEv
// DEMANGLED: advancealtar::StageTimeLineParameter::importSummonObject(void)
int StageTimeLineParameter::importSummonObject() {
    bool success = false;
    while (1) {
        _SummonObject object;
        object.summon_object_id = ScanInt(&success);
        if (!success) {
            return 0;
        }

        std::string enemy_text;
        if (ScanStr(&enemy_text) != 1) {
            return 523;
        }
        if (enemy_text == "enemy") {
            object.is_enemy = 1;
        } else if (enemy_text == "friend") {
            object.is_enemy = 0;
        } else {
            printf("advancealtar : SummonObject.isEnemy undefined='%s'\n",
                   enemy_text.c_str());
            return 536;
        }

        std::string type_text;
        if (ScanStr(&type_text) != 1) {
            return 541;
        }
        if (type_text == "normal") {
            object.summon_type = 0;
        } else if (type_text == "named") {
            object.summon_type = 1;
        } else if (type_text == "boss") {
            object.summon_type = 2;
        } else {
            printf("advancealtar : SummonObject.Type undefined='%s'\n",
                   type_text.c_str());
            return 558;
        }

        object.monster_id = ScanInt(&success);
        if (!success) {
            return 562;
        }
        object.count = static_cast<int16_t>(ScanInt(&success));
        if (!success) {
            return 565;
        }
        object.hp_rate = ScanInt(&success);
        if (!success) {
            return 568;
        }
        object.first_x = ScanInt(&success);
        if (!success) {
            return 571;
        }
        object.first_y = ScanInt(&success);
        if (!success) {
            return 574;
        }
        object.direction = ScanInt(&success);
        if (!success) {
            return 577;
        }

        summon_objects.insert(
            std::make_pair(object.summon_object_id, object));
    }
}

// IDA_EA: 0x889A56E
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter18importActionDefineEv
// DEMANGLED: advancealtar::StageTimeLineParameter::importActionDefine(void)
int StageTimeLineParameter::importActionDefine() {
    bool success = false;
    while (1) {
        ActionDefine action_define;
        std::string tag;
        if (ScanType(tag, true) != 1) {
            return 595;
        }

        if (tag == "[/action define]") {
            return 0;
        }

        if (tag == "[normal]" || tag == "[named]" || tag == "[boss]") {
            action_define.action_define_id = ScanInt(&success);
            if (!success) {
                return 619;
            }

            while (1) {
                _Action action;
                action.summon_object_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                std::map<int, _SummonObject>::iterator summon_it =
                    summon_objects.find(action.summon_object_id);
                if (summon_it == summon_objects.end()) {
                    printf("advancealtar : not found SummonObject Id = %d\n",
                           action.summon_object_id);
                    return 634;
                }
                action.summon_data = &summon_it->second;

                action.first_delay_ms = ScanInt(&success);
                if (!success) {
                    return 639;
                }
                if (action.first_delay_ms < 0) {
                    printf("advancealtar :  action.firstDelayMs %d < 0\n",
                           action.first_delay_ms);
                    return 645;
                }

                action.next_delay_ms = ScanInt(&success);
                if (!success) {
                    return 649;
                }
                if (action.next_delay_ms <= 499) {
                    printf("advancealtar :  action.nextDelayMs %d < 500\n",
                           action.next_delay_ms);
                    return 656;
                }

                const int repeat_count = ScanInt(&success);
                if (!success) {
                    return 660;
                }
                if (repeat_count) {
                    action.has_repeat_count = 1;
                    action.repeat_count = static_cast<int16_t>(repeat_count);
                } else {
                    action.has_repeat_count = 0;
                }

                action_define.actions.push_back(action);
            }

            action_defines.insert(
                std::make_pair(action_define.action_define_id,
                               action_define));
        } else {
            printf("advancealtar : ActionDefine.Type undefined='%s'\n",
                   tag.c_str());
            return 615;
        }
    }
}

// IDA_EA: 0x889A930
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter11importPhaseERNS_8TimeLineE
// DEMANGLED: advancealtar::StageTimeLineParameter::importPhase(advancealtar::TimeLine &)
int StageTimeLineParameter::importPhase(TimeLine& target_time_line) {
    bool success = false;
    while (1) {
        std::string tag;
        if (ScanType(tag, true) != 1) {
            return 689;
        }

        if (tag == "[/time line]") {
            return 0;
        }

        if (tag == "[phase]") {
            Phase phase;
            while (1) {
                std::string phase_tag;
                if (ScanType(phase_tag, true) != 1) {
                    return 702;
                }

                if (phase_tag == "[/phase]") {
                    target_time_line.phases.push_back(phase);
                    break;
                }

                if (phase_tag == "[duration]") {
                    phase.duration_ms = ScanInt(&success);
                    if (!success) {
                        return 710;
                    }
                } else if (phase_tag == "[type]") {
                    std::string type_text;
                    if (ScanStr(&type_text) != 1) {
                        return 716;
                    }
                    if (type_text == "normal") {
                        phase.phase_type = 0;
                    } else if (type_text == "named") {
                        phase.phase_type = 1;
                    } else if (type_text == "boss") {
                        phase.phase_type = 2;
                    } else {
                        return 732;
                    }
                } else if (phase_tag == "[action assign]") {
                    while (1) {
                        _ActionAssign action_assign;
                        action_assign.action_define_id = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        action_assign.first_frame =
                            static_cast<int16_t>(ScanInt(&success));
                        if (!success) {
                            return 749;
                        }
                        action_assign.last_frame =
                            static_cast<int16_t>(ScanInt(&success));
                        if (!success) {
                            return 752;
                        }
                        phase.action_assigns.push_back(action_assign);
                    }
                }
            }
        }
    }
}

}  // namespace advancealtar
