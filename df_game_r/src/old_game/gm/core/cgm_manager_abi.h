#ifndef OLDSRV_GLOBAL_CGM_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_CGM_MANAGER_ABI_H_

#include <cstddef>
#include <map>
#include <pthread.h>

// IDA unit: CGM_Manager.  This is the one canonical declaration of the
// manager's recovered i386 object layout; consumers only use its two proven
// member functions and never re-project the storage themselves.
struct CGMRecord {
    unsigned char gm_mode;
    unsigned char reserved[3];
    int gm_level;
};

class CGM_Manager {
public:
    // IDA_EA: 0x829917C
    // IDA_NAME: _ZN11CGM_Manager10TurnGmModeEj
    int TurnGmMode(unsigned int account_id);

    // IDA_EA: 0x8299276
    // IDA_NAME: _ZN11CGM_Manager16GetCurrentGmModeEj
    int GetCurrentGmMode(unsigned int account_id);

    // IDA_EA: 0x82993D8
    // IDA_NAME: _ZN11CGM_Manager10GetGmLevelEj
    int GetGmLevel(unsigned int account_id);

    // IDA_EA: 0x829948C
    // IDA_NAME: _ZN11CGM_Manager4IsGmEj
    // DEMANGLED: CGM_Manager::IsGm(unsigned int)
    bool IsGm(unsigned int account_id);

    // Physical fields remain declared only here so the owning implementation
    // and the i386 ABI assertions share one definition.
    std::map<unsigned int, CGMRecord> gm_records;
    void* db_handle;
    pthread_rwlock_t rwlock;
};

#if defined(__i386__)
static_assert(sizeof(CGMRecord) == 0x8u,
              "CGMRecord IDA ABI size must stay 0x8");
static_assert(offsetof(CGM_Manager, db_handle) == 0x18u,
              "CGM_Manager::db_handle IDA offset must stay 0x18");
static_assert(offsetof(CGM_Manager, rwlock) == 0x1Cu,
              "CGM_Manager::rwlock IDA offset must stay 0x1C");
#endif

#endif  // OLDSRV_GLOBAL_CGM_MANAGER_ABI_H_
