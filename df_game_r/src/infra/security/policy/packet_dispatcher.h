#ifndef OLDSRV_SECURITY_PACKET_DISPATCHER_H_
#define OLDSRV_SECURITY_PACKET_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/i_dispatcher.h"
#include "infra/network/dispatcher/local_china_error_dispatcher.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;
class PacketBuf;
namespace message_counter {
class CUserPacketCounter;
}

// IDA_EA: C2 @0x8590A2E, get_dispatcher @0x85948E2,
// doDispatch @0x8594922. The dispatcher ABI remains the restored
// IDispatcher hierarchy, while the registry capacity follows the direct
// 140828 CMD enum instead of the old 606-slot protocol.
class PacketDispatcher {
public:
    // IDA_EA: 0x8590A2E / 0x8594840
    // IDA_NAME: _ZN16PacketDispatcherC2Ev / _ZN16PacketDispatcherD2Ev
    PacketDispatcher();
    ~PacketDispatcher();

    IDispatcher* getDispatcher(int packet_id);
    IDispatcher* get_dispatcher(int packet_id);
    int server_load_monitoring();
    int log_recv_packet(CUser* user,
                        ENUM_PACKETCLASS packet_class,
                        uint16_t packet_id,
                        char* packet,
                        int packet_len);
    int doDispatch(CUser* user,
                   ENUM_PACKETCLASS packet_class,
                   uint16_t packet_id,
                   char* packet,
                   int packet_len,
                   ENUM_PACKETCLASS source_class,
                   uint16_t source_packet_id);
    // IDA_EA: 0x8594E52
    // IDA_NAME: _ZN16PacketDispatcher8dispatchEiPci
    int dispatch(int user_index, char* packet, int packet_len);

    // Installs 140828 project-owned CMD520/521 only after the producer,
    // policy, and single authority gate are ready.  Repeated calls are
    // idempotent; a partially occupied pair fails closed.
    bool ConfigureChannelZombieMatchingDispatchers();

    // IDA global written by DispatchThread before it consumes DISPATCH_Q.
    // It is static storage and therefore has no effect on the 0x990 object.
    static message_counter::CUserPacketCounter* pUserPacketCounter_;

public:
    // Public only for the audited i386 layout assertion below; registration
    // and dispatch access remain confined to PacketDispatcher methods.
    IDispatcher* dispatchers_[ENUM_CMDPACKET_END];
    CLocalChinaErrorDispatcher local_china_error_dispatcher_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(PacketDispatcher, local_china_error_dispatcher_) ==
                  ENUM_CMDPACKET_END * sizeof(IDispatcher*),
              "PacketDispatcher target registry must precede local policy");
#endif

#endif  // OLDSRV_SECURITY_PACKET_DISPATCHER_H_
