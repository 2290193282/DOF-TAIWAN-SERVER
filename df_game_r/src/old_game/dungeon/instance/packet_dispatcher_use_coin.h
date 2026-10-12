#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_USE_COIN_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_USE_COIN_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/DisPatcher_UseCoin.  The client sends only
// the character unique-id to revive; the 13-byte C2S header is inherited.
#pragma pack(push, 1)
struct MSG_USE_COIN : MSG_BASE {
    uint16_t target_unique_id;  // +0x0d
};
#pragma pack(pop)

static_assert(sizeof(MSG_USE_COIN) == 0x0fu,
              "CMD041 use-coin request must retain its 15-byte ABI");

// IDA unit: 99_misc_global/stUseCoinParam.  The result is stored at +4 and
// the acknowledged unique id at +8 before send() emits CMD041's reply.
struct stUseCoinParam : ParamBase {
    stUseCoinParam()
        : result_code(0),
          target_unique_id(0),
          free_revive_mode(0),
          reserved_0b(0) {}
    virtual ~stUseCoinParam() {}

    int result_code;              // +0x04
    uint16_t target_unique_id;    // +0x08
    // 140828 success tail.  The client takes its no-coin-consumption path
    // only when this byte is one.
    uint8_t free_revive_mode;     // +0x0a
    uint8_t reserved_0b;          // +0x0b
};

#if defined(__i386__)
static_assert(sizeof(stUseCoinParam) == 0x0cu,
              "CMD041 parameter must retain the 140828 12-byte ABI");
static_assert(offsetof(stUseCoinParam, result_code) == 0x04u,
              "CMD041 result-code offset mismatch");
static_assert(offsetof(stUseCoinParam, target_unique_id) == 0x08u,
              "CMD041 acknowledged-id offset mismatch");
static_assert(offsetof(stUseCoinParam, free_revive_mode) == 0x0au,
              "CMD041 free-revive-mode offset mismatch");
#endif

class CParty;

class DisPatcher_UseCoin
    : public IPacketDispatcher<MSG_USE_COIN,
                               stUseCoinParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85952F4
    DisPatcher_UseCoin();
    // IDA_EA: 0x823276A / 0x823279A
    virtual ~DisPatcher_UseCoin();

    // IDA_EA: 0x81CA5E4
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x81CA852
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x81CAA42
    virtual void send(CUser* user, ParamBase& param);

private:
    // IDA_EA: 0x81CA63A / 0x81CA75C / 0x81CA7D6 / 0x81CAB3A
    int Dungeon_check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    int WarRoom_check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    int usedCoinDungeonCheck(CUser* user, CParty* party);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_UseCoin) == 0x04u,
              "CMD041 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_USE_COIN_H_
