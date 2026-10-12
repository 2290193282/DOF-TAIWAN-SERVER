#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_SERIA_ROOM_ANIMATION_DECO_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_SERIA_ROOM_ANIMATION_DECO_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/DB_UpdateSeriaRoomAnimationDeco.
// IDA_EA: dispatch @0x844ADC0; makeRequest @0x844AF74; ctor/dtor
// @0x844FEBE/@0x845ABB2/@0x845ABE2. DBTask slot 798.
class DB_UpdateSeriaRoomAnimationDeco : public DBDispatcher {
public:
    DB_UpdateSeriaRoomAnimationDeco();
    virtual ~DB_UpdateSeriaRoomAnimationDeco();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            short server_id,
                            int charac_no,
                            short receive_flow,
                            short deco_flow);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_SERIA_ROOM_ANIMATION_DECO_H_
