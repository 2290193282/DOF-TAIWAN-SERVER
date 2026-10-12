#include "old_data/db/core/db_terminate_server.h"


// IDA_EA: 0x841A284
// IDA_NAME: _ZN18DB_TerminateServerC2Ev
// DEMANGLED: DB_TerminateServer::DB_TerminateServer(void)
DB_TerminateServer::DB_TerminateServer() {}

// IDA_EA: 0x841A2A0
// IDA_NAME: _ZN18DB_TerminateServerD2Ev
// DEMANGLED: DB_TerminateServer::~DB_TerminateServer()
// IDA_EA: 0x841A2D0
// IDA_NAME: _ZN18DB_TerminateServerD0Ev
// DEMANGLED: DB_TerminateServer::~DB_TerminateServer() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_TerminateServer::~DB_TerminateServer() {}

// IDA_EA: 0x841A2EE
// IDA_NAME: _ZN18DB_TerminateServer8dispatchEiiP6Stream
// DEMANGLED: DB_TerminateServer::dispatch(int,int,Stream *)
bool DB_TerminateServer::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}
