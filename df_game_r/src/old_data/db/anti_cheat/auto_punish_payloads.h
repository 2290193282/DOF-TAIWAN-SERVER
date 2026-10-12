#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_AUTO_PUNISH_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_AUTO_PUNISH_PAYLOADS_H_

#include <stdint.h>


#pragma pack(push, 1)

// IDA wire type used by DB_AccountHack_CheckMyAccount @0x8439F06.
struct SIG_CHECK_MY_ACCOUNT_IN_DB {
    uint32_t uid;
    uint32_t account_id;
    char charac_name[29];
    uint32_t package_money;
};

// IDA wire type returned by DB_AccountHack_CheckMyAccount @0x843A0C5.
struct SIG_CHECK_MY_ACCOUNT_OK {
    uint32_t uid;
    uint32_t package_money;
    uint32_t owner_account_id;
};

// IDA wire types used by DB_AutoPunishedSecondUserData @0x843A462.
struct SIG_SECOND_USER_DATA {
    uint32_t account_id;
    uint32_t request_gold_low;
    uint32_t request_gold_high;
    uint32_t user_idx;
    uint32_t related_account_id;
};

struct SIG_SECOND_USER_DATA_SET {
    uint32_t request_gold_low;
    uint32_t request_gold_high;
    uint32_t database_gold_low;
    uint32_t database_gold_high;
    uint32_t trade_count;
    uint32_t related_account_id;
};

// IDA wire type used by DB_InsertAutoPunishFirstUser @0x842D6CE.
struct SIG_INSERT_AUTO_PUNISH_FIRST_USER {
    uint32_t account_id;
    char ip[16];
    uint32_t hack_type;
    uint32_t count;
    uint32_t hack_sub_type;
    uint32_t hack_sub_count;
    uint16_t punish_flag;
    uint16_t reserved_26;
    uint32_t punish_value;
};

// IDA wire type used by DB_InsertAutoPunishSecondUser @0x842DEB4.
struct SIG_INSERT_AUTO_PUNISH_SECOND_USER {
    uint32_t account_id;
    uint32_t hack_account_id;
    uint32_t trade_gold_low;
    uint32_t trade_gold_high;
    uint8_t punish;
    uint8_t reserved_11[3];
};

#pragma pack(pop)

static_assert(sizeof(SIG_CHECK_MY_ACCOUNT_IN_DB) == 41,
              "SIG_CHECK_MY_ACCOUNT_IN_DB IDA ABI size must stay 41");
static_assert(sizeof(SIG_CHECK_MY_ACCOUNT_OK) == 12,
              "SIG_CHECK_MY_ACCOUNT_OK IDA ABI size must stay 12");
static_assert(sizeof(SIG_SECOND_USER_DATA) == 20,
              "SIG_SECOND_USER_DATA IDA ABI size must stay 20");
static_assert(sizeof(SIG_SECOND_USER_DATA_SET) == 24,
              "SIG_SECOND_USER_DATA_SET IDA ABI size must stay 24");
static_assert(sizeof(SIG_INSERT_AUTO_PUNISH_FIRST_USER) == 44,
              "SIG_INSERT_AUTO_PUNISH_FIRST_USER IDA ABI size must stay 44");
static_assert(sizeof(SIG_INSERT_AUTO_PUNISH_SECOND_USER) == 20,
              "SIG_INSERT_AUTO_PUNISH_SECOND_USER IDA ABI size must stay 20");


#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_AUTO_PUNISH_PAYLOADS_H_
