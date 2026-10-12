#include "old_game/activity/reward/reward_key.h"


typedef char rewardSubKey_Size_Must_Be_0x02[
    (sizeof(rewardSubKey) == 0x02) ? 1 : -1];
typedef char rewardKey_Size_Must_Be_0x05[
    (sizeof(rewardKey) == 0x05) ? 1 : -1];

// IDA_EA: 0x80F1190
// IDA_NAME: _ZN12rewardSubKeyC2Ev
// DEMANGLED: rewardSubKey::rewardSubKey(void)
rewardSubKey::rewardSubKey()
    : first(0),
      second(0) {}

// IDA_EA: 0x80F11A2
// IDA_NAME: _ZNK12rewardSubKeyltERKS_
// DEMANGLED: rewardSubKey::operator<(rewardSubKey const&)const
bool rewardSubKey::operator<(const rewardSubKey& rhs) const {
    uint8_t lhs_value;
    uint8_t rhs_value;
    if (first == rhs.first) {
        lhs_value = second;
        rhs_value = rhs.second;
    } else {
        lhs_value = first;
        rhs_value = rhs.first;
    }
    return lhs_value < rhs_value;
}

// IDA_EA: 0x80F11DE
// IDA_NAME: _ZN12rewardSubKey5clearEv
// DEMANGLED: rewardSubKey::clear(void)
rewardSubKey* rewardSubKey::clear() {
    first = 0;
    second = 0;
    return this;
}

// IDA_EA: 0x80F11F0
// IDA_NAME: _ZN9rewardKeyC2Ev
// DEMANGLED: rewardKey::rewardKey(void)
rewardKey::rewardKey()
    : first(0),
      second(0),
      third(0),
      sub_key() {}

// IDA_EA: 0x80F121A
// IDA_NAME: _ZNK9rewardKeyltERKS_
// DEMANGLED: rewardKey::operator<(rewardKey const&)const
bool rewardKey::operator<(const rewardKey& rhs) const {
    if (first != rhs.first) {
        return first < rhs.first;
    }
    if (second != rhs.second) {
        return second < rhs.second;
    }
    if (third == rhs.third) {
        return sub_key < rhs.sub_key;
    }
    return third < rhs.third;
}

// IDA_EA: 0x80F12AC
// IDA_NAME: _ZN9rewardKey5clearEv
// DEMANGLED: rewardKey::clear(void)
rewardSubKey* rewardKey::clear() {
    first = 0;
    second = 0;
    third = 0;
    return sub_key.clear();
}
