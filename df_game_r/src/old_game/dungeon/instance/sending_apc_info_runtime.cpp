#include "old_game/dungeon/instance/sending_apc_info_runtime.h"

#include "common/config.h"
#include <cstdio>
#include <cstring>

#include "old_game/dungeon/instance/tod_layer.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85FE860
// IDA_NAME: _ZN15SendingAPC_InfoC2Ev
// DEMANGLED: SendingAPC_Info::SendingAPC_Info(void)
SendingAPC_Info::SendingAPC_Info() : user_charac_no_(0), packet_() {
    packet_.clear();
    memset(&apc_info_, 0, sizeof(apc_info_));
}

// IDA_EA: 0x85FE8CA
// IDA_NAME: _ZN15SendingAPC_InfoD2Ev
// DEMANGLED: SendingAPC_Info::~SendingAPC_Info()
SendingAPC_Info::~SendingAPC_Info() {}

// IDA_EA: 0x85FE8E0
// IDA_NAME: _ZN15SendingAPC_Info8set_dataERK8_APCInfo
// DEMANGLED: SendingAPC_Info::set_data(_APCInfo const&)
int SendingAPC_Info::set_data(const _APCInfo& info) {
    std::memcpy(&apc_info_, &info, sizeof(apc_info_));
    return 47;
}

// IDA_EA: 0x85FE906
// IDA_NAME: _ZN15SendingAPC_Info12Set_APC_InfoERK9TOD_LayerRK8_APCInfo
// DEMANGLED: SendingAPC_Info::Set_APC_Info(TOD_Layer const&,_APCInfo const&)
int SendingAPC_Info::Set_APC_Info(const TOD_Layer& layer,
                                  const _APCInfo& info) {
    packet_.clear();
    user_charac_no_ = info.user_charac_no;
    packet_.put_header(0, 346);
    packet_.put_byte(layer.GetLayer());

    const int charac_name_length =
        static_cast<int>(std::strlen(info.user_charac_name));
    packet_.put_int(charac_name_length);
    packet_.put_str(info.user_charac_name, charac_name_length);
    packet_.put_byte(info.level);
    packet_.put_byte(info.job);
    packet_.put_byte(info.grow_type);

    const int guild_name_length = static_cast<int>(std::strlen(info.guild_name));
    packet_.put_int(guild_name_length);
    packet_.put_str(info.guild_name, guild_name_length);
    packet_.put_int(info.pvp_grade);
    for (int equipment_slot = 0;
         equipment_slot <= kTODLegacyAvatarLastSlot;
         ++equipment_slot) {
        packet_.put_int(info.equipment_item_ids[equipment_slot]);
    }
    // 140828 inserts weapon-avatar at target slot 10.  The original DB/Inter
    // record has no such field, so publish an explicit empty projection and
    // shift the original weapon/title/tail slots by one on the wire.
    packet_.put_int(0);
    for (int equipment_slot = kTODLegacyWeaponSlot;
         equipment_slot < kTODLegacyEquipmentTypeCount;
         ++equipment_slot) {
        packet_.put_int(info.equipment_item_ids[equipment_slot]);
    }

    const int creature_name_length =
        static_cast<int>(std::strlen(info.creature_name));
    packet_.put_int(creature_name_length);
    packet_.put_str(info.creature_name, creature_name_length);
    packet_.put_int(info.creature_id);
    // 140828 receiver sub_4E92A0 reads one trailing byte after creature_id
    // on both the layer-zero and named-APC branches.  It is unused in this
    // receiver; emit it explicitly rather than leaving it to cipher padding.
    packet_.put_byte(0);
    packet_.finalize(true);
    return 47;
}

// IDA_EA: 0x85FEBCA
// IDA_NAME: _ZN15SendingAPC_Info13Send_APC_InfoERK9TOD_LayerP5CUser
// DEMANGLED: SendingAPC_Info::Send_APC_Info(TOD_Layer const&,CUser *)
void SendingAPC_Info::Send_APC_Info(const TOD_Layer& layer, CUser* user) {
    if (!user) {
        return;
    }

    Set_APC_Info(layer, apc_info_);
    PacketGuard packet(&packet_);
    user->Send(packet);

    char message[256];
    std::snprintf(
        message, sizeof(message),
        "TOD : setting layer:%d, name:%s, lv:%d, job:%d, grow_type:%d, "
        "guild_name:%s, pvp_grade:%d, creature_name:%s, creature_id:%d",
        layer.GetLayer(), apc_info_.user_charac_name, apc_info_.level,
        apc_info_.job, apc_info_.grow_type, apc_info_.guild_name,
        apc_info_.pvp_grade, apc_info_.creature_name, apc_info_.creature_id);
    TowerOfDespairMgr::SendMessageToCUser(user, message);
}

// IDA_EA: 0x85FF12C / 0x85FF136
uint32_t SendingAPC_Info::get_user_charac_no() const {
    return user_charac_no_;
}

const char* SendingAPC_Info::get_user_charac_name() const {
    return apc_info_.user_charac_name;
}
