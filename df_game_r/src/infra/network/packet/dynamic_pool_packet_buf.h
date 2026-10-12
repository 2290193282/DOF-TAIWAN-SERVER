#ifndef OLDSRV_PROTOCOL_DYNAMIC_POOL_PACKET_BUF_H_
#define OLDSRV_PROTOCOL_DYNAMIC_POOL_PACKET_BUF_H_

#include "infra/network/packet/packet_buf.h"

#include <map>
#include <queue>
#include <vector>


class DynamicPoolPacketBuf {
public:
    class CNode : public PacketBuf {
    public:
        CNode();
        ~CNode();

        unsigned int pool_dummy_value() const;
        unsigned char pool_ref_count_value() const;
        void increment_pool_ref_count();
        void decrement_pool_ref_count();

    private:
        unsigned int pool_dummy_;
        unsigned char pool_ref_count_;
    };

    explicit DynamicPoolPacketBuf(int initial_count);
    ~DynamicPoolPacketBuf();

    DynamicPoolPacketBuf* SetPoolSize(unsigned short initial_count,
                                      unsigned short grow_count);
    PacketBuf* Acquire();
    int Remain() const;
    int Size() const;
    int MoreAlloc(int count);
    int Free(PacketBuf* packet);

private:
    std::queue<CNode*> free_nodes_;
    std::vector<CNode*> chunks_;
    std::map<void*, int> pointer_index_;
    int total_size_;
    unsigned short initial_count_;
    unsigned short grow_count_;
    int error_code_;
};


#endif  // OLDSRV_PROTOCOL_DYNAMIC_POOL_PACKET_BUF_H_
