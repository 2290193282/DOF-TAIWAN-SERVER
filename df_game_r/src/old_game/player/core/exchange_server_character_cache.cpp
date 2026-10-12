#include "old_game/player/core/exchange_server_character_cache.h"

#include <cstring>
#include <ctime>
#include <utility>

#include "infra/log/format/trace.h"

namespace {

const std::time_t kCacheLifetimeSeconds = 29;

class ScopedCacheLock {
public:
    explicit ScopedCacheLock(pthread_mutex_t* mutex) : mutex_(mutex) {
        pthread_mutex_lock(mutex_);
    }

    ~ScopedCacheLock() {
        pthread_mutex_unlock(mutex_);
    }

private:
    ScopedCacheLock(const ScopedCacheLock&);
    ScopedCacheLock& operator=(const ScopedCacheLock&);

    pthread_mutex_t* mutex_;
};

uint32_t LastAccess(const CACHE_CHARACTER_TYPE* character) {
    return character->last_access_time;
}

void SetLastAccess(CACHE_CHARACTER_TYPE* character, std::time_t timestamp) {
    character->last_access_time = static_cast<uint32_t>(timestamp);
}

void ResetAcquiredPayload(CACHE_CHARACTER_TYPE* character) {
    // IDA @0x8490A8A clears the complete exchange payload before either a
    // fresh or already-cached record is handed to the caller.  Its nested
    // constructors were run when the static pool was created, so this is the
    // original reuse protocol rather than an uninitialised allocation path.
    std::memset(character, 0, sizeof(*character));
    SetLastAccess(character, std::time(0));
}

}  // namespace

namespace exchange_server {

// IDA_EA: 0x8490E98
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgrC2Ev
// DEMANGLED: exchange_server::CCacheCharacterMgr::CCacheCharacterMgr(void)
CCacheCharacterMgr::CCacheCharacterMgr() : mutex_(), pool_(), cached_() {
    pthread_mutex_init(&mutex_, 0);
}

// IDA_EA: 0x8490F04
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgrD2Ev
// DEMANGLED: exchange_server::CCacheCharacterMgr::~CCacheCharacterMgr()
CCacheCharacterMgr::~CCacheCharacterMgr() {
    pthread_mutex_destroy(&mutex_);
}

// IDA_EA: 0x8490992
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgr14CollectGarbageEv
// DEMANGLED: exchange_server::CCacheCharacterMgr::CollectGarbage(void)
int CCacheCharacterMgr::CollectGarbage() {
    const std::time_t now = std::time(0);
    int collected = 0;
    for (std::map<int, CACHE_CHARACTER_TYPE*>::iterator it = cached_.begin();
         it != cached_.end();) {
        CACHE_CHARACTER_TYPE* const character = it->second;
        if (now - static_cast<std::time_t>(LastAccess(character)) <=
            kCacheLifetimeSeconds) {
            ++it;
            continue;
        }

        pool_.Free(character);
        it = cached_.erase(it);
        collected = 1;
    }
    return collected;
}

// IDA_EA: 0x8490A8A
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgr16AcquireCharacterEi
// DEMANGLED: exchange_server::CCacheCharacterMgr::AcquireCharacter(int)
CACHE_CHARACTER_TYPE* CCacheCharacterMgr::AcquireCharacter(int charac_no) {
    ScopedCacheLock lock(&mutex_);

    std::map<int, CACHE_CHARACTER_TYPE*>::iterator found =
        cached_.find(charac_no);
    if (found != cached_.end()) {
        CACHE_CHARACTER_TYPE* const character = found->second;

        // Withdraw the published cache entry before clearing it for a full
        // rebuild.  ReturnCacheCharacter runs on another queue and must not
        // observe the temporary zero-filled CHARAC_DATA image (notably the
        // guild key).  CacheCharacterMemory republishes only after copying
        // the complete snapshot.
        cached_.erase(found);
        ResetAcquiredPayload(character);
        return character;
    }

    if (!pool_.Remain() && !CollectGarbage()) {
        return 0;
    }

    CACHE_CHARACTER_TYPE* const character = pool_.Acquire();
    if (character) {
        ResetAcquiredPayload(character);
    }
    return character;
}

// IDA_EA: 0x8490BF4
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgr13FreeCharacterEP20CACHE_CHARACTER_TYPE
// DEMANGLED: exchange_server::CCacheCharacterMgr::FreeCharacter(CACHE_CHARACTER_TYPE *)
int CCacheCharacterMgr::FreeCharacter(CACHE_CHARACTER_TYPE* character) {
    ScopedCacheLock lock(&mutex_);
    pool_.Free(character);

    const int in_use = 1000 - pool_.Remain();
    if (in_use != static_cast<int>(cached_.size())) {
        cMyTrace trace(
            "void exchange_server::CCacheCharacterMgr::FreeCharacter(CACHE_CHARACTER_TYPE*)",
            78,
            0);
        trace("FreeCharacter() error (%d,%d,%d)", 232);
    }
    return 1;
}

// IDA_EA: 0x8490CEE
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgr17GetCacheCharacterEiPP20CACHE_CHARACTER_TYPEb
// DEMANGLED: exchange_server::CCacheCharacterMgr::GetCacheCharacter(int,CACHE_CHARACTER_TYPE **,bool)
int CCacheCharacterMgr::GetCacheCharacter(int charac_no,
                                          CACHE_CHARACTER_TYPE** character,
                                          bool erase_cached) {
    ScopedCacheLock lock(&mutex_);
    std::map<int, CACHE_CHARACTER_TYPE*>::iterator found =
        cached_.find(charac_no);
    if (found == cached_.end()) {
        return 0;
    }

    *character = found->second;
    if (erase_cached) {
        cached_.erase(found);
    }
    return 1;
}

// IDA_EA: 0x8490DCA
// IDA_NAME: _ZN15exchange_server18CCacheCharacterMgr14CacheCharacterEiP20CACHE_CHARACTER_TYPE
// DEMANGLED: exchange_server::CCacheCharacterMgr::CacheCharacter(int,CACHE_CHARACTER_TYPE *)
int CCacheCharacterMgr::CacheCharacter(int charac_no,
                                       CACHE_CHARACTER_TYPE* character) {
    ScopedCacheLock lock(&mutex_);
    std::pair<std::map<int, CACHE_CHARACTER_TYPE*>::iterator, bool> inserted =
        cached_.insert(std::make_pair(charac_no, character));
    if (!inserted.second) {
        inserted.first->second = character;
    }
    return 1;
}

// IDA_EA: 0x8490F71
// IDA_NAME: _ZN15exchange_server28GetInstanceCacheCharacterMgrEv
// DEMANGLED: exchange_server::GetInstanceCacheCharacterMgr(void)
CCacheCharacterMgr* GetInstanceCacheCharacterMgr() {
    static CCacheCharacterMgr instance;
    return &instance;
}

// IDA_EA: 0x848F220
// IDA_NAME: _ZN15exchange_server22AcquireCharacterMemoryEi
// DEMANGLED: exchange_server::AcquireCharacterMemory(int)
CACHE_CHARACTER_TYPE* AcquireCharacterMemory(int charac_no) {
    return GetInstanceCacheCharacterMgr()->AcquireCharacter(charac_no);
}

// IDA_EA: 0x848F23C
// IDA_NAME: _ZN15exchange_server19FreeCharacterMemoryEP20CACHE_CHARACTER_TYPE
// DEMANGLED: exchange_server::FreeCharacterMemory(CACHE_CHARACTER_TYPE *)
int FreeCharacterMemory(CACHE_CHARACTER_TYPE* character) {
    return GetInstanceCacheCharacterMgr()->FreeCharacter(character);
}

// IDA_EA: 0x848F258
// IDA_NAME: _ZN15exchange_server20CacheCharacterMemoryEiP20CACHE_CHARACTER_TYPE
// DEMANGLED: exchange_server::CacheCharacterMemory(int,CACHE_CHARACTER_TYPE *)
int CacheCharacterMemory(int charac_no, CACHE_CHARACTER_TYPE* character) {
    return GetInstanceCacheCharacterMgr()->CacheCharacter(charac_no,
                                                            character);
}

}  // namespace exchange_server
