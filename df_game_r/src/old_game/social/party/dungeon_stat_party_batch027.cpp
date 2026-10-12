#include "old_game/dungeon/instance/dungeon_stat_runtime.h"

// IDA_EA: 0x84869D6
// IDA_NAME: _ZN5Party5ResetEv
// DEMANGLED: Party::Reset(void)
Party* Party::Reset() {
    PlayInfo::Reset();
    clear_count = 0;
    return this;
}

// IDA_EA: 0x84869F4
// IDA_NAME: _ZN5PartyC2Ev
// DEMANGLED: Party::Party(void)
Party::Party()
    : IData(),
      PlayInfo(),
      clear_count(0) {
    Reset();
}

// IDA_EA: 0x8486A1E
// IDA_NAME: _ZN5PartyD2Ev
// DEMANGLED: Party::~Party()
Party::~Party() {}

// IDA_EA: 0x8486A24
// IDA_NAME: _ZN8JobParty5ResetEv
// DEMANGLED: JobParty::Reset(void)
JobParty* JobParty::Reset() {
    job_count = 0;
    return this;
}

// IDA_EA: 0x8486A32
// IDA_NAME: _ZN8JobPartyC2Ev
// DEMANGLED: JobParty::JobParty(void)
JobParty::JobParty()
    : IData(),
      job_count(0) {
    Reset();
}
