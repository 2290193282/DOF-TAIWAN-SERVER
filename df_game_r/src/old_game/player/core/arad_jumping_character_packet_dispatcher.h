#ifndef OLDSRV_OLD_GAME_USERINFO_ARAD_JUMPING_CHARACTER_PACKET_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_ARAD_JUMPING_CHARACTER_PACKET_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

namespace arad {

// This is a queue-1 ARAD inner record, not a C2S overlay.
//
// Dispatcher_AradJumpingCharacter::dispatch_sig @0x819F1EC first constructs
// the nine-byte prefix, then consumes eight C2S bytes into +0x09..+0x10,
// writes the account/character identity into +0x01/+0x05, and finally queues
// exactly 0x11 bytes under internal code 45.  The old Sig C2/init only clear
// the prefix; the client-origin suffix is deliberately filled by the
// dispatcher instead of being default-initialized here.
#pragma pack(push, 1)
struct SigAradJumpingCharacter {
    uint8_t reserved_zero;              // +0x00: cleared by init
    uint32_t acc_id;                    // +0x01: CUser::get_acc_id()
    uint32_t charac_no;                 // +0x05: CUser::get_charac_no(-1)
    uint8_t ticket_type;                // +0x09: C2S byte, valid 1..2
    uint8_t grow_type;                  // +0x0a: C2S byte, valid 1..4
    uint16_t material_slot;             // +0x0b: C2S short
    uint32_t opaque_client_value;       // +0x0d: C2S int, not consumed here

    // IDA_EA: 0x81A0990
    // IDA_NAME: _ZN4arad23SigAradJumpingCharacterC2Ev
    // DEMANGLED: arad::SigAradJumpingCharacter::SigAradJumpingCharacter(void)
    SigAradJumpingCharacter();

    // IDA_EA: 0x81A09A4
    // IDA_NAME: _ZN4arad23SigAradJumpingCharacter4initEv
    // DEMANGLED: arad::SigAradJumpingCharacter::init(void)
    SigAradJumpingCharacter* init();
};
#pragma pack(pop)

static_assert(offsetof(SigAradJumpingCharacter, acc_id) == 0x01u,
              "CMD585 internal account identity must stay at +0x01");
static_assert(offsetof(SigAradJumpingCharacter, charac_no) == 0x05u,
              "CMD585 internal character identity must stay at +0x05");
static_assert(offsetof(SigAradJumpingCharacter, ticket_type) == 0x09u,
              "CMD585 C2S ticket type must stay at +0x09");
static_assert(offsetof(SigAradJumpingCharacter, grow_type) == 0x0au,
              "CMD585 C2S grow type must stay at +0x0a");
static_assert(offsetof(SigAradJumpingCharacter, material_slot) == 0x0bu,
              "CMD585 C2S material slot must stay at +0x0b");
static_assert(offsetof(SigAradJumpingCharacter, opaque_client_value) == 0x0du,
              "CMD585 C2S opaque integer must stay at +0x0d");
static_assert(sizeof(SigAradJumpingCharacter) == 0x11u,
              "CMD585 ARAD inner queue record must remain 17 bytes");

}  // namespace arad

namespace ARAD {
namespace DISPATCHER {

// CMD585: ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET.
// This class is intentionally unregistered here.  The root PacketDispatcher
// table and the internal reward leaf are owned by separate closure work.
class Dispatcher_AradJumpingCharacter
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597BCE
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterC2Ev
    Dispatcher_AradJumpingCharacter();

    // IDA_EA: 0x81A0DA2
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterD2Ev
    // IDA_EA: 0x81A0DD2
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterD0Ev
    virtual ~Dispatcher_AradJumpingCharacter();

    // IDA_EA: 0x819F1EC
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x819F44C
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter14checkWorkStateEP5CUserRN4arad23SigAradJumpingCharacterE
    int checkWorkState(CUser* user, arad::SigAradJumpingCharacter& payload);

    // IDA_EA: 0x81A0B22
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B2C
    // IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AradJumpingCharacter) == 0x04u,
              "CMD585 dispatcher must retain the i386 base-only ABI");
#endif

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_USERINFO_ARAD_JUMPING_CHARACTER_PACKET_DISPATCHER_H_
