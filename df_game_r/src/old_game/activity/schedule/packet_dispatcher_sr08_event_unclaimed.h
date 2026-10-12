#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_SR08_EVENT_UNCLAIMED_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_SR08_EVENT_UNCLAIMED_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;


// CMD490 begins directly after the packed 13-byte legacy C2S frame header.
#pragma pack(push, 1)
struct MSG_LOG_INGAME_ADVERTISEMENT : public MSG_BASE {
    MSG_LOG_INGAME_ADVERTISEMENT();

    char state;               // +0x0d
    short view_code;          // +0x0e
    char type_code;           // +0x10
    uint32_t banner_id;       // +0x11
};
#pragma pack(pop)


// The original constructor only invokes ParamBase's constructor.  result is
// intentionally not initialized until Dispatcher_LogIngameAdvertisement::process.
struct stLogIngameAdvertisement : public ParamBase {
    stLogIngameAdvertisement();
    virtual ~stLogIngameAdvertisement();

    int result;               // +0x04 on i386
};


#if defined(__i386__)
static_assert(sizeof(MSG_LOG_INGAME_ADVERTISEMENT) == 0x15u,
              "CMD490 request must retain the old packed 0x15-byte ABI");
static_assert(sizeof(stLogIngameAdvertisement) == 0x08u,
              "CMD490 parameter must retain the old ParamBase/result ABI");
#endif


class Dispatcher_LogIngameAdvertisement
    : public IPacketDispatcher<MSG_LOG_INGAME_ADVERTISEMENT,
                               stLogIngameAdvertisement,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_LogIngameAdvertisement();
    virtual ~Dispatcher_LogIngameAdvertisement();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};


#if defined(__i386__)
static_assert(sizeof(Dispatcher_LogIngameAdvertisement) == 0x04u,
              "CMD490 dispatcher must retain the old vptr-only ABI");
#endif


#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_SR08_EVENT_UNCLAIMED_H_
