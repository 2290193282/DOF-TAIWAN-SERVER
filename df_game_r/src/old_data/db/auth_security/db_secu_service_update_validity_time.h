#ifndef OLDSRV_DB_DB_SECU_SERVICE_UPDATE_VALIDITY_TIME_H_
#define OLDSRV_DB_DB_SECU_SERVICE_UPDATE_VALIDITY_TIME_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
struct SIG_SECURITY_CARD_SERVICE_VALIDITYTIME {
    uint32_t account_id;     // +0x00
    uint32_t seconds;        // +0x04
    uint16_t service_type;   // +0x08
    char ip[16];             // +0x0a
    uint8_t reserved_1a[2];  // +0x1a, old typed Stream tail
};
#pragma pack(pop)

static_assert(sizeof(SIG_SECURITY_CARD_SERVICE_VALIDITYTIME) == 0x1cu,
              "security validity-time DB ABI must stay 28 bytes");


// IDA_EA: 0x843A264/0x843A34E/0x844F1D6/0x845CB14/0x845CB44.
// IDA unit: 06_userinfo_stat_inventory/DB_SecuServiceUpdateValidityTime; B
// owns DB dispatcher restoration.
class DB_SecuServiceUpdateValidityTime : public DBDispatcher {
public:
    DB_SecuServiceUpdateValidityTime();
    virtual ~DB_SecuServiceUpdateValidityTime();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int seconds,
                            unsigned short service_type,
                            char* ip);
};

#endif  // OLDSRV_DB_DB_SECU_SERVICE_UPDATE_VALIDITY_TIME_H_
