#ifndef OLDSRV_DB_DB_UPDATE_AVATAR_JEWEL_SLOT_H_
#define OLDSRV_DB_DB_UPDATE_AVATAR_JEWEL_SLOT_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 04_character_select_create/DB_UpdateAvatarJewelSlot.
class DB_UpdateAvatarJewelSlot : public DBDispatcher {
public:
    // IDA_EA: 0x844EC96.
    DB_UpdateAvatarJewelSlot();
    // IDA_EA: 0x845DAEC / 0x845DB1C.
    virtual ~DB_UpdateAvatarJewelSlot();

    // IDA_EA: 0x843077C.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA_EA: 0x843081C.
    static void makeRequest(unsigned int charac_no, unsigned int avatar_ui_id,
                            const stAvatarEmblemInfo_t* jewel_socket);
};

#endif  // OLDSRV_DB_DB_UPDATE_AVATAR_JEWEL_SLOT_H_
