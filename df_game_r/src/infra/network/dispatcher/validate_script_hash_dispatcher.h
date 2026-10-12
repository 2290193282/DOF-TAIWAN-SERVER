#ifndef OLDSRV_PROTOCOL_VALIDATE_SCRIPT_HASH_DISPATCHER_H_
#define OLDSRV_PROTOCOL_VALIDATE_SCRIPT_HASH_DISPATCHER_H_

#include <stdint.h>

#include "old_data/PVF/data/datamanager/script_hash_param.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

#pragma pack(push, 1)
// CMD 334 / HACK_SCRIPT_HASH.  The old unit has no distinct constructor;
// dispatch_template constructs the packed storage and read() fills all bytes.
struct MSG_SCRIPT_HASH : public MSG_BASE {
    uint8_t script_type;
    uint8_t script_index;
    uint32_t script_hash;
};
#pragma pack(pop)

static_assert(sizeof(MSG_SCRIPT_HASH) == 0x13u,
              "CMD334 message must preserve its 19-byte old-server ABI");

class Dispatcher_ValidateScriptHash
    : public IPacketDispatcher<MSG_SCRIPT_HASH,
                               ScriptHashParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ValidateScriptHash();
    virtual ~Dispatcher_ValidateScriptHash();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
    WongWork::ENUM_HACKTYPE get_hack_type(char script_type);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ValidateScriptHash) == 0x04u,
              "CMD334 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_PROTOCOL_VALIDATE_SCRIPT_HASH_DISPATCHER_H_
