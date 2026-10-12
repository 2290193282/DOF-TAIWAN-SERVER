#ifndef OLDSRV_CHARACTER_SELECT_GAME_MASTER_CHARACTER_H_
#define OLDSRV_CHARACTER_SELECT_GAME_MASTER_CHARACTER_H_


class CUser;

// IDA unit: 04_character_select_create/CGameMasterCharacter.
class CGameMasterCharacter {
public:
    CGameMasterCharacter();

    virtual void sendServerState(CUser* user, const char* state);
};

// IDA unit: 04_character_select_create/CGameMasterCharacterItemDropRate.
class CGameMasterCharacterItemDropRate : public CGameMasterCharacter {
public:
    CGameMasterCharacterItemDropRate();

    virtual void sendServerState(CUser* user, const char* state);
};


#endif  // OLDSRV_CHARACTER_SELECT_GAME_MASTER_CHARACTER_H_
