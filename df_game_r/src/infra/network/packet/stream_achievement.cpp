#include "old_data/db/inventory/achievement_payload.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"

// IDA_EA: 0x8450922
// IDA_NAME: _ZN6Stream12GetOutBufferI24stAchievement_DBSaveDataEEPT_v
// DEMANGLED: stAchievement_DBSaveData * Stream::GetOutBuffer<stAchievement_DBSaveData>(void)
template <>
stAchievement_DBSaveData*
Stream::GetOutBuffer<stAchievement_DBSaveData>() {
    const int bytes = sizeof(stAchievement_DBSaveData);
    if (!enable_out(bytes)) {
        return 0;
    }
    stAchievement_DBSaveData* const result =
        reinterpret_cast<stAchievement_DBSaveData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x828EB48
// IDA_NAME: _ZN6Stream11GetInBufferI24stAchievement_DBSaveDataEEPT_v
// DEMANGLED: stAchievement_DBSaveData * Stream::GetInBuffer<stAchievement_DBSaveData>(void)
template <>
stAchievement_DBSaveData*
Stream::GetInBuffer<stAchievement_DBSaveData>() {
    const int bytes = sizeof(stAchievement_DBSaveData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    stAchievement_DBSaveData* const result =
        reinterpret_cast<stAchievement_DBSaveData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x828E1E0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24stAchievement_DBSaveDataEEPT_v
// DEMANGLED: stAchievement_DBSaveData * CStreamGuard::GetInBuffer<stAchievement_DBSaveData>(void)
template <>
stAchievement_DBSaveData*
CStreamGuard::GetInBuffer<stAchievement_DBSaveData>() {
    return stream_->GetInBuffer<stAchievement_DBSaveData>();
}
