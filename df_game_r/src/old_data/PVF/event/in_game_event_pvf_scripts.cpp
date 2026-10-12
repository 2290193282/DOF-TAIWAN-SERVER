#include "old_data/PVF/event/in_game_event_pvf_scripts.h"

#include "old_data/PVF/event/object_bring_up_rule.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/event/action_mail_script.h"
#include "common/manager/global_tools.h"

#include <utility>


typedef char AddItemCountDBScript_Size_Must_Be_0x24[
    (sizeof(AddItemCountDBScript) == 0x24) ? 1 : -1];
typedef char FatigueQuantityScript_Size_Must_Be_0x10[
    (sizeof(FatigueQuantityScript) == 0x10) ? 1 : -1];
typedef char ObjectBringUpScript_Size_Must_Be_0x34[
    (sizeof(ObjectBringUpScript) == 0x34) ? 1 : -1];
typedef char UsedFatigueGiveItemScript_Size_Must_Be_0x64[
    (sizeof(UsedFatigueGiveItemScript) == 0x64) ? 1 : -1];
typedef char PcBangPlayTimeScript_Size_Must_Be_0x18[
    (sizeof(PcBangPlayTimeScript) == 0x18) ? 1 : -1];

// IDA_EA: 0x8986134
// IDA_NAME: _ZN20AddItemCountDBScriptC2Ev
// DEMANGLED: AddItemCountDBScript::AddItemCountDBScript(void)
AddItemCountDBScript::AddItemCountDBScript()
    : InGameEventScript(), values_08(), values_14(), value_20() {}

// IDA_EA: 0x8985C74
// IDA_NAME: _ZN21FatigueQuantityScriptC2Ev
// DEMANGLED: FatigueQuantityScript::FatigueQuantityScript(void)
FatigueQuantityScript::FatigueQuantityScript()
    : InGameEventScript(), value_08(), flag_0c(0) {
    value_08.clear();
}

// IDA_EA: 0x8985E0A
// IDA_NAME: _ZN19ObjectBringUpScriptC2Ev
// DEMANGLED: ObjectBringUpScript::ObjectBringUpScript(void)
ObjectBringUpScript::ObjectBringUpScript()
    : InGameEventScript(),
      value_08(),
      flag_0c(0),
      values_10(),
      mail_rules_1c(),
      bring_up_rules_28() {
    value_08.clear();
}

// IDA_EA: 0x8985EEC
// IDA_NAME: _ZN25UsedFatigueGiveItemScriptC2Ev
// DEMANGLED: UsedFatigueGiveItemScript::UsedFatigueGiveItemScript(void)
UsedFatigueGiveItemScript::UsedFatigueGiveItemScript()
    : InGameEventScript(),
      value_08(),
      flag_0c(0),
      value_10(0),
      flag_14(0),
      value_18(-1),
      value_1c(-1),
      send_mail_20() {
    value_08.clear();
    send_mail_20.reset();
}

// IDA_EA: 0x8985FB8
// IDA_NAME: _ZN20PcBangPlayTimeScriptC2Ev
// DEMANGLED: PcBangPlayTimeScript::PcBangPlayTimeScript(void)
PcBangPlayTimeScript::PcBangPlayTimeScript()
    : InGameEventScript(),
      value_08(),
      value_0c(0),
      flag_10(1),
      value_12(32),
      value_14(32) {
    value_08.clear();
}

// IDA_EA: 0x897FC99
// IDA_NAME: _Z31importAddItemCountDBEventScriptP20AddItemCountDBScriptPKc
// DEMANGLED: importAddItemCountDBEventScript(AddItemCountDBScript *,char const*)
int importAddItemCountDBEventScript(AddItemCountDBScript* script,
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
            ScanStr(&script->value_20);
        } else if (tag == "[item index]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->values_08.push_back(value);
            }
        } else if (tag == "[quest index]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->values_14.push_back(value);
            }
        }
    }

    return success ? 1 : 0;
}

// IDA_EA: 0x8982FC6
// IDA_NAME: _Z32importFatigueQuantityEventScriptP21FatigueQuantityScriptPKc
// DEMANGLED: importFatigueQuantityEventScript(FatigueQuantityScript *,char const*)
int importFatigueQuantityEventScript(FatigueQuantityScript* script,
                                     const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->value_08);
        } else if (tag == "[user type]") {
            script->flag_0c =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
        }
    }

    return 0;
}

// IDA_EA: 0x898491B
// IDA_NAME: _Z36importUsedFatigueGiveItemEventScriptP25UsedFatigueGiveItemScriptPKc
// DEMANGLED: importUsedFatigueGiveItemEventScript(UsedFatigueGiveItemScript *,char const*)
int importUsedFatigueGiveItemEventScript(UsedFatigueGiveItemScript* script,
                                         const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = true;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return success ? 1 : 0;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->value_08);
        } else if (tag == "[user type]") {
            script->flag_0c = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[used fatigue]") {
            script->value_10 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[charc event]") {
            script->flag_14 = 1;
        } else if (tag == "[apply job type]") {
            std::string job_text;
            ScanStr(&job_text);
            script->value_18 = ::getCharacterJobToEnumValue(job_text);
        } else if (tag == "[used fatigue limit]") {
            script->value_1c = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[send mail]") {
            if (importActionMailScript(&script->send_mail_20) != 1) {
                success = false;
            }
        }
    }

    return success ? 1 : 0;
}

// IDA_EA: 0x898512C
// IDA_NAME: _Z26importPcBangPlayTimeScriptP20PcBangPlayTimeScriptPKc
// DEMANGLED: importPcBangPlayTimeScript(PcBangPlayTimeScript *,char const*)
int importPcBangPlayTimeScript(PcBangPlayTimeScript* script,
                               const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->value_08);
        } else if (tag == "[play time]") {
            script->value_0c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[number of time]") {
            script->flag_10 =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[start tag]") {
            script->value_12 =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[end tag]") {
            script->value_14 =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        }
    }

    return 0;
}
