#include "old_data/db/character/load_etc_payload.h"


// IDA_EA: 0x8450976
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_REDEEMINFO_LIST_TO_DBEEPT_v
// DEMANGLED: SIG_REDEEMINFO_LIST_TO_DB * Stream::GetOutBuffer<SIG_REDEEMINFO_LIST_TO_DB>(void)
template <>
SIG_REDEEMINFO_LIST_TO_DB*
Stream::GetOutBuffer<SIG_REDEEMINFO_LIST_TO_DB>() {
    const int bytes = sizeof(SIG_REDEEMINFO_LIST_TO_DB);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_REDEEMINFO_LIST_TO_DB* const result =
        reinterpret_cast<SIG_REDEEMINFO_LIST_TO_DB*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
