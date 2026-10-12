#ifndef OLDSRV_DB_SIG_DISPATCHER_H_
#define OLDSRV_DB_SIG_DISPATCHER_H_


// IDA unit: 13_database_dispatch/ISigDispatcher.
//
// i386 vtable address point 0x8B2DB88:
//   +0x00 __cxa_pure_virtual for dispatch_sig(CUser *, char *, int)
//   +0x04 ISigDispatcher::D2
//   +0x08 ISigDispatcher::D0
//
// Keep dispatch_sig ahead of the destructor.  InterDispatcher calls slot 0
// directly when routing a database reply; declaring only a virtual destructor
// moves the ABI by one slot and turns every such reply into a destructor call.
class CUser;

class ISigDispatcher {
public:
    ISigDispatcher();

    virtual int dispatch_sig(CUser* user, char* payload,
                             int payload_size) = 0;
    virtual ~ISigDispatcher();
};

#endif  // OLDSRV_DB_SIG_DISPATCHER_H_
