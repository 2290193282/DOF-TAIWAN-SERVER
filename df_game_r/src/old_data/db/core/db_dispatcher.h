#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_DISPATCHER_H_


class Stream;

// IDA unit: 13_database_dispatch/DBDispatcher.
class DBDispatcher {
public:
    DBDispatcher();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    virtual ~DBDispatcher();
};

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_DISPATCHER_H_
