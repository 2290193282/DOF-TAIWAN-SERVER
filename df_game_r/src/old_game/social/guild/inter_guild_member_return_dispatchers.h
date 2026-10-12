#ifndef OLDSRV_OLD_GAME_GROUP_INTER_GUILD_MEMBER_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_GROUP_INTER_GUILD_MEMBER_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

#include <time.h>

class CUser;

class Inter_MonitorCallGuildMembersNext : public ISigDispatcher {
public:
    Inter_MonitorCallGuildMembersNext();
    virtual ~Inter_MonitorCallGuildMembersNext();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorCallGuildAllMembers : public ISigDispatcher {
public:
    Inter_MonitorCallGuildAllMembers();
    virtual ~Inter_MonitorCallGuildAllMembers();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorCallGuildAllMembersNext : public ISigDispatcher {
public:
    Inter_MonitorCallGuildAllMembersNext();
    virtual ~Inter_MonitorCallGuildAllMembersNext();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA units: Inter_Monitor{SetSubGuildMasterReply,
// NoticeGuildMemberInfo,NoticeRequestMemberEnterToRequester,
// NoticeRequestMemberEnterToResponser,NoticeMemberEnterOk,
// NoticeMemberEnterReply,NoticeMemberSecede,NoticeMemberSecedeToSeceder,
// NoticeDeleteMemberId,NoticeMemberChatMsg,CallMemberList,PayTaxToUpper}.
// Each object is a base-only InterDispatcher leaf; all state is carried by
// the incoming monitor packet.
class Inter_MonitorSetSubGuildMasterReply : public ISigDispatcher {
public:
    Inter_MonitorSetSubGuildMasterReply();
    virtual ~Inter_MonitorSetSubGuildMasterReply();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildMemberInfo : public ISigDispatcher {
public:
    Inter_MonitorNoticeGuildMemberInfo();
    virtual ~Inter_MonitorNoticeGuildMemberInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeRequestMemberEnterToRequester : public ISigDispatcher {
public:
    Inter_MonitorNoticeRequestMemberEnterToRequester();
    virtual ~Inter_MonitorNoticeRequestMemberEnterToRequester();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeRequestMemberEnterToResponser : public ISigDispatcher {
public:
    Inter_MonitorNoticeRequestMemberEnterToResponser();
    virtual ~Inter_MonitorNoticeRequestMemberEnterToResponser();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberEnterOk : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberEnterOk();
    virtual ~Inter_MonitorNoticeMemberEnterOk();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberEnterReply : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberEnterReply();
    virtual ~Inter_MonitorNoticeMemberEnterReply();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberSecede : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberSecede();
    virtual ~Inter_MonitorNoticeMemberSecede();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberSecedeToSeceder : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberSecedeToSeceder();
    virtual ~Inter_MonitorNoticeMemberSecedeToSeceder();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeDeleteMemberId : public ISigDispatcher {
public:
    Inter_MonitorNoticeDeleteMemberId();
    virtual ~Inter_MonitorNoticeDeleteMemberId();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberChatMsg : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberChatMsg();
    virtual ~Inter_MonitorNoticeMemberChatMsg();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorCallMemberList : public ISigDispatcher {
public:
    Inter_MonitorCallMemberList();
    virtual ~Inter_MonitorCallMemberList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorPayTaxToUpper : public ISigDispatcher {
public:
    Inter_MonitorPayTaxToUpper();
    virtual ~Inter_MonitorPayTaxToUpper();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorGuildWarInfo : public ISigDispatcher {
public:
    Inter_MonitorGuildWarInfo();
    virtual ~Inter_MonitorGuildWarInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorGuildPointDel : public ISigDispatcher {
public:
    Inter_MonitorGuildPointDel();
    virtual ~Inter_MonitorGuildPointDel();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x82A5A4C
    // IDA_NAME: _ZN26Inter_MonitorGuildPointDel17getLastDeleteTimeEv
    static int getLastDeleteTime();

private:
    // IDA_EA: 0x84CE6D2 / 0x84EB100
    static time_t _makeDefaultTime();
    static time_t setLastDeleteTime();
    static time_t g_lastDeleteTime;
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorCallGuildMembersNext) == 0x04u,
              "Inter_MonitorCallGuildMembersNext must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorCallGuildAllMembers) == 0x04u,
              "Inter_MonitorCallGuildAllMembers must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorCallGuildAllMembersNext) == 0x04u,
              "Inter_MonitorCallGuildAllMembersNext must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorSetSubGuildMasterReply) == 0x04u,
              "Inter_MonitorSetSubGuildMasterReply must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildMemberInfo) == 0x04u,
              "Inter_MonitorNoticeGuildMemberInfo must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeRequestMemberEnterToRequester) == 0x04u,
              "Inter_MonitorNoticeRequestMemberEnterToRequester must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeRequestMemberEnterToResponser) == 0x04u,
              "Inter_MonitorNoticeRequestMemberEnterToResponser must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberEnterOk) == 0x04u,
              "Inter_MonitorNoticeMemberEnterOk must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberEnterReply) == 0x04u,
              "Inter_MonitorNoticeMemberEnterReply must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberSecede) == 0x04u,
              "Inter_MonitorNoticeMemberSecede must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberSecedeToSeceder) == 0x04u,
              "Inter_MonitorNoticeMemberSecedeToSeceder must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeDeleteMemberId) == 0x04u,
              "Inter_MonitorNoticeDeleteMemberId must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberChatMsg) == 0x04u,
              "Inter_MonitorNoticeMemberChatMsg must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorCallMemberList) == 0x04u,
              "Inter_MonitorCallMemberList must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorPayTaxToUpper) == 0x04u,
              "Inter_MonitorPayTaxToUpper must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorGuildWarInfo) == 0x04u,
              "Inter_MonitorGuildWarInfo must retain the base-only ABI");
static_assert(sizeof(Inter_MonitorGuildPointDel) == 0x04u,
              "Inter_MonitorGuildPointDel must retain the base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_INTER_GUILD_MEMBER_RETURN_DISPATCHERS_H_
