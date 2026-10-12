#ifndef OLDSRV_PROTOCOL_PACKET_POOL_H_
#define OLDSRV_PROTOCOL_PACKET_POOL_H_

#include "infra/network/packet/dynamic_pool_packet_buf.h"

#include <pthread.h>


class PacketPool {
public:
    PacketPool();
    PacketPool(int initial_count, int grow_count);
    ~PacketPool();

    PacketBuf* Acquire();
    PacketPool* setThreadID(unsigned long thread_id);
    pthread_t getThreadID() const;
    int Free(PacketBuf* packet);
    int Size() const;
    int Remain() const;

private:
    DynamicPoolPacketBuf* pool_;
    pthread_t thread_id_;
};


#endif  // OLDSRV_PROTOCOL_PACKET_POOL_H_
