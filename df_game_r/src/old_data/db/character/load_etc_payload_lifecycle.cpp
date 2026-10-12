#include "old_data/db/character/load_etc_payload.h"

// The outer lifetime provider is intentionally isolated from the nested
// constructor definitions.  The old C2 has observable direct calls in member
// order, and its exception cleanup destroys only GameResultType/PvpResultType.

// IDA_EA: 0x8491F86
// IDA_NAME: _ZN12SIG_LOAD_ETCC2Ev
// DEMANGLED: SIG_LOAD_ETC::SIG_LOAD_ETC(void)
SIG_LOAD_ETC::SIG_LOAD_ETC() {}

// IDA_EA: 0x84921F0
// IDA_NAME: _ZN12SIG_LOAD_ETCD2Ev
// DEMANGLED: SIG_LOAD_ETC::~SIG_LOAD_ETC()
SIG_LOAD_ETC::~SIG_LOAD_ETC() {}
