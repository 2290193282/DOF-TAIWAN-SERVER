#ifndef OLDSRV_GLOBAL_CREATE_CHARACTER_SKILL_ABI_H_
#define OLDSRV_GLOBAL_CREATE_CHARACTER_SKILL_ABI_H_

struct _Mastered_skill;

// IDA_EA: 0x8604FE2
// IDA_NAME: _Z25addSkillOnCreateCharacterP15_Mastered_skilli
// DEMANGLED: addSkillOnCreateCharacter(_Mastered_skill *,int)
void addSkillOnCreateCharacter(_Mastered_skill* mastered_skill, int job)
    __asm__("_Z25addSkillOnCreateCharacterP15_Mastered_skilli");

#endif  // OLDSRV_GLOBAL_CREATE_CHARACTER_SKILL_ABI_H_
