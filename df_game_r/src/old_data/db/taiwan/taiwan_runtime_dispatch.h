#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RUNTIME_DISPATCH_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RUNTIME_DISPATCH_H_

#include <map>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/concurrent/thread/thread_interface.h"


class CUser;


// IDA_EA: 0x84BBAFA.  The original is a 852-entry direct routing table.
// The table stays physically complete even while unrelated, unrecovered
// leaves remain null and take the original "get_dispatcher" failure route.
class InterDispatcher {
public:
    enum { kDispatcherCount = 852, kTaiwanDispatcherSlot = 846 };

    InterDispatcher();
    ~InterDispatcher();

    ISigDispatcher* get_dispatcher(int code);
    InterDispatcher* register_dispatcher(int code, ISigDispatcher* dispatcher);
    int dispatch(int code, int user_idx, char* payload, int payload_size);

private:
    ISigDispatcher* dispatchers_[kDispatcherCount];
};


class InterLoadServiceRestrictPolicy : public TaiwanInterfaceInterDispatcher {
public:
    InterLoadServiceRestrictPolicy();
    virtual ~InterLoadServiceRestrictPolicy();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class InterLoadGeolocation : public TaiwanInterfaceInterDispatcher {
public:
    InterLoadGeolocation();
    virtual ~InterLoadGeolocation();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


// IDA's TaiwanInternalDispatcher is a 24-byte std::map owner embedded in the
// outer 28-byte ISigDispatcher at +4.  It owns the ten typed leaf handlers;
// unknown inner codes retain the original diagnostic/failure behavior.
class TaiwanInternalDispatcher {
public:
    TaiwanInternalDispatcher();
    ~TaiwanInternalDispatcher();

    bool init();
    TaiwanInterfaceInterDispatcher* getDispatcher(TaiwanInternalPack::T code);
    bool addDispatcher(TaiwanInternalPack::T code,
                       TaiwanInterfaceInterDispatcher* dispatcher);
    bool delDispatcher(TaiwanInternalPack::T code);

private:
    typedef std::map<TaiwanInternalPack::T, TaiwanInterfaceInterDispatcher*>
        DispatcherMap;

    struct FnDelete {
        void operator()(std::pair<const TaiwanInternalPack::T,
                                  TaiwanInterfaceInterDispatcher*>& entry);
    };

    DispatcherMap dispatchers_;
};


// IDA_EA: 0x84E8BBA.  Outer InterDispatcher slot 846 receives a Taiwan
// inner-code dword then forwards the rest of the response to this map.
class Inter_DispatcherTaiwan : public ISigDispatcher {
public:
    Inter_DispatcherTaiwan();
    virtual ~Inter_DispatcherTaiwan();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

private:
    TaiwanInternalDispatcher dispatcher_;
};


// IDA_EA: 0x847E268 / 0x847E2D2.  This is the real DISPATCH_Q worker core;
// it decodes the two leading i386 integers and releases the source stream to
// its original pool after routing.
class DispatchThread : public ThreadInterface {
public:
    DispatchThread();
    virtual ~DispatchThread();
    // External state keeps the recovered 0x0c layout intact.  Dispatch is a
    // continuation consumer during shutdown and is stopped only after the
    // DB_Q/queue6 causal closure has completed.
    void requestStopAfterDrain();
    virtual void dispatch(void* user_data);
};


// Restores the DB + dispatcher subset of create_thread that owns all Taiwan
// startup requests.  It is invoked only after Init has completed the original
// startup registrations, exactly before the server-ready notice.
bool StartTaiwanDatabaseRuntime();
void StopTaiwanDatabaseRuntime();


#if defined(__i386__)
static_assert(sizeof(InterDispatcher) == 0xD50u,
              "InterDispatcher must retain its original 852-pointer layout");
static_assert(sizeof(TaiwanInternalDispatcher) == 0x18u,
              "Taiwan internal dispatcher map must retain its i386 layout");
static_assert(sizeof(Inter_DispatcherTaiwan) == 0x1Cu,
              "Inter_DispatcherTaiwan must retain the old 28-byte ABI");
static_assert(sizeof(DispatchThread) == 0x0Cu,
              "DispatchThread must only contain the ThreadInterface base");
#endif

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_RUNTIME_DISPATCH_H_
