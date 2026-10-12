#ifndef OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_VALUE_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_VALUE_TYPES_H_

#include <map>
#include <string>
#include <vector>

// PVF Event value records used by the Seria-room RDAR parsers.  These are
// B-owned data types; runtime consumers include this canonical header.

// IDA_EA: 0x83CF9BA, 0x89625E8, 0x8962606, 0x8962E6E.
class seriaRoomDecoAniInfo {
public:
    seriaRoomDecoAniInfo();
    seriaRoomDecoAniInfo(const seriaRoomDecoAniInfo& rhs);
    ~seriaRoomDecoAniInfo();

    seriaRoomDecoAniInfo* clear();

    std::string ani_path;
    int draw_layer;
    int x;
    int y;
    int z;
};

// IDA_EA: 0x83DD54A, 0x89623BA, 0x89623EE, 0x8963240, 0x8963306.
class seriaRoomDecoEventSoundInfo {
public:
    seriaRoomDecoEventSoundInfo();
    seriaRoomDecoEventSoundInfo(const seriaRoomDecoEventSoundInfo& rhs);
    ~seriaRoomDecoEventSoundInfo();

    seriaRoomDecoEventSoundInfo* clear();
    seriaRoomDecoEventSoundInfo& operator=(
        const seriaRoomDecoEventSoundInfo& rhs);

    int trigger_id;
    int object_id;
    std::string sound_path;
    int play_type;
};

// IDA_EA: 0x8962634, 0x8965FCC.
class seriaRoomDecoRewardInfo {
public:
    seriaRoomDecoRewardInfo();
    ~seriaRoomDecoRewardInfo();

    int item_id;
    int item_count;
    int unknown;
};

// IDA_EA: 0x8962422, 0x8962656, 0x89626E8, 0x8962912.
class seriaDecoObjectInfo {
public:
    seriaDecoObjectInfo();
    seriaDecoObjectInfo(const seriaDecoObjectInfo& rhs);
    ~seriaDecoObjectInfo();

    seriaDecoObjectInfo* clear();

    std::string stay_animation;
    std::string walk_animation;
    std::string run_animation;
    std::vector<std::string> random_animations;
};

// The old parser owns this map directly; the class has no separately emitted
// lifecycle function in the original binary.
class seriaRoomDecoEventObjectListScript {
public:
    std::map<int, seriaDecoObjectInfo> object_infos;
};

static_assert(sizeof(seriaRoomDecoAniInfo) == 0x14u,
              "seriaRoomDecoAniInfo IDA ABI size must stay 0x14");
static_assert(sizeof(seriaRoomDecoEventSoundInfo) == 0x10u,
              "seriaRoomDecoEventSoundInfo IDA ABI size must stay 0x10");
static_assert(sizeof(seriaRoomDecoRewardInfo) == 0x0cu,
              "seriaRoomDecoRewardInfo IDA ABI size must stay 0x0c");
static_assert(sizeof(seriaDecoObjectInfo) == 0x18u,
              "seriaDecoObjectInfo IDA ABI size must stay 0x18");
static_assert(sizeof(seriaRoomDecoEventObjectListScript) == 0x18u,
              "seriaRoomDecoEventObjectListScript IDA ABI size must stay 0x18");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_SERIA_ROOM_DECO_VALUE_TYPES_H_
