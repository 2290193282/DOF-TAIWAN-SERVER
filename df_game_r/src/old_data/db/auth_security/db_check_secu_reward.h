#ifndef OLDSRV_DB_DB_CHECK_SECU_REWARD_H_
#define OLDSRV_DB_DB_CHECK_SECU_REWARD_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


struct SIG_CHECK_USER_SECU_REWARD {
    uint32_t account_id;
    uint32_t service_type;
};

struct SIG_SEND_USER_REWARD {
    uint32_t service_type;
};

static_assert(sizeof(SIG_CHECK_USER_SECU_REWARD) == 8,
              "SIG_CHECK_USER_SECU_REWARD IDA ABI size must stay 8");
static_assert(sizeof(SIG_SEND_USER_REWARD) == 4,
              "SIG_SEND_USER_REWARD IDA ABI size must stay 4");


// IDA unit: 11_event_runtime/DB_CheckSecuReward; restored in B because the
// active security-card/passpad DB dispatchers emit old DB signal 171.
class DB_CheckSecuReward : public DBDispatcher {
public:
    DB_CheckSecuReward();
    virtual ~DB_CheckSecuReward();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int account_id,
                            int service_type);
};

#endif  // OLDSRV_DB_DB_CHECK_SECU_REWARD_H_
