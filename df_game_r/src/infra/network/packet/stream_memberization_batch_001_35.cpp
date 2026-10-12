#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/load_etc_payload.h"

// IDA_EA: 0x80dd968
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_BOOSTER_GAGE_DATAEEPT_v
// DEMANGLED: SIG_BOOSTER_GAGE_DATA * Stream::GetInBuffer<SIG_BOOSTER_GAGE_DATA>(void)
template <>
SIG_BOOSTER_GAGE_DATA* Stream::GetInBuffer<SIG_BOOSTER_GAGE_DATA>() {
    const int bytes = 8;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_BOOSTER_GAGE_DATA* result =
        reinterpret_cast<SIG_BOOSTER_GAGE_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80DD7DC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_BOOSTER_GAGE_DATAEEPT_v
// DEMANGLED: SIG_BOOSTER_GAGE_DATA * CStreamGuard::GetInBuffer<SIG_BOOSTER_GAGE_DATA>(void)
template <>
SIG_BOOSTER_GAGE_DATA* CStreamGuard::GetInBuffer<SIG_BOOSTER_GAGE_DATA>() {
    return stream_->GetInBuffer<SIG_BOOSTER_GAGE_DATA>();
}

// IDA_EA: 0x811df42
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_ITEM_DICTIONARY_FLAGEEPT_v
// DEMANGLED: SIG_ITEM_DICTIONARY_FLAG * Stream::GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>(void)
template <>
SIG_ITEM_DICTIONARY_FLAG* Stream::GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>() {
    const int bytes = 5004;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_ITEM_DICTIONARY_FLAG* result =
        reinterpret_cast<SIG_ITEM_DICTIONARY_FLAG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x811DF2C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_ITEM_DICTIONARY_FLAGEEPT_v
// DEMANGLED: SIG_ITEM_DICTIONARY_FLAG * CStreamGuard::GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>(void)
template <>
SIG_ITEM_DICTIONARY_FLAG* CStreamGuard::GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>() {
    return stream_->GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>();
}

// IDA_EA: 0x823979c
// IDA_NAME: _ZN6Stream11GetInBufferI14SIG_LOAD_SKILLEEPT_v
// DEMANGLED: SIG_LOAD_SKILL * Stream::GetInBuffer<SIG_LOAD_SKILL>(void)
template <>
SIG_LOAD_SKILL* Stream::GetInBuffer<SIG_LOAD_SKILL>() {
    const int bytes = 1044;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_SKILL* result =
        reinterpret_cast<SIG_LOAD_SKILL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D46
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI14SIG_LOAD_SKILLEEPT_v
// DEMANGLED: SIG_LOAD_SKILL * CStreamGuard::GetInBuffer<SIG_LOAD_SKILL>(void)
template <>
SIG_LOAD_SKILL* CStreamGuard::GetInBuffer<SIG_LOAD_SKILL>() {
    return stream_->GetInBuffer<SIG_LOAD_SKILL>();
}

// IDA_EA: 0x8239944
// IDA_NAME: _ZN6Stream11GetInBufferI12SIG_LOAD_ETCEEPT_v
// DEMANGLED: SIG_LOAD_ETC * Stream::GetInBuffer<SIG_LOAD_ETC>(void)
template <>
SIG_LOAD_ETC* Stream::GetInBuffer<SIG_LOAD_ETC>() {
    const int bytes = 84832;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_ETC* result =
        reinterpret_cast<SIG_LOAD_ETC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D9E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI12SIG_LOAD_ETCEEPT_v
// DEMANGLED: SIG_LOAD_ETC * CStreamGuard::GetInBuffer<SIG_LOAD_ETC>(void)
template <>
SIG_LOAD_ETC* CStreamGuard::GetInBuffer<SIG_LOAD_ETC>() {
    return stream_->GetInBuffer<SIG_LOAD_ETC>();
}

// IDA_EA: 0x85460da
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_ITEM_MAKING_SKILLEEPT_v
// DEMANGLED: SIG_ITEM_MAKING_SKILL * Stream::GetInBuffer<SIG_ITEM_MAKING_SKILL>(void)
template <>
SIG_ITEM_MAKING_SKILL* Stream::GetInBuffer<SIG_ITEM_MAKING_SKILL>() {
    const int bytes = 28;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_ITEM_MAKING_SKILL* result =
        reinterpret_cast<SIG_ITEM_MAKING_SKILL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8545FE8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_ITEM_MAKING_SKILLEEPT_v
// DEMANGLED: SIG_ITEM_MAKING_SKILL * CStreamGuard::GetInBuffer<SIG_ITEM_MAKING_SKILL>(void)
template <>
SIG_ITEM_MAKING_SKILL* CStreamGuard::GetInBuffer<SIG_ITEM_MAKING_SKILL>() {
    return stream_->GetInBuffer<SIG_ITEM_MAKING_SKILL>();
}

// IDA_EA: 0x85e9338
// IDA_NAME: _ZN6Stream11GetInBufferI28SIG_REQUEST_PVP_MISSION_LISTEEPT_v
// DEMANGLED: SIG_REQUEST_PVP_MISSION_LIST * Stream::GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>(void)
template <>
SIG_REQUEST_PVP_MISSION_LIST* Stream::GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>() {
    const int bytes = 330;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQUEST_PVP_MISSION_LIST* result =
        reinterpret_cast<SIG_REQUEST_PVP_MISSION_LIST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x85E8056
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_REQUEST_PVP_MISSION_LISTEEPT_v
// DEMANGLED: SIG_REQUEST_PVP_MISSION_LIST * CStreamGuard::GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>(void)
template <>
SIG_REQUEST_PVP_MISSION_LIST* CStreamGuard::GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>() {
    return stream_->GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>();
}

// IDA_EA: 0x85f81b2
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_REDEEMINFO_LIST_TO_DBEEPT_v
// DEMANGLED: SIG_REDEEMINFO_LIST_TO_DB * Stream::GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>(void)
template <>
SIG_REDEEMINFO_LIST_TO_DB* Stream::GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>() {
    const int bytes = 1436;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REDEEMINFO_LIST_TO_DB* result =
        reinterpret_cast<SIG_REDEEMINFO_LIST_TO_DB*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x85F7F7E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_REDEEMINFO_LIST_TO_DBEEPT_v
// DEMANGLED: SIG_REDEEMINFO_LIST_TO_DB * CStreamGuard::GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>(void)
template <>
SIG_REDEEMINFO_LIST_TO_DB* CStreamGuard::GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>() {
    return stream_->GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>();
}

// IDA_EA: 0x8643a44
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_REQUEST_TOD_USER_STATEEEPT_v
// DEMANGLED: SIG_REQUEST_TOD_USER_STATE * Stream::GetInBuffer<SIG_REQUEST_TOD_USER_STATE>(void)
template <>
SIG_REQUEST_TOD_USER_STATE* Stream::GetInBuffer<SIG_REQUEST_TOD_USER_STATE>() {
    const int bytes = 24;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQUEST_TOD_USER_STATE* result =
        reinterpret_cast<SIG_REQUEST_TOD_USER_STATE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8643A2E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_REQUEST_TOD_USER_STATEEEPT_v
// DEMANGLED: SIG_REQUEST_TOD_USER_STATE * CStreamGuard::GetInBuffer<SIG_REQUEST_TOD_USER_STATE>(void)
template <>
SIG_REQUEST_TOD_USER_STATE* CStreamGuard::GetInBuffer<SIG_REQUEST_TOD_USER_STATE>() {
    return stream_->GetInBuffer<SIG_REQUEST_TOD_USER_STATE>();
}
