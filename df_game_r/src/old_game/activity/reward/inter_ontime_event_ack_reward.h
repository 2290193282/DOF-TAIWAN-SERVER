#ifndef OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_EVENT_ACK_REWARD_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_EVENT_ACK_REWARD_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 11_event_runtime/Inter_OnTimeEventAckReward.
// The dispatcher has no state beyond ISigDispatcher.
class Inter_OnTimeEventAckReward : public ISigDispatcher {
public:
    Inter_OnTimeEventAckReward();
    virtual ~Inter_OnTimeEventAckReward();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
    int SendOnTimeRewardByMail(unsigned int charac_no,
                               unsigned int item_id,
                               unsigned int item_count);
};

#if defined(__i386__)
static_assert(sizeof(Inter_OnTimeEventAckReward) == 0x04u,
              "Inter_OnTimeEventAckReward must retain its base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_EVENT_ACK_REWARD_H_
