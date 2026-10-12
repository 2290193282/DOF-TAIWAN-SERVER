#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_FINISH_LOAD_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_FINISH_LOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/DisPatcher_FinishLoad.  The two client
// checksums begin immediately after the 13-byte command-frame header.
#pragma pack(push, 1)
struct MSG_FINISH_LOADING : MSG_BASE {
    int32_t inventory_checksum;  // +0x0d
    int32_t skill_checksum;      // +0x11
};
#pragma pack(pop)

static_assert(sizeof(MSG_FINISH_LOADING) == 0x15u,
              "CMD040 finish-loading request must retain its 21-byte ABI");

// IDA unit: 99_misc_global/stFinishLoadParam.
struct stFinishLoadParam : ParamBase {
    stFinishLoadParam() : result_code(0) {}
    virtual ~stFinishLoadParam() {}

    int result_code;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stFinishLoadParam) == 0x08u,
              "CMD040 parameter must retain the original 8-byte ABI");
static_assert(offsetof(stFinishLoadParam, result_code) == 0x04u,
              "CMD040 result-code offset mismatch");
#endif

class DisPatcher_FinishLoad
    : public IPacketDispatcher<MSG_FINISH_LOADING,
                               stFinishLoadParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595A6C
    DisPatcher_FinishLoad();
    // IDA_EA: 0x8265540 / 0x8265570
    virtual ~DisPatcher_FinishLoad();

    // IDA_EA: 0x8264B66 / 0x826503C / 0x8265492
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int check_state(CUser* user, PacketBuf& packet);

private:
    // IDA_EA: 0x8264BFE / 0x8264C28
    int check_error(CUser* user, MSG_BASE& message);
    char Party_hack_check(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_FinishLoad) == 0x04u,
              "CMD040 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_FINISH_LOAD_H_
