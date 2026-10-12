#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_LIVE_EVENT_DATA_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_LIVE_EVENT_DATA_RUNTIME_H_

#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// IDA unit: 09_dungeon_pvp_party/CPvPLiveEventData.
// CDataMgr slot 3 owns this persisted PvP live-event play counter.
class CPvPLiveEventData : public charac_expand::CData {
public:
    // IDA_EA: 0x8269054
    // IDA_NAME: _ZN17CPvPLiveEventDataC2Ev
    // DEMANGLED: CPvPLiveEventData::CPvPLiveEventData(void)
    CPvPLiveEventData();

    // IDA_EA: 0x8269070
    // IDA_NAME: _ZN17CPvPLiveEventDataD2Ev
    // DEMANGLED: CPvPLiveEventData::~CPvPLiveEventData()
    // IDA_EA: 0x82690A0
    // IDA_NAME: _ZN17CPvPLiveEventDataD0Ev
    // DEMANGLED: CPvPLiveEventData::~CPvPLiveEventData()
    virtual ~CPvPLiveEventData();

    // IDA_EA: 0x826948E
    // IDA_NAME: _ZN17CPvPLiveEventData18ResetDailyMidnightEv
    // DEMANGLED: CPvPLiveEventData::ResetDailyMidnight(void)
    virtual void ResetDailyMidnight();

    // IDA_EA: 0x82690BE
    // IDA_NAME: _ZN17CPvPLiveEventData8loadDataEP5CUserPc
    // DEMANGLED: CPvPLiveEventData::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x8269206
    // IDA_NAME: _ZNK17CPvPLiveEventData7getDataEPc
    // DEMANGLED: CPvPLiveEventData::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x826921C
    // IDA_NAME: _ZN17CPvPLiveEventData17IncreasePlayCountEP5CUser
    // DEMANGLED: CPvPLiveEventData::IncreasePlayCount(CUser *)
    bool IncreasePlayCount(CUser* user);

protected:
    // IDA_EA: 0x82690D4
    // IDA_NAME: _ZN17CPvPLiveEventData9_saveDataEP5CUser
    // DEMANGLED: CPvPLiveEventData::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x826920C
    // IDA_NAME: _ZN17CPvPLiveEventData6_resetEv
    // DEMANGLED: CPvPLiveEventData::_reset(void)
    virtual int _reset();

private:
    int play_count_;  // IDA +0x08
};

static_assert(sizeof(CPvPLiveEventData) == 0x0cu,
              "CPvPLiveEventData IDA ABI size must stay 0x0c");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_LIVE_EVENT_DATA_RUNTIME_H_
