#ifndef OLDSRV_OLD_DATA_DB_CORE_I_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_CORE_I_DISPATCHER_H_


// IDA unit: 13_database_dispatch/IDispatcher.
//
// The old i386 vtable is: dispatch_template (pure), D2, D0.  Keep the
// declaration order because PacketDispatcher stores this interface and calls
// its first slot directly.
class CUser;
class PacketBuf;

class IDispatcher {
public:
    IDispatcher();

    virtual int dispatch_template(CUser* user, PacketBuf& packet) = 0;
    virtual ~IDispatcher();
};


#endif  // OLDSRV_OLD_DATA_DB_CORE_I_DISPATCHER_H_
