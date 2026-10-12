#ifndef OLDSRV_APSYSTEM_ACTION_POINT_DB_EXCHANGE_PROC_ABI_H_
#define OLDSRV_APSYSTEM_ACTION_POINT_DB_EXCHANGE_PROC_ABI_H_

#include <stdint.h>

#include "old_data/db/legacy_dispatch/action_point_payload.h"

class CUser;
struct MSG_CHARAC_SELECT;

namespace APSystem {

// IDA unit: APSystem_CActionPointManager.  CUser owns this real child at
// expanded +0x8d5b4; the following CharacAdvanceAltarManager at +0x8e2b0 fixes the
// complete 0xcfc-byte i386 extent.
struct CActionPointManager {
    static bool _IsGMOn;

    // IDA_EA: 0x8694BD0
    // IDA_NAME: _ZN8APSystem19CActionPointManagerC2Ev
    CActionPointManager();

    // IDA_EA: 0x8694BF6
    // IDA_NAME: _ZN8APSystem19CActionPointManagerD2Ev
    ~CActionPointManager();

    // IDA_EA: 0x8122AF4
    // IDA_NAME: _ZN8APSystem19CActionPointManager5ResetEv
    CActionPointManager* Reset();
    // IDA_EA: 0x8121B0C
    // IDA_NAME: _ZN8APSystem19CActionPointManager3SetERNS_22_SIG_LOAD_ACTION_POINTE
    // DEMANGLED: APSystem::CActionPointManager::Set(APSystem::_SIG_LOAD_ACTION_POINT &)
    _SIG_LOAD_ACTION_POINT* Set(_SIG_LOAD_ACTION_POINT& action_point);
    const _SIG_LOAD_ACTION_POINT* GetActionPoint() const;

    // IDA_EA: 0x8121B3E
    // IDA_NAME: _ZN8APSystem19CActionPointManager6GM_SetER5CUserj
    // DEMANGLED: APSystem::CActionPointManager::GM_Set(CUser &,unsigned int)
    bool GM_Set(CUser& user, uint32_t action_point);

    // IDA_EA: 0x8121D12
    // IDA_NAME: _ZN8APSystem19CActionPointManager8GM_ResetEv
    // DEMANGLED: APSystem::CActionPointManager::GM_Reset(void)
    bool GM_Reset();

    uint8_t dirty_;                    // +0x000
    uint8_t pad_001_003_[3];
    uint32_t previous_action_point_;   // +0x004
    _SIG_LOAD_ACTION_POINT action_;    // +0x008
};

#if defined(__i386__)
static_assert(sizeof(CActionPointManager) == 0xcfcu,
              "APSystem::CActionPointManager must preserve old i386 extent");
#endif

// The old-server callers pass only the declared arguments to these symbols;
// CDBAndExchangeServerProc has no instance argument at either call site.
struct CDBAndExchangeServerProc {
    static int RequestLoadActionPointFromDBAndES(
        CUser& user,
        MSG_CHARAC_SELECT& select);

    static void GetActionPointFromMemory(
        const CUser& user,
        _SIG_LOAD_ACTION_POINT& action_point);
};

}  // namespace APSystem

#endif  // OLDSRV_APSYSTEM_ACTION_POINT_DB_EXCHANGE_PROC_ABI_H_
