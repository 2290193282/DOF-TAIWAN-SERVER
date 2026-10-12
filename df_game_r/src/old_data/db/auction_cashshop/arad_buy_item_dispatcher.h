#ifndef OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_BUY_ITEM_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_BUY_ITEM_DISPATCHER_H_

#include <stddef.h>


class CUser;
class Stream;


namespace ARAD {

class Arad_Dispatcher_Manager;
#if defined(__i386__)
struct AradDispatcherManagerI386LayoutVerifier;
#endif

// Old df_game_r internal DB/internal-return dispatcher base.  It is not the
// separate DBTask-732 ARAD::DISPATCHER map class.
class Arad_Dispatcher {
public:
    // IDA_EA: 0x818EDD0
    // IDA_NAME: _ZN4ARAD15Arad_DispatcherC2Ei
    // DEMANGLED: ARAD::Arad_Dispatcher::Arad_Dispatcher(int)
    explicit Arad_Dispatcher(int dispatch_code);

    // IDA_EA: 0x818E900 / 0x818E926
    // IDA_NAME: _ZN4ARAD15Arad_DispatcherD2Ev / _ZN4ARAD15Arad_DispatcherD0Ev
    // DEMANGLED: ARAD::Arad_Dispatcher::~Arad_Dispatcher()
    virtual ~Arad_Dispatcher();

    // IDA_EA: 0x818E84C
    // IDA_NAME: _ZN4ARAD15Arad_Dispatcher11db_dispatchEiiP6Stream
    // DEMANGLED: ARAD::Arad_Dispatcher::db_dispatch(int,int,Stream *)
    virtual int db_dispatch(int dispatch_code, int user_id, Stream* stream);

    // IDA_EA: 0x818E856
    // IDA_NAME: _ZN4ARAD15Arad_Dispatcher14inter_dispatchEP5CUserPci
    // DEMANGLED: ARAD::Arad_Dispatcher::inter_dispatch(CUser *,char *,int)
    virtual int inter_dispatch(CUser* user, char* payload, int length);

    // IDA_EA: 0x818E860
    // IDA_NAME: _ZN4ARAD15Arad_Dispatcher19register_dispatcherEPS0_
    // DEMANGLED: ARAD::Arad_Dispatcher::register_dispatcher(ARAD::Arad_Dispatcher *)
    int register_dispatcher(Arad_Dispatcher* dispatcher);

    // IDA_EA: 0x818E87C
    // IDA_NAME: _ZN4ARAD15Arad_Dispatcher21unregister_dispatcherEi
    // DEMANGLED: ARAD::Arad_Dispatcher::unregister_dispatcher(int)
    int unregister_dispatcher(int dispatch_code);

    // The original spelling is dispathc_code, and its spelling is ABI.
    // IDA_EA: 0x818E898
    // IDA_NAME: _ZN4ARAD15Arad_Dispatcher13dispathc_codeEv
    // DEMANGLED: ARAD::Arad_Dispatcher::dispathc_code(void)
    int dispathc_code();

private:
    int dispatch_code_;
};


class Arad_Dispatcher_BuyItemCheck_Impl : public Arad_Dispatcher {
public:
    // IDA_EA: 0x818E9CC
    // IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplC2Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::Arad_Dispatcher_BuyItemCheck_Impl(void)
    Arad_Dispatcher_BuyItemCheck_Impl();

    // IDA_EA: 0x818E9F0 / 0x818EA20
    // IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplD2Ev / _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplD0Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::~Arad_Dispatcher_BuyItemCheck_Impl()
    virtual ~Arad_Dispatcher_BuyItemCheck_Impl();

    // IDA_EA: 0x818EB0C
    // IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_Impl11db_dispatchEiiP6Stream
    // DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::db_dispatch(int,int,Stream *)
    virtual int db_dispatch(int dispatch_code, int user_id, Stream* stream);

    // IDA_EA: 0x818EA3E
    // IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_Impl14inter_dispatchEP5CUserPci
    // DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::inter_dispatch(CUser *,char *,int)
    virtual int inter_dispatch(CUser* user, char* payload, int length);
};


// This manager is a private old-service internal transport table.  Code 506
// has no proven startup ingress, so merely providing this class does not
// activate it.
class Arad_Dispatcher_Manager {
public:
    enum { kDispatcherSlots = 852 };

    // IDA_EA: 0x818FDD4
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_ManagerC2Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::Arad_Dispatcher_Manager(void)
    Arad_Dispatcher_Manager();

    // IDA_EA: 0x818FE32
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_ManagerD2Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::~Arad_Dispatcher_Manager()
    ~Arad_Dispatcher_Manager();

    // IDA_EA: 0x818FE38
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager4openEv
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::open(void)
    int open();

    // IDA_EA: 0x818FE98
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager19register_dispatcherEPNS_15Arad_DispatcherE
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::register_dispatcher(ARAD::Arad_Dispatcher *)
    int register_dispatcher(Arad_Dispatcher* dispatcher);

    // IDA_EA: 0x818FED6
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager21unregister_dispatcherEi
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::unregister_dispatcher(int)
    int unregister_dispatcher(int dispatch_code);

    // IDA_EA: 0x818FF6C
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager14inter_dispatchEiP5CUserPci
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::inter_dispatch(int,CUser *,char *,int)
    int inter_dispatch(int dispatch_code,
                       CUser* user,
                       char* payload,
                       int length);

    // IDA_EA: 0x8190080
    // IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager11db_dispatchEiiP6Stream
    // DEMANGLED: ARAD::Arad_Dispatcher_Manager::db_dispatch(int,int,Stream *)
    int db_dispatch(int dispatch_code, int user_id, Stream* stream);

private:
#if defined(__i386__)
    friend struct AradDispatcherManagerI386LayoutVerifier;
#endif
    Arad_Dispatcher* inter_dispatchers_[kDispatcherSlots];
    Arad_Dispatcher* db_dispatchers_[kDispatcherSlots];
};


#if defined(__i386__)
static_assert(sizeof(Arad_Dispatcher) == 0x08u,
              "Arad_Dispatcher must retain the old 8-byte i386 layout");
static_assert(sizeof(Arad_Dispatcher_BuyItemCheck_Impl) == 0x08u,
              "BuyItemCheck dispatcher must add no old-service fields");
struct AradDispatcherManagerI386LayoutVerifier {
    static_assert(sizeof(Arad_Dispatcher_Manager) == 0x1aa0u,
                  "Arad_Dispatcher_Manager must retain its 852-by-two table ABI");
    static_assert(offsetof(Arad_Dispatcher_Manager, db_dispatchers_) == 0xd50u,
                  "DB dispatch table must begin after 852 inter pointers");
};
#endif

}  // namespace ARAD


#endif  // OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_BUY_ITEM_DISPATCHER_H_
