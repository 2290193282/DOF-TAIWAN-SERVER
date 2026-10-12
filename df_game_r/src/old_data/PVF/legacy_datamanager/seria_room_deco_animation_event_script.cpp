#include "old_data/PVF/event/datamanager/seria_room_deco_animation_event_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>


// IDA_EA: 0x83731CA
// IDA_NAME: _ZN27seriaRoomDecoAniEventScriptC2Ev
// DEMANGLED: seriaRoomDecoAniEventScript::seriaRoomDecoAniEventScript(void)
seriaRoomDecoAniEventScript::seriaRoomDecoAniEventScript()
    : reward_items(),
      using_items(),
      animations(),
      finish_sound(),
      finish_animations_by_layer() {
    clear();
}

// IDA_EA: 0x83732A4
// IDA_NAME: _ZN27seriaRoomDecoAniEventScript5clearEv
// DEMANGLED: seriaRoomDecoAniEventScript::clear(void)
int seriaRoomDecoAniEventScript::clear() {
    reward_items.clear();
    using_items.clear();
    animations.clear();
    finish_animations_by_layer.clear();
    finish_sound.clear();
    return 0;
}

// IDA_EA: 0x837C46A
// IDA_NAME: _ZN27seriaRoomDecoAniEventScriptD2Ev
// DEMANGLED: seriaRoomDecoAniEventScript::~seriaRoomDecoAniEventScript()
seriaRoomDecoAniEventScript::~seriaRoomDecoAniEventScript() {}

// IDA_EA: 0x8960861
// IDA_NAME: _Z33importSeriaRoomAnimationDecoEventP27seriaRoomDecoAniEventScriptPKc
// DEMANGLED: importSeriaRoomAnimationDecoEvent(seriaRoomDecoAniEventScript *,char const*)
int importSeriaRoomAnimationDecoEvent(seriaRoomDecoAniEventScript* script,
                                      const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    std::string tag;
    std::string draw_layer_name;

    while (ScanType(tag, true) == 1) {
        if (tag == "[using item]") {
            script->using_items.clear();
            int item_id = 0;
            int item_count = 0;
            while (ScanInt(&item_id) == 1) {
                if (ScanInt(&item_count) != 1) {
                    return 0;
                }
                script->using_items.insert(
                    std::make_pair(item_id, item_count));
            }
        } else if (tag == "[reward item]") {
            script->reward_items.clear();
            while (1) {
                seriaRoomDecoRewardInfo reward_info;
                if (ScanInt(&reward_info.item_id) != 1) {
                    break;
                }
                if (ScanInt(&reward_info.item_count) != 1) {
                    return 0;
                }
                if (ScanInt(&reward_info.unknown) != 1) {
                    return 0;
                }
                script->reward_items.push_back(reward_info);
            }
        } else if (tag == "[ani list]") {
            script->animations.clear();
            seriaRoomDecoAniInfo ani_info;
            int animation_id = 0;
            while (1) {
                ani_info.clear();
                if (ScanInt(&animation_id) != 1) {
                    break;
                }
                if (ScanStr(&ani_info.ani_path) != 1) {
                    return 0;
                }
                if (ScanStr(&draw_layer_name) != 1) {
                    return 0;
                }
                ani_info.draw_layer = getDrawLayer(draw_layer_name.c_str());
                if (ScanInt(&ani_info.x) != 1) {
                    return 0;
                }
                if (ScanInt(&ani_info.y) != 1) {
                    return 0;
                }
                if (ScanInt(&ani_info.z) != 1) {
                    return 0;
                }
                script->animations.insert(
                    std::make_pair(animation_id, ani_info));
            }
        } else if (tag == "[finish ani]") {
            seriaRoomDecoAniInfo ani_info;
            std::string layer_tag;
            while (1) {
                ani_info.clear();
                if (ScanStr(&layer_tag) != 1) {
                    break;
                }

                seriaRoomDecoAniEventScript::aniLayer::T layer =
                    seriaRoomDecoAniEventScript::aniLayer::FRONT;
                if (layer_tag == "[front]") {
                    layer = seriaRoomDecoAniEventScript::aniLayer::FRONT;
                } else if (layer_tag == "[back]") {
                    layer = seriaRoomDecoAniEventScript::aniLayer::BACK;
                } else {
                    return 0;
                }

                if (ScanStr(&ani_info.ani_path) != 1) {
                    return 0;
                }
                if (ScanStr(&draw_layer_name) != 1) {
                    return 0;
                }
                ani_info.draw_layer = getDrawLayer(draw_layer_name.c_str());
                if (ScanInt(&ani_info.x) != 1) {
                    return 0;
                }
                if (ScanInt(&ani_info.y) != 1) {
                    return 0;
                }
                if (ScanInt(&ani_info.z) != 1) {
                    return 0;
                }
                script->finish_animations_by_layer.insert(
                    std::make_pair(layer, ani_info));
            }
        } else if (tag == "[finish sound]") {
            ScanStr(&script->finish_sound);
        }
    }

    return 1;
}
