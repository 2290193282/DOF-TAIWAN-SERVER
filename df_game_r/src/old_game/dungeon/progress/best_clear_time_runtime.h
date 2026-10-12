#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_BEST_CLEAR_TIME_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_BEST_CLEAR_TIME_RUNTIME_H_

#include <map>

#include "old_data/db/dungeon/best_clear_time_payload.h"
#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// IDA unit: 99_misc_global/BestClearTime.
// charac_expand::CDataMgr slot 18 owns the persisted dungeon best-time map.
class BestClearTime : public charac_expand::CData {
public:
    // IDA_EA: 0x80C851C
    // IDA_NAME: _ZN13BestClearTimeC2Ev
    // DEMANGLED: BestClearTime::BestClearTime(void)
    BestClearTime();

    // IDA_EA: 0x80C8596
    // IDA_NAME: _ZN13BestClearTimeD2Ev
    // DEMANGLED: BestClearTime::~BestClearTime()
    // IDA_EA: 0x80C85F8
    // IDA_NAME: _ZN13BestClearTimeD0Ev
    // DEMANGLED: BestClearTime::~BestClearTime()
    virtual ~BestClearTime();

    // IDA_EA: 0x80C8776
    // IDA_NAME: _ZN13BestClearTime8loadDataEP5CUserPc
    // DEMANGLED: BestClearTime::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x80C883C
    // IDA_NAME: _ZNK13BestClearTime7getDataEPc
    // DEMANGLED: BestClearTime::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x80C8928
    // IDA_NAME: _ZN13BestClearTime16setBestClearTimeEiii
    // DEMANGLED: BestClearTime::setBestClearTime(int,int,int)
    void setBestClearTime(int dungeon_id, int difficulty, int clear_time);

    // IDA_EA: 0x80C8AC0
    // IDA_NAME: _ZN13BestClearTime16getBestClearTimeEii
    // DEMANGLED: BestClearTime::getBestClearTime(int,int)
    int getBestClearTime(int dungeon_id, int difficulty);

    // IDA_EA: 0x80C8B44
    // IDA_NAME: _ZN13BestClearTime12setNewRecordEb
    // DEMANGLED: BestClearTime::setNewRecord(bool)
    void setNewRecord(bool new_record);

    // IDA_EA: 0x80C8B5C
    // IDA_NAME: _ZN13BestClearTime14checkNewRecordEv
    // DEMANGLED: BestClearTime::checkNewRecord(void)
    bool checkNewRecord();

protected:
    // IDA_EA: 0x80C8616
    // IDA_NAME: _ZN13BestClearTime9_saveDataEP5CUser
    // DEMANGLED: BestClearTime::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x80C8758
    // IDA_NAME: _ZN13BestClearTime6_resetEv
    // DEMANGLED: BestClearTime::_reset(void)
    virtual int _reset();

private:
    typedef std::map<int, SIG_BEST_CLEAR_TIME_DIFFICULTY> BestTimeMap;

    BestTimeMap best_times_;  // IDA +0x08
    bool new_record_;         // IDA +0x20
};

static_assert(sizeof(BestClearTime) == 0x24u,
              "BestClearTime IDA ABI size must stay 0x24");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_BEST_CLEAR_TIME_RUNTIME_H_
