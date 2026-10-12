#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_DISPATCHER_H_

#include <map>

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/core/taiwan_interface_db_dispatcher.h"


class Stream;


// IDA_EA: 0x815A134.  This is the Taiwan inner-code map embedded at +0x04
// in DB_DispatcherTaiwan.  It owns its concrete database handlers.
class TaiwanDBDispatcher {
public:
    enum InitMode {
        INIT_MODE_LEGACY = 0,
        INIT_MODE_FEATURE_DB = 1,
    };

    TaiwanDBDispatcher();
    ~TaiwanDBDispatcher();

    bool init();
    bool init(InitMode mode);
    TaiwanInterfaceDBDispatcher* getDispatcher(TaiwanInternalPack::T code);
    bool addDispatcher(TaiwanInternalPack::T code,
                       TaiwanInterfaceDBDispatcher* dispatcher);
    bool delDispatcher(TaiwanInternalPack::T code);

private:
    typedef std::map<TaiwanInternalPack::T, TaiwanInterfaceDBDispatcher*>
        DispatcherMap;

    class FnDelete {
    public:
        void operator()(DispatcherMap::value_type& entry);
    };

    DispatcherMap dispatchers_;
};


// IDA_EA: 0x844C9D4.  DBTask slot 846 owns this aggregate dispatcher.  The
// first word of every Taiwan request is therefore the real outer code 846,
// followed by user index and the Taiwan inner-code dword.
class DB_DispatcherTaiwan : public DBDispatcher {
public:
    DB_DispatcherTaiwan();
    explicit DB_DispatcherTaiwan(TaiwanDBDispatcher::InitMode mode);
    virtual ~DB_DispatcherTaiwan();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

private:
    TaiwanDBDispatcher dispatcher_;
};


#if defined(__i386__)
static_assert(sizeof(TaiwanDBDispatcher) == 0x18u,
              "Taiwan DB dispatcher map must retain its original i386 layout");
static_assert(sizeof(DB_DispatcherTaiwan) == 0x1cu,
              "DB_DispatcherTaiwan must retain the original 28-byte ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_DISPATCHER_H_
