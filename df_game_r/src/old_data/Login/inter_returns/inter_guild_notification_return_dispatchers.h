#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NOTIFICATION_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NOTIFICATION_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_ReplyGuildLetter : public ISigDispatcher { public: Inter_ReplyGuildLetter(); virtual ~Inter_ReplyGuildLetter(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_CallGuildInviteToCaller : public ISigDispatcher { public: Inter_CallGuildInviteToCaller(); virtual ~Inter_CallGuildInviteToCaller(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_ReplyGuildInviteToInvited : public ISigDispatcher { public: Inter_ReplyGuildInviteToInvited(); virtual ~Inter_ReplyGuildInviteToInvited(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_ReplyGuildInviteToCaller : public ISigDispatcher { public: Inter_ReplyGuildInviteToCaller(); virtual ~Inter_ReplyGuildInviteToCaller(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_NoticeGuildMasterDelegate : public ISigDispatcher { public: Inter_NoticeGuildMasterDelegate(); virtual ~Inter_NoticeGuildMasterDelegate(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_GuildReplyGuildMasterDelegate : public ISigDispatcher { public: Inter_GuildReplyGuildMasterDelegate(); virtual ~Inter_GuildReplyGuildMasterDelegate(); virtual int dispatch_sig(CUser*, char*, int); };
class Inter_NoticeHasBeenGuildMaster : public ISigDispatcher { public: Inter_NoticeHasBeenGuildMaster(); virtual ~Inter_NoticeHasBeenGuildMaster(); virtual int dispatch_sig(CUser*, char*, int); };

#if defined(__i386__)
static_assert(sizeof(Inter_ReplyGuildLetter) == 0x04u, "Inter_ReplyGuildLetter must retain the original base-only ABI");
static_assert(sizeof(Inter_CallGuildInviteToCaller) == 0x04u, "Inter_CallGuildInviteToCaller must retain the original base-only ABI");
static_assert(sizeof(Inter_ReplyGuildInviteToInvited) == 0x04u, "Inter_ReplyGuildInviteToInvited must retain the original base-only ABI");
static_assert(sizeof(Inter_ReplyGuildInviteToCaller) == 0x04u, "Inter_ReplyGuildInviteToCaller must retain the original base-only ABI");
static_assert(sizeof(Inter_NoticeGuildMasterDelegate) == 0x04u, "Inter_NoticeGuildMasterDelegate must retain the original base-only ABI");
static_assert(sizeof(Inter_GuildReplyGuildMasterDelegate) == 0x04u, "Inter_GuildReplyGuildMasterDelegate must retain the original base-only ABI");
static_assert(sizeof(Inter_NoticeHasBeenGuildMaster) == 0x04u, "Inter_NoticeHasBeenGuildMaster must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_GUILD_NOTIFICATION_RETURN_DISPATCHERS_H_
