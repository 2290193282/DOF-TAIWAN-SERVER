#include "old_data/PVF/event/seria_room_deco_event_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <utility>


// IDA_EA: 0x8372EF4
// IDA_NAME: _ZN24seriaRoomDecoEventScriptC2Ev
// DEMANGLED: seriaRoomDecoEventScript::seriaRoomDecoEventScript(void)
seriaRoomDecoEventScript::seriaRoomDecoEventScript()
    : using_items(),
      object_max_count(0),
      reward_items(),
      sound_infos(),
      fatigue(0),
      reward_item_mail_expire(0) {
    reward_items.clear();
    using_items.clear();
    sound_infos.clear();
}

// IDA_EA: 0x837C352
// IDA_NAME: _ZN24seriaRoomDecoEventScriptD2Ev
// DEMANGLED: seriaRoomDecoEventScript::~seriaRoomDecoEventScript(void)
seriaRoomDecoEventScript::~seriaRoomDecoEventScript() {}

// IDA_EA: 0x895E4BF
// IDA_NAME: _Z24importSeriaRoomDecoEventP24seriaRoomDecoEventScriptPKc
// DEMANGLED: importSeriaRoomDecoEvent(seriaRoomDecoEventScript *,char const*)
int importSeriaRoomDecoEvent(seriaRoomDecoEventScript* script,
                             const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[object max count]") {
            ScanInt(&script->object_max_count);
        } else if (tag == "[using item]") {
            script->using_items.clear();
            int item = 0;
            while (ScanInt(&item)) {
                script->using_items.push_back(item);
            }
        } else if (tag == "[fatigue]") {
            ScanInt(&script->fatigue);
        } else if (tag == "[reward list]") {
            while (1) {
                int key = 0;
                if (!ScanInt(&key)) {
                    break;
                }
                int value = 0;
                ScanInt(&value);
                script->reward_items.insert(std::make_pair(key, value));
            }
        } else if (tag == "[reward item mail expire]") {
            ScanInt(&script->reward_item_mail_expire);
        } else if (tag == "[sound info]") {
            script->sound_infos.clear();
            seriaRoomDecoEventSoundInfo sound_info;
            int sentinel = 0;
            while (1) {
                sound_info.clear();
                if (!ScanInt(&sentinel)) {
                    break;
                }
                ScanInt(&sound_info.trigger_id);
                ScanInt(&sound_info.object_id);
                ScanStr(&sound_info.sound_path);
                ScanInt(&sound_info.play_type);
                script->sound_infos.push_back(sound_info);
            }
        }
    }

    return 1;
}

// IDA_EA: 0x895E79B
// IDA_NAME: _Z34importSeriaRoomDecoEventObjectListP34seriaRoomDecoEventObjectListScriptPKc
// DEMANGLED: importSeriaRoomDecoEventObjectList(seriaRoomDecoEventObjectListScript *,char const*)
int importSeriaRoomDecoEventObjectList(
    seriaRoomDecoEventObjectListScript* script,
    const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (script == 0) {
        return 0;
    }

    std::string tag;
    std::string random_animation;
    while (ScanType(tag, true) == 1) {
        if (tag != "[event object list]") {
            continue;
        }

        while (ScanType(tag, true) == 1) {
            seriaDecoObjectInfo info;
            int object_index = -1;
            if (tag != "[object index]") {
                continue;
            }

            info.clear();
            ScanInt(&object_index);
            bool reading_object = true;
            while (reading_object && ScanType(tag, true) == 1) {
                if (tag == "[stay animation]") {
                    ScanStr(&info.stay_animation);
                } else if (tag == "[walk animation]") {
                    ScanStr(&info.walk_animation);
                } else if (tag == "[run animation]") {
                    ScanStr(&info.run_animation);
                } else if (tag == "[ramdom animation]") {
                    while (ScanStr(&random_animation) == 1) {
                        info.random_animations.push_back(random_animation);
                    }
                } else if (tag == "[/object index]") {
                    reading_object = false;
                    if (object_index != -1) {
                        script->object_infos.insert(
                            std::make_pair(object_index, info));
                    }
                }
            }
        }
    }
    return 1;
}
