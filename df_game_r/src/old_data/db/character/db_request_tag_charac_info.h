#ifndef OLDSRV_DB_DB_REQUEST_TAG_CHARAC_INFO_H_
#define OLDSRV_DB_DB_REQUEST_TAG_CHARAC_INFO_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/character/tag_charac_info_payload.h"

#include <stdint.h>


// IDA unit: 04_character_select_create/DB_RequestTagCharacInfo.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at signal 766.
class DB_RequestTagCharacInfo : public DBDispatcher {
public:
    DB_RequestTagCharacInfo();
    virtual ~DB_RequestTagCharacInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    void sendResult(int user_idx, SIG_REQUEST_TAG_CHARAC_INFO* payload);
};

#endif  // OLDSRV_DB_DB_REQUEST_TAG_CHARAC_INFO_H_
