#ifndef OLDSRV_DB_DB_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK_H_
#define OLDSRV_DB_DB_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

// IDA_EA: Inter_LoadCharac::dispatch_sig @0x84C3398,
// CStreamGuard::GetInBuffer<...> @0x84F1578, and
// DB_SaveSameMIDOtherCharLoginHack::dispatch @0x8422680.
// The original queues this exact 0x20-byte packed LogDB record on an account
// mismatch.  +0x0C is the 20-byte peer-IP string written by GetPeerIP2.
#pragma pack(push, 1)
struct SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK {
    uint32_t first_mid;       // +0x00: CUser account id
    uint32_t second_mid;      // +0x04: account id returned by DB_LoadCharac
    int32_t charac_no;        // +0x08: loaded CHARAC_DATA::character_no
    char peer_ip[20];         // +0x0C: CNetwork<4096,450000>::GetPeerIP2
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK) == 0x20u,
              "same-MID login-hack signal must stay 32 bytes");
static_assert(offsetof(SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK, first_mid) == 0x00u &&
                  offsetof(SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK, second_mid) == 0x04u &&
                  offsetof(SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK, charac_no) == 0x08u &&
                  offsetof(SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK, peer_ip) == 0x0cu,
              "same-MID login-hack signal fields must retain the IDA offsets");


// IDA_EA: 0x844E462/0x845F0DC/0x845F10C/0x8422680.
// IDA unit: 02_login_account/DB_SaveSameMIDOtherCharLoginHack.
class DB_SaveSameMIDOtherCharLoginHack : public DBDispatcher {
public:
    DB_SaveSameMIDOtherCharLoginHack();
    virtual ~DB_SaveSameMIDOtherCharLoginHack();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK_H_
