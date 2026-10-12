#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_Monitor_MegaphoneMsg : public ISigDispatcher {
public:
    Inter_Monitor_MegaphoneMsg();
    virtual ~Inter_Monitor_MegaphoneMsg();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Monitor_RequestBlackListResult : public ISigDispatcher {
public:
    Inter_Monitor_RequestBlackListResult();
    virtual ~Inter_Monitor_RequestBlackListResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeBlackList : public ISigDispatcher {
public:
    Inter_MonitorNoticeBlackList();
    virtual ~Inter_MonitorNoticeBlackList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_DB_RequestBlackCount : public ISigDispatcher {
public:
    Inter_DB_RequestBlackCount();
    virtual ~Inter_DB_RequestBlackCount();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_WebNoticeSingle : public ISigDispatcher {
public:
    Inter_WebNoticeSingle();
    virtual ~Inter_WebNoticeSingle();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MemberExpLevelUp : public ISigDispatcher {
public:
    Inter_MemberExpLevelUp();
    virtual ~Inter_MemberExpLevelUp();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GMRequestMid : public ISigDispatcher {
public:
    Inter_GMRequestMid();
    virtual ~Inter_GMRequestMid();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Monitor_MegaphoneMsg) == 0x04u,
              "Inter_Monitor_MegaphoneMsg must retain the original base-only ABI");
static_assert(sizeof(Inter_Monitor_RequestBlackListResult) == 0x04u,
              "Inter_Monitor_RequestBlackListResult must retain the original base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeBlackList) == 0x04u,
              "Inter_MonitorNoticeBlackList must retain the original base-only ABI");
static_assert(sizeof(Inter_DB_RequestBlackCount) == 0x04u,
              "Inter_DB_RequestBlackCount must retain the original base-only ABI");
static_assert(sizeof(Inter_WebNoticeSingle) == 0x04u,
              "Inter_WebNoticeSingle must retain the original base-only ABI");
static_assert(sizeof(Inter_MemberExpLevelUp) == 0x04u,
              "Inter_MemberExpLevelUp must retain the original base-only ABI");
static_assert(sizeof(Inter_GMRequestMid) == 0x04u,
              "Inter_GMRequestMid must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_MONITOR_RETURN_DISPATCHERS_H_
