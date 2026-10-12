#include "old_game/activity/condition/character_restrict.h"

#include "infra/log/format/trace.h"


char* CharacterRestrict::enumStringCharacters[] = {
    const_cast<char*>("RestrictBegin")
};

typedef char CRestrictCharacCreationEvent_Size_Must_Be_0x10[
    (sizeof(CRestrictCharacCreationEvent) == 0x10) ? 1 : -1];

// IDA_EA: 0x8273C44
// IDA_NAME: _ZN28CRestrictCharacCreationEventC2Ev
// DEMANGLED: CRestrictCharacCreationEvent::CRestrictCharacCreationEvent(void)
CRestrictCharacCreationEvent::CRestrictCharacCreationEvent()
    : CEventBase(),
      limit_create_character(2) {}

// IDA_EA: 0x8273C6A
// IDA_NAME: _ZN28CRestrictCharacCreationEventD2Ev
// DEMANGLED: CRestrictCharacCreationEvent::~CRestrictCharacCreationEvent()
// IDA_EA: 0x8273C9A
// IDA_NAME: _ZN28CRestrictCharacCreationEventD0Ev
// DEMANGLED: CRestrictCharacCreationEvent::~CRestrictCharacCreationEvent() deleting destructor
CRestrictCharacCreationEvent::~CRestrictCharacCreationEvent() {}

// IDA_EA: 0x8273CB8
// IDA_NAME: _ZN28CRestrictCharacCreationEvent10StartEventEv
// DEMANGLED: CRestrictCharacCreationEvent::StartEvent(void)
void CRestrictCharacCreationEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 23, 0);
    trace("Restrict Character Creation Start!");
}

// IDA_EA: 0x8273D0A
// IDA_NAME: _ZN28CRestrictCharacCreationEvent10StartEventE10Word_Param
// DEMANGLED: CRestrictCharacCreationEvent::StartEvent(Word_Param)
void CRestrictCharacCreationEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    limit_create_character = static_cast<uint16_t>(param);
    cMyTrace trace("StartEvent", 35, 0);
    trace("Restrict Character Creation Start!");
}

// IDA_EA: 0x8273D68
// IDA_NAME: _ZN28CRestrictCharacCreationEvent8EndEventEv
// DEMANGLED: CRestrictCharacCreationEvent::EndEvent(void)
void CRestrictCharacCreationEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 43, 0);
    trace("Restrict Character Creation End!");
}

// IDA_EA: 0x8273DBA
// IDA_NAME: _ZNK28CRestrictCharacCreationEvent23getLimitCreateCharacterEi
// DEMANGLED: CRestrictCharacCreationEvent::getLimitCreateCharacter(int)const
int CRestrictCharacCreationEvent::getLimitCreateCharacter(
    int charac_count) const {
    if (limit_create_character != 0) {
        if (limit_create_character == 1) {
            return 8;
        }
    } else if (!charac_count) {
        return 7;
    }
    return 0;
}

// IDA_EA: 0x816D472
// IDA_NAME: _ZN17CharacterRestrict12enumToStringENS_4EnumE
// DEMANGLED: CharacterRestrict::enumToString(CharacterRestrict::Enum)
char* CharacterRestrict::enumToString(CharacterRestrict::Enum value) {
    return enumStringCharacters[value];
}

// CreateCharacterRestrictPolicy bodies migrated to
// old_game/userinfo/character_restrict_policy_runtime.cpp.
