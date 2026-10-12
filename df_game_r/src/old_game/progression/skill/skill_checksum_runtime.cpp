#include "old_game/progression/skill/skill_checksum_runtime.h"

// IDA_EA: 0x88923F1
// IDA_NAME: _Z16getSkillChecksumiii
// DEMANGLED: getSkillChecksum(int,int,int)
int getSkillChecksum(int slot_index, int skill_index, int skill_level) {
    if (slot_index <= 149) {
        return 10 * slot_index * (skill_index + skill_level) / 10;
    }
    return 0;
}

// IDA_EA: 0x8892445
// IDA_NAME: _Z19finishSkillChecksumPii
// DEMANGLED: finishSkillChecksum(int *,int)
int finishSkillChecksum(int* checksum, int seed) {
    if (!checksum) {
        return 0;
    }
    *checksum ^= seed;
    return *checksum;
}
