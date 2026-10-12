#ifndef OLDSRV_LOGIN_INTER_NOTICE_H_
#define OLDSRV_LOGIN_INTER_NOTICE_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

#include <stddef.h>
#include <stdint.h>

class CUser;

// IDA_EA: Inter_Notice::dispatch_sig @0x84C5212.  The first byte is carried
// by the DB reply but is not consumed by this receiver; the following 80-byte
// field and the terminating length byte are passed to GameWorld unchanged.
#pragma pack(push, 1)
struct SIG_INTER_NOTICE {
    uint8_t reply_kind;       // +0x00, not consumed by this receiver
    char message[80];         // +0x01
    uint8_t message_length;   // +0x51
};
#pragma pack(pop)

static_assert(sizeof(SIG_INTER_NOTICE) == 0x52u,
              "Inter_Notice reply ABI must remain 82 bytes");
static_assert(offsetof(SIG_INTER_NOTICE, message) == 0x01u &&
                  offsetof(SIG_INTER_NOTICE, message_length) == 0x51u,
              "Inter_Notice reply offsets changed");

class Inter_Notice : public ISigDispatcher {
public:
    // IDA_EA: 0x84C5212
    // IDA_NAME: _ZN12Inter_Notice12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84E9F32 / 0x84E9F4E / 0x84E9F7E
    Inter_Notice();
    virtual ~Inter_Notice();
};

#if defined(__i386__)
static_assert(sizeof(Inter_Notice) == 0x04u,
              "Inter_Notice must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_NOTICE_H_
