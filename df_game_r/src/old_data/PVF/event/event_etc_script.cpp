#include "old_data/PVF/event/event_etc_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>

namespace {

bool LastEventEtcTokenIs(const char* expected) {
    return g_dnfLex.getLastToken() == DNF_TOK_TYPE &&
           g_dnfLex.getLastTokenDataAsString() == expected;
}

bool SkipIntPairSection(const char* closing_tag, int expected_pairs) {
    int first = 0;
    int pair_count = 0;
    while (ScanInt(&first)) {
        int second = 0;
        if (!ScanInt(&second)) {
            return false;
        }
        ++pair_count;
    }
    return pair_count == expected_pairs && LastEventEtcTokenIs(closing_tag);
}

bool SkipAnimationSection(const char* closing_tag, int expected_entries) {
    int animation_index = 0;
    int entry_count = 0;
    while (ScanInt(&animation_index)) {
        std::string animation_path;
        if (ScanStr(&animation_path) != 1 || animation_path.empty()) {
            return false;
        }
        ++entry_count;
    }
    return entry_count == expected_entries &&
           LastEventEtcTokenIs(closing_tag);
}

}  // namespace

// IDA_EA: 0x8372E66
// IDA_NAME: _ZN14EventEtcScriptC2Ev
// DEMANGLED: EventEtcScript::EventEtcScript(void)
EventEtcScript::EventEtcScript() {
    clear();
}

// IDA_EA: 0x8372ECE
// IDA_NAME: _ZN14EventEtcScript5clearEv
// DEMANGLED: EventEtcScript::clear(void)
void EventEtcScript::clear() {
    event_ids.clear();
    event_ids.reserve(7u);
}

// IDA_EA: 0x837DDA0
// IDA_NAME: _ZN14EventEtcScriptD2Ev
// DEMANGLED: EventEtcScript::~EventEtcScript()
EventEtcScript::~EventEtcScript() {}

// IDA_EA: 0x895E30F
// IDA_NAME: _Z20importEventEtcScriptP14EventEtcScriptPKc
// DEMANGLED: importEventEtcScript(EventEtcScript *,char const*)
int importEventEtcScript(EventEtcScript* script,
                         const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[christmas event start]") {
            continue;
        }
        if (tag == "[seria room tree material list]") {
            int item_index = -1;
            while (ScanInt(&item_index)) {
                script->event_ids.push_back(item_index);
            }
            if (!LastEventEtcTokenIs("[/seria room tree material list]")) {
                return 0;
            }
        } else if (tag == "[seria room deco material list]") {
            int item_index = 0;
            int ordinal = 0;
            while (ScanInt(&item_index)) {
                script->values.insert(std::make_pair(
                    static_cast<unsigned int>(item_index),
                    static_cast<unsigned int>(ordinal)));
                ++ordinal;
            }
            if (!LastEventEtcTokenIs("[/seria room deco material list]")) {
                return 0;
            }
        } else if (tag == "[each npc deco index matching]") {
            if (!SkipIntPairSection("[/each npc deco index matching]", 16)) {
                return 0;
            }
        } else if (tag == "[npc deco material list]") {
            if (!SkipIntPairSection("[/npc deco material list]", 15)) {
                return 0;
            }
        } else if (tag == "[deco animation list]") {
            if (!SkipAnimationSection("[/deco animation list]", 32)) {
                return 0;
            }
        } else if (tag == "[christmas event end]") {
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}
