#include "old_data/db/inventory/db_insert_ticket.h"
#include "old_data/db/core/db_mgr.h"
#include "common/manager/global_data.h"

#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

struct SIG_INSERT_TICKET {
    unsigned int charac_no;
    int village;
};

typedef char SIG_INSERT_TICKET_Size_Must_Be_8[
    (sizeof(SIG_INSERT_TICKET) == 8) ? 1 : -1];

// IDA_EA: 0x8403A4E
// IDA_NAME: _ZN15DB_InsertTicketC2Ev
// DEMANGLED: DB_InsertTicket::DB_InsertTicket(void)
DB_InsertTicket::DB_InsertTicket() {}

// IDA_EA: 0x8403A6A
// IDA_NAME: _ZN15DB_InsertTicketD2Ev
// DEMANGLED: DB_InsertTicket::~DB_InsertTicket()
// IDA_EA: 0x8403A9A
// IDA_NAME: _ZN15DB_InsertTicketD0Ev
// DEMANGLED: DB_InsertTicket::~DB_InsertTicket() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertTicket::~DB_InsertTicket() {}

// IDA_EA: 0x8403AB8
// IDA_NAME: _ZN15DB_InsertTicket8dispatchEiiP6Stream
// DEMANGLED: DB_InsertTicket::dispatch(int,int,Stream *)
bool DB_InsertTicket::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_INSERT_TICKET ticket;
    if (stream->get_binary(&ticket, sizeof(ticket)) != 1) {
        return false;
    }

    return InsertTicket(&ticket) == true;
}

// IDA_EA: 0x8403B08
// IDA_NAME: _ZN15DB_InsertTicket12InsertTicketEP17SIG_INSERT_TICKET
// DEMANGLED: DB_InsertTicket::InsertTicket(SIG_INSERT_TICKET *)
bool DB_InsertTicket::InsertTicket(SIG_INSERT_TICKET* ticket) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query("inSert village_ticket values(%u,%d)",
                         ticket->charac_no,
                         ticket->village);
    return db_handle->exec(true) == true;
}
