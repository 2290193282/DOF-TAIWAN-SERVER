#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_USER_STATE_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_USER_STATE_RUNTIME_H_

#include "old_data/db/character/load_etc_payload.h"
#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// IDA unit: 99_misc_global/TOD_UserState.
// Character expand-data kind 13.  Its complete 0x20-byte persisted state is
// the exact 0x18-byte SIG_REQUEST_TOD_USER_STATE payload after CData.
class TOD_UserState : public charac_expand::CData {
public:
    // IDA_EA: 0x8643460
    // IDA_NAME: _ZN13TOD_UserStateC2Ev
    // DEMANGLED: TOD_UserState::TOD_UserState(void)
    TOD_UserState();

    // IDA_EA: 0x86434AE
    // IDA_NAME: _ZN13TOD_UserStateD2Ev
    // DEMANGLED: TOD_UserState::~TOD_UserState()
    // IDA_EA: 0x86434DE
    // IDA_NAME: _ZN13TOD_UserStateD0Ev
    // DEMANGLED: TOD_UserState::~TOD_UserState()
    virtual ~TOD_UserState();

    // IDA_EA: 0x86434FC
    // IDA_NAME: _ZN13TOD_UserState8loadDataEP5CUserPc
    // DEMANGLED: TOD_UserState::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x86435CA
    // IDA_NAME: _ZNK13TOD_UserState7getDataEPc
    // DEMANGLED: TOD_UserState::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x8643624
    // IDA_NAME: _ZN13TOD_UserState10ResetDailyEv
    // DEMANGLED: TOD_UserState::ResetDaily(void)
    virtual void ResetDaily();

    // IDA_EA: 0x864383E
    // IDA_NAME: _ZNK13TOD_UserState18getTodayEnterLayerEv
    // DEMANGLED: TOD_UserState::getTodayEnterLayer(void)const
    TOD_Layer getTodayEnterLayer() const;

    // IDA_EA: 0x8643872
    // IDA_NAME: _ZNK13TOD_UserState13getEnterCountEv
    // DEMANGLED: TOD_UserState::getEnterCount(void)const
    int getEnterCount() const;

    // IDA_EA: 0x864387E
    // IDA_NAME: _ZNK13TOD_UserState16getLastClearTimeEv
    // DEMANGLED: TOD_UserState::getLastClearTime(void)const
    int getLastClearTime() const;

    // IDA_EA: 0x864388A
    // IDA_NAME: _ZN13TOD_UserState18IncreaseEnterCountEv
    // DEMANGLED: TOD_UserState::IncreaseEnterCount(void)
    charac_expand::CData* IncreaseEnterCount();

    // IDA_EA: 0x86438FC
    // IDA_NAME: _ZN13TOD_UserState13setEnterLayerERK9TOD_Layer
    // DEMANGLED: TOD_UserState::setEnterLayer(TOD_Layer const&)
    TOD_UserState* setEnterLayer(const TOD_Layer& layer);

    // IDA_EA: 0x864390E
    // IDA_NAME: _ZN13TOD_UserState10ClearLayerEv
    // DEMANGLED: TOD_UserState::ClearLayer(void)
    int ClearLayer();

    // IDA_EA: 0x8643942
    // IDA_NAME: _ZNK13TOD_UserState17getLastEnterLayerEv
    // DEMANGLED: TOD_UserState::getLastEnterLayer(void)const
    TOD_Layer getLastEnterLayer() const;

protected:
    // IDA_EA: 0x8643642
    // IDA_NAME: _ZN13TOD_UserState9_saveDataEP5CUser
    // DEMANGLED: TOD_UserState::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x86437EC
    // IDA_NAME: _ZN13TOD_UserState6_resetEv
    // DEMANGLED: TOD_UserState::_reset(void)
    virtual int _reset();

private:
    SIG_REQUEST_TOD_USER_STATE state_;  // IDA +0x08, size 0x18
};

static_assert(sizeof(TOD_UserState) == 0x20u,
              "TOD_UserState IDA ABI size must stay 0x20");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_USER_STATE_RUNTIME_H_
