#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_KILL_MONSTER_INFO_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_KILL_MONSTER_INFO_RUNTIME_H_

#include <stdint.h>

#include <map>

#include "old_game/player/core/charac_expand_cdata.h"
#include "infra/network/packet/stream.h"


class CUser;

// IDA unit: 99_misc_global/CKillMonsterInfo.
// charac_expand::CDataMgr slot 12 owns this persisted kill-history state.
class CKillMonsterInfo : public charac_expand::CData {
public:
    CKillMonsterInfo();
    virtual ~CKillMonsterInfo();

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;

    bool CheckBossKill(uint32_t monster_index,
                       uint8_t dungeon_difficulty);
    bool CheckNamedKill(uint32_t monster_index,
                        uint8_t dungeon_difficulty);
    bool CheckAPCBossKill(uint32_t monster_index,
                          uint8_t dungeon_difficulty);

    int RegisterBossKill(uint32_t monster_index,
                         uint8_t dungeon_difficulty);
    int RegisterNamedKill(uint32_t monster_index,
                          uint8_t dungeon_difficulty);
    int RegisterAPCBossKill(uint32_t monster_index,
                            uint8_t dungeon_difficulty);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    typedef std::map<uint32_t, uint8_t> KillMonsterMap;

    bool ConvertStructToMap(stMonsterInfo_t* source,
                            unsigned int count,
                            KillMonsterMap& destination,
                            bool is_apc);
    bool ConvertMapToStrcut(const KillMonsterMap& source,
                            stMonsterInfo_t* destination,
                            unsigned int capacity) const;
    bool CheckMonsterKill(const KillMonsterMap& source,
                          uint32_t monster_index,
                          uint8_t dungeon_difficulty);
    int GetDungeonDifficultyFlag(uint8_t dungeon_difficulty);
    int RegisterMonsterKill(KillMonsterMap& destination,
                            uint32_t monster_index,
                            uint8_t dungeon_difficulty,
                            bool is_apc);

    KillMonsterMap boss_info_;      // IDA +0x08
    KillMonsterMap named_info_;     // IDA +0x20
    KillMonsterMap apc_boss_info_;  // IDA +0x38
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CKillMonsterInfo) == 0x50u,
              "CKillMonsterInfo IDA ABI size must stay 0x50");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_KILL_MONSTER_INFO_RUNTIME_H_
