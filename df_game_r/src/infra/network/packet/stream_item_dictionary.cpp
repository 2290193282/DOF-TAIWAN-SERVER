#include "old_data/db/character/load_etc_payload.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x845418A
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_ITEM_DICTIONARY_FLAGEEPT_v
// DEMANGLED: SIG_ITEM_DICTIONARY_FLAG * Stream::GetOutBuffer<SIG_ITEM_DICTIONARY_FLAG>(void)
template <>
SIG_ITEM_DICTIONARY_FLAG*
Stream::GetOutBuffer<SIG_ITEM_DICTIONARY_FLAG>() {
    const int bytes = sizeof(SIG_ITEM_DICTIONARY_FLAG);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_ITEM_DICTIONARY_FLAG* result =
        reinterpret_cast<SIG_ITEM_DICTIONARY_FLAG*>(get_buffer());
    out_ptr(bytes);
    return result;
}
