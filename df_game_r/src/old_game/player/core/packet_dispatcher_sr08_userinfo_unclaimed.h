#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_SR08_USERINFO_UNCLAIMED_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_SR08_USERINFO_UNCLAIMED_H_

#include <stdint.h>

#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/game_etc_option_payload.h"
#include "old_game/player/core/character_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;


// Native CMD197, CMD198 and CMD365 use their observed packed C2S payload
// alignment immediately after the 13-byte 140828 MSG_BASE header.
#pragma pack(push, 1)
struct MSG_SAVE_GAME_OPTION_1 : public MSG_BASE {
    MSG_SAVE_GAME_OPTION_1();

    uint32_t option_length;         // +0x0d
    CGameEtcOptionPayload option;   // +0x11
};

struct MSG_SAVE_GAME_OPTION_2 : public MSG_BASE {
    MSG_SAVE_GAME_OPTION_2();

    uint32_t option_length;  // +0x0d
    CHotkeyOption option;    // +0x11, variable wire extent <= 1280
};

struct MSG_QUICK_CHAT : public MSG_BASE {
    MSG_QUICK_CHAT();
    bool isLastMessage();

    uint32_t option_length;          // +0x0d
    uint8_t quick_chat_type;         // +0x11
    CQuickChattingOption option;     // +0x12
};
#pragma pack(pop)

struct stQuickChatParam : public ParamBase {
    stQuickChatParam();
    virtual ~stQuickChatParam();
};

#if defined(__i386__)
static_assert(sizeof(MSG_SAVE_GAME_OPTION_1) ==
                  sizeof(MSG_BASE) + sizeof(uint32_t) +
                      sizeof(CGameEtcOptionPayload),
              "CMD197 request extent must follow the configured option ABI");
static_assert(sizeof(MSG_SAVE_GAME_OPTION_2) == 0x511u,
              "CMD198 decoded request must hold the complete 1280-byte text");
static_assert(sizeof(MSG_QUICK_CHAT) == 0x13eu,
              "CMD365 request must retain its packed extent");
static_assert(sizeof(stQuickChatParam) == 0x04u,
              "CMD365 parameter must retain its vptr-only ABI");
#endif

class Dispatcher_SaveGameOption_1
    : public IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SaveGameOption_1();
    virtual ~Dispatcher_SaveGameOption_1();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

class Dispatcher_SaveGameOption_2
    : public IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SaveGameOption_2();
    virtual ~Dispatcher_SaveGameOption_2();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

class Dispatcher_SaveGameOption_QuickChatting
    : public IPacketDispatcher<MSG_QUICK_CHAT,
                               stQuickChatParam,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SaveGameOption_QuickChatting();
    virtual ~Dispatcher_SaveGameOption_QuickChatting();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SaveGameOption_ChattingEmoticon
    : public IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SaveGameOption_ChattingEmoticon();
    virtual ~Dispatcher_SaveGameOption_ChattingEmoticon();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SaveGameOption_1) == 0x04u,
              "CMD197 dispatcher must retain its vptr-only ABI");
static_assert(sizeof(Dispatcher_SaveGameOption_2) == 0x04u,
              "CMD198 dispatcher must retain its vptr-only ABI");
static_assert(sizeof(Dispatcher_SaveGameOption_QuickChatting) == 0x04u,
              "CMD365 dispatcher must retain its vptr-only ABI");
static_assert(sizeof(Dispatcher_SaveGameOption_ChattingEmoticon) == 0x04u,
              "CMD470 dispatcher must retain its vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_SR08_USERINFO_UNCLAIMED_H_
