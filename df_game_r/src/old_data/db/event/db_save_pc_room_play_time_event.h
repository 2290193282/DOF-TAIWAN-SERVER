#ifndef OLDSRV_DB_DB_SAVE_PC_ROOM_PLAY_TIME_EVENT_H_
#define OLDSRV_DB_DB_SAVE_PC_ROOM_PLAY_TIME_EVENT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 09_dungeon_pvp_party/DB_SavePcRoomPlayTimeEvent.
class DB_SavePcRoomPlayTimeEvent : public DBDispatcher {
public:
    DB_SavePcRoomPlayTimeEvent();
    virtual ~DB_SavePcRoomPlayTimeEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int total_play_time,
                            unsigned char reward_count);
};

#endif  // OLDSRV_DB_DB_SAVE_PC_ROOM_PLAY_TIME_EVENT_H_
