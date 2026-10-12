#include "old_data/Login/inter_returns/inter_load_skill.h"

// IDA_EA: 0x84E9E5E
// IDA_NAME: _ZN15Inter_LoadSkillC2Ev
// DEMANGLED: Inter_LoadSkill::Inter_LoadSkill(void)
Inter_LoadSkill::Inter_LoadSkill() {}

// IDA_EA: 0x84E9E7A / 0x84E9EAA
// IDA_NAME: _ZN15Inter_LoadSkillD2Ev / D0Ev
// DEMANGLED: Inter_LoadSkill::~Inter_LoadSkill()
Inter_LoadSkill::~Inter_LoadSkill() {}

// IDA_EA: 0x84C5152
// IDA_NAME: _ZN15Inter_LoadSkill12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadSkill::dispatch_sig(CUser *,char *,int)
// The disassembled original is exactly `xor eax,eax; ret`; keep its real
// no-side-effect behavior rather than manufacturing a load transaction.
int Inter_LoadSkill::dispatch_sig(CUser*, char*, int) {
    return 0;
}
