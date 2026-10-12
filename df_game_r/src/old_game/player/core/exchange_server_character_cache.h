#ifndef OLDSRV_OLD_GAME_USERINFO_EXCHANGE_SERVER_CHARACTER_CACHE_H_
#define OLDSRV_OLD_GAME_USERINFO_EXCHANGE_SERVER_CHARACTER_CACHE_H_

#include <map>
#include <pthread.h>

#include "old_game/player/core/character_cache_data.h"

// IDA units: 04_character_select_create/exchange_server_CCacheCharacterMgr
// and 99_misc_global/exchange_server.  CACHE_CHARACTER_TYPE instances are
// pooled exchange payloads: ownership moves from AcquireCharacterMemory to
// CacheCharacterMemory, or back to the pool through FreeCharacterMemory.
namespace exchange_server {

class CCacheCharacterMgr {
public:
    CCacheCharacterMgr();
    ~CCacheCharacterMgr();

    int CollectGarbage();
    CACHE_CHARACTER_TYPE* AcquireCharacter(int charac_no);
    int FreeCharacter(CACHE_CHARACTER_TYPE* character);
    int GetCacheCharacter(int charac_no,
                          CACHE_CHARACTER_TYPE** character,
                          bool erase_cached);
    int CacheCharacter(int charac_no, CACHE_CHARACTER_TYPE* character);

private:
    pthread_mutex_t mutex_;                         // IDA +0x00
    StaticPool_CACHE_CHARACTER_TYPE_1000 pool_;     // IDA +0x18
    std::map<int, CACHE_CHARACTER_TYPE*> cached_;   // IDA +0x48
};

CCacheCharacterMgr* GetInstanceCacheCharacterMgr();
CACHE_CHARACTER_TYPE* AcquireCharacterMemory(int charac_no);
int FreeCharacterMemory(CACHE_CHARACTER_TYPE* character);
int CacheCharacterMemory(int charac_no, CACHE_CHARACTER_TYPE* character);

}  // namespace exchange_server

#if defined(__i386__)
static_assert(sizeof(exchange_server::CCacheCharacterMgr) == 0x60u,
              "CCacheCharacterMgr old ABI size must stay 0x60");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_EXCHANGE_SERVER_CHARACTER_CACHE_H_
