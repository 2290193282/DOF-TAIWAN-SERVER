#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_RESTRICT_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_RESTRICT_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8273DBA in the companion source.


#include <stdint.h>

#include <vector>

#include "old_game/activity/reward/event_base.h"


// IDA unit: 04_character_select_create/CRestrictCharacCreationEvent.
class CRestrictCharacCreationEvent : public CEventBase {
public:
    CRestrictCharacCreationEvent();
    virtual ~CRestrictCharacCreationEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int getLimitCreateCharacter(int charac_count) const;

private:
    int limit_create_character;
};

// IDA unit: 04_character_select_create/CharacterRestrict.
struct CharacterRestrict {
    enum Enum {
        RestrictBegin = 0
    };

    static char* enumToString(Enum value);

    static char* enumStringCharacters[];
};

// CreateCharacterRestrictPolicy is owned by
// old_game/userinfo/character_restrict_policy_runtime.cpp.  The legacy
// character-selection header deliberately no longer declares a divergent
// non-virtual layout.


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_RESTRICT_H_
