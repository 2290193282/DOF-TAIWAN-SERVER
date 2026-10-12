#include "infra/network/packet/stream_guard.h"
#include "old_data/db/game_option/db_save_game_option_chatting_emoticon.h"

// IDA_EA: 0x823a250
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_SAVE_CHATTING_EMOTICONEEPT_v
// DEMANGLED: SIG_SAVE_CHATTING_EMOTICON * Stream::GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>(void)
template <>
SIG_SAVE_CHATTING_EMOTICON* Stream::GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>() {
    const int bytes = 532;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_CHATTING_EMOTICON* result =
        reinterpret_cast<SIG_SAVE_CHATTING_EMOTICON*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236F8C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_SAVE_CHATTING_EMOTICONEEPT_v
// DEMANGLED: SIG_SAVE_CHATTING_EMOTICON * CStreamGuard::GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>(void)
template <>
SIG_SAVE_CHATTING_EMOTICON* CStreamGuard::GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>() {
    return stream_->GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>();
}
