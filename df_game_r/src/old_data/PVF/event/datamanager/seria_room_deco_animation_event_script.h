#ifndef OLDSRV_DATAMANAGER_SERIA_ROOM_DECO_ANIMATION_EVENT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_SERIA_ROOM_DECO_ANIMATION_EVENT_SCRIPT_H_

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/event/seria_room_deco_value_types.h"

class seriaRoomDecoAniEventScript {
public:
    struct aniLayer {
        enum T {
            FRONT = 0,
            BACK = 1
        };
    };

    seriaRoomDecoAniEventScript();
    ~seriaRoomDecoAniEventScript();

    int clear();

    std::vector<seriaRoomDecoRewardInfo> reward_items;
    std::map<int, int> using_items;
    std::map<int, seriaRoomDecoAniInfo> animations;
    std::string finish_sound;
    std::map<aniLayer::T, seriaRoomDecoAniInfo> finish_animations_by_layer;

private:
    seriaRoomDecoAniEventScript(const seriaRoomDecoAniEventScript&);
    seriaRoomDecoAniEventScript& operator=(
        const seriaRoomDecoAniEventScript&);
};

int importSeriaRoomAnimationDecoEvent(seriaRoomDecoAniEventScript* script,
                                      const char* filename);

static_assert(sizeof(seriaRoomDecoAniEventScript) == 0x58u,
              "seriaRoomDecoAniEventScript IDA ABI size must stay 0x58");

#endif  // OLDSRV_DATAMANAGER_SERIA_ROOM_DECO_ANIMATION_EVENT_SCRIPT_H_
