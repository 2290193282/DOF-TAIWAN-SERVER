#ifndef OLDSRV_DB_DB_HACK_IP_SAVE_H_
#define OLDSRV_DB_DB_HACK_IP_SAVE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_HackIPSave.
// IDA_EA: dispatch @0x8436DEC; makeRequest @0x8436FF8;
// ctor/dtor @0x844F032/@0x845D216/@0x845D246.
class DB_HackIPSave : public DBDispatcher {
public:
    DB_HackIPSave();
    virtual ~DB_HackIPSave();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(const char* c_class_ip,
                            const char* full_ip,
                            unsigned int hack_type,
                            unsigned int hack_sub_type);
};

#endif  // OLDSRV_DB_DB_HACK_IP_SAVE_H_
