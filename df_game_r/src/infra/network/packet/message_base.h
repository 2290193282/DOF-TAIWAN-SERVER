#ifndef OLDSRV_PROTOCOL_MESSAGE_BASE_H_
#define OLDSRV_PROTOCOL_MESSAGE_BASE_H_

#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/MSG_BASE.  The user-confirmed legacy C2S field
// layout is the packed CS_Header contract.  Keep MSG_BASE as a distinct C++
// type so its original C2 symbol and derived-message ABI remain canonical.
// Dispatcher reads may populate this 13-byte object from the payload cursor;
// inheriting the layout does not add an outer-header copy operation.
struct MSG_BASE : CS_Header {
    MSG_BASE();
};

static_assert(sizeof(CS_Header) == 13u,
              "legacy client-to-server frame header must stay packed at 13 bytes");
static_assert(sizeof(MSG_BASE) == sizeof(CS_Header),
              "MSG_BASE must preserve the legacy 13-byte base extent");

// IDA unit: 99_misc_global/ParamBase. Polymorphic common dispatcher parameter
// base used by packet dispatcher specializations.
struct ParamBase {
    ParamBase();
    ParamBase(const ParamBase& rhs);
    virtual ~ParamBase();
};


#endif  // OLDSRV_PROTOCOL_MESSAGE_BASE_H_
