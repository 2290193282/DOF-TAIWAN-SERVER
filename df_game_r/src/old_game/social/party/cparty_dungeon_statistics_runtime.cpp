#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/player/core/character_packets.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

namespace {

void PutByte(uint8_t* payload, uint32_t offset, uint8_t value) {
    std::memcpy(payload + offset, &value, sizeof(value));
}

void PutWord(uint8_t* payload, uint32_t offset, uint16_t value) {
    std::memcpy(payload + offset, &value, sizeof(value));
}

void PutInt(uint8_t* payload, uint32_t offset, int value) {
    std::memcpy(payload + offset, &value, sizeof(value));
}

void PutDungeonHeader(uint8_t* payload, CBattle_Field* battle_field) {
    PutWord(payload,
            0u,
            static_cast<uint16_t>(G_CEnvironment()->get_channel_no()));
    PutInt(payload, 2u, battle_field->get_dungeon_index());
    PutByte(payload,
            6u,
            static_cast<uint8_t>(battle_field->get_dungeon_diff()));
}

void PutDungeonPlayFields(uint8_t* payload,
                          uint32_t offset,
                          const PlayInfo& play_info) {
    PutInt(payload, offset + 0u,
           static_cast<int>(play_info.clear_time_seconds_));
    PutInt(payload, offset + 4u,
           static_cast<int>(play_info.die_count_));
    PutInt(payload, offset + 8u,
           static_cast<int>(play_info.hp_consume_));
    PutInt(payload, offset + 12u,
           static_cast<int>(play_info.mp_consume_));
    PutInt(payload, offset + 16u,
           static_cast<int>(play_info.hit_count_));
    PutInt(payload, offset + 20u,
           static_cast<int>(play_info.hit_per_average_damage_));
    PutInt(payload, offset + 24u,
           static_cast<int>(play_info.hp_recovery_));
    PutInt(payload, offset + 28u,
           static_cast<int>(play_info.mp_recovery_));
}

void SendStatisticPacket(CUser* user, char* packet, int packet_size) {
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(packet, packet_size);
}

}  // namespace

// IDA_EA: 0x85B5098
// IDA_NAME: _ZN6CParty21SendDungeonCharacInfoEP5CUserb
// DEMANGLED: CParty::SendDungeonCharacInfo(CUser *,bool)
int CParty::SendDungeonCharacInfo(CUser* user, bool cleared) {
    Character& character = user->character_;
    character.clear_time_seconds_ = static_cast<uint32_t>(
        GlobalData::system_time()->getCurSec() - game_start_sec_);
    character.character_level_ =
        static_cast<uint32_t>(user->get_charac_level());

    if (character.party_member_count_) {
        SendDungeonStatisticParty(cleared, user);
    }
    if (character.party_member_count_) {
        SendDungeonStatisticPartyJob(cleared, user);
    }
    if (character.party_member_count_) {
        SendDungeonStatisticPartyCharac(cleared, user);
    }
    return 1;
}

// IDA_EA: 0x85B5158
// IDA_NAME: _ZN6CParty25SendDungeonStatisticPartyEbP5CUser
// DEMANGLED: CParty::SendDungeonStatisticParty(bool,CUser *)
int CParty::SendDungeonStatisticParty(bool cleared, CUser* user) {
    Packet_Dungeon_Statistic_Party packet;
    uint8_t* const payload = packet.raw_0a;
    const Character& character = user->character_;
    PutDungeonHeader(payload, &battle_field_);
    PutByte(payload, 7u, battle_data_.dungeon_start_statistic_flags_[1]);
    PutByte(payload, 8u, battle_data_.dungeon_start_statistic_flags_[2]);
    PutByte(payload, 9u, battle_data_.dungeon_start_statistic_flags_[3]);
    PutByte(payload, 10u, cleared ? 1u : 0u);
    PutByte(payload,
            11u,
            static_cast<uint8_t>(character.party_member_count_));
    PutDungeonPlayFields(payload, 12u, character);
    PutInt(payload, 44u,
           static_cast<int>(character.reason_one_or_two_exp_));
    PutInt(payload, 48u,
           static_cast<int>(character.dungeon_bonus_exp_));
    PutWord(payload,
            52u,
            static_cast<uint16_t>(character.character_level_));
    SendStatisticPacket(user,
                        reinterpret_cast<char*>(&packet),
                        sizeof(packet));
    return 1;
}

// IDA_EA: 0x85B52A6
// IDA_NAME: _ZN6CParty28SendDungeonStatisticPartyJobEbP5CUser
// DEMANGLED: CParty::SendDungeonStatisticPartyJob(bool,CUser *)
int CParty::SendDungeonStatisticPartyJob(bool cleared, CUser* user) {
    Packet_Dungeon_Statistic_Party_Job packet;
    uint8_t* const payload = packet.raw_0a;
    PutDungeonHeader(payload, &battle_field_);
    PutByte(payload, 7u, battle_data_.dungeon_start_statistic_flags_[1]);
    PutByte(payload, 8u, battle_data_.dungeon_start_statistic_flags_[2]);
    PutByte(payload, 9u, battle_data_.dungeon_start_statistic_flags_[3]);
    PutByte(payload, 10u, cleared ? 1u : 0u);
    PutInt(payload, 11u, user->get_charac_job());
    PutByte(payload,
            15u,
            static_cast<uint8_t>(user->getCurCharacGrowType()));
    PutByte(payload,
            16u,
            static_cast<uint8_t>(user->character_.party_member_count_));
    PutInt(payload,
           17u,
           cleared ? static_cast<int>(game_result_set_.raw_48)
                   : 0);
    SendStatisticPacket(user,
                        reinterpret_cast<char*>(&packet),
                        sizeof(packet));
    return 1;
}

// IDA_EA: 0x85B53AA
// IDA_NAME: _ZN6CParty31SendDungeonStatisticPartyCharacEbP5CUser
// DEMANGLED: CParty::SendDungeonStatisticPartyCharac(bool,CUser *)
int CParty::SendDungeonStatisticPartyCharac(bool cleared, CUser* user) {
    Packet_Dungeon_Statistic_Party_Charac packet;
    uint8_t* const payload = packet.raw_0a;
    const Character& character = user->character_;
    PutDungeonHeader(payload, &battle_field_);
    PutByte(payload, 7u, battle_data_.dungeon_start_statistic_flags_[0]);
    PutByte(payload, 8u, cleared ? 1u : 0u);
    PutInt(payload, 9u, user->get_charac_job());
    PutByte(payload,
            13u,
            static_cast<uint8_t>(user->getCurCharacGrowType()));
    PutByte(payload,
            14u,
            static_cast<uint8_t>(character.party_member_count_));
    PutDungeonPlayFields(payload, 15u, character);
    PutInt(payload, 47u,
           static_cast<int>(character.reason_one_or_two_exp_));
    PutInt(payload, 51u,
           static_cast<int>(character.dungeon_bonus_exp_));
    PutWord(payload,
            55u,
            static_cast<uint16_t>(character.character_level_));
    PutInt(payload,
           57u,
           cleared ? static_cast<int>(game_result_set_.raw_48)
                   : 0);
    SendStatisticPacket(user,
                        reinterpret_cast<char*>(&packet),
                        sizeof(packet));
    return 1;
}
