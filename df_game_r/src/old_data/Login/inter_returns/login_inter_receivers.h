#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_LOGIN_INTER_RECEIVERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_LOGIN_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// InterDispatcher table entries [0] and [1].  These run before a user
// reaches the login reply path: [1] tears down an existing game session and
// [2] informs a same-account session that it has been superseded.  Both are
// base-only ISigDispatcher objects in the original i386 table.
class Inter_Disconnect : public ISigDispatcher {
public:
    // IDA_EA: 0x84BEB3C
    // IDA_NAME: _ZN16Inter_Disconnect12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84E9B0E / 0x84E9B2A / 0x84E9B5A
    Inter_Disconnect();
    virtual ~Inter_Disconnect();
};

class Inter_SessionClose : public ISigDispatcher {
public:
    // IDA_EA: 0x84C7AF0
    // IDA_NAME: _ZN18Inter_SessionClose12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EA006 / 0x84EA022 / 0x84EA052
    Inter_SessionClose();
    virtual ~Inter_SessionClose();
};

// InterDispatcher table entry [65].  The original object has no fields beyond
// ISigDispatcher; the reply is the monitor-login completion transaction, not
// the DB_Login SIG_LOGIN_DATA request/reply structure.
class Inter_MonitorLoginOK : public ISigDispatcher {
public:
    // IDA_EA: 0x84C7E0C
    // IDA_NAME: _ZN20Inter_MonitorLoginOK12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorLoginOK::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EA1AE / 0x84EA1CA / 0x84EA1FA
    // IDA_NAME: _ZN20Inter_MonitorLoginOKC2Ev
    // IDA_NAME: _ZN20Inter_MonitorLoginOKD2Ev
    // IDA_NAME: _ZN20Inter_MonitorLoginOKD0Ev
    // DEMANGLED: Inter_MonitorLoginOK::Inter_MonitorLoginOK(void)
    // DEMANGLED: Inter_MonitorLoginOK::~Inter_MonitorLoginOK()
    Inter_MonitorLoginOK();
    virtual ~Inter_MonitorLoginOK();
};

// InterDispatcher leaves whose reply payloads are produced by the active
// account/security DB workers.  They remain declaration-only registration
// units: InterDispatcher activation is intentionally owned by its full table
// restoration.
class Inter_SecurityCardActivate : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDB44
    // IDA_NAME: _ZN26Inter_SecurityCardActivateC2Ev
    // DEMANGLED: Inter_SecurityCardActivate::Inter_SecurityCardActivate(void)
    Inter_SecurityCardActivate();

    // IDA_EA: 0x84F8FEE
    // IDA_NAME: _ZN26Inter_SecurityCardActivateD2Ev
    // IDA_EA: 0x84F901E
    // IDA_NAME: _ZN26Inter_SecurityCardActivateD0Ev
    // DEMANGLED: Inter_SecurityCardActivate::~Inter_SecurityCardActivate()
    virtual ~Inter_SecurityCardActivate();

    // IDA_EA: 0x84D3D56
    // IDA_NAME: _ZN26Inter_SecurityCardActivate12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SecurityCardActivate::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SecurityCardIssue : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDB28
    // IDA_NAME: _ZN23Inter_SecurityCardIssueC2Ev
    // DEMANGLED: Inter_SecurityCardIssue::Inter_SecurityCardIssue(void)
    Inter_SecurityCardIssue();

    // IDA_EA: 0x84F903C
    // IDA_NAME: _ZN23Inter_SecurityCardIssueD2Ev
    // IDA_EA: 0x84F906C
    // IDA_NAME: _ZN23Inter_SecurityCardIssueD0Ev
    // DEMANGLED: Inter_SecurityCardIssue::~Inter_SecurityCardIssue()
    virtual ~Inter_SecurityCardIssue();

    // IDA_EA: 0x84D3A22
    // IDA_NAME: _ZN23Inter_SecurityCardIssue12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SecurityCardIssue::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SecurityCardRetransfer : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDB60
    // IDA_NAME: _ZN28Inter_SecurityCardRetransferC2Ev
    // DEMANGLED: Inter_SecurityCardRetransfer::Inter_SecurityCardRetransfer(void)
    Inter_SecurityCardRetransfer();

    // IDA_EA: 0x84F8FA0
    // IDA_NAME: _ZN28Inter_SecurityCardRetransferD2Ev
    // IDA_EA: 0x84F8FD0
    // IDA_NAME: _ZN28Inter_SecurityCardRetransferD0Ev
    // DEMANGLED: Inter_SecurityCardRetransfer::~Inter_SecurityCardRetransfer()
    virtual ~Inter_SecurityCardRetransfer();

    // IDA_EA: 0x84D3E78
    // IDA_NAME: _ZN28Inter_SecurityCardRetransfer12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SecurityCardRetransfer::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Hack_CheckMyAccount : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE5FC
    // IDA_NAME: _ZN25Inter_Hack_CheckMyAccountC2Ev
    // DEMANGLED: Inter_Hack_CheckMyAccount::Inter_Hack_CheckMyAccount(void)
    Inter_Hack_CheckMyAccount();

    // IDA_EA: 0x84F71C4
    // IDA_NAME: _ZN25Inter_Hack_CheckMyAccountD2Ev
    // IDA_EA: 0x84F71F4
    // IDA_NAME: _ZN25Inter_Hack_CheckMyAccountD0Ev
    // DEMANGLED: Inter_Hack_CheckMyAccount::~Inter_Hack_CheckMyAccount()
    virtual ~Inter_Hack_CheckMyAccount();

    // IDA_EA: 0x84E08FC
    // IDA_NAME: _ZN25Inter_Hack_CheckMyAccount12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_Hack_CheckMyAccount::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SendTimeSyncLogin : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE998
    // IDA_NAME: _ZN23Inter_SendTimeSyncLoginC2Ev
    // DEMANGLED: Inter_SendTimeSyncLogin::Inter_SendTimeSyncLogin(void)
    Inter_SendTimeSyncLogin();

    // IDA_EA: 0x84F6852
    // IDA_NAME: _ZN23Inter_SendTimeSyncLoginD2Ev
    // IDA_EA: 0x84F6882
    // IDA_NAME: _ZN23Inter_SendTimeSyncLoginD0Ev
    // DEMANGLED: Inter_SendTimeSyncLogin::~Inter_SendTimeSyncLogin()
    virtual ~Inter_SendTimeSyncLogin();

    // IDA_EA: 0x84E33EC
    // IDA_NAME: _ZN23Inter_SendTimeSyncLogin12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_SendTimeSyncLogin::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PunishCancel : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEB20
    // IDA_NAME: _ZN18Inter_PunishCancelC2Ev
    // DEMANGLED: Inter_PunishCancel::Inter_PunishCancel(void)
    Inter_PunishCancel();

    // IDA_EA: 0x84F6324
    // IDA_NAME: _ZN18Inter_PunishCancelD2Ev
    // IDA_EA: 0x84F6354
    // IDA_NAME: _ZN18Inter_PunishCancelD0Ev
    // DEMANGLED: Inter_PunishCancel::~Inter_PunishCancel()
    virtual ~Inter_PunishCancel();

    // IDA_EA: 0x84E4B02
    // IDA_NAME: _ZN18Inter_PunishCancel12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_PunishCancel::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 02_login_account/Inter_NoticePCRoomPlayerLoginOut.
class Inter_NoticePCRoomPlayerLoginOut : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE068
    // IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutC2Ev
    // DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::Inter_NoticePCRoomPlayerLoginOut(void)
    Inter_NoticePCRoomPlayerLoginOut();

    // IDA_EA: 0x84F8100
    // IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutD2Ev
    // IDA_EA: 0x84F8130
    // IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutD0Ev
    // DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::~Inter_NoticePCRoomPlayerLoginOut()
    virtual ~Inter_NoticePCRoomPlayerLoginOut();

    // IDA_EA: 0x84DA1FA
    // IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOut12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 02_login_account/Inter_MonitorPunishUserReq.
class Inter_MonitorPunishUserReq : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE1F0
    // IDA_NAME: _ZN26Inter_MonitorPunishUserReqC2Ev
    // DEMANGLED: Inter_MonitorPunishUserReq::Inter_MonitorPunishUserReq(void)
    Inter_MonitorPunishUserReq();

    // IDA_EA: 0x84F7D0A
    // IDA_NAME: _ZN26Inter_MonitorPunishUserReqD2Ev
    // IDA_EA: 0x84F7D3A
    // IDA_NAME: _ZN26Inter_MonitorPunishUserReqD0Ev
    // DEMANGLED: Inter_MonitorPunishUserReq::~Inter_MonitorPunishUserReq()
    virtual ~Inter_MonitorPunishUserReq();

    // IDA_EA: 0x84DB294
    // IDA_NAME: _ZN26Inter_MonitorPunishUserReq12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorPunishUserReq::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Disconnect) == 0x04u,
              "Inter_Disconnect must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SessionClose) == 0x04u,
              "Inter_SessionClose must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorLoginOK) == 0x04u,
              "Inter_MonitorLoginOK must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SecurityCardActivate) == 0x04u,
              "Inter_SecurityCardActivate must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SecurityCardIssue) == 0x04u,
              "Inter_SecurityCardIssue must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SecurityCardRetransfer) == 0x04u,
              "Inter_SecurityCardRetransfer must retain its base-only i386 ABI");
static_assert(sizeof(Inter_Hack_CheckMyAccount) == 0x04u,
              "Inter_Hack_CheckMyAccount must retain its base-only i386 ABI");
static_assert(sizeof(Inter_SendTimeSyncLogin) == 0x04u,
              "Inter_SendTimeSyncLogin must retain its base-only i386 ABI");
static_assert(sizeof(Inter_PunishCancel) == 0x04u,
              "Inter_PunishCancel must retain its base-only i386 ABI");
static_assert(sizeof(Inter_NoticePCRoomPlayerLoginOut) == 0x04u,
              "Inter_NoticePCRoomPlayerLoginOut must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorPunishUserReq) == 0x04u,
              "Inter_MonitorPunishUserReq must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_LOGIN_INTER_RECEIVERS_H_
