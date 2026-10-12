#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_SHUTDOWN_QUIESCENCE_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_SHUTDOWN_QUIESCENCE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


class Stream;

// Shutdown-only causal barrier for the reconstructed database graph:
//
//   NATIVE_0828_DB_Q --829(begin)--> DB_Q --823(fence)--> queue 6 --829(ack)
//
// The wire record is pointer-free and the signal is deliberately absent from
// the legacy DBTask map.  Only the queue-6 task owns signal 829; DB_Q observes
// it solely as the validated target of the already-owned signal-823 fence.
namespace db_shutdown_quiescence {

enum {
    kNativeSignal = 829,
    kMagic = 0x8290828,
    kVersion = 1,
};

enum Phase {
    kPhaseBegin = 1,
    kPhaseAck = 2,
};

struct Message {
    uint32_t magic;
    uint32_t version;
    uint32_t phase;
    uint32_t generation;
};

static_assert(sizeof(Message) == 16u,
              "shutdown-quiescence message must remain fixed");

enum ActivityDomain {
    kActivityDispatch = 0,
    kActivityLegacyDb = 1,
    kActivityLegacyLogDb = 2,
    kActivityNativeDb = 3,
    kActivityDomainCount = 4,
};

// Activity tracking is external to every recovered fixed-layout class.  A
// generation is stable only if its epochs do not move and no domain is inside
// a handler when the causal ack returns.
void BeginActivity(ActivityDomain domain, int signal);
void EndActivity(ActivityDomain domain);
uint32_t ActivityEpoch(ActivityDomain domain);
bool ActivityInFlight(ActivityDomain domain);

uint32_t AllocateGeneration();
bool QueueBegin(uint32_t generation);
// 0 = pending, 1 = acknowledged, -1 = failed/fail-closed.
int GetRoundStatus(uint32_t generation);

// Validation hook used by the legacy 823 fence.  Only a complete 829/-1/ACK
// envelope is accepted as a shutdown target.
bool ValidateFenceAckTarget(const unsigned char* bytes, uint32_t size);

class DB_ShutdownQuiescence : public DBDispatcher {
public:
    DB_ShutdownQuiescence();
    virtual ~DB_ShutdownQuiescence();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace db_shutdown_quiescence_0828

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_SHUTDOWN_QUIESCENCE_0828_H_
