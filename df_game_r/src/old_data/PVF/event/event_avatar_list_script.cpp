#include "old_data/PVF/event/event_avatar_list_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>

// IDA_EA: 0x8372E10
// IDA_NAME: _ZN21EventAvatarListScriptC2Ev
// DEMANGLED: EventAvatarListScript::EventAvatarListScript(void)
EventAvatarListScript::EventAvatarListScript() {
    clear();
}

// IDA_EA: 0x8372E52
// IDA_NAME: _ZN21EventAvatarListScript5clearEv
// DEMANGLED: EventAvatarListScript::clear(void)
int EventAvatarListScript::clear() {
    avatar_indexes.clear();
    return 0;
}

// IDA_EA: 0x837DD8C
// IDA_NAME: _ZN21EventAvatarListScriptD2Ev
// DEMANGLED: EventAvatarListScript::~EventAvatarListScript()
EventAvatarListScript::~EventAvatarListScript() {}

// IDA_EA: 0x895E17C
// IDA_NAME: _ZN21EventAvatarListScript17isfindEventAvatarEi
// DEMANGLED: EventAvatarListScript::isfindEventAvatar(int)
int EventAvatarListScript::isfindEventAvatar(int avatar_index) {
    if (avatar_indexes.empty()) {
        return 0;
    }
    return avatar_indexes.find(avatar_index) != avatar_indexes.end();
}

// IDA_EA: 0x895E1ED
// IDA_NAME: _Z27importEventAvatarListScriptP21EventAvatarListScriptPKc
// DEMANGLED: importEventAvatarListScript(EventAvatarListScript *,char const*)
int importEventAvatarListScript(EventAvatarListScript* script,
                                const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();
    std::string tag;
    bool success = false;
    bool saw_start = false;
    bool in_avatar_list = false;
    bool saw_avatar_list = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[event avatar list start]") {
            if (saw_start || in_avatar_list) {
                return 0;
            }
            saw_start = true;
        } else if (tag == "[avatar list]") {
            if (!saw_start || in_avatar_list || saw_avatar_list) {
                return 0;
            }
            in_avatar_list = true;
            saw_avatar_list = true;
            while (true) {
                bool is_type = false;
                std::string closing_tag;
                const int avatar_index = ScanTypeOrInt(
                    is_type, closing_tag, success, true);
                if (!success) {
                    return 0;
                }
                if (is_type) {
                    if (closing_tag != "[/avatar list]") {
                        return 0;
                    }
                    in_avatar_list = false;
                    break;
                }
                script->avatar_indexes.insert(avatar_index);
            }
        } else if (tag == "[/avatar list]") {
            if (!in_avatar_list) {
                return 0;
            }
            in_avatar_list = false;
        } else if (tag == "[event avatar list end]") {
            success = saw_start && saw_avatar_list && !in_avatar_list;
            break;
        } else {
            return 0;
        }
    }
    return success ? 1 : 0;
}
