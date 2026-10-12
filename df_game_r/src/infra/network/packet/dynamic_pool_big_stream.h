#ifndef OLDSRV_PROTOCOL_DYNAMIC_POOL_BIG_STREAM_H_
#define OLDSRV_PROTOCOL_DYNAMIC_POOL_BIG_STREAM_H_

#include <list>
#include <map>
#include <queue>
#include <vector>

#include "infra/network/packet/stream.h"


// IDA: specialized DynamicPool<BigStream> instance used by BigStreamPool.
class DynamicPoolBigStream {
public:
    class CNode : public BigStream {
    public:
        CNode();
        virtual ~CNode();

        unsigned int pool_dummy_value() const;
        unsigned char pool_ref_count_value() const;
        void increment_pool_ref_count();
        void decrement_pool_ref_count();
    };

    explicit DynamicPoolBigStream(int initial_count);
    ~DynamicPoolBigStream();

    DynamicPoolBigStream* SetPoolSize(unsigned short initial_count,
                                      unsigned short grow_count);
    BigStream* Acquire();
    bool IsValidPtr(BigStream* stream) const;
    int Free(BigStream* stream);
    int Size() const;
    int Remain() const;
    int MoreAlloc(int count);

private:
    std::queue<CNode*> free_nodes_;
    std::vector<CNode*> chunks_;
    std::map<void*, int> pointer_index_;
    int total_size_;
    unsigned short initial_count_;
    unsigned short grow_count_;
    int error_code_;
};


#endif  // OLDSRV_PROTOCOL_DYNAMIC_POOL_BIG_STREAM_H_
