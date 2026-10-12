#include "infra/network/packet/stream_guard.h"
#include "old_data/db/game_option/db_save_game_option_quick_chatting.h"

// IDA_EA: 0x8239dfc
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_SAVE_GAME_OPTION_QUICKCHATEEPT_v
// DEMANGLED: SIG_SAVE_GAME_OPTION_QUICKCHAT * Stream::GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>(void)
template <>
SIG_SAVE_GAME_OPTION_QUICKCHAT* Stream::GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>() {
    const int bytes = 604;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_GAME_OPTION_QUICKCHAT* result =
        reinterpret_cast<SIG_SAVE_GAME_OPTION_QUICKCHAT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236ECA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_SAVE_GAME_OPTION_QUICKCHATEEPT_v
// DEMANGLED: SIG_SAVE_GAME_OPTION_QUICKCHAT * CStreamGuard::GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>(void)
template <>
SIG_SAVE_GAME_OPTION_QUICKCHAT* CStreamGuard::GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>() {
    return stream_->GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>();
}
