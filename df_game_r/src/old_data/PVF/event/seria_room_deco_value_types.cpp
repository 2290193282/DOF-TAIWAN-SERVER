#include "old_data/PVF/event/seria_room_deco_value_types.h"


// IDA_EA: 0x83CF9BA
// IDA_NAME: _ZN20seriaRoomDecoAniInfoD2Ev
// DEMANGLED: seriaRoomDecoAniInfo::~seriaRoomDecoAniInfo()
seriaRoomDecoAniInfo::~seriaRoomDecoAniInfo() {}

// IDA_EA: 0x89625E8
// IDA_NAME: _ZN20seriaRoomDecoAniInfoC2Ev
// DEMANGLED: seriaRoomDecoAniInfo::seriaRoomDecoAniInfo(void)
seriaRoomDecoAniInfo::seriaRoomDecoAniInfo()
    : ani_path() {
    clear();
}

// IDA_EA: 0x8962606
// IDA_NAME: _ZN20seriaRoomDecoAniInfo5clearEv
// DEMANGLED: seriaRoomDecoAniInfo::clear(void)
seriaRoomDecoAniInfo* seriaRoomDecoAniInfo::clear() {
    draw_layer = 7;
    x = 0;
    y = 0;
    z = 0;
    return this;
}

// IDA_EA: 0x8962E6E
// IDA_NAME: _ZN20seriaRoomDecoAniInfoC2ERKS_
// DEMANGLED: seriaRoomDecoAniInfo::seriaRoomDecoAniInfo(seriaRoomDecoAniInfo const&)
seriaRoomDecoAniInfo::seriaRoomDecoAniInfo(
    const seriaRoomDecoAniInfo& rhs)
    : ani_path(rhs.ani_path),
      draw_layer(rhs.draw_layer),
      x(rhs.x),
      y(rhs.y),
      z(rhs.z) {}

// IDA_EA: 0x83DD54A
// IDA_NAME: _ZN27seriaRoomDecoEventSoundInfoD2Ev
// DEMANGLED: seriaRoomDecoEventSoundInfo::~seriaRoomDecoEventSoundInfo()
seriaRoomDecoEventSoundInfo::~seriaRoomDecoEventSoundInfo() {}

// IDA_EA: 0x89623BA
// IDA_NAME: _ZN27seriaRoomDecoEventSoundInfoC2Ev
// DEMANGLED: seriaRoomDecoEventSoundInfo::seriaRoomDecoEventSoundInfo(void)
seriaRoomDecoEventSoundInfo::seriaRoomDecoEventSoundInfo()
    : trigger_id(0),
      object_id(0),
      sound_path(),
      play_type(0) {}

// IDA_EA: 0x89623EE
// IDA_NAME: _ZN27seriaRoomDecoEventSoundInfo5clearEv
// DEMANGLED: seriaRoomDecoEventSoundInfo::clear(void)
seriaRoomDecoEventSoundInfo* seriaRoomDecoEventSoundInfo::clear() {
    sound_path.clear();
    trigger_id = 0;
    object_id = 0;
    play_type = 0;
    return this;
}

// IDA_EA: 0x8963240
// IDA_NAME: _ZN27seriaRoomDecoEventSoundInfoC2ERKS_
// DEMANGLED: seriaRoomDecoEventSoundInfo::seriaRoomDecoEventSoundInfo(seriaRoomDecoEventSoundInfo const&)
seriaRoomDecoEventSoundInfo::seriaRoomDecoEventSoundInfo(
    const seriaRoomDecoEventSoundInfo& rhs)
    : trigger_id(rhs.trigger_id),
      object_id(rhs.object_id),
      sound_path(rhs.sound_path),
      play_type(rhs.play_type) {}

// IDA_EA: 0x8963306
// IDA_NAME: _ZN27seriaRoomDecoEventSoundInfoaSERKS_
// DEMANGLED: seriaRoomDecoEventSoundInfo::operator=(seriaRoomDecoEventSoundInfo const&)
seriaRoomDecoEventSoundInfo& seriaRoomDecoEventSoundInfo::operator=(
    const seriaRoomDecoEventSoundInfo& rhs) {
    trigger_id = rhs.trigger_id;
    object_id = rhs.object_id;
    sound_path = rhs.sound_path;
    play_type = rhs.play_type;
    return *this;
}

// IDA_EA: 0x8962634
// IDA_NAME: _ZN23seriaRoomDecoRewardInfoC2Ev
// DEMANGLED: seriaRoomDecoRewardInfo::seriaRoomDecoRewardInfo(void)
seriaRoomDecoRewardInfo::seriaRoomDecoRewardInfo()
    : item_id(0),
      item_count(0),
      unknown(0) {}

// IDA_EA: 0x8965FCC
// IDA_NAME: _ZN23seriaRoomDecoRewardInfoD2Ev
// DEMANGLED: seriaRoomDecoRewardInfo::~seriaRoomDecoRewardInfo()
seriaRoomDecoRewardInfo::~seriaRoomDecoRewardInfo() {}

// IDA_EA: 0x8962656
// IDA_NAME: _ZN19seriaDecoObjectInfoC2Ev
// DEMANGLED: seriaDecoObjectInfo::seriaDecoObjectInfo(void)
seriaDecoObjectInfo::seriaDecoObjectInfo()
    : stay_animation(),
      walk_animation(),
      run_animation(),
      random_animations() {}

// IDA_EA: 0x8962912
// IDA_NAME: _ZN19seriaDecoObjectInfoC2ERKS_
// DEMANGLED: seriaDecoObjectInfo::seriaDecoObjectInfo(seriaDecoObjectInfo const&)
seriaDecoObjectInfo::seriaDecoObjectInfo(const seriaDecoObjectInfo& rhs)
    : stay_animation(rhs.stay_animation),
      walk_animation(rhs.walk_animation),
      run_animation(rhs.run_animation),
      random_animations(rhs.random_animations) {}

// IDA_EA: 0x89626E8
// IDA_NAME: _ZN19seriaDecoObjectInfoD2Ev
// DEMANGLED: seriaDecoObjectInfo::~seriaDecoObjectInfo(void)
seriaDecoObjectInfo::~seriaDecoObjectInfo() {}

// IDA_EA: 0x8962422
// IDA_NAME: _ZN19seriaDecoObjectInfo5clearEv
// DEMANGLED: seriaDecoObjectInfo::clear(void)
seriaDecoObjectInfo* seriaDecoObjectInfo::clear() {
    stay_animation.clear();
    walk_animation.clear();
    run_animation.clear();
    random_animations.clear();
    return this;
}
