#include "old_game/player/core/cuser_global_info_handle.h"

#include <cstring>

// IDA_EA: 0x86AD820
// IDA_NAME: _ZN21CUserGlobalInfoHandleC2Ev
// DEMANGLED: CUserGlobalInfoHandle::CUserGlobalInfoHandle(void)
CUserGlobalInfoHandle::CUserGlobalInfoHandle() : next_unique_id_(1) {
    // The old C2 clears [0, 0xfffe] only.  The singleton has static storage,
    // so byte 0xffff is already zero-initialized and remains the permanently
    // reserved value; preserve that exact allocation domain.
    std::memset(used_, 0, 0xffffu);
}

// IDA_EA: 0x86AD85A
// IDA_NAME: _ZN21CUserGlobalInfoHandleD2Ev
// DEMANGLED: CUserGlobalInfoHandle::~CUserGlobalInfoHandle()
CUserGlobalInfoHandle::~CUserGlobalInfoHandle() {}

// IDA_EA: 0x86AD8C2
// IDA_NAME: _ZN21CUserGlobalInfoHandle13find_uniqueidEv
// DEMANGLED: CUserGlobalInfoHandle::find_uniqueid(void)
int CUserGlobalInfoHandle::find_uniqueid() {
    for (uint32_t value = next_unique_id_; value <= 0xfffeu; ++value) {
        if (used_[value] != 1) {
            used_[value] = 1;
            return static_cast<int>(value);
        }
    }
    return -1;
}

// IDA_EA: 0x86AD910
// IDA_NAME: _ZN21CUserGlobalInfoHandle12get_uniqueidEv
// DEMANGLED: CUserGlobalInfoHandle::get_uniqueid(void)
int CUserGlobalInfoHandle::get_uniqueid() {
    int unique_id = find_uniqueid();
    if (unique_id == -1) {
        next_unique_id_ = 1;
        unique_id = find_uniqueid();
    }
    next_unique_id_ = static_cast<uint32_t>(unique_id + 1);
    return unique_id;
}

// IDA_EA: 0x86AD95E
// IDA_NAME: _ZN21CUserGlobalInfoHandle19reset_uniqueid_flagEt
// DEMANGLED: CUserGlobalInfoHandle::reset_uniqueid_flag(unsigned short)
int CUserGlobalInfoHandle::reset_uniqueid_flag(uint16_t unique_id) {
    used_[unique_id] = 0;
    return static_cast<int>(unique_id);
}

// IDA_EA: 0x86AD85F
// IDA_NAME: _Z29CUserGlobalInfoHandleInstancev
// DEMANGLED: CUserGlobalInfoHandleInstance(void)
CUserGlobalInfoHandle* CUserGlobalInfoHandleInstance() {
    static CUserGlobalInfoHandle instance;
    return &instance;
}
