#include "old_data/db/inventory/db_load_ticket.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>



// IDA_EA: 0x8419572
// IDA_NAME: _ZN13DB_LoadTicketC2Ev
// DEMANGLED: DB_LoadTicket::DB_LoadTicket(void)
DB_LoadTicket::DB_LoadTicket() {}

// IDA_EA: 0x841958E
// IDA_NAME: _ZN13DB_LoadTicketD2Ev
// DEMANGLED: DB_LoadTicket::~DB_LoadTicket()
// IDA_EA: 0x84195BE
// IDA_NAME: _ZN13DB_LoadTicketD0Ev
// DEMANGLED: DB_LoadTicket::~DB_LoadTicket() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadTicket::~DB_LoadTicket() {}

// IDA_EA: 0x84195DC
// IDA_NAME: _ZN13DB_LoadTicket8dispatchEiiP6Stream
// DEMANGLED: DB_LoadTicket::dispatch(int,int,Stream *)
bool DB_LoadTicket::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_LOAD_TICKET ticket;
    if (stream->get_binary(&ticket, sizeof(ticket)) != 1) {
        return false;
    }

    if (LoadTicket(&ticket) != true) {
        ticket.result = 1;
    }
    return SendResult(signal, user_idx, &ticket) == true;
}

// IDA_EA: 0x841968A
// IDA_NAME: _ZN13DB_LoadTicket10LoadTicketEP15SIG_LOAD_TICKET
// DEMANGLED: DB_LoadTicket::LoadTicket(SIG_LOAD_TICKET *)
bool DB_LoadTicket::LoadTicket(SIG_LOAD_TICKET* ticket) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect village from village_ticket where charac_no=%u",
                         ticket->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    int n99 = 0;
    for (; n99 <= 99 && db_handle->fetch() == true; ++n99) {
        if (db_handle->get_short(0, ticket->villages[n99]) != true) {
            return false;
        }
    }
    ticket->count = n99;
    return true;
}

// IDA_EA: 0x8419770
// IDA_NAME: _ZN13DB_LoadTicket10SendResultEiiP15SIG_LOAD_TICKET
// DEMANGLED: DB_LoadTicket::SendResult(int,int,SIG_LOAD_TICKET *)
bool DB_LoadTicket::SendResult(int signal, int user_idx,
                               SIG_LOAD_TICKET* ticket) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 19223);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(ticket, sizeof(*ticket));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
