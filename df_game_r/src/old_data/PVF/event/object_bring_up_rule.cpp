#include "old_data/PVF/event/object_bring_up_rule.h"

#include "old_data/PVF/event/action_mail_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <utility>
#include <vector>


typedef char ObjectBringUpRule_Size_Must_Be_0x0c[
    (sizeof(ObjectBringUpScript::ObjectBringUpRule) == 0x0c) ? 1 : -1];

// IDA_EA: 0x8985DD4
// IDA_NAME: _ZN19ObjectBringUpScript17ObjectBringUpRuleC2Ev
// DEMANGLED: ObjectBringUpScript::ObjectBringUpRule::ObjectBringUpRule(void)
ObjectBringUpScript::ObjectBringUpRule::ObjectBringUpRule() {
    reset();
}

// IDA_EA: 0x8985DE8
// IDA_NAME: _ZN19ObjectBringUpScript17ObjectBringUpRule5resetEv
// DEMANGLED: ObjectBringUpScript::ObjectBringUpRule::reset(void)
ObjectBringUpScript::ObjectBringUpRule* ObjectBringUpScript::ObjectBringUpRule::reset() {
    value_00 = -1;
    value_04 = -1;
    value_08 = -1;
    return this;
}

// IDA_EA: 0x8983E48
// IDA_NAME: _Z30importObjectBringUpEventScriptP19ObjectBringUpScriptPKc
// DEMANGLED: importObjectBringUpEventScript(ObjectBringUpScript *,char const*)
int importObjectBringUpEventScript(ObjectBringUpScript* script,
                                   const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->value_08);
        } else if (tag == "[material item]") {
            if (ScanType(tag, true) != 1) {
                return 0;
            }
            bool scan_material_items = true;
            if (tag == "[exchange rule]") {
                script->flag_0c = static_cast<uint8_t>(ScanInt(&success));
                scan_material_items = success;
            }
            if (scan_material_items) {
                while (1) {
                    const int value = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script->values_10.push_back(value);
                }
            }
        } else if (tag == "[eventitem send]") {
            std::vector<unsigned int> fatigues;
            while (ScanType(tag, true) == 1) {
                if (tag == "[get eventitem fatigue]") {
                    while (1) {
                        const int value = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        fatigues.push_back(static_cast<unsigned int>(value));
                    }
                } else if (tag == "[send mail]") {
                    Action_SendMail mail;
                    mail.reset();
                    success = importActionMailScript(&mail) != 0;
                    if (success) {
                        script->mail_rules_1c.push_back(
                            std::make_pair(fatigues, mail));
                        fatigues.clear();
                    } else {
                        break;
                    }
                } else if (tag == "[/eventitem send]") {
                    break;
                }
            }
        } else if (tag == "[send]") {
            ObjectBringUpScript::ObjectBringUpRule rule;
            while (ScanType(tag, true) == 1) {
                if (tag == "[reward start count]") {
                    rule.value_00 = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[reward end count]") {
                    rule.value_04 = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[reward mutiple number]") {
                    rule.value_08 = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[send mail]") {
                    Action_SendMail mail;
                    mail.reset();
                    success = importActionMailScript(&mail) != 0;
                    if (success) {
                        if (rule.value_00 != -1 && rule.value_04 != -1 &&
                            rule.value_08 != -1) {
                            script->bring_up_rules_28.push_back(
                                std::make_pair(rule, mail));
                        }
                        rule.reset();
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return success ? 1 : 0;
}
