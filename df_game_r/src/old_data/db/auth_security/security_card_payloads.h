#ifndef OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SECURITY_CARD_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SECURITY_CARD_PAYLOADS_H_

#include <stdint.h>


struct SIG_SECURITY_CARD_ADD_CERT_CNT {
    uint32_t account_id;
};

struct SIG_SECURITY_CARD_DISUSE {
    uint32_t account_id;
};

// The old cancel-count provider deliberately reuses this IDA wire type.
struct SIG_SECURITY_CARD_UPDATE_FAIL_CNT {
    uint32_t account_id;
    uint32_t count;
};

static_assert(sizeof(SIG_SECURITY_CARD_ADD_CERT_CNT) == 4,
              "SIG_SECURITY_CARD_ADD_CERT_CNT IDA ABI size must stay 4");
static_assert(sizeof(SIG_SECURITY_CARD_DISUSE) == 4,
              "SIG_SECURITY_CARD_DISUSE IDA ABI size must stay 4");
static_assert(sizeof(SIG_SECURITY_CARD_UPDATE_FAIL_CNT) == 8,
              "SIG_SECURITY_CARD_UPDATE_FAIL_CNT IDA ABI size must stay 8");


#endif  // OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SECURITY_CARD_PAYLOADS_H_
