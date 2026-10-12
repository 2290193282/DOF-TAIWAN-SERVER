#ifndef OLDSRV_OLD_DATA_PVF_MAP_DATAMANAGER_DUNGEON_RUNTIME_H_
#define OLDSRV_OLD_DATA_PVF_MAP_DATAMANAGER_DUNGEON_RUNTIME_H_

#include <stdint.h>

#include <vector>

#include "old_data/PVF/dungeon/dungeon_script_value_types.h"

class CUser;

namespace dungeon_runtime {

// ABI-safe projection of one DungeonInCountInfo.etc row.  The registry is
// owned by map_runtime.cpp; no CDungeon/STDungeonScript layout is extended.
struct InCountRecord {
    int channel_type;
    int dungeon_index;
    int maximum_count;
    int reset_value;
};

// Typed 0828 [map move monster] row recovered from the IDA 9.1 client
// ImportDungeonScript owner.  The client groups repeated keys and accumulates
// the record+0x18 value, but no post-import client selector was found.  Server projection
// therefore admits a group only when every non-accumulator field is identical
// and materializes that common projection exactly once.
struct MapMoveMonster {
    int group;
    int monster_id;
    int level;
    int enabled;
    int spawn_mode;
    int position_x;
    int position_y;
    int value_18;
    int aggregate_value;
    int specific_drop_count;
    int random_drop_count;
    int value_24;
    int rank;
    int special_class;
    int tail_value;
};

// Ordered PVF registry queries.  The limited list contains only rows whose
// dungeon script declares the recovered Golden-Commerce mechanism.
bool GetInCountRecords(std::vector<InCountRecord>* records);
bool GetLimitedInCountRecords(std::vector<InCountRecord>* records);
int GetInCountMaximum(int dungeon_index);
bool IsLimitedInCountDungeon(int dungeon_index);

// Recovered DGN metadata/selection queries.
bool IsPowerWarDungeon(int dungeon_index);
bool IsGoldenCommerceDungeon(int dungeon_index);
bool IsMoveMapEvenEnemyDungeon(int dungeon_index);
bool GetFixedDifficulty(int dungeon_index, int* difficulty);
bool GetMapMoveMonsterProjection(int dungeon_index,
                                 int group,
                                 MapMoveMonster* projection);

// ABI-safe 140828 dungeon extensions.  The corresponding client importer
// keeps these values outside the legacy server's CDungeon layout; the restored
// server therefore publishes them through the same per-dungeon sidecar used by
// the other 0828-only DGN policies.
bool IsLimitItemStatDungeon(int dungeon_index);
bool IsLimitEscapeCharacterDungeon(int dungeon_index);
bool UsesCharacterRespawnPosition(int dungeon_index);
bool GetAbsoluteChangeStat(int dungeon_index,
                           AbsoluteChangeStat::T stat,
                           float* value);
bool GetRealDungeonMonsterCheckup(int dungeon_index,
                                  std::vector<int>* monster_ids);
bool GetDungeonZoomRate(int dungeon_index, float* zoom_rate);
bool IsRoomMoveKeepStateDungeon(int dungeon_index);
bool UsesDungeonCountdown(int dungeon_index);

// Character-owned limited-entry state.  The implementation is a mutex-
// protected sidecar keyed by CUser/character identity and persists through
// the existing fixed-size SIG_LOAD/UPDATE_DIMENSION payload.
void BeginInCountLoad(CUser* user, uint32_t charac_no);
bool ApplyInCountLoad(CUser* user,
                      uint32_t charac_no,
                      uint8_t packed,
                      bool reset_for_today);
bool IsInCountLoaded(CUser* user);
bool GetRemainingInCount(CUser* user,
                         int dungeon_index,
                         int* maximum,
                         int* remaining);
bool TryConsumeInCount(CUser* user, int dungeon_index);
void ResetDailyInCount(CUser* user);
void MarkDailyResetPending(CUser* user);
bool IsInCountDirty(CUser* user);
bool PackInCountForSave(CUser* user, uint8_t* packed);
void MarkInCountSaved(CUser* user, uint8_t packed);
void ResetInCountUser(CUser* user);

}  // namespace dungeon_runtime

#endif  // OLDSRV_OLD_DATA_PVF_MAP_DATAMANAGER_DUNGEON_RUNTIME_H_
