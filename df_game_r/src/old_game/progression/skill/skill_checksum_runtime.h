#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHECKSUM_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHECKSUM_RUNTIME_H_

// IDA_EA: 0x88923F1 / 0x8892445.  These two global helpers are the exact
// checksum tail used by SkillSlot; they have no object-layout dependency.
int getSkillChecksum(int slot_index, int skill_index, int skill_level);
int finishSkillChecksum(int* checksum, int seed);

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHECKSUM_RUNTIME_H_
