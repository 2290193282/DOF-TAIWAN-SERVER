#ifndef OLDSRV_DB_DB_INSERT_TICKET_H_
#define OLDSRV_DB_DB_INSERT_TICKET_H_

#include "old_data/db/core/db_dispatcher.h"


struct SIG_INSERT_TICKET;

// IDA unit: 13_database_dispatch/DB_InsertTicket.
class DB_InsertTicket : public DBDispatcher {
public:
    DB_InsertTicket();
    virtual ~DB_InsertTicket();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool InsertTicket(SIG_INSERT_TICKET* ticket);
};

#endif  // OLDSRV_DB_DB_INSERT_TICKET_H_
