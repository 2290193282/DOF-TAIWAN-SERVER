#ifndef OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_LOGIN_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_LOGIN_DISPATCHER_H_

#include "old_data/db/account/db_login.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/auction_cashshop/arad_buy_item_dispatcher.h"

#include <vector>

namespace ARAD {

class Arad_Dispatcher_Login_Impl : public Arad_Dispatcher {
public:
    // IDA_EA: 0x818EED0
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_ImplC2Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::Arad_Dispatcher_Login_Impl()
    Arad_Dispatcher_Login_Impl();

    // IDA_EA: 0x818EEF4 / 0x818EF24
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_ImplD2Ev / _ZN4ARAD26Arad_Dispatcher_Login_ImplD0Ev
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::~Arad_Dispatcher_Login_Impl()
    virtual ~Arad_Dispatcher_Login_Impl();

    // IDA_EA: 0x818EF4C
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl11db_dispatchEiiP6Stream
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::db_dispatch(int,int,Stream *)
    virtual int db_dispatch(int dispatch_code, int user_idx, Stream* stream);

    // IDA_EA: 0x818EF42
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl14inter_dispatchEP5CUserPci
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::inter_dispatch(CUser *,char *,int)
    virtual int inter_dispatch(CUser* user, char* payload, int length);

    // IDA_EA: 0x818F1CA
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl16premium_info_setEP14SIG_LOGIN_DATARSt6vectorIN8DB_Login16tagDNFEventEntryESaIS5_EEi
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::premium_info_set(SIG_LOGIN_DATA *,std::vector<DB_Login::tagDNFEventEntry,std::allocator<DB_Login::tagDNFEventEntry>> &,int)
    int premium_info_set(
        SIG_LOGIN_DATA* login,
        std::vector<DB_Login::tagDNFEventEntry>& event_entries,
        int user_idx);

    // IDA_EA: 0x818F4A6
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl23pad_certifiaction_checkEP14SIG_LOGIN_DATA
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::pad_certifiaction_check(SIG_LOGIN_DATA *)
    static int pad_certifiaction_check(SIG_LOGIN_DATA* login);

    // IDA_EA: 0x818F746
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl22account_password_checkEP14SIG_LOGIN_DATAPc
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::account_password_check(SIG_LOGIN_DATA *,char *)
    int account_password_check(SIG_LOGIN_DATA* login, char* password);

    // IDA_EA: 0x818F970
    // IDA_NAME: _ZN4ARAD26Arad_Dispatcher_Login_Impl17punish_user_checkEP14SIG_LOGIN_DATA
    // DEMANGLED: ARAD::Arad_Dispatcher_Login_Impl::punish_user_check(SIG_LOGIN_DATA *)
    int punish_user_check(SIG_LOGIN_DATA* login);
};

#if defined(__i386__)
static_assert(sizeof(Arad_Dispatcher_Login_Impl) == 0x08u,
              "Login dispatcher must add no old-service fields");
#endif

}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_LOGIN_DISPATCHER_H_
