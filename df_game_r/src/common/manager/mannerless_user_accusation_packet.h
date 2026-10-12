#ifndef OLDSRV_GLOBAL_MANNERLESS_USER_ACCUSATION_PACKET_H_
#define OLDSRV_GLOBAL_MANNERLESS_USER_ACCUSATION_PACKET_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

// Internal Monitor TCP record used by CMD140 and its later Inter/DB return
// path.  It is not a client C2S/S2C frame.
#pragma pack(push, 1)
struct Packet_Mannerless_User_Accusation {
    // IDA_EA: 0x822C1E6
    // IDA_NAME: _ZN33Packet_Mannerless_User_AccusationC2Ev
    // DEMANGLED: Packet_Mannerless_User_Accusation::Packet_Mannerless_User_Accusation(void)
    Packet_Mannerless_User_Accusation();

    // IDA_EA: 0x822C1CA
    // IDA_NAME: _ZN33Packet_Mannerless_User_Accusation24isNeedTypeForChattingMsgEi
    // DEMANGLED: Packet_Mannerless_User_Accusation::isNeedTypeForChattingMsg(int)
    // The old mangling takes only an int.  This is static despite Hex-Rays'
    // misleading synthetic `this` label.
    static bool isNeedTypeForChattingMsg(int accusation_type);

    PacketHeader header;                  // +0x000
    int32_t uid;                          // +0x00a
    int32_t charac_no;                    // +0x00e
    int32_t accusation_type;              // +0x012
    int32_t reporter_server_group;        // +0x016
    int32_t accused_server_group;         // +0x01a
    int32_t reporter_acc_id;              // +0x01e
    char reporter_name[30];               // +0x022
    char accused_name[30];                // +0x040
    char chat_message[256];               // +0x05e
    uint8_t result_or_status;             // +0x15e
};
#pragma pack(pop)

static_assert(sizeof(Packet_Mannerless_User_Accusation) == 0x15fu,
              "CMD140 Monitor record must retain its 0x15f-byte ABI");
static_assert(offsetof(Packet_Mannerless_User_Accusation, uid) == 0x0au &&
                  offsetof(Packet_Mannerless_User_Accusation, charac_no) == 0x0eu &&
                  offsetof(Packet_Mannerless_User_Accusation, accusation_type) == 0x12u &&
                  offsetof(Packet_Mannerless_User_Accusation, reporter_server_group) == 0x16u &&
                  offsetof(Packet_Mannerless_User_Accusation, accused_server_group) == 0x1au &&
                  offsetof(Packet_Mannerless_User_Accusation, reporter_acc_id) == 0x1eu &&
                  offsetof(Packet_Mannerless_User_Accusation, reporter_name) == 0x22u &&
                  offsetof(Packet_Mannerless_User_Accusation, accused_name) == 0x40u &&
                  offsetof(Packet_Mannerless_User_Accusation, chat_message) == 0x5eu &&
                  offsetof(Packet_Mannerless_User_Accusation, result_or_status) == 0x15eu,
              "CMD140 Monitor record field offsets changed");

#endif  // OLDSRV_GLOBAL_MANNERLESS_USER_ACCUSATION_PACKET_H_
