#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_BOARD_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_BOARD_DISPATCHERS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The guild service reply keeps its twenty-four-byte common prefix ahead of
// zero or more 165-byte board records.  These layouts are consumed directly
// by the two old Inter handlers; retaining the fields avoids an untyped
// payload+offset implementation.
#pragma pack(push, 1)
struct GuildBoardReplyPrefix {
    PacketHeader header;      // +0x00
    uint16_t error;           // +0x0a
    uint16_t request_type;    // +0x0c: 9002/open, 9006/write
    uint8_t total_count;      // +0x0e
    uint8_t reserved_0f[4];   // +0x0f..+0x12
    uint32_t charac_no;       // +0x13
    uint8_t entry_count;      // +0x17
};

struct GuildBoardReplyEntry {
    char text[120];           // +0x00
    int32_t created_time;     // +0x78
    uint32_t board_index;     // +0x7c
    uint32_t author_charac_no; // +0x80
    uint8_t author_job;       // +0x84
    uint8_t reserved_85;      // +0x85
    uint8_t can_delete;       // +0x86
    char author_name[30];     // +0x87
};
#pragma pack(pop)

static_assert(sizeof(GuildBoardReplyPrefix) == 0x18u &&
                  offsetof(GuildBoardReplyPrefix, charac_no) == 0x13u &&
                  sizeof(GuildBoardReplyEntry) == 0xa5u &&
                  offsetof(GuildBoardReplyEntry, author_name) == 0x87u,
              "guild-board service reply ABI changed");

// CMD341--343 all share the original state-3 MSG_GUILD_BOARD template.  The
// concrete instances are intentionally not inserted into PacketDispatcher's
// 606-slot root table in this closure.
class Dispatcher_Open_Guild_Board
    : public IPacketDispatcher<MSG_GUILD_BOARD,
                               stGuildBoardParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85971D4
    Dispatcher_Open_Guild_Board();
    // IDA_EA: 0x8245502 / 0x8245532
    virtual ~Dispatcher_Open_Guild_Board();

    // IDA_EA: 0x81D307E / 0x81D30D0
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Write_On_The_GuildBoard
    : public IPacketDispatcher<MSG_GUILD_BOARD,
                               stGuildBoardParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85971F0
    Dispatcher_Write_On_The_GuildBoard();
    // IDA_EA: 0x82454B4 / 0x82454E4
    virtual ~Dispatcher_Write_On_The_GuildBoard();

    // IDA_EA: 0x81D314A / 0x81D31EE / 0x81D3320
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

class Dispatcher_Delete_GuildBoard_Text
    : public IPacketDispatcher<MSG_GUILD_BOARD,
                               stGuildBoardParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859720C
    Dispatcher_Delete_GuildBoard_Text();
    // IDA_EA: 0x8245466 / 0x8245496
    virtual ~Dispatcher_Delete_GuildBoard_Text();

    // IDA_EA: 0x81D334E / 0x81D33A0
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA units: 99_misc_global/Inter_GuildBoard{Data,Delete}.  Registration is
// owned by the still-unrestored InterDispatcher root, not by these leaves.
class Inter_GuildBoardData : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE4C8 / 0x84F751E / 0x84F754E
    Inter_GuildBoardData();
    virtual ~Inter_GuildBoardData();

    // IDA_EA: 0x84DEE6A
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GuildBoardDelete : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE4E4 / 0x84F74D0 / 0x84F7500
    Inter_GuildBoardDelete();
    virtual ~Inter_GuildBoardDelete();

    // IDA_EA: 0x84DF2AC
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Open_Guild_Board) == 0x04u &&
                  sizeof(Dispatcher_Write_On_The_GuildBoard) == 0x04u &&
                  sizeof(Dispatcher_Delete_GuildBoard_Text) == 0x04u &&
                  sizeof(Inter_GuildBoardData) == 0x04u &&
                  sizeof(Inter_GuildBoardDelete) == 0x04u,
              "CMD341--343 handler objects must retain their vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_BOARD_DISPATCHERS_H_
