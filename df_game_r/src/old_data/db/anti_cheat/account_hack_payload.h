#ifndef DB_ACCOUNT_HACK_PAYLOAD_H_
#define DB_ACCOUNT_HACK_PAYLOAD_H_

#include <stdint.h>

#pragma pack(push, 1)

struct SIG_CHECK_MY_ACCOUNT_IN_DB {
    uint32_t uid;              // +0
    uint32_t account_id;       // +4
    char charac_name[29];      // +8
    uint32_t package_money;    // +37
};

struct SIG_CHECK_MY_ACCOUNT_OK {
    uint32_t uid;              // +0
    uint32_t package_money;    // +4
    uint32_t owner_account_id; // +8
};

struct SIG_SECOND_USER_DATA {
    uint32_t m_id;             // +0
    uint32_t gold_low;         // +4
    uint32_t gold_high;        // +8
    uint32_t user_idx;         // +12
    uint32_t acc_id;           // +16
};

struct SIG_SECOND_USER_DATA_SET {
    uint32_t req_gold_low;     // +0
    uint32_t req_gold_high;    // +4
    uint32_t db_gold_low;      // +8
    uint32_t db_gold_high;     // +12
    uint32_t trade_count;      // +16
    uint32_t acc_id;           // +20
};

struct SIG_INSERT_AUTO_PUNISH_FIRST_USER {
    uint32_t account_id;       // +0
    char ip[16];               // +4
    uint32_t hack_type;        // +20
    uint32_t count;            // +24
    uint32_t hack_sub_type;    // +28
    uint32_t hack_sub_count;   // +32
    uint16_t punish_flag;      // +36
    uint8_t reserved_26[2];    // +38
    uint32_t punish_value;     // +40
};

struct SIG_INSERT_AUTO_PUNISH_SECOND_USER {
    uint32_t m_id;             // +0
    uint32_t hack_m_id;        // +4
    uint32_t gold_low;         // +8
    uint32_t gold_high;        // +12
    uint8_t punish;            // +16
    uint8_t reserved_11[3];    // +17
};

#pragma pack(pop)

typedef char SIG_CHECK_MY_ACCOUNT_IN_DB_Size_Must_Be_41[
    (sizeof(SIG_CHECK_MY_ACCOUNT_IN_DB) == 41) ? 1 : -1];
typedef char SIG_CHECK_MY_ACCOUNT_OK_Size_Must_Be_12[
    (sizeof(SIG_CHECK_MY_ACCOUNT_OK) == 12) ? 1 : -1];
typedef char SIG_SECOND_USER_DATA_Size_Must_Be_20[
    (sizeof(SIG_SECOND_USER_DATA) == 20) ? 1 : -1];
typedef char SIG_SECOND_USER_DATA_SET_Size_Must_Be_24[
    (sizeof(SIG_SECOND_USER_DATA_SET) == 24) ? 1 : -1];
typedef char SIG_INSERT_AUTO_PUNISH_FIRST_USER_Size_Must_Be_44[
    (sizeof(SIG_INSERT_AUTO_PUNISH_FIRST_USER) == 44) ? 1 : -1];
typedef char SIG_INSERT_AUTO_PUNISH_SECOND_USER_Size_Must_Be_20[
    (sizeof(SIG_INSERT_AUTO_PUNISH_SECOND_USER) == 20) ? 1 : -1];

#endif  // DB_ACCOUNT_HACK_PAYLOAD_H_
