#ifndef OLDSRV_OLD_DATA_CHANNEL_INTER_RETURNS_CHANNEL_INTER_RECEIVERS_H_
#define OLDSRV_OLD_DATA_CHANNEL_INTER_RETURNS_CHANNEL_INTER_RECEIVERS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"


class CUser;

// Packet_Monitor_Other_Channel_Chat C2 @0x822B8E6 and the paired
// Inter_MonitorNoticeOtherChannelChat receiver prove this complete transport
// record.  Reserved scalar members deliberately preserve the uninitialized
// bytes that the old C2 did not write; they are not a byte-storage substitute.
#pragma pack(push, 1)
struct SIG_MONITOR_OTHER_CHANNEL_CHAT {
    PacketHeader header;             // +0x00, type 0x3f3
    uint8_t chat_type;               // +0x0a
    uint8_t reserved_0b;             // +0x0b
    uint8_t reserved_0c;             // +0x0c
    uint8_t reserved_0d;             // +0x0d
    uint8_t sender_marker;           // +0x0e
    uint8_t reserved_0f;             // +0x0f
    uint8_t reserved_10;             // +0x10
    uint8_t reserved_11;             // +0x11
    uint8_t reserved_12;             // +0x12
    uint32_t target_charac_no;       // +0x13
    char sender_name[30];            // +0x17
    uint8_t relay_type;              // +0x35
    uint8_t message_length;          // +0x36
    char message[260];               // +0x37
};

// This is a protocol record only.  Old-server IDA proves that every linked
// item is copied with PacketBuf::get_binary/put_binary at exactly 104 bytes;
// it does not expose the in-memory C++ hyperlink_item_info layout.  Keeping
// that distinction prevents this wire record from being mistaken for an item
// object definition.
struct HyperlinkItemWire {
    uint8_t serialized[104];
};

// Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig @0x84E5528
// consumes this monitor-service reply.  It is deliberately a receiver record,
// not an alias for Packet_Monitor_Other_Channel_Chat_Hyper_Link: the outgoing
// packet is 0x274 bytes, whereas this received monitor reply has the exact
// +0x36/+0x37 and +0x16f/+0x170 offsets below.
struct SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK {
    PacketHeader header;                    // +0x00
    uint8_t chat_type;                      // +0x0a
    uint8_t reserved_0b;                    // +0x0b
    uint8_t reserved_0c;                    // +0x0c
    uint8_t reserved_0d;                    // +0x0d
    uint8_t reserved_0e;                    // +0x0e
    uint8_t reserved_0f;                    // +0x0f
    uint8_t reserved_10;                    // +0x10
    uint8_t reserved_11;                    // +0x11
    uint8_t reserved_12;                    // +0x12
    uint32_t target_charac_no;              // +0x13
    char sender_name[30];                   // +0x17, NUL-terminated
    uint8_t relay_type;                     // +0x35
    uint8_t hyperlink_count;                // +0x36, original client input: 1..3
    HyperlinkItemWire hyperlinks[3];        // +0x37, 104 bytes each
    uint8_t message_length;                 // +0x16f
    char message[256];                      // +0x170
};

// Old InterDispatcher reply consumed by
// Inter_MonitorNoticeGuildLoginOut::dispatch_sig @0x84C93E4.  The name is a
// NUL-terminated tail, rather than an untyped copied packet buffer.
struct SIG_MONITOR_GUILD_LOGIN_OUT {
    PacketHeader header;             // +0x00
    uint8_t event_flag;              // +0x0a
    uint8_t reserved_0b_0e[4];       // +0x0b
    uint32_t target_charac_no;       // +0x0f
    uint8_t notification_type;       // +0x13
    char member_name[1];             // +0x14, NUL-terminated tail
};
#pragma pack(pop)

static_assert(sizeof(SIG_MONITOR_OTHER_CHANNEL_CHAT) == 0x13bu,
              "other-channel chat packet ABI must stay 0x13b bytes");
static_assert(sizeof(HyperlinkItemWire) == 104u,
              "linked-item wire record must stay 104 bytes");
static_assert(sizeof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK) == 0x270u,
              "other-channel hyperlink monitor record ABI must stay 0x270 bytes");
static_assert(offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT, target_charac_no) ==
                  0x13u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT, sender_name) ==
                      0x17u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT, relay_type) ==
                      0x35u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT, message_length) ==
                      0x36u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT, message) == 0x37u,
              "other-channel chat observed offsets changed");
static_assert(offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                       target_charac_no) == 0x13u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           sender_name) == 0x17u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           relay_type) == 0x35u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           hyperlink_count) == 0x36u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           hyperlinks) == 0x37u &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           message_length) == 0x16fu &&
                  offsetof(SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK,
                           message) == 0x170u,
              "other-channel hyperlink observed offsets changed");
static_assert(offsetof(SIG_MONITOR_GUILD_LOGIN_OUT, event_flag) == 0x0au &&
                  offsetof(SIG_MONITOR_GUILD_LOGIN_OUT, target_charac_no) ==
                      0x0fu &&
                  offsetof(SIG_MONITOR_GUILD_LOGIN_OUT, notification_type) ==
                      0x13u &&
                  offsetof(SIG_MONITOR_GUILD_LOGIN_OUT, member_name) == 0x14u,
              "guild login-out monitor offsets changed");

// Inter_PvPChannelUserCount::dispatch_sig @0x84D8262 preserves the incoming
// monitor header and the two pre-school scalar fields, fills the five exact
// values below, and returns the same 0x2A-byte record through the selected
// monitor TCP proxy.  The old service has no local producer for the two
// preserved +0x0A/+0x0E values, so they stay explicit scalar ABI fields rather
// than being guessed as a C++ object or replaced with raw byte storage.
#pragma pack(push, 1)
struct SIG_PVP_CHANNEL_USER_COUNT {
    PacketHeader header;              // +0x00, preserved by this receiver
    uint32_t reserved_0a;             // +0x0A, preserved
    uint32_t reserved_0e;             // +0x0E, preserved
    uint32_t school_no;               // +0x12, CSchoolMgr lookup key
    uint32_t channel_no;              // +0x16, filled from CEnvironment
    uint32_t school_user_count;       // +0x1A, filled from CSchoolMgr
    uint32_t reply_server_group;      // +0x1E, monitor proxy selection
    uint32_t players_per_team;        // +0x22, zero-extended CLeagueMatch byte
    uint32_t battle_mode;             // +0x26, zero-extended CLeagueMatch byte
};
#pragma pack(pop)

static_assert(sizeof(SIG_PVP_CHANNEL_USER_COUNT) == 0x2au,
              "PvP channel user-count monitor record must stay 42 bytes");
static_assert(offsetof(SIG_PVP_CHANNEL_USER_COUNT, school_no) == 0x12u &&
                  offsetof(SIG_PVP_CHANNEL_USER_COUNT, channel_no) == 0x16u &&
                  offsetof(SIG_PVP_CHANNEL_USER_COUNT, school_user_count) ==
                      0x1au &&
                  offsetof(SIG_PVP_CHANNEL_USER_COUNT, reply_server_group) ==
                      0x1eu &&
                  offsetof(SIG_PVP_CHANNEL_USER_COUNT, players_per_team) ==
                      0x22u &&
                  offsetof(SIG_PVP_CHANNEL_USER_COUNT, battle_mode) == 0x26u,
              "PvP channel user-count observed offsets changed");

class Inter_DeleteChannel : public ISigDispatcher {
public:
    // IDA_EA: 0x84E9BE2
    // IDA_NAME: _ZN19Inter_DeleteChannelC2Ev
    // DEMANGLED: Inter_DeleteChannel::Inter_DeleteChannel(void)
    Inter_DeleteChannel();

    // IDA_EA: 0x84E9BFE
    // IDA_NAME: _ZN19Inter_DeleteChannelD2Ev
    // IDA_EA: 0x84E9C2E
    // IDA_NAME: _ZN19Inter_DeleteChannelD0Ev
    // DEMANGLED: Inter_DeleteChannel::~Inter_DeleteChannel()
    virtual ~Inter_DeleteChannel();

    // IDA_EA: 0x84BF7EA
    // IDA_NAME: _ZN19Inter_DeleteChannel12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_DeleteChannel::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PvPChannelInfo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDEC4
    // IDA_NAME: _ZN20Inter_PvPChannelInfoC2Ev
    // DEMANGLED: Inter_PvPChannelInfo::Inter_PvPChannelInfo(void)
    Inter_PvPChannelInfo();

    // IDA_EA: 0x84F8592
    // IDA_NAME: _ZN20Inter_PvPChannelInfoD2Ev
    // IDA_EA: 0x84F85C2
    // IDA_NAME: _ZN20Inter_PvPChannelInfoD0Ev
    // DEMANGLED: Inter_PvPChannelInfo::~Inter_PvPChannelInfo()
    virtual ~Inter_PvPChannelInfo();

    // IDA_EA: 0x84D8096
    // IDA_NAME: _ZN20Inter_PvPChannelInfo12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_PvPChannelInfo::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PvPChannelUserCount : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDEE0
    // IDA_NAME: _ZN25Inter_PvPChannelUserCountC2Ev
    // DEMANGLED: Inter_PvPChannelUserCount::Inter_PvPChannelUserCount(void)
    Inter_PvPChannelUserCount();

    // IDA_EA: 0x84F8544
    // IDA_NAME: _ZN25Inter_PvPChannelUserCountD2Ev
    // IDA_EA: 0x84F8574
    // IDA_NAME: _ZN25Inter_PvPChannelUserCountD0Ev
    // DEMANGLED: Inter_PvPChannelUserCount::~Inter_PvPChannelUserCount()
    virtual ~Inter_PvPChannelUserCount();

    // IDA_EA: 0x84D8262
    // IDA_NAME: _ZN25Inter_PvPChannelUserCount12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_PvPChannelUserCount::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeOtherChannelChat : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA98C
    // IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatC2Ev
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChat::Inter_MonitorNoticeOtherChannelChat(void)
    Inter_MonitorNoticeOtherChannelChat();

    // IDA_EA: 0x84EA9A8
    // IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatD2Ev
    // IDA_EA: 0x84EA9D8
    // IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatD0Ev
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChat::~Inter_MonitorNoticeOtherChannelChat()
    virtual ~Inter_MonitorNoticeOtherChannelChat();

    // IDA_EA: 0x84CB1AC
    // IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChat12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChat::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeOtherChannelChatHyperLink : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEC54
    // IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkC2Ev
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::Inter_MonitorNoticeOtherChannelChatHyperLink(void)
    Inter_MonitorNoticeOtherChannelChatHyperLink();

    // IDA_EA: 0x84F5F7C
    // IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkD2Ev
    // IDA_EA: 0x84F5FAC
    // IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkD0Ev
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::~Inter_MonitorNoticeOtherChannelChatHyperLink()
    virtual ~Inter_MonitorNoticeOtherChannelChatHyperLink();

    // IDA_EA: 0x84E5528
    // IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLink12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeMemberLoginOut : public ISigDispatcher {
public:
    // IDA_EA: 0x84EAB9E
    // IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutC2Ev
    // DEMANGLED: Inter_MonitorNoticeMemberLoginOut::Inter_MonitorNoticeMemberLoginOut(void)
    Inter_MonitorNoticeMemberLoginOut();

    // IDA_EA: 0x84EABBA
    // IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutD2Ev
    // IDA_EA: 0x84EABEA
    // IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutD0Ev
    // DEMANGLED: Inter_MonitorNoticeMemberLoginOut::~Inter_MonitorNoticeMemberLoginOut()
    virtual ~Inter_MonitorNoticeMemberLoginOut();

    // IDA_EA: 0x84CC90A
    // IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOut12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorNoticeMemberLoginOut::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorNoticeGuildLoginOut : public ISigDispatcher {
public:
    // IDA_EA: 0x84EA568
    // IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutC2Ev
    // DEMANGLED: Inter_MonitorNoticeGuildLoginOut::Inter_MonitorNoticeGuildLoginOut(void)
    Inter_MonitorNoticeGuildLoginOut();

    // IDA_EA: 0x84EA584
    // IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutD2Ev
    // IDA_EA: 0x84EA5B4
    // IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutD0Ev
    // DEMANGLED: Inter_MonitorNoticeGuildLoginOut::~Inter_MonitorNoticeGuildLoginOut()
    virtual ~Inter_MonitorNoticeGuildLoginOut();

    // IDA_EA: 0x84C93E4
    // IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOut12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MonitorNoticeGuildLoginOut::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_DeleteChannel) == 0x04u,
              "Inter_DeleteChannel must retain its base-only i386 ABI");
static_assert(sizeof(Inter_PvPChannelInfo) == 0x04u,
              "Inter_PvPChannelInfo must retain its base-only i386 ABI");
static_assert(sizeof(Inter_PvPChannelUserCount) == 0x04u,
              "Inter_PvPChannelUserCount must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorNoticeOtherChannelChat) == 0x04u,
              "Inter_MonitorNoticeOtherChannelChat must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorNoticeOtherChannelChatHyperLink) == 0x04u,
              "Inter_MonitorNoticeOtherChannelChatHyperLink must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorNoticeMemberLoginOut) == 0x04u,
              "Inter_MonitorNoticeMemberLoginOut must retain its base-only i386 ABI");
static_assert(sizeof(Inter_MonitorNoticeGuildLoginOut) == 0x04u,
              "Inter_MonitorNoticeGuildLoginOut must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_DATA_CHANNEL_INTER_RETURNS_CHANNEL_INTER_RECEIVERS_H_
