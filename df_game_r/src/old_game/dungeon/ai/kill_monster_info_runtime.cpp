#include "old_game/dungeon/ai/kill_monster_info_runtime.h"

#include <cstring>
#include <utility>

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

bool IsValidMonsterIndex(uint32_t monster_index, bool is_apc) {
    CDataManager* const data_manager = G_CDataManager();
    if (is_apc) {
        CAICharacterList* const ai_characters =
            data_manager->get_ai_character_list();
        return ai_characters->get(monster_index) != 0;
    }
    return data_manager->find_monster(static_cast<int>(monster_index)) != 0;
}

}  // namespace

// IDA_EA: 0x854C9B4
// IDA_NAME: _ZN16CKillMonsterInfoC2Ev
// DEMANGLED: CKillMonsterInfo::CKillMonsterInfo(void)
CKillMonsterInfo::CKillMonsterInfo()
    : charac_expand::CData(),
      boss_info_(),
      named_info_(),
      apc_boss_info_() {
    _reset();
}

// IDA_EA: 0x854CA72
// IDA_NAME: _ZN16CKillMonsterInfoD2Ev
// DEMANGLED: CKillMonsterInfo::~CKillMonsterInfo()
// IDA_EA: 0x854CB24
// IDA_NAME: _ZN16CKillMonsterInfoD0Ev
// DEMANGLED: CKillMonsterInfo::~CKillMonsterInfo()
CKillMonsterInfo::~CKillMonsterInfo() {}

// IDA_EA: 0x854CB42
// IDA_NAME: _ZN16CKillMonsterInfo18ConvertStructToMapEP15stMonsterInfo_tjRSt3mapIjhSt4lessIjESaISt4pairIKjhEEEb
// DEMANGLED: CKillMonsterInfo::ConvertStructToMap(stMonsterInfo_t *,unsigned int,std::map<unsigned int,unsigned char,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,unsigned char>>> &,bool)
bool CKillMonsterInfo::ConvertStructToMap(stMonsterInfo_t* source,
                                          unsigned int count,
                                          KillMonsterMap& destination,
                                          bool is_apc) {
    for (unsigned int index = 0; index < count; ++index) {
        const uint32_t monster_index = source[index].value;
        if (!monster_index) {
            break;
        }
        if (!IsValidMonsterIndex(monster_index, is_apc)) {
            LogManager::logFormat(
                1,
                "KillMonsterInfo.cpp",
                "bool CKillMonsterInfo::ConvertStructToMap(stMonsterInfo_t*, size_t, killMonsterMap&, bool)",
                is_apc ? 0x2e : 0x25,
                "ConvertStrToMap ERROR : mobIdx is not valid(%u)",
                monster_index);
            continue;
        }
        destination.insert(
            std::make_pair(monster_index, source[index].flag));
    }
    return true;
}

// IDA_EA: 0x854CCBC
// IDA_NAME: _ZNK16CKillMonsterInfo18ConvertMapToStrcutERKSt3mapIjhSt4lessIjESaISt4pairIKjhEEEP15stMonsterInfo_tj
// DEMANGLED: CKillMonsterInfo::ConvertMapToStrcut(std::map<unsigned int,unsigned char,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,unsigned char>>> const&,stMonsterInfo_t *,unsigned int)const
bool CKillMonsterInfo::ConvertMapToStrcut(
    const KillMonsterMap& source,
    stMonsterInfo_t* destination,
    unsigned int capacity) const {
    if (source.empty()) {
        return true;
    }
    if (source.size() > capacity) {
        return false;
    }

    unsigned int index = 0;
    for (KillMonsterMap::const_iterator iter = source.begin();
         iter != source.end() && index < capacity;
         ++iter, ++index) {
        destination[index].value = iter->first;
        destination[index].flag = iter->second;
    }
    return true;
}

// IDA_EA: 0x854CDDC
// IDA_NAME: _ZN16CKillMonsterInfo8loadDataEP5CUserPc
// DEMANGLED: CKillMonsterInfo::loadData(CUser *,char *)
bool CKillMonsterInfo::loadData(CUser* user, char* data) {
    (void)user;
    _reset();
    SIG_LOAD_KILL_MONSTER_INFO* const payload =
        reinterpret_cast<SIG_LOAD_KILL_MONSTER_INFO*>(data);
    ConvertStructToMap(payload->boss_info, 1500u, boss_info_, false);
    ConvertStructToMap(payload->named_info, 6000u, named_info_, false);
    ConvertStructToMap(
        payload->apc_boss_info, 1500u, apc_boss_info_, true);
    return 1;
}

// IDA_EA: 0x854CE90
// IDA_NAME: _ZNK16CKillMonsterInfo7getDataEPc
// DEMANGLED: CKillMonsterInfo::getData(char *)const
void CKillMonsterInfo::getData(char* data) const {
    SIG_LOAD_KILL_MONSTER_INFO* const payload =
        reinterpret_cast<SIG_LOAD_KILL_MONSTER_INFO*>(data);
    ConvertMapToStrcut(boss_info_, payload->boss_info, 1500u);
    ConvertMapToStrcut(named_info_, payload->named_info, 6000u);
    ConvertMapToStrcut(
        apc_boss_info_, payload->apc_boss_info, 1500u);
}

// IDA_EA: 0x854CF16
// IDA_NAME: _ZN16CKillMonsterInfo9_saveDataEP5CUser
// DEMANGLED: CKillMonsterInfo::_saveData(CUser *)
int CKillMonsterInfo::_saveData(CUser* user) {
    Stream* const stream = GlobalData::big_stream_pool()->Acquire(
        "KillMonsterInfo.cpp", 107);
    CStreamGuard guard(stream, true);
    *guard << 0x281;
    *guard << user->GetUID();
    SIG_LOAD_KILL_MONSTER_INFO* const payload =
        guard->GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    ConvertMapToStrcut(boss_info_, payload->boss_info, 1500u);
    ConvertMapToStrcut(named_info_, payload->named_info, 6000u);
    ConvertMapToStrcut(
        apc_boss_info_, payload->apc_boss_info, 1500u);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x854D0B8
// IDA_NAME: _ZN16CKillMonsterInfo6_resetEv
// DEMANGLED: CKillMonsterInfo::_reset(void)
int CKillMonsterInfo::_reset() {
    boss_info_.clear();
    named_info_.clear();
    apc_boss_info_.clear();
    // The old 17-instruction body ends immediately after the final
    // std::map::clear call and does not synthesize a success value.
}

// IDA_EA: 0x854D0EA
// IDA_NAME: _ZN16CKillMonsterInfo16CheckMonsterKillERKSt3mapIjhSt4lessIjESaISt4pairIKjhEEEjh
// DEMANGLED: CKillMonsterInfo::CheckMonsterKill(std::map<unsigned int,unsigned char,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,unsigned char>>> const&,unsigned int,unsigned char)
bool CKillMonsterInfo::CheckMonsterKill(
    const KillMonsterMap& source,
    uint32_t monster_index,
    uint8_t dungeon_difficulty) {
    const KillMonsterMap::const_iterator found =
        source.find(monster_index);
    if (found == source.end()) {
        return false;
    }
    return (found->second &
            static_cast<uint8_t>(
                GetDungeonDifficultyFlag(dungeon_difficulty))) != 0;
}

// IDA_EA: 0x854D184
// IDA_NAME: _ZN16CKillMonsterInfo13CheckBossKillEjh
// DEMANGLED: CKillMonsterInfo::CheckBossKill(unsigned int,unsigned char)
bool CKillMonsterInfo::CheckBossKill(uint32_t monster_index,
                                     uint8_t dungeon_difficulty) {
    return CheckMonsterKill(
        boss_info_, monster_index, dungeon_difficulty);
}

// IDA_EA: 0x854D1B6
// IDA_NAME: _ZN16CKillMonsterInfo14CheckNamedKillEjh
// DEMANGLED: CKillMonsterInfo::CheckNamedKill(unsigned int,unsigned char)
bool CKillMonsterInfo::CheckNamedKill(uint32_t monster_index,
                                      uint8_t dungeon_difficulty) {
    return CheckMonsterKill(
        named_info_, monster_index, dungeon_difficulty);
}

// IDA_EA: 0x854D1E8
// IDA_NAME: _ZN16CKillMonsterInfo16CheckAPCBossKillEjh
// DEMANGLED: CKillMonsterInfo::CheckAPCBossKill(unsigned int,unsigned char)
bool CKillMonsterInfo::CheckAPCBossKill(uint32_t monster_index,
                                        uint8_t dungeon_difficulty) {
    return CheckMonsterKill(
        apc_boss_info_, monster_index, dungeon_difficulty);
}

// IDA_EA: 0x854D21A
// IDA_NAME: _ZN16CKillMonsterInfo24GetDungeonDifficultyFlagEh
// DEMANGLED: CKillMonsterInfo::GetDungeonDifficultyFlag(unsigned char)
int CKillMonsterInfo::GetDungeonDifficultyFlag(
    uint8_t dungeon_difficulty) {
    if (dungeon_difficulty <= 4u) {
        return 1 << dungeon_difficulty;
    }
    return 0;
}

// IDA_EA: 0x854D24C
// IDA_NAME: _ZN16CKillMonsterInfo19RegisterMonsterKillERSt3mapIjhSt4lessIjESaISt4pairIKjhEEEjhb
// DEMANGLED: CKillMonsterInfo::RegisterMonsterKill(std::map<unsigned int,unsigned char,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,unsigned char>>> &,unsigned int,unsigned char,bool)
int CKillMonsterInfo::RegisterMonsterKill(
    KillMonsterMap& destination,
    uint32_t monster_index,
    uint8_t dungeon_difficulty,
    bool is_apc) {
    if (dungeon_difficulty > 4u ||
        !IsValidMonsterIndex(monster_index, is_apc)) {
        return 0;
    }

    KillMonsterMap::iterator found = destination.find(monster_index);
    if (found != destination.end()) {
        found->second = static_cast<uint8_t>(
            found->second | GetDungeonDifficultyFlag(dungeon_difficulty));
        alter();
        return 1;
    }

    const uint8_t difficulty_flag = static_cast<uint8_t>(
        GetDungeonDifficultyFlag(dungeon_difficulty));
    const std::pair<KillMonsterMap::iterator, bool> inserted =
        destination.insert(std::make_pair(monster_index, difficulty_flag));
    if (!inserted.second) {
        LogManager::logFormat(
            1,
            "KillMonsterInfo.cpp",
            "bool CKillMonsterInfo::RegisterMonsterKill(killMonsterMap&, unsigned int, unsigned char, bool)",
            0xc7,
            "KillMonsterInfo::RegisterMonsterKill Fail(%u, %u)",
            monster_index,
            dungeon_difficulty);
        return 0;
    }
    alter();
    return 1;
}

// IDA_EA: 0x854D418
// IDA_NAME: _ZN16CKillMonsterInfo16RegisterBossKillEjh
// DEMANGLED: CKillMonsterInfo::RegisterBossKill(unsigned int,unsigned char)
int CKillMonsterInfo::RegisterBossKill(uint32_t monster_index,
                                       uint8_t dungeon_difficulty) {
    return RegisterMonsterKill(
        boss_info_, monster_index, dungeon_difficulty, false);
}

// IDA_EA: 0x854D452
// IDA_NAME: _ZN16CKillMonsterInfo17RegisterNamedKillEjh
// DEMANGLED: CKillMonsterInfo::RegisterNamedKill(unsigned int,unsigned char)
int CKillMonsterInfo::RegisterNamedKill(uint32_t monster_index,
                                        uint8_t dungeon_difficulty) {
    return RegisterMonsterKill(
        named_info_, monster_index, dungeon_difficulty, false);
}

// IDA_EA: 0x854D48C
// IDA_NAME: _ZN16CKillMonsterInfo19RegisterAPCBossKillEjh
// DEMANGLED: CKillMonsterInfo::RegisterAPCBossKill(unsigned int,unsigned char)
int CKillMonsterInfo::RegisterAPCBossKill(uint32_t monster_index,
                                          uint8_t dungeon_difficulty) {
    return RegisterMonsterKill(
        apc_boss_info_, monster_index, dungeon_difficulty, true);
}
