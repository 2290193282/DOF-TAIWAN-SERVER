#ifndef OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_HANDLER_H_
#define OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_HANDLER_H_
class CUser;

// IDA unit: 04_character_select_create/CEventCharacterHandler.
// This stateless event service owns the one-time level/growth/quest/skill/item
// transition performed during the first world entry of an event character.
class CEventCharacterHandler {
public:
    // IDA_EA: 0x848E29E
    // IDA_NAME: _ZN22CEventCharacterHandlerC2Ev
    // DEMANGLED: CEventCharacterHandler::CEventCharacterHandler(void)
    CEventCharacterHandler();

    // IDA_EA: 0x848E2A4
    // IDA_NAME: _ZN22CEventCharacterHandler19_makeEventCharacterEP5CUseri
    int _makeEventCharacter(CUser* user, int target_level);

    // IDA_EA: 0x848E636
    // IDA_NAME: _ZN22CEventCharacterHandler22_UpdateEventCharacInfoEP5CUser
    int _UpdateEventCharacInfo(CUser* user);

    // IDA_EA: 0x848E652
    // IDA_NAME: _ZN22CEventCharacterHandler16_SetClearedQuestEP5CUser
    char _SetClearedQuest(CUser* user);

    // IDA_EA: 0x848E7D6
    // IDA_NAME: _ZN22CEventCharacterHandler15_MasterNewSkillEP5CUserii
    int _MasterNewSkill(CUser* user, int skill_index, int target_level);

    // IDA_EA: 0x848E8E2
    // IDA_NAME: _ZN22CEventCharacterHandler8_AddItemEP5CUsermii
    int _AddItem(CUser* user,
                 unsigned long item_index,
                 int count,
                 int upgrade);

    // IDA_EA: 0x848EA50
    // IDA_NAME: _ZN22CEventCharacterHandler12_SetGrowTypeEP5CUserii
    int _SetGrowType(CUser* user, int grow_type_kind, int grow_type);

    // IDA_EA: 0x848EADE
    // IDA_NAME: _ZN22CEventCharacterHandler9_SetLevelEP5CUseri
    int _SetLevel(CUser* user, int target_level);

    // IDA_EA: 0x848EBD6
    // IDA_NAME: _ZN22CEventCharacterHandler17_checkMakeCommandEP5CUseriRi
    int _checkMakeCommand(CUser* user, int target_level, int& event_job);
};

// IDA_EA: 0x848E258
// IDA_NAME: _Z30CEventCharacterHandlerInstancev
// DEMANGLED: CEventCharacterHandlerInstance(void)
CEventCharacterHandler* CEventCharacterHandlerInstance();


#endif  // OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_HANDLER_H_
