#ifndef OLDSRV_CHANNEL_CHANNEL_PACKETS_H_
#define OLDSRV_CHANNEL_CHANNEL_PACKETS_H_

#include "infra/network/packet/PacketHeader.h"
#include "old_data/channel/arad_server_state_message.h"

#include <cstddef>
#include <stdint.h>

#pragma pack(push, 1)

struct Packet_ChannelType {
    Packet_ChannelType();

    PacketHeader header;
    unsigned char payload[0x12 - sizeof(PacketHeader)];
};

struct Packet_PvPChannelInfo {
    Packet_PvPChannelInfo();

    PacketHeader header;
    unsigned char payload[0x17F8 - sizeof(PacketHeader)];
};

struct Packet_Monitor_Other_Channel_Chat {
    Packet_Monitor_Other_Channel_Chat();

    PacketHeader header;
    unsigned char chat_type;
    unsigned char reserved_0b_0d[3];
    uint32_t account_id;
    unsigned char sender_marker;
    uint32_t charac_no;
    uint32_t item_position;
    unsigned char target_name_length;
    char target_name[30];
    unsigned char message_length;
    char message[256];
};

struct Packet_Monitor_Guild_Chat {
    Packet_Monitor_Guild_Chat();

    PacketHeader header;
    uint32_t guild_key;
    uint32_t charac_no;
    unsigned char message_length;
    char message[256];
};

struct Packet_Monitor_Member_Chat {
    Packet_Monitor_Member_Chat();

    PacketHeader header;
    uint32_t member_key;
    unsigned char message_length;
    char message[256];
};

struct Packet_Monitor_MegaPhone {
    Packet_Monitor_MegaPhone();

    Packet_Monitor_MegaPhone* set(unsigned char channel_no,
                                  uint16_t unique_id,
                                  const char* charac_name,
                                  unsigned char message_length,
                                  const char* message,
                                  unsigned char chat_type);

    PacketHeader header;
    unsigned char reserved_0a;
    unsigned char channel_no;
    uint16_t unique_id;
    unsigned char chat_type;
    char charac_name[30];
    unsigned char message_length;
    char message[256];
};

struct Packet_Monitor_Other_Channel_Chat_Hyper_Link {
    Packet_Monitor_Other_Channel_Chat_Hyper_Link();

    PacketHeader header;
    // PacketHeader occupies +0x00..+0x09.  The first two fields are left
    // intact by the original C2 and are populated by the world chat router.
    uint32_t chat_type;                  // +0x0a
    uint32_t account_id;                 // +0x0e, C2 zeroes; unused here
    unsigned char sender_marker;         // +0x12, C2 zeroes; unused here
    uint32_t charac_no;                  // +0x13
    uint32_t item_position;              // +0x17
    unsigned char target_name_length;    // +0x1b
    char target_name[30];                // +0x1c
    unsigned char hyperlink_count;       // +0x3a
    unsigned char hyperlinks[3][104];    // +0x3b
    unsigned char message_length;        // +0x173
    char message[256];                   // +0x174
};

struct Packet_Monitor_Guild_Chat_Hyper_Link {
    Packet_Monitor_Guild_Chat_Hyper_Link();

    PacketHeader header;
    uint32_t guild_key;                  // +0x0a
    uint32_t charac_no;                  // +0x0e
    unsigned char hyperlink_count;       // +0x12
    unsigned char hyperlinks[3][104];    // +0x13
    unsigned char message_length;        // +0x14b
    char message[256];                   // +0x14c
};

struct Packet_Monitor_Member_Chat_Hyper_Link {
    Packet_Monitor_Member_Chat_Hyper_Link();

    PacketHeader header;
    uint32_t member_key;                 // +0x0a
    unsigned char hyperlink_count;       // +0x0e
    unsigned char hyperlinks[3][104];    // +0x0f
    unsigned char message_length;        // +0x147
    char message[256];                   // +0x148
};

struct Packet_Monitor_MegaPhone_Hyper_Link {
    Packet_Monitor_MegaPhone_Hyper_Link();

    Packet_Monitor_MegaPhone_Hyper_Link* set(
        unsigned char channel_no,
        uint16_t unique_id,
        const char* charac_name,
        unsigned char message_length,
        const char* message,
        unsigned char chat_type,
        unsigned char hyperlink_count,
        const unsigned char hyperlinks[3][104]);

    PacketHeader header;
    unsigned char reserved_0a;           // +0x0a
    unsigned char channel_no;            // +0x0b
    uint16_t unique_id;                  // +0x0c
    unsigned char chat_type;             // +0x0e
    char charac_name[30];                // +0x0f
    unsigned char hyperlink_count;       // +0x2d
    unsigned char hyperlinks[3][104];    // +0x2e
    unsigned char message_length;        // +0x166
    char message[256];                   // +0x167
};

#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Other_Channel_Chat) == 0x13Bu,
              "other-channel chat packet size must stay 0x13b");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, chat_type) == 0x0Au,
              "other-channel chat type offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, account_id) == 0x0Eu,
              "other-channel account id offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, sender_marker) == 0x12u,
              "other-channel sender marker offset must stay 0x12");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, charac_no) == 0x13u,
              "other-channel character number offset must stay 0x13");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, item_position) == 0x17u,
              "other-channel item position offset must stay 0x17");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, target_name_length) ==
                  0x1Bu,
              "other-channel target-name length offset must stay 0x1b");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, target_name) == 0x1Cu,
              "other-channel target-name offset must stay 0x1c");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, message_length) ==
                  0x3Au,
              "other-channel message length offset must stay 0x3a");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat, message) == 0x3Bu,
              "other-channel message offset must stay 0x3b");

static_assert(sizeof(Packet_Monitor_Guild_Chat) == 0x113u,
              "guild chat packet size must stay 0x113");
static_assert(offsetof(Packet_Monitor_Guild_Chat, guild_key) == 0x0Au,
              "guild chat key offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Guild_Chat, charac_no) == 0x0Eu,
              "guild chat character number offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_Guild_Chat, message_length) == 0x12u,
              "guild chat message length offset must stay 0x12");
static_assert(offsetof(Packet_Monitor_Guild_Chat, message) == 0x13u,
              "guild chat message offset must stay 0x13");

static_assert(sizeof(Packet_Monitor_Member_Chat) == 0x10Fu,
              "member chat packet size must stay 0x10f");
static_assert(offsetof(Packet_Monitor_Member_Chat, member_key) == 0x0Au,
              "member chat key offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Member_Chat, message_length) == 0x0Eu,
              "member chat message length offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_Member_Chat, message) == 0x0Fu,
              "member chat message offset must stay 0x0f");

static_assert(sizeof(Packet_Monitor_MegaPhone) == 0x12Eu,
              "megaphone packet size must stay 0x12e");
static_assert(offsetof(Packet_Monitor_MegaPhone, reserved_0a) == 0x0Au,
              "megaphone reserved byte offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_MegaPhone, channel_no) == 0x0Bu,
              "megaphone channel offset must stay 0x0b");
static_assert(offsetof(Packet_Monitor_MegaPhone, unique_id) == 0x0Cu,
              "megaphone unique id offset must stay 0x0c");
static_assert(offsetof(Packet_Monitor_MegaPhone, chat_type) == 0x0Eu,
              "megaphone chat type offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_MegaPhone, charac_name) == 0x0Fu,
              "megaphone character-name offset must stay 0x0f");
static_assert(offsetof(Packet_Monitor_MegaPhone, message_length) == 0x2Du,
              "megaphone message length offset must stay 0x2d");
static_assert(offsetof(Packet_Monitor_MegaPhone, message) == 0x2Eu,
              "megaphone message offset must stay 0x2e");

static_assert(sizeof(Packet_Monitor_Other_Channel_Chat_Hyper_Link) == 0x274u,
              "other-channel hyperlink chat packet size must stay 0x274");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       chat_type) == 0x0Au,
              "other-channel hyperlink chat type offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       charac_no) == 0x13u,
              "other-channel hyperlink character number offset must stay 0x13");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       item_position) == 0x17u,
              "other-channel hyperlink item position offset must stay 0x17");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       target_name_length) == 0x1Bu,
              "other-channel hyperlink target-name length offset must stay 0x1b");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       target_name) == 0x1Cu,
              "other-channel hyperlink target-name offset must stay 0x1c");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       hyperlink_count) == 0x3Au,
              "other-channel hyperlink count offset must stay 0x3a");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       hyperlinks) == 0x3Bu,
              "other-channel hyperlink records offset must stay 0x3b");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       message_length) == 0x173u,
              "other-channel hyperlink message length offset must stay 0x173");
static_assert(offsetof(Packet_Monitor_Other_Channel_Chat_Hyper_Link,
                       message) == 0x174u,
              "other-channel hyperlink message offset must stay 0x174");

static_assert(sizeof(Packet_Monitor_Guild_Chat_Hyper_Link) == 0x24Cu,
              "guild hyperlink chat packet size must stay 0x24c");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link, guild_key) ==
                  0x0Au,
              "guild hyperlink key offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link, charac_no) ==
                  0x0Eu,
              "guild hyperlink character number offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link,
                       hyperlink_count) == 0x12u,
              "guild hyperlink count offset must stay 0x12");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link, hyperlinks) ==
                  0x13u,
              "guild hyperlink records offset must stay 0x13");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link,
                       message_length) == 0x14Bu,
              "guild hyperlink message length offset must stay 0x14b");
static_assert(offsetof(Packet_Monitor_Guild_Chat_Hyper_Link, message) ==
                  0x14Cu,
              "guild hyperlink message offset must stay 0x14c");

static_assert(sizeof(Packet_Monitor_Member_Chat_Hyper_Link) == 0x248u,
              "member hyperlink chat packet size must stay 0x248");
static_assert(offsetof(Packet_Monitor_Member_Chat_Hyper_Link, member_key) ==
                  0x0Au,
              "member hyperlink key offset must stay 0x0a");
static_assert(offsetof(Packet_Monitor_Member_Chat_Hyper_Link,
                       hyperlink_count) == 0x0Eu,
              "member hyperlink count offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_Member_Chat_Hyper_Link, hyperlinks) ==
                  0x0Fu,
              "member hyperlink records offset must stay 0x0f");
static_assert(offsetof(Packet_Monitor_Member_Chat_Hyper_Link,
                       message_length) == 0x147u,
              "member hyperlink message length offset must stay 0x147");
static_assert(offsetof(Packet_Monitor_Member_Chat_Hyper_Link, message) ==
                  0x148u,
              "member hyperlink message offset must stay 0x148");

static_assert(sizeof(Packet_Monitor_MegaPhone_Hyper_Link) == 0x267u,
              "hyperlink megaphone packet size must stay 0x267");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, channel_no) ==
                  0x0Bu,
              "hyperlink megaphone channel offset must stay 0x0b");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, unique_id) ==
                  0x0Cu,
              "hyperlink megaphone unique id offset must stay 0x0c");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, chat_type) ==
                  0x0Eu,
              "hyperlink megaphone chat type offset must stay 0x0e");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, charac_name) ==
                  0x0Fu,
              "hyperlink megaphone character name offset must stay 0x0f");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link,
                       hyperlink_count) == 0x2Du,
              "hyperlink megaphone count offset must stay 0x2d");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, hyperlinks) ==
                  0x2Eu,
              "hyperlink megaphone records offset must stay 0x2e");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link,
                       message_length) == 0x166u,
              "hyperlink megaphone message length offset must stay 0x166");
static_assert(offsetof(Packet_Monitor_MegaPhone_Hyper_Link, message) ==
                  0x167u,
              "hyperlink megaphone message offset must stay 0x167");

#endif  // OLDSRV_CHANNEL_CHANNEL_PACKETS_H_
