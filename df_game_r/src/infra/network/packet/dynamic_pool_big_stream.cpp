#include "infra/network/packet/dynamic_pool_big_stream.h"


namespace {

const unsigned int kDynamicPoolDummy = 36641;

}  // namespace

// IDA_EA: 0x82C1A48
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE5CNodeC2Ev
// DEMANGLED: DynamicPool<BigStream>::CNode::CNode(void)
DynamicPoolBigStream::CNode::CNode() : BigStream() {
    pool_dummy_ = kDynamicPoolDummy;
    pool_ref_count_ = 0;
}

// IDA_EA: 0x82AD9CC
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE5CNodeD2Ev
// DEMANGLED: DynamicPool<BigStream>::CNode::~CNode()
// IDA_EA: 0x82AD9FC
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE5CNodeD0Ev
// DEMANGLED: DynamicPool<BigStream>::CNode::~CNode() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DynamicPoolBigStream::CNode::~CNode() {}

unsigned int DynamicPoolBigStream::CNode::pool_dummy_value() const {
    return pool_dummy_;
}

unsigned char DynamicPoolBigStream::CNode::pool_ref_count_value() const {
    return pool_ref_count_;
}

void DynamicPoolBigStream::CNode::increment_pool_ref_count() {
    ++pool_ref_count_;
}

void DynamicPoolBigStream::CNode::decrement_pool_ref_count() {
    --pool_ref_count_;
}

// IDA_EA: 0x82AD830
// IDA_NAME: _ZN11DynamicPoolI9BigStreamEC2Ei
// DEMANGLED: DynamicPool<BigStream>::DynamicPool(int)
DynamicPoolBigStream::DynamicPoolBigStream(int initial_count)
    : free_nodes_(),
      chunks_(),
      pointer_index_(),
      total_size_(0),
      initial_count_(static_cast<unsigned short>(initial_count)),
      grow_count_(static_cast<unsigned short>(initial_count)),
      error_code_(0) {
    MoreAlloc(initial_count);
}

// IDA_EA: 0x82ADA1A
// IDA_NAME: _ZN11DynamicPoolI9BigStreamED2Ev
// DEMANGLED: DynamicPool<BigStream>::~DynamicPool()
DynamicPoolBigStream::~DynamicPoolBigStream() {
    for (std::vector<CNode*>::iterator it = chunks_.begin();
         it != chunks_.end(); ++it) {
        delete[] *it;
    }
    chunks_.clear();
    while (!free_nodes_.empty()) {
        free_nodes_.pop();
    }
    pointer_index_.clear();
    total_size_ = 0;
}

// IDA_EA: 0x82AD952
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE11SetPoolSizeEtt
// DEMANGLED: DynamicPool<BigStream>::SetPoolSize(unsigned short,unsigned short)
DynamicPoolBigStream* DynamicPoolBigStream::SetPoolSize(
    unsigned short initial_count, unsigned short grow_count) {
    initial_count_ = initial_count;
    grow_count_ = grow_count;
    return this;
}

// IDA_EA: 0x82ADB60
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE7AcquireEv
// DEMANGLED: DynamicPool<BigStream>::Acquire(void)
BigStream* DynamicPoolBigStream::Acquire() {
    if (free_nodes_.empty()) {
        MoreAlloc(grow_count_);
    }
    if (free_nodes_.empty()) {
        return 0;
    }

    CNode* node = free_nodes_.front();
    free_nodes_.pop();
    if (node->pool_dummy_value() != kDynamicPoolDummy) {
        error_code_ = 3;
    }
    if (node->pool_ref_count_value() != 0) {
        error_code_ = 4;
    }
    node->increment_pool_ref_count();
    return node;
}

// IDA_EA: 0x82ADDA4
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE10IsValidPtrEPS0_
// DEMANGLED: DynamicPool<BigStream>::IsValidPtr(BigStream*)
bool DynamicPoolBigStream::IsValidPtr(BigStream* stream) const {
    return pointer_index_.find(stream) != pointer_index_.end();
}

// IDA_EA: 0x82ADE0C
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE4FreeEPS0_
// DEMANGLED: DynamicPool<BigStream>::Free(BigStream*)
int DynamicPoolBigStream::Free(BigStream* stream) {
    if (stream == 0) {
        return 0;
    }
    CNode* node = static_cast<CNode*>(stream);
    free_nodes_.push(node);
    if (node->pool_dummy_value() != kDynamicPoolDummy) {
        error_code_ = 5;
    }
    if (node->pool_ref_count_value() != 1) {
        error_code_ = 6;
    }
    node->decrement_pool_ref_count();
    return 1;
}

// IDA_EA: 0x82ADF1C
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE4SizeEv
// DEMANGLED: DynamicPool<BigStream>::Size(void)
int DynamicPoolBigStream::Size() const {
    return total_size_;
}

// IDA_EA: 0x82ADF28
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE6RemainEv
// DEMANGLED: DynamicPool<BigStream>::Remain(void)
int DynamicPoolBigStream::Remain() const {
    return static_cast<int>(free_nodes_.size());
}

// IDA_EA: 0x82B85B8
// IDA_NAME: _ZN11DynamicPoolI9BigStreamE9MoreAllocEi
// DEMANGLED: DynamicPool<BigStream>::MoreAlloc(int)
int DynamicPoolBigStream::MoreAlloc(int count) {
    if (count <= 0) {
        return 0;
    }
    CNode* chunk = new CNode[count];
    if (chunk == 0) {
        return 0;
    }
    chunks_.push_back(chunk);
    for (int index = 0; index < count; ++index) {
        CNode* node = &chunk[index];
        free_nodes_.push(node);
        pointer_index_[node] = 1;
    }
    total_size_ += count;
    return 1;
}
