#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_BUSINESS_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_BUSINESS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD140.  The frozen C group route intentionally owns this vptr-only
// state-0 handler; the Monitor packet value type itself remains A-owned.
class Dispatcher_ReportMannerlessUser
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85961DC
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserC2Ev
    // DEMANGLED: Dispatcher_ReportMannerlessUser::Dispatcher_ReportMannerlessUser(void)
    Dispatcher_ReportMannerlessUser();

    // IDA_EA: 0x82428D4
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserD2Ev
    // DEMANGLED: Dispatcher_ReportMannerlessUser::~Dispatcher_ReportMannerlessUser()
    // IDA_EA: 0x8242904
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserD0Ev
    // DEMANGLED: Dispatcher_ReportMannerlessUser::~Dispatcher_ReportMannerlessUser()
    virtual ~Dispatcher_ReportMannerlessUser();

    // IDA_EA: 0x8206124
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ReportMannerlessUser::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82338B2
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ReportMannerlessUser::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82338BC
    // IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ReportMannerlessUser::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ReportMannerlessUser) == 0x04u,
              "CMD140 must retain the old vptr-only dispatcher ABI");
#endif

// CMD143. The old handler carries no per-instance state: it only validates
// the selected character and forwards the current guild identity through the
// original Guild proxy path.
class Dispatcher_CallGuildAllMembers
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596214
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersC2Ev
    // DEMANGLED: Dispatcher_CallGuildAllMembers::Dispatcher_CallGuildAllMembers(void)
    Dispatcher_CallGuildAllMembers();

    // IDA_EA: 0x82433CC
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersD2Ev
    // IDA_EA: 0x82433FC
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersD0Ev
    virtual ~Dispatcher_CallGuildAllMembers();

    // IDA_EA: 0x81FFB1A
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823356C
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233576
    // IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CallGuildAllMembers) == 0x04u,
              "CMD143 must retain the old vptr-only dispatcher ABI");
#endif

// CMD154. The old instance is vptr-only; the nonvirtual response helper is
// part of the exact handler identity set and writes the original CMD154 reply.
class Dispatcher_CallGuildInvite
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859632C
    // IDA_NAME: _ZN26Dispatcher_CallGuildInviteC2Ev
    // DEMANGLED: Dispatcher_CallGuildInvite::Dispatcher_CallGuildInvite(void)
    Dispatcher_CallGuildInvite();

    // IDA_EA: 0x82425C8
    // IDA_NAME: _ZN26Dispatcher_CallGuildInviteD2Ev
    // DEMANGLED: Dispatcher_CallGuildInvite::~Dispatcher_CallGuildInvite()
    // IDA_EA: 0x82425F8
    // IDA_NAME: _ZN26Dispatcher_CallGuildInviteD0Ev
    // DEMANGLED: Dispatcher_CallGuildInvite::~Dispatcher_CallGuildInvite()
    virtual ~Dispatcher_CallGuildInvite();

    // IDA_EA: 0x820791C
    // IDA_NAME: _ZN26Dispatcher_CallGuildInvite12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CallGuildInvite::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823397A
    // IDA_NAME: _ZN26Dispatcher_CallGuildInvite4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CallGuildInvite::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233984
    // IDA_NAME: _ZN26Dispatcher_CallGuildInvite7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CallGuildInvite::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8207852
    // IDA_NAME: _ZN26Dispatcher_CallGuildInvite24send_packet_guild_inviteEP5CUserh
    // DEMANGLED: Dispatcher_CallGuildInvite::send_packet_guild_invite(CUser *,unsigned char)
    void send_packet_guild_invite(CUser* user, unsigned char error);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CallGuildInvite) == 0x04u,
              "CMD154 must retain the old vptr-only dispatcher ABI");
#endif

// CMD155. This is likewise a vptr-only state-0 handler. Its GameWorld
// integrated-PvP branch is in the body, not modeled as per-instance state.
class Dispatcher_ReplyGuildInvite
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596348
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteC2Ev
    // DEMANGLED: Dispatcher_ReplyGuildInvite::Dispatcher_ReplyGuildInvite(void)
    Dispatcher_ReplyGuildInvite();

    // IDA_EA: 0x824257A
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteD2Ev
    // DEMANGLED: Dispatcher_ReplyGuildInvite::~Dispatcher_ReplyGuildInvite()
    // IDA_EA: 0x82425AA
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteD0Ev
    // DEMANGLED: Dispatcher_ReplyGuildInvite::~Dispatcher_ReplyGuildInvite()
    virtual ~Dispatcher_ReplyGuildInvite();

    // IDA_EA: 0x8207B52
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ReplyGuildInvite::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823398E
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ReplyGuildInvite::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233998
    // IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ReplyGuildInvite::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ReplyGuildInvite) == 0x04u,
              "CMD155 must retain the old vptr-only dispatcher ABI");
#endif

// CMD157. No state is stored in the handler; the announcement text is parsed
// and forwarded only after the old role and filter checks.
class Dispatcher_NotifyMessageToGuild
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596380
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildC2Ev
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::Dispatcher_NotifyMessageToGuild(void)
    Dispatcher_NotifyMessageToGuild();

    // IDA_EA: 0x82424DE
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildD2Ev
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::~Dispatcher_NotifyMessageToGuild()
    // IDA_EA: 0x824250E
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildD0Ev
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::~Dispatcher_NotifyMessageToGuild()
    virtual ~Dispatcher_NotifyMessageToGuild();

    // IDA_EA: 0x8207FFE
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82339B6
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82339C0
    // IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_NotifyMessageToGuild::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_NotifyMessageToGuild) == 0x04u,
              "CMD157 must retain the old vptr-only dispatcher ABI");
#endif

// CMD158. The authority check and protection check belong to the dispatch
// path; no storage beyond the base vptr is present in the old instance.
class Dispatcher_GuildMasterDelegate
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859639C
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateC2Ev
    // DEMANGLED: Dispatcher_GuildMasterDelegate::Dispatcher_GuildMasterDelegate(void)
    Dispatcher_GuildMasterDelegate();

    // IDA_EA: 0x8242490
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateD2Ev
    // DEMANGLED: Dispatcher_GuildMasterDelegate::~Dispatcher_GuildMasterDelegate()
    // IDA_EA: 0x82424C0
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateD0Ev
    // DEMANGLED: Dispatcher_GuildMasterDelegate::~Dispatcher_GuildMasterDelegate()
    virtual ~Dispatcher_GuildMasterDelegate();

    // IDA_EA: 0x82082D6
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_GuildMasterDelegate::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82339CA
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GuildMasterDelegate::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82339D4
    // IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GuildMasterDelegate::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GuildMasterDelegate) == 0x04u,
              "CMD158 must retain the old vptr-only dispatcher ABI");
#endif

// CMD227. The member memo buffer is a request-local 21-byte value, not class
// storage, and is zeroed before the old packet reader uses it.
class Dispatcher_WriteGuildMemberMemo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596AF0
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoC2Ev
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::Dispatcher_WriteGuildMemberMemo(void)
    Dispatcher_WriteGuildMemberMemo();

    // IDA_EA: 0x8241248
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoD2Ev
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::~Dispatcher_WriteGuildMemberMemo()
    // IDA_EA: 0x8241278
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoD0Ev
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::~Dispatcher_WriteGuildMemberMemo()
    virtual ~Dispatcher_WriteGuildMemberMemo();

    // IDA_EA: 0x821E748
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233EA2
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233EAC
    // IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_WriteGuildMemberMemo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_WriteGuildMemberMemo) == 0x04u,
              "CMD227 must retain the old vptr-only dispatcher ABI");
#endif

// CMD129.  The instance carries only the dispatcher vptr; all fields belong
// to the exact Guild UDP request built for this one dispatch call.
class Dispatcher_SetSubGuildMaster
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596134
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterC2Ev
    // DEMANGLED: Dispatcher_SetSubGuildMaster::Dispatcher_SetSubGuildMaster(void)
    Dispatcher_SetSubGuildMaster();

    // IDA_EA: 0x8242A0C
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterD2Ev
    // DEMANGLED: Dispatcher_SetSubGuildMaster::~Dispatcher_SetSubGuildMaster()
    // IDA_EA: 0x8242A3C
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterD0Ev
    // DEMANGLED: Dispatcher_SetSubGuildMaster::~Dispatcher_SetSubGuildMaster()
    virtual ~Dispatcher_SetSubGuildMaster();

    // IDA_EA: 0x82053E6
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_SetSubGuildMaster::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233862
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_SetSubGuildMaster::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823386C
    // IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_SetSubGuildMaster::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SetSubGuildMaster) == 0x04u,
              "CMD129 must retain the old vptr-only dispatcher ABI");
#endif

// CMD182.  The handler's state stays in CUser/inventory/protection providers;
// this dispatcher is also only the inherited vptr in the original binary.
class Dispatcher_ChangeGuildName
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596690
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildNameC2Ev
    // DEMANGLED: Dispatcher_ChangeGuildName::Dispatcher_ChangeGuildName(void)
    Dispatcher_ChangeGuildName();

    // IDA_EA: 0x8241DDC
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildNameD2Ev
    // DEMANGLED: Dispatcher_ChangeGuildName::~Dispatcher_ChangeGuildName()
    // IDA_EA: 0x8241E0C
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildNameD0Ev
    // DEMANGLED: Dispatcher_ChangeGuildName::~Dispatcher_ChangeGuildName()
    virtual ~Dispatcher_ChangeGuildName();

    // IDA_EA: 0x82134A8
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildName12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ChangeGuildName::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B1E
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildName4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeGuildName::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B28
    // IDA_NAME: _ZN26Dispatcher_ChangeGuildName7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ChangeGuildName::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeGuildName) == 0x04u,
              "CMD182 must retain the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_BUSINESS_H_
