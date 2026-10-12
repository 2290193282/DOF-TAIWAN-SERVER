#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_PAUSE_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_PAUSE_H_

#include <stdint.h>

#include "old_game/activity/schedule/advance_altar_dispatchers.h"

namespace CMDPacketStruct {

// IDA unit: 13_database_dispatch/advancealtar/_ReqAdvanceAltarPause.
struct _ReqAdvanceAltarPause : public STBaseRequest {
    _ReqAdvanceAltarPause();

    uint8_t raw_00[0x0d];
    uint8_t paused_0d;
};

// IDA unit: 13_database_dispatch/advancealtar/_ResAdvanceAltarPause.
struct _ResAdvanceAltarPause : public STBaseResponse {
    _ResAdvanceAltarPause();
    ~_ResAdvanceAltarPause();

    int32_t result_04;
    uint8_t paused_08;
    uint8_t raw_09[3];
};

#if defined(__i386__)
static_assert(sizeof(_ReqAdvanceAltarPause) == 0x0eu,
              "CMD578 request must retain its 14-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarPause) == 0x0cu,
              "CMD578 response must retain its 12-byte wire extent");
#endif

}  // namespace CMDPacketStruct

namespace advancealtar {

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_Pause.
class Dispatcher_Pause
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595778
    // IDA_NAME: _ZN12advancealtar16Dispatcher_PauseC2Ev
    Dispatcher_Pause();
    // IDA_EA: 0x81423C2 / 0x81423F2
    // IDA_NAME: _ZN12advancealtar16Dispatcher_PauseD2Ev /
    //           _ZN12advancealtar16Dispatcher_PauseD0Ev
    virtual ~Dispatcher_Pause();

    // IDA_EA: 0x8140A1C
    // IDA_NAME: _ZN12advancealtar16Dispatcher_Pause12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user, CMDPacketStruct::STBaseResponse& response);

    // IDA_EA: 0x8142410 / 0x814241A
    // IDA_NAME: _ZN12advancealtar16Dispatcher_Pause4readER9PacketBufR8MSG_BASE /
    //           _ZN12advancealtar16Dispatcher_Pause7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Pause) == 0x04u,
              "CMD578 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_PAUSE_H_
