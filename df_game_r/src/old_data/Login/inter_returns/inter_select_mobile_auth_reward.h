#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SELECT_MOBILE_AUTH_REWARD_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SELECT_MOBILE_AUTH_REWARD_H_

#include <stdint.h>

#include "common/manager/taiwan_interface_inter_dispatcher.h"

// Taiwan internal Inter route 12.  The C2 request writes only the first five
// bytes; the explicit tail keeps the old eight-byte queue extent visible.
class InterSelectMobileAuthReward : public TaiwanInterfaceInterDispatcher {
public:
    struct Msg {
        uint32_t account_id;
        uint8_t rewarded;
        uint8_t reserved_05[3];
    };

    // IDA_EA: 0x81620FE
    // IDA_NAME: _ZN27InterSelectMobileAuthRewardC2Ev
    InterSelectMobileAuthReward();

    // IDA_EA: 0x8162122 (D2), 0x8162152 (D0)
    // IDA_NAME: _ZN27InterSelectMobileAuthRewardD2Ev / D0Ev
    virtual ~InterSelectMobileAuthReward();

    // IDA_EA: 0x816132A
    // IDA_NAME: _ZN27InterSelectMobileAuthReward12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x81615AE
    // IDA_NAME: _ZN27InterSelectMobileAuthReward11makeRequestEijb
    static void makeRequest(int user_idx, unsigned int account_id, bool rewarded);
};

#if defined(__i386__)
static_assert(sizeof(InterSelectMobileAuthReward::Msg) == 0x08u,
              "InterSelectMobileAuthReward::Msg must retain its wire ABI");
static_assert(sizeof(InterSelectMobileAuthReward) == 0x08u,
              "InterSelectMobileAuthReward must retain the Taiwan Inter ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_SELECT_MOBILE_AUTH_REWARD_H_
