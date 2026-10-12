#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_TYPES_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_TYPES_H_

#include <stdint.h>

#include "infra/network/packet/message_base.h"

// IDA unit: 99_misc_global/MSG_SELECT_STRIKER.  The two request bytes are
// intentionally packed immediately after the 13-byte MSG_BASE frame header.
#pragma pack(push, 1)
struct MSG_SELECT_STRIKER : public MSG_BASE {
    // IDA_EA: 0x822E2D0
    // IDA_NAME: _ZN18MSG_SELECT_STRIKERC2Ev
    // DEMANGLED: MSG_SELECT_STRIKER::MSG_SELECT_STRIKER(void)
    MSG_SELECT_STRIKER();

    int8_t charac_index;          // +0x0d; C2 initializes -1
    uint8_t striker_skill_index;  // +0x0e; C2 initializes 0
};
#pragma pack(pop)

// IDA unit: 99_misc_global/stSelectStriker.  Its old C2 only builds the
// ParamBase vptr; process() writes result before it is inspected.
struct stSelectStriker : public ParamBase {
    // IDA_EA: 0x824A3D8
    // IDA_NAME: _ZN15stSelectStrikerC2Ev
    // DEMANGLED: stSelectStriker::stSelectStriker(void)
    stSelectStriker();

    // IDA_EA: 0x824A3F4 / 0x824A424
    virtual ~stSelectStriker();

    int result;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(MSG_SELECT_STRIKER) == 0x0fu,
              "CMD488 request must retain the old 15-byte wire ABI");
static_assert(sizeof(stSelectStriker) == 0x08u,
              "CMD488 parameter must retain ParamBase plus result");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_TYPES_H_
