#include "old_game/player/session/online_preliminary_match_list_runtime.h"
#include "old_game/player/session/online_preliminary_member_runtime.h"
#include "infra/network/packet/online_preliminary_stream.h"


// IDA_EA: 0x8586D22
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_ONLINE_PRELIMINARY_MEMBEREEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_MEMBER * Stream::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>(void)
template <>
SIG_ONLINE_PRELIMINARY_MEMBER*
Stream::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>() {
    while (enable_in(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER)) != true) {
        resize(size() * 2);
    }
    SIG_ONLINE_PRELIMINARY_MEMBER* result =
        reinterpret_cast<SIG_ONLINE_PRELIMINARY_MEMBER*>(buffer_ + in_ptr_);
    in_ptr(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER));
    return result;
}

// IDA_EA: 0x8586982
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_ONLINE_PRELIMINARY_MEMBEREEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_MEMBER * CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>(void)
template <>
SIG_ONLINE_PRELIMINARY_MEMBER*
CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>() {
    return stream_->GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER>();
}

// IDA_EA: 0x8587326
// IDA_NAME: _ZN6Stream11GetInBufferI40SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LISTEEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST * Stream::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>(void)
template <>
SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST*
Stream::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>() {
    while (enable_in(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST)) !=
           true) {
        resize(size() * 2);
    }
    SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST* result =
        reinterpret_cast<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST*>(
            buffer_ + in_ptr_);
    in_ptr(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST));
    return result;
}

// IDA_EA: 0x8586CB4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI40SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LISTEEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST * CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>(void)
template <>
SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST*
CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>() {
    return stream_->GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>();
}
