#ifndef OLDSRV_11_EVENT_RUNTIME_REWARD_KEY_H_
#define OLDSRV_11_EVENT_RUNTIME_REWARD_KEY_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x80F1190 in the companion source.


#include <stdint.h>


// IDA unit: 11_event_runtime/rewardSubKey.
struct rewardSubKey {
    rewardSubKey();

    bool operator<(const rewardSubKey& rhs) const;
    rewardSubKey* clear();

    uint8_t first;
    uint8_t second;
};

// IDA unit: 11_event_runtime/rewardKey.
struct rewardKey {
    rewardKey();

    bool operator<(const rewardKey& rhs) const;
    rewardSubKey* clear();

    uint8_t first;
    uint8_t second;
    uint8_t third;
    rewardSubKey sub_key;
};


#endif  // OLDSRV_11_EVENT_RUNTIME_REWARD_KEY_H_
