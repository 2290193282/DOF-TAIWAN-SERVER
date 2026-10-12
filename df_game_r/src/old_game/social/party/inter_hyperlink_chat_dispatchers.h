#ifndef OLDSRV_OLD_GAME_GROUP_INTER_HYPERLINK_CHAT_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_GROUP_INTER_HYPERLINK_CHAT_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_MonitorNoticeGuildChatMsgHyperLink : public ISigDispatcher {
public:
    Inter_MonitorNoticeGuildChatMsgHyperLink();
    virtual ~Inter_MonitorNoticeGuildChatMsgHyperLink();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberChatMsgHyperLink : public ISigDispatcher {
public:
    Inter_MonitorNoticeMemberChatMsgHyperLink();
    virtual ~Inter_MonitorNoticeMemberChatMsgHyperLink();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Monitor_MegaphoneMsgHyperLink : public ISigDispatcher {
public:
    Inter_Monitor_MegaphoneMsgHyperLink();
    virtual ~Inter_Monitor_MegaphoneMsgHyperLink();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorNoticeGuildChatMsgHyperLink) == 0x04u &&
                  sizeof(Inter_MonitorNoticeMemberChatMsgHyperLink) == 0x04u &&
                  sizeof(Inter_Monitor_MegaphoneMsgHyperLink) == 0x04u,
              "Inter hyperlink chat leaves must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_INTER_HYPERLINK_CHAT_DISPATCHERS_H_
