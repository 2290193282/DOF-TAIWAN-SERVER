#ifndef OLDSRV_DB_DB_SECU_SERVICE_REWARD_H_
#define OLDSRV_DB_DB_SECU_SERVICE_REWARD_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
struct SIG_SECU_SERVICE_REWARD {
    uint32_t account_id;       // +0x00
    int8_t service_type;       // +0x04
    uint8_t reserved_05[3];    // +0x05, old typed Stream tail
};
#pragma pack(pop)

static_assert(sizeof(SIG_SECU_SERVICE_REWARD) == 0x08u,
              "security-service reward DB ABI must stay eight bytes");


// IDA_EA: 0x8437764/0x8437844/0x844F0A2/0x845D0DE/0x845D10E.
// IDA unit: 06_userinfo_stat_inventory/DB_SecuServiceReward; B owns DB
// dispatcher restoration.
class DB_SecuServiceReward : public DBDispatcher {
public:
    DB_SecuServiceReward();
    virtual ~DB_SecuServiceReward();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, char service_type);
};

#endif  // OLDSRV_DB_DB_SECU_SERVICE_REWARD_H_
