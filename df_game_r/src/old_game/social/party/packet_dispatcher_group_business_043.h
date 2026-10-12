#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GROUP_BUSINESS_043_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GROUP_BUSINESS_043_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// The four state-3 dispatcher messages intentionally retain the old
// unconstructed-stack layout.  MSG_BASE is 13 bytes; only the integer-bearing
// messages gain the three bytes of natural alignment before their length.
struct MSG_REQUEST_MEMBER_ENTER : MSG_BASE {
    int target_name_length;
    char target_name[30];
};

struct MSG_MEMBER_ENTER_REPLY : MSG_BASE {
    uint8_t accepted;
};

struct MSG_MEMBER_SECEDE : MSG_BASE {
    int target_name_length;
    char target_name[30];
};

struct stRequestMemberEnterParam : ParamBase {
    stRequestMemberEnterParam();
    virtual ~stRequestMemberEnterParam();

    int result;
};

struct stMemberEnterReplyParam : ParamBase {
    stMemberEnterReplyParam();
    virtual ~stMemberEnterReplyParam();

    int result;
};

struct stMemberSecedeParam : ParamBase {
    stMemberSecedeParam();
    virtual ~stMemberSecedeParam();

    int result;
};

struct stCallMemberListParam : ParamBase {
    stCallMemberListParam();
    virtual ~stCallMemberListParam();

    int result;
};

#if defined(__i386__)
static_assert(offsetof(MSG_REQUEST_MEMBER_ENTER, target_name_length) == 0x10u,
              "MSG_REQUEST_MEMBER_ENTER length offset must stay 0x10");
static_assert(offsetof(MSG_REQUEST_MEMBER_ENTER, target_name) == 0x14u,
              "MSG_REQUEST_MEMBER_ENTER name offset must stay 0x14");
static_assert(sizeof(MSG_REQUEST_MEMBER_ENTER) == 0x34u,
              "MSG_REQUEST_MEMBER_ENTER raw stack extent must stay 0x34");
static_assert(offsetof(MSG_MEMBER_ENTER_REPLY, accepted) == 0x0du,
              "MSG_MEMBER_ENTER_REPLY reply offset must stay 0x0d");
static_assert(sizeof(MSG_MEMBER_ENTER_REPLY) == 0x0eu,
              "MSG_MEMBER_ENTER_REPLY raw stack extent must stay 0x0e");
static_assert(offsetof(MSG_MEMBER_SECEDE, target_name_length) == 0x10u,
              "MSG_MEMBER_SECEDE length offset must stay 0x10");
static_assert(offsetof(MSG_MEMBER_SECEDE, target_name) == 0x14u,
              "MSG_MEMBER_SECEDE name offset must stay 0x14");
static_assert(sizeof(MSG_MEMBER_SECEDE) == 0x34u,
              "MSG_MEMBER_SECEDE raw stack extent must stay 0x34");
static_assert(sizeof(stRequestMemberEnterParam) == 0x08u,
              "stRequestMemberEnterParam must stay ParamBase plus result");
static_assert(sizeof(stMemberEnterReplyParam) == 0x08u,
              "stMemberEnterReplyParam must stay ParamBase plus result");
static_assert(sizeof(stMemberSecedeParam) == 0x08u,
              "stMemberSecedeParam must stay ParamBase plus result");
static_assert(sizeof(stCallMemberListParam) == 0x08u,
              "stCallMemberListParam must stay ParamBase plus result");
#endif


class DisPatcher_SendMess
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_SendMess();
    virtual ~DisPatcher_SendMess();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CallGuildMembers
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CallGuildMembers();
    virtual ~Dispatcher_CallGuildMembers();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CallGuildCreateRight
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CallGuildCreateRight();
    virtual ~Dispatcher_CallGuildCreateRight();

    void send_packet_guildcreateright(CUser* user, unsigned char result);
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Score_Scroll_State
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Score_Scroll_State();
    virtual ~Dispatcher_Score_Scroll_State();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Card_Select_Right_State
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Card_Select_Right_State();
    virtual ~Dispatcher_Card_Select_Right_State();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Select_Card
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Select_Card();
    virtual ~Dispatcher_Select_Card();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CallGuildLevelUp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CallGuildLevelUp();
    virtual ~Dispatcher_CallGuildLevelUp();

    void send_packet_guild_levelup(CUser* user, unsigned char error);
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CallGuildInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CallGuildInfo();
    virtual ~Dispatcher_CallGuildInfo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestMemberEnter
    : public IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                               stRequestMemberEnterParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_RequestMemberEnter();
    virtual ~Dispatcher_RequestMemberEnter();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

class Dispatcher_MemberEnterReply
    : public IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                               stMemberEnterReplyParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_MemberEnterReply();
    virtual ~Dispatcher_MemberEnterReply();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_MemberSecede
    : public IPacketDispatcher<MSG_MEMBER_SECEDE,
                               stMemberSecedeParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_MemberSecede();
    virtual ~Dispatcher_MemberSecede();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CallMemberList
    : public IPacketDispatcher<MSG_BASE,
                               stCallMemberListParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_CallMemberList();
    virtual ~Dispatcher_CallMemberList();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class DisPatcher_RequestGuildWarInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_RequestGuildWarInfo();
    virtual ~DisPatcher_RequestGuildWarInfo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestGuildSecede
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestGuildSecede();
    virtual ~Dispatcher_RequestGuildSecede();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};


#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GROUP_BUSINESS_043_H_
