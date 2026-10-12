#ifndef OLDSRV_OLD_DATA_DB_EVENT_ADVANCE_ALTAR_DB_H_
#define OLDSRV_OLD_DATA_DB_EVENT_ADVANCE_ALTAR_DB_H_

#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_data/db/core/db_dispatcher.h"

#include <string>

namespace advancealtar {

// IDA units: 13_database_dispatch/advancealtar/DB_Load and DB_Update.
class DB_Load : public DBDispatcher {
public:
    DB_Load();
    virtual ~DB_Load();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool Select(_CharacAdvanceAltarDbData& data);

    bool parseStage(char* text, _Stage* stages, int count);
    bool parseSlot(char* text, _Slot* slots, int count);
    bool parseBuyItem(char* text, _BuyItem* buy_items, int count);
    bool parseReward(char* text, _Reward* rewards, int count);
    bool SendResult(int signal,
                    int user_idx,
                    _CharacAdvanceAltarDbData& data,
                    bool select_succeeded);
};

class DB_Update : public DBDispatcher {
public:
    DB_Update();
    virtual ~DB_Update();

    static void makeRequest(int user_idx,
                            const _CharacAdvanceAltarDbData& data);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool update(const _CharacAdvanceAltarDbData& data);

    bool makeStageString(std::string& text,
                         const _Stage* stages,
                         int count);
    bool makeSlotString(std::string& text,
                        const _Slot* slots,
                        int count);
    bool makeBuyItemString(std::string& text,
                           const _BuyItem* buy_items,
                           int count);
    bool makeRewardString(std::string& text,
                          const _Reward* rewards,
                          int count);
};

}  // namespace advancealtar

#endif  // OLDSRV_OLD_DATA_DB_EVENT_ADVANCE_ALTAR_DB_H_
