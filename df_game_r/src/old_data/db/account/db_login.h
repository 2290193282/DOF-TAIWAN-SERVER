#ifndef OLDSRV_DB_DB_LOGIN_H_
#define OLDSRV_DB_DB_LOGIN_H_

#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>
#include <vector>


class MySQL;

// IDA_EA: 0x840F48E/0x840F4AA/0x840F4DA/0x840F4F8/0x840F51C.
// IDA unit: 13_database_dispatch/DB_Login, reached from the 02 login-account
// DB flow. Signal 3 is registered in DBTask; the old SSO/password/token path
// and passpad conversion path are restored. Encrypted user-info decrypt now
// follows the old CRijndael/DNFFLib path through local old-symbol bindings;
// formal public CRijndael ownership remains tracked in type_requests_B.md.
class DB_Login : public DBDispatcher {
public:
    struct tagDNFEventEntry {
        // The old vector allocator copies all six words and allocates 0x18
        // bytes per element.  Both event loaders leave +0x04 unwritten; do
        // not assign it a made-up business meaning or initialize it.
        int32_t event_id;       // +0x00
        uint32_t opaque_04;     // +0x04, deliberately uninitialized
        int32_t occ_time;       // +0x08
        int32_t server_id;      // +0x0C
        int32_t charac_no;      // +0x10
        int32_t obtain_time;    // +0x14

        ~tagDNFEventEntry();
    };

    typedef std::vector<tagDNFEventEntry> vecDNFEventEntry_t;

    DB_Login();
    virtual ~DB_Login();

    MySQL* _getGameDB(ENUM_DB_HANDLE_IDX handle_idx);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool getCharacLinkSystemData(SIG_LOGIN_DATA* payload);
    bool getBreakAwayMemberData(SIG_LOGIN_DATA* payload);
    bool GetLoginCommonData(SIG_LOGIN_DATA* payload);
    bool GetReturnUserInfo(SIG_LOGIN_DATA* payload);
    bool GetDailyCleanPadPoint(SIG_LOGIN_DATA* payload);
    bool GetPunishUserInfo(SIG_LOGIN_DATA* payload);
    bool _isJoinedSchoolPointEvent(unsigned int account_id);
    bool _isAutoPunishedFirstUser(unsigned int account_id);
    bool _getAutoPunishedSecondUserData(unsigned int account_id,
                                        unsigned int& trade_count,
                                        unsigned long long& trade_gold);
    int _getSecurityCardInfo(SIG_LOGIN_DATA* payload);
    int _getPassPadInfo(SIG_LOGIN_DATA* payload);
    int _getRewardInfo(SIG_LOGIN_DATA* payload);
    int _convertPassPadInfo(SIG_LOGIN_DATA* payload);
    MySQL* _GetDNFAccountDB();
    bool _LoadDNFEventEntry(unsigned int account_id,
                            vecDNFEventEntry_t& entries);
    bool _LoadEventItem(SIG_LOGIN_DATA* payload,
                        int event_id,
                        vecDNFEventEntry_t& entries);
    void GetPremiumInfo(int user_idx,
                        SIG_LOGIN_DATA* payload,
                        vecDNFEventEntry_t& entries);
    bool RemovePreminuInfo(SIG_LOGIN_DATA* payload,
                           std::vector<int>& event_ids);
    bool _IsPunishedUser(unsigned int account_id);
    bool _isPunishedHackUser(unsigned int account_id);
    bool IsPCRoomIP(char* ip);
    bool CreateAccount(SIG_LOGIN_DATA* payload, MySQL* db_handle);
    bool _DecryptUserInfo(char* encrypted, char* output, unsigned int account_id);
    bool GetAccountInfo(SIG_LOGIN_DATA* payload);
    bool GetWhiteAccount(SIG_LOGIN_DATA* payload);
    bool GetSchoolName(int school_id, char* dest);
    bool GetBadgeCount(SIG_LOGIN_DATA* payload);
    bool GetGameDBInfo(SIG_LOGIN_DATA* payload);
    bool SaveLoginTime(unsigned int account_id, int login_time, char* ip);
    int _getGameDBCharacView(SIG_LOGIN_DATA* payload);
    bool SendResult(int signal, int user_idx, SIG_LOGIN_DATA* payload);
    bool UpdateLoginStat(int user_idx, SIG_LOGIN_DATA* payload);
    bool GetCheckPickUpRandomOptionItem(SIG_LOGIN_DATA* payload);
    bool GetOntimeUserRecvIdx(SIG_LOGIN_DATA* payload);
    bool GetUserFatigueCount(SIG_LOGIN_DATA* payload);
    bool GetUserInGameEventHistory(SIG_LOGIN_DATA* payload);
    bool CheckLoginIP(SIG_LOGIN_DATA* payload);
    bool GetMercenarySystemInfo(SIG_LOGIN_DATA* payload);
    bool GetGrowthWeaponEvent(SIG_LOGIN_DATA* payload);
    bool GetGrowthCreatureEvent(SIG_LOGIN_DATA* payload);
    bool GetAccountCharacterLevelUpOnceGift(SIG_LOGIN_DATA* payload);
    bool GetBingoEvent(SIG_LOGIN_DATA* payload);
    bool IsThisIPLoginPermissionGranted(SIG_LOGIN_DATA* payload);
};

static_assert(sizeof(DB_Login::tagDNFEventEntry) == 0x18u,
              "tagDNFEventEntry must keep the old 24-byte vector ABI");
static_assert(offsetof(DB_Login::tagDNFEventEntry, event_id) == 0x00u,
              "tagDNFEventEntry event ID must stay at +0x00");
static_assert(offsetof(DB_Login::tagDNFEventEntry, opaque_04) == 0x04u,
              "tagDNFEventEntry opaque slot must stay at +0x04");
static_assert(offsetof(DB_Login::tagDNFEventEntry, occ_time) == 0x08u,
              "tagDNFEventEntry occurrence time must stay at +0x08");
static_assert(offsetof(DB_Login::tagDNFEventEntry, server_id) == 0x0cu,
              "tagDNFEventEntry server ID must stay at +0x0C");
static_assert(offsetof(DB_Login::tagDNFEventEntry, charac_no) == 0x10u,
              "tagDNFEventEntry character number must stay at +0x10");
static_assert(offsetof(DB_Login::tagDNFEventEntry, obtain_time) == 0x14u,
              "tagDNFEventEntry obtain time must stay at +0x14");

#endif  // OLDSRV_DB_DB_LOGIN_H_
