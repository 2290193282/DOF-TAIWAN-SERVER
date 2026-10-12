#ifndef OLDSRV_GLOBAL_CHARACTER_NAME_CHANGE_PROXY_RUNTIME_H_
#define OLDSRV_GLOBAL_CHARACTER_NAME_CHANGE_PROXY_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/Packet_Change_Char_Name.  The same 0x34-byte
// request is forwarded over the guild and monitor TCP connections.
#pragma pack(push, 1)
struct Packet_Change_Char_Name : PacketHeader {
    // IDA_EA: 0x846FA00
    // IDA_NAME: _ZN23Packet_Change_Char_NameC2Ev
    // DEMANGLED: Packet_Change_Char_Name::Packet_Change_Char_Name(void)
    Packet_Change_Char_Name();

    uint32_t account_id;   // +0x0a
    uint32_t charac_no;    // +0x0e
    uint32_t guild_id;     // +0x12
    char charac_name[30];  // +0x16
};
#pragma pack(pop)

static_assert(sizeof(Packet_Change_Char_Name) == 0x34u,
              "character-name-change packet must retain old 0x34-byte ABI");
static_assert(offsetof(Packet_Change_Char_Name, account_id) == 0x0au &&
                  offsetof(Packet_Change_Char_Name, charac_no) == 0x0eu &&
                  offsetof(Packet_Change_Char_Name, guild_id) == 0x12u &&
                  offsetof(Packet_Change_Char_Name, charac_name) == 0x16u,
              "character-name-change packet field offsets changed");

#endif  // OLDSRV_GLOBAL_CHARACTER_NAME_CHANGE_PROXY_RUNTIME_H_
