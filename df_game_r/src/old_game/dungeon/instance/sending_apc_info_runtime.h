#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_SENDING_APC_INFO_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_SENDING_APC_INFO_RUNTIME_H_

#include <stdint.h>
#include <stddef.h>

#include "common/config.h"
#include "infra/network/packet/packet_buf.h"

class CUser;
struct TOD_Layer;

enum {
    // The DB/Inter ABI is the old 188-byte record.  The 140828 client adds a
    // weapon-avatar display slot only on the outgoing NOTI346 projection.
    kTODLegacyEquipmentTypeCount = 22,
    kTODLegacyAvatarLastSlot = 9,
    kTODLegacyWeaponSlot = 10,
};

// Canonical DB/Inter payload populated by DB_TOD_Reset_UserAPC and retained
// by SendingAPC_Info.  Keep this record at the original 0xBC-byte stride.
struct _APCInfo {
    uint32_t user_charac_no;              // IDA +0x00
    char user_charac_name[32];            // IDA +0x04
    int32_t level;                        // IDA +0x24
    int32_t job;                          // IDA +0x28
    int32_t grow_type;                    // IDA +0x2C
    char guild_name[24];                  // IDA +0x30
    int32_t pvp_grade;                    // IDA +0x48
    uint32_t equipment_item_ids[kTODLegacyEquipmentTypeCount]; // +0x4C
    char creature_name[16];               // +0xA4
    int32_t creature_id;                  // +0xB4
    uint32_t guild_id;                    // +0xB8
};

class SendingAPC_Info {
public:
    // IDA_EA: 0x85FE860
    // IDA_NAME: _ZN15SendingAPC_InfoC2Ev
    // DEMANGLED: SendingAPC_Info::SendingAPC_Info(void)
    SendingAPC_Info();

    // IDA_EA: 0x85FE8CA
    // IDA_NAME: _ZN15SendingAPC_InfoD2Ev
    // DEMANGLED: SendingAPC_Info::~SendingAPC_Info()
    ~SendingAPC_Info();

    // IDA_EA: 0x85FE8E0
    // IDA_NAME: _ZN15SendingAPC_Info8set_dataERK8_APCInfo
    // DEMANGLED: SendingAPC_Info::set_data(_APCInfo const&)
    int set_data(const _APCInfo& info);

    // IDA_EA: 0x85FE906
    // IDA_NAME: _ZN15SendingAPC_Info12Set_APC_InfoERK9TOD_LayerRK8_APCInfo
    // DEMANGLED: SendingAPC_Info::Set_APC_Info(TOD_Layer const&,_APCInfo const&)
    int Set_APC_Info(const TOD_Layer& layer, const _APCInfo& info);

    // IDA_EA: 0x85FEBCA
    // IDA_NAME: _ZN15SendingAPC_Info13Send_APC_InfoERK9TOD_LayerP5CUser
    // DEMANGLED: SendingAPC_Info::Send_APC_Info(TOD_Layer const&,CUser *)
    void Send_APC_Info(const TOD_Layer& layer, CUser* user);

    // IDA_EA: 0x85FF12C / 0x85FF136
    uint32_t get_user_charac_no() const;
    const char* get_user_charac_name() const;

private:
    uint32_t user_charac_no_;  // IDA +0x00000
    PacketBuf packet_;         // IDA +0x00004
    _APCInfo apc_info_;        // IDA +0x17338
};

static_assert(sizeof(_APCInfo) == 0xBCu,
              "_APCInfo DB/Inter payload must stay 0xBC");
static_assert(offsetof(_APCInfo, creature_name) == 0xA4u &&
                  offsetof(_APCInfo, creature_id) == 0xB4u &&
                  offsetof(_APCInfo, guild_id) == 0xB8u,
              "_APCInfo original field offsets drifted");
#if defined(__i386__)
static_assert(sizeof(SendingAPC_Info) == 0x173F4u,
              "SendingAPC_Info target object must stay 0x173F4");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_SENDING_APC_INFO_RUNTIME_H_
