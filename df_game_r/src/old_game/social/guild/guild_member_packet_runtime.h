#ifndef OLDSRV_OLD_GAME_GROUP_GUILD_MEMBER_PACKET_RUNTIME_H_
#define OLDSRV_OLD_GAME_GROUP_GUILD_MEMBER_PACKET_RUNTIME_H_

#include <stdint.h>

class PacketGuard;
class CUser;

// Monitor guild-member records have no padding in the old wire ABI.  The
// opaque middle string is emitted verbatim as the second client string.
#pragma pack(push, 1)
struct ST_Guild_Mem_Info {
    uint8_t job;                 // +0x00
    uint8_t guild_member_grade;  // +0x01 legacy name: packed grow type, NOT guild rank
    uint16_t level;              // +0x02
    char character_name[30];     // +0x04
    char member_message[21];     // +0x22
    uint8_t channel_no;          // +0x37
    uint8_t state;               // +0x38
    uint8_t block_flag;          // +0x39
    uint8_t master_grade;        // +0x3a authoritative guild member grade
    uint32_t logout_time;        // +0x3b
};
#pragma pack(pop)

static_assert(sizeof(ST_Guild_Mem_Info) == 0x3fu,
              "ST_Guild_Mem_Info must retain the old 63-byte Monitor ABI");

class FunctionGuildMember {
public:
    // The authenticated roster is also an authoritative source of the
    // current character's member grade.  Never infer a grade from guild level.
    static bool SyncCurrentMemberGrade(CUser* user, uint32_t charac_no,
                                      uint32_t guild_key, int count,
                                      const ST_Guild_Mem_Info* members);
    static void RefreshGuildBenefits(CUser* user);
    // IDA_EA: 0x84CA37A
    // IDA_NAME: _ZN19FunctionGuildMember26MakePacketGuildMembersInfoER11PacketGuardiP17ST_Guild_Mem_Info
    static bool MakePacketGuildMembersInfo(PacketGuard& packet,
                                           int count,
                                           ST_Guild_Mem_Info* members);
};

#endif  // OLDSRV_OLD_GAME_GROUP_GUILD_MEMBER_PACKET_RUNTIME_H_
