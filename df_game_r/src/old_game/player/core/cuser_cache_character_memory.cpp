#include "old_game/player/core/cuser.h"

#include "old_data/db/core/action_point_db_exchange_proc_abi.h"
#include "core/config/env/environment.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"
#include "old_game/player/core/charac_expand_cdata.h"
#include "old_game/player/core/exchange_server_character_cache.h"

#include <cstring>

// IDA_EA: 0x864C85A
// IDA_NAME: _ZN5CUser20CacheCharacterMemoryEv
// DEMANGLED: CUser::CacheCharacterMemory(void)
int CUser::CacheCharacterMemory() {
    if (!getCurCharacR() || get_state() <= 2) {
        return 0;
    }
    if (!IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1))) {
        cMyTrace trace("bool CUser::CacheCharacterMemory()", 4408, 0);
        trace("inventory section is not authoritative");
        return 0;
    }

    const int charac_no = getCurCharacNo();
    const int charac_idx = get_charac_idx(
        static_cast<unsigned int>(charac_no));
    if (charac_idx < 0) {
        cMyTrace trace("bool CUser::CacheCharacterMemory()", 4412, 0);
        trace("get_charac_idx() return = %d", charac_idx);
        return 0;
    }

    CACHE_CHARACTER_TYPE* const cache =
        exchange_server::AcquireCharacterMemory(charac_no);
    if (!cache) {
        cMyTrace trace("bool CUser::CacheCharacterMemory()", 4432, 0);
        trace("AcquireCharacterMemory() return = NULL, charac_no(%d)",
              charac_no);
        return 0;
    }

    GetRentInfo(cache->inventory.value);
    GetCharacInfo(&cache->charac.value);
    GetInventoryInfo(&cache->inventory.value);
    getCurCharacQuestR()->get_quest(&cache->quest.value);
    GetTicket(&cache->ticket);
    GetDungeonClear(&cache->dungeon_clear);
    GetBlackListCount(&cache->black_count);
    APSystem::CDBAndExchangeServerProc::GetActionPointFromMemory(
        *this, cache->action_point.value);
    GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(12))
        ->getData(reinterpret_cast<char*>(&cache->kill_monster.value));
    advancealtar::DBAndExchangeServerProc::getFromMemory(
        *this, cache->advance_altar.value);
    GetEtcInfo(&cache->etc.value);
    cache->etc.value.account_id = static_cast<uint32_t>(get_acc_id());
    cache->etc.value.charac_no = static_cast<uint32_t>(charac_no);
    if (!npc_gift::ExportToCache(this, &cache->etc.value)) {
        // A partial SIG_LOAD_ETC snapshot is unsafe: a later cache hit would
        // turn an unavailable sidecar into an authoritative empty NPC state.
        // Refuse this cache entry so the next selection falls back to DB.
        exchange_server::FreeCharacterMemory(cache);
        cMyTrace trace("bool CUser::CacheCharacterMemory()", 4462, 0);
        trace("NPC relationship cache export failed, charac_no(%d)",
              charac_no);
        return 0;
    }

    cache->etc.value.tag_charac_no =
        cache->charac.value.charac_data.tag_character_no;
    cache->inventory.value.tag_charac_no = cache->etc.value.tag_charac_no;
    std::memcpy(&cache->load_state,
                &cache_character_load_state_,
                sizeof(cache->load_state));

    if (exchange_server::CacheCharacterMemory(charac_no, cache) == 1) {
        SetLastLoginChannelNo(G_CEnvironment()->get_channel_no());
        SetLastLoginCharacNo(charac_no);
        return 1;
    }

    exchange_server::FreeCharacterMemory(cache);
    cMyTrace trace("bool CUser::CacheCharacterMemory()", 4496, 0);
    trace("CacheCharacterMemory() return = false");
    return 0;
}

// IDA_EA: 0x864DE58
// IDA_NAME: _ZNK5CUser15GetDungeonClearEP22SIG_LOAD_DUNGEON_CLEAR
// DEMANGLED: CUser::GetDungeonClear(SIG_LOAD_DUNGEON_CLEAR *)const
int CUser::GetDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload) const {
    payload->result = 0;
    if (!getCurCharacR()) {
        cMyTrace trace(
            "bool CUser::GetDungeonClear(SIG_LOAD_DUNGEON_CLEAR*) const",
            5120,
            5);
        trace("CUser::GetDungeonClear User  m_selected is null");
        return 0;
    }

    reinterpret_cast<const WongWork::CDungeonClear*>(getDungeonClearRefR())
        ->toString(payload->dungeon, sizeof(payload->dungeon));
    return 1;
}

// IDA_EA: 0x867846E
// IDA_NAME: _ZNK5CUser9GetTicketEP15SIG_LOAD_TICKET
// DEMANGLED: CUser::GetTicket(SIG_LOAD_TICKET *)const
SIG_LOAD_TICKET* CUser::GetTicket(SIG_LOAD_TICKET* payload) const {
    payload->result = 0;

    uint32_t count = 0;
    for (std::list<int>::const_iterator it = reset_list_.begin();
         it != reset_list_.end() && count < 100u;
         ++it, ++count) {
        payload->villages[count] = *it;
    }
    payload->count = count;
    return payload;
}

// IDA_EA: 0x867F1D4
// IDA_NAME: _ZNK5CUser17GetBlackListCountEP19SIG_REQ_BLACK_COUNT
// DEMANGLED: CUser::GetBlackListCount(SIG_REQ_BLACK_COUNT *)const
int CUser::GetBlackListCount(SIG_REQ_BLACK_COUNT* payload) const {
    payload->result = 0;
    payload->black_count = static_cast<uint32_t>(getCurrCharacBlackCount());
    return 1;
}
