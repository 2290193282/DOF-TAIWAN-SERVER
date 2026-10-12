#include "infra/network/packet/stream.h"

#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h"

#include <cstring>

// IDA_EA: 0x8328110
// IDA_NAME: _ZN22SIG_CERASHOP_ERROR_LOGC2Ev
// DEMANGLED: SIG_CERASHOP_ERROR_LOG::SIG_CERASHOP_ERROR_LOG(void)
SIG_CERASHOP_ERROR_LOG::SIG_CERASHOP_ERROR_LOG() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8451E30
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_GOBLIN_UPDATE_REWARDEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_REWARD * Stream::GetOutBuffer<SIG_GOBLIN_UPDATE_REWARD>(void)
template <>
SIG_GOBLIN_UPDATE_REWARD*
Stream::GetOutBuffer<SIG_GOBLIN_UPDATE_REWARD>() {
    const int bytes = sizeof(SIG_GOBLIN_UPDATE_REWARD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_GOBLIN_UPDATE_REWARD* const result =
        reinterpret_cast<SIG_GOBLIN_UPDATE_REWARD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84528C6
// IDA_NAME: _ZN6Stream12GetOutBufferI35SIG_INSERT_0712_WINTER_EVENT_COUPONEEPT_v
// DEMANGLED: SIG_INSERT_0712_WINTER_EVENT_COUPON * Stream::GetOutBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>(void)
template <>
SIG_INSERT_0712_WINTER_EVENT_COUPON*
Stream::GetOutBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>() {
    const int bytes = sizeof(SIG_INSERT_0712_WINTER_EVENT_COUPON);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_0712_WINTER_EVENT_COUPON* const result =
        reinterpret_cast<SIG_INSERT_0712_WINTER_EVENT_COUPON*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452930
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_INSERT_ACHIEVE_EVENTEEPT_v
// DEMANGLED: SIG_INSERT_ACHIEVE_EVENT * Stream::GetOutBuffer<SIG_INSERT_ACHIEVE_EVENT>(void)
template <>
SIG_INSERT_ACHIEVE_EVENT*
Stream::GetOutBuffer<SIG_INSERT_ACHIEVE_EVENT>() {
    const int bytes = sizeof(SIG_INSERT_ACHIEVE_EVENT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_ACHIEVE_EVENT* const result =
        reinterpret_cast<SIG_INSERT_ACHIEVE_EVENT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84535E8
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_SAVE_CONDITION_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CONDITION_EVENT * Stream::GetOutBuffer<SIG_SAVE_CONDITION_EVENT>(void)
template <>
SIG_SAVE_CONDITION_EVENT*
Stream::GetOutBuffer<SIG_SAVE_CONDITION_EVENT>() {
    const int bytes = sizeof(SIG_SAVE_CONDITION_EVENT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_CONDITION_EVENT* const result =
        reinterpret_cast<SIG_SAVE_CONDITION_EVENT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453990
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_SAVE_BLOOD_CLEAR_COUNTEEPT_v
// DEMANGLED: SIG_SAVE_BLOOD_CLEAR_COUNT * Stream::GetOutBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>(void)
template <>
SIG_SAVE_BLOOD_CLEAR_COUNT*
Stream::GetOutBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>() {
    const int bytes = sizeof(SIG_SAVE_BLOOD_CLEAR_COUNT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_BLOOD_CLEAR_COUNT* const result =
        reinterpret_cast<SIG_SAVE_BLOOD_CLEAR_COUNT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453FB0
// IDA_NAME: _ZN6Stream12GetOutBufferI31SIG_STATISTIC_CERASHOP_GIFT_LOGEEPT_v
// DEMANGLED: SIG_STATISTIC_CERASHOP_GIFT_LOG * Stream::GetOutBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>(void)
template <>
SIG_STATISTIC_CERASHOP_GIFT_LOG*
Stream::GetOutBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>() {
    const int bytes = sizeof(SIG_STATISTIC_CERASHOP_GIFT_LOG);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_STATISTIC_CERASHOP_GIFT_LOG* const result =
        reinterpret_cast<SIG_STATISTIC_CERASHOP_GIFT_LOG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845491E
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_CERASHOP_ERROR_LOGEEPT_v
// DEMANGLED: SIG_CERASHOP_ERROR_LOG * Stream::GetOutBuffer<SIG_CERASHOP_ERROR_LOG>(void)
template <>
SIG_CERASHOP_ERROR_LOG*
Stream::GetOutBuffer<SIG_CERASHOP_ERROR_LOG>() {
    const int bytes = sizeof(SIG_CERASHOP_ERROR_LOG);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CERASHOP_ERROR_LOG* const result =
        reinterpret_cast<SIG_CERASHOP_ERROR_LOG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8455A5A
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_GOBLIN_UPDATE_REWARDEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_REWARD * Stream::GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>(void)
template <>
SIG_GOBLIN_UPDATE_REWARD*
Stream::GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>() {
    const int bytes = sizeof(SIG_GOBLIN_UPDATE_REWARD);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GOBLIN_UPDATE_REWARD* const result =
        reinterpret_cast<SIG_GOBLIN_UPDATE_REWARD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845660E
// IDA_NAME: _ZN6Stream11GetInBufferI35SIG_INSERT_0712_WINTER_EVENT_COUPONEEPT_v
// DEMANGLED: SIG_INSERT_0712_WINTER_EVENT_COUPON * Stream::GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>(void)
template <>
SIG_INSERT_0712_WINTER_EVENT_COUPON*
Stream::GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>() {
    const int bytes = sizeof(SIG_INSERT_0712_WINTER_EVENT_COUPON);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_0712_WINTER_EVENT_COUPON* const result =
        reinterpret_cast<SIG_INSERT_0712_WINTER_EVENT_COUPON*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456678
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_INSERT_ACHIEVE_EVENTEEPT_v
// DEMANGLED: SIG_INSERT_ACHIEVE_EVENT * Stream::GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>(void)
template <>
SIG_INSERT_ACHIEVE_EVENT*
Stream::GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>() {
    const int bytes = sizeof(SIG_INSERT_ACHIEVE_EVENT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_ACHIEVE_EVENT* const result =
        reinterpret_cast<SIG_INSERT_ACHIEVE_EVENT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x86A19EE
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_SAVE_CONDITION_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CONDITION_EVENT * Stream::GetInBuffer<SIG_SAVE_CONDITION_EVENT>(void)
template <>
SIG_SAVE_CONDITION_EVENT*
Stream::GetInBuffer<SIG_SAVE_CONDITION_EVENT>() {
    const int bytes = sizeof(SIG_SAVE_CONDITION_EVENT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_CONDITION_EVENT* const result =
        reinterpret_cast<SIG_SAVE_CONDITION_EVENT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x86A1A58
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_SAVE_BLOOD_CLEAR_COUNTEEPT_v
// DEMANGLED: SIG_SAVE_BLOOD_CLEAR_COUNT * Stream::GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>(void)
template <>
SIG_SAVE_BLOOD_CLEAR_COUNT*
Stream::GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>() {
    const int bytes = sizeof(SIG_SAVE_BLOOD_CLEAR_COUNT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_BLOOD_CLEAR_COUNT* const result =
        reinterpret_cast<SIG_SAVE_BLOOD_CLEAR_COUNT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x86A25B2
// IDA_NAME: _ZN6Stream11GetInBufferI31SIG_STATISTIC_CERASHOP_GIFT_LOGEEPT_v
// DEMANGLED: SIG_STATISTIC_CERASHOP_GIFT_LOG * Stream::GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>(void)
template <>
SIG_STATISTIC_CERASHOP_GIFT_LOG*
Stream::GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>() {
    const int bytes = sizeof(SIG_STATISTIC_CERASHOP_GIFT_LOG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_STATISTIC_CERASHOP_GIFT_LOG* const result =
        reinterpret_cast<SIG_STATISTIC_CERASHOP_GIFT_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}
