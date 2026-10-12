#ifndef OLDSRV_DB_DB_LOAD_TICKET_H_
#define OLDSRV_DB_DB_LOAD_TICKET_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/inventory/load_ticket_payload.h"

// IDA_EA: 0x8419572/0x841958E/0x84195BE/0x84195DC/0x841968A/0x8419770.
// IDA unit: 13_database_dispatch/DB_LoadTicket.
class DB_LoadTicket : public DBDispatcher {
public:
    DB_LoadTicket();
    virtual ~DB_LoadTicket();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool LoadTicket(SIG_LOAD_TICKET* ticket);
    bool SendResult(int signal, int user_idx, SIG_LOAD_TICKET* ticket);
};

#endif  // OLDSRV_DB_DB_LOAD_TICKET_H_
