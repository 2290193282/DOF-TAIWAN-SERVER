#include "old_game/activity/schedule/event_character_info.h"


typedef char STEventCharacterItem_Size_Must_Be_0x0c[
    (sizeof(STEventCharacterItem) == 0x0c) ? 1 : -1];
typedef char STEventCharacterInfo_Size_Must_Be_0x1c[
    (sizeof(STEventCharacterInfo) == 0x1c) ? 1 : -1];
typedef char STEventCharacterInfo_level_Size_Must_Be_0x18[
    (sizeof(STEventCharacterInfo_level) == 0x18) ? 1 : -1];

// IDA_EA: 0x895CDF8
// IDA_NAME: _ZN20STEventCharacterItemD2Ev
// DEMANGLED: STEventCharacterItem::~STEventCharacterItem()
STEventCharacterItem::~STEventCharacterItem() {}

// IDA_EA: 0x83CE3EE
// IDA_NAME: _ZN20STEventCharacterInfoD2Ev
// DEMANGLED: STEventCharacterInfo::~STEventCharacterInfo()
STEventCharacterInfo::~STEventCharacterInfo() {}

// IDA_EA: 0x848ECCE
// IDA_NAME: _ZN20STEventCharacterInfoC2Ev
// DEMANGLED: STEventCharacterInfo::STEventCharacterInfo(void)
STEventCharacterInfo::STEventCharacterInfo()
    : grow_type(0),
      awakening_type(0),
      pad_02(),
      skills(),
      items() {}

// IDA_EA: 0x895B8FE
// IDA_NAME: _ZN20STEventCharacterInfoaSERKS_
// DEMANGLED: STEventCharacterInfo::operator=(STEventCharacterInfo const&)
STEventCharacterInfo& STEventCharacterInfo::operator=(
    const STEventCharacterInfo& other) {
    grow_type = other.grow_type;
    awakening_type = other.awakening_type;
    pad_02[0] = other.pad_02[0];
    pad_02[1] = other.pad_02[1];
    skills = other.skills;
    items = other.items;
    return *this;
}

// IDA_EA: 0x895BA32
// IDA_NAME: _ZN20STEventCharacterInfoC2ERKS_
// DEMANGLED: STEventCharacterInfo::STEventCharacterInfo(STEventCharacterInfo const&)
STEventCharacterInfo::STEventCharacterInfo(
    const STEventCharacterInfo& other)
    : grow_type(other.grow_type),
      awakening_type(other.awakening_type),
      pad_02(),
      skills(other.skills),
      items(other.items) {
    pad_02[0] = other.pad_02[0];
    pad_02[1] = other.pad_02[1];
}

// IDA_EA: 0x83CE4EA
// IDA_NAME: _ZN26STEventCharacterInfo_levelD2Ev
// DEMANGLED: STEventCharacterInfo_level::~STEventCharacterInfo_level()
STEventCharacterInfo_level::~STEventCharacterInfo_level() {}

// IDA_EA: 0x895B890
// IDA_NAME: _ZN26STEventCharacterInfo_levelC2Ev
// DEMANGLED: STEventCharacterInfo_level::STEventCharacterInfo_level(void)
STEventCharacterInfo_level::STEventCharacterInfo_level()
    : infos_by_job() {}

// IDA_EA: 0x895BB54
// IDA_NAME: _ZN26STEventCharacterInfo_levelC2ERKS_
// DEMANGLED: STEventCharacterInfo_level::STEventCharacterInfo_level(STEventCharacterInfo_level const&)
STEventCharacterInfo_level::STEventCharacterInfo_level(
    const STEventCharacterInfo_level& other)
    : infos_by_job(other.infos_by_job) {}
