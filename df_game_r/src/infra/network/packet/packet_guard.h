#ifndef OLDSRV_PROTOCOL_PACKET_GUARD_H_
#define OLDSRV_PROTOCOL_PACKET_GUARD_H_

#include "infra/network/packet/interface_packet_buf.h"


class PacketPool;

class PacketGuard : public InterfacePacketBuf {
public:
    PacketGuard();
    explicit PacketGuard(PacketBuf* packet);
    ~PacketGuard();

private:
    PacketPool* pool_;
    bool external_packet_;
};


#endif  // OLDSRV_PROTOCOL_PACKET_GUARD_H_
