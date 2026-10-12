#include "infra/network/packet/stream_guard.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA_EA: 0x8457e86
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_SAVE_PC_ROOM_PLAY_TIMEEEPT_v
// DEMANGLED: SIG_SAVE_PC_ROOM_PLAY_TIME * Stream::GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>(void)
template <>
SIG_SAVE_PC_ROOM_PLAY_TIME* Stream::GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>() {
    const int bytes = 9;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_PC_ROOM_PLAY_TIME* result =
        reinterpret_cast<SIG_SAVE_PC_ROOM_PLAY_TIME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84545BC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_SAVE_PC_ROOM_PLAY_TIMEEEPT_v
// DEMANGLED: SIG_SAVE_PC_ROOM_PLAY_TIME * CStreamGuard::GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>(void)
template <>
SIG_SAVE_PC_ROOM_PLAY_TIME* CStreamGuard::GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>() {
    return stream_->GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>();
}
