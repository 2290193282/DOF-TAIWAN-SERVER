#ifndef OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_EVENT_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_EVENT_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/event/seria_room_deco_value_types.h"

class seriaRoomDecoEventScript {
public:
    seriaRoomDecoEventScript();
    ~seriaRoomDecoEventScript();

    std::vector<int> using_items;
    int32_t object_max_count;
    std::map<int, int> reward_items;
    std::vector<seriaRoomDecoEventSoundInfo> sound_infos;
    int32_t fatigue;
    int32_t reward_item_mail_expire;

private:
    seriaRoomDecoEventScript(const seriaRoomDecoEventScript&);
    seriaRoomDecoEventScript& operator=(const seriaRoomDecoEventScript&);
};

int importSeriaRoomDecoEvent(seriaRoomDecoEventScript* script,
                             const char* filename);

// IDA_EA: 0x895E79B
// IDA_NAME: _Z34importSeriaRoomDecoEventObjectListP34seriaRoomDecoEventObjectListScriptPKc
// DEMANGLED: importSeriaRoomDecoEventObjectList(seriaRoomDecoEventObjectListScript *,char const*)
int importSeriaRoomDecoEventObjectList(
    seriaRoomDecoEventObjectListScript* script,
    const char* filename);

static_assert(sizeof(seriaRoomDecoEventScript) == 0x3Cu,
              "seriaRoomDecoEventScript IDA ABI size must stay 0x3C");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_EVENT_SCRIPT_H_
