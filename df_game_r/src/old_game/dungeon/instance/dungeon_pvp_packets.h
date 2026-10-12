#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_PACKETS_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_PACKETS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822BD60 in the companion source.


#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 09_dungeon_pvp_party/Packet_Party_Result_Statistic.
struct Packet_Party_Result_Statistic : public PacketHeader {
    Packet_Party_Result_Statistic();

    uint8_t raw_0a[0x04];
};

// IDA unit: 09_dungeon_pvp_party/Packet_Party_Ping_Statistic.
struct Packet_Party_Ping_Statistic : public PacketHeader {
    Packet_Party_Ping_Statistic();

    uint8_t raw_0a[0x04];
};

// IDA unit: 09_dungeon_pvp_party/Packet_Pvp_Ping_Statistic.
struct Packet_Pvp_Ping_Statistic : public PacketHeader {
    Packet_Pvp_Ping_Statistic();

    uint8_t raw_0a[0x04];
};

// IDA unit: 09_dungeon_pvp_party/Packet_Fair_Pvp_Ping_Statistic.
struct Packet_Fair_Pvp_Ping_Statistic : public PacketHeader {
    Packet_Fair_Pvp_Ping_Statistic();

    uint16_t word_0a;
    uint16_t word_0c;
};

// IDA unit: 09_dungeon_pvp_party/Packet_PcRoomPlayTimeReward.
struct Packet_PcRoomPlayTimeReward : public PacketHeader {
    Packet_PcRoomPlayTimeReward();

    uint8_t byte_0a;
    uint32_t dword_0b;
    uint32_t dword_0f;
};

// IDA unit: 09_dungeon_pvp_party/Packet_Dungeon_Statistic_Party.
struct Packet_Dungeon_Statistic_Party : public PacketHeader {
    Packet_Dungeon_Statistic_Party();

    uint8_t raw_0a[0x36];
};

// IDA unit: 09_dungeon_pvp_party/Packet_Dungeon_Statistic_Party_Job.
struct Packet_Dungeon_Statistic_Party_Job : public PacketHeader {
    Packet_Dungeon_Statistic_Party_Job();

    uint8_t raw_0a[0x15];
};

// IDA unit: 09_dungeon_pvp_party/Packet_Blood_dungeon_statistic.
struct Packet_Blood_dungeon_statistic : public PacketHeader {
    Packet_Blood_dungeon_statistic();

    uint8_t raw_0a[0x0a];
};

// IDA unit: 09_dungeon_pvp_party/Packet_HellParty_Statistic_Item.
struct Packet_HellParty_Statistic_Item : public PacketHeader {
    Packet_HellParty_Statistic_Item();

    // CBattle_Field::kill_monster @0x830D023 writes these fields before
    // forwarding this frame through CHellPartyItemStatistic.
    uint8_t active_flag;                 // +0x0a
    int32_t dungeon_index;               // +0x0b
    uint8_t dungeon_difficulty;          // +0x0f
    uint8_t kill_data_byte;              // +0x10, low byte of argument four
    uint8_t statistics_enabled;          // +0x11
    int32_t item_rarity_counts[6];       // +0x12..+0x29
};

static_assert(offsetof(Packet_HellParty_Statistic_Item, active_flag) == 0x0au,
              "Hell Party statistic active flag must stay at +0x0a");
static_assert(offsetof(Packet_HellParty_Statistic_Item, dungeon_index) == 0x0bu,
              "Hell Party statistic dungeon index must stay at +0x0b");
static_assert(offsetof(Packet_HellParty_Statistic_Item, item_rarity_counts) == 0x12u,
              "Hell Party statistic rarity counters must stay at +0x12");
static_assert(sizeof(Packet_HellParty_Statistic_Item) == 0x2au,
              "Hell Party statistic packet must retain the old 0x2a-byte ABI");

// IDA unit: 09_dungeon_pvp_party/Packet_DeathTower_Statistic_Value.
struct Packet_DeathTower_Statistic_Value : public PacketHeader {
    Packet_DeathTower_Statistic_Value();

    uint8_t non_tutorial_dungeon;
    uint16_t charac_level;
    uint32_t statistic_field;
    uint32_t value;
};

// IDA unit: 09_dungeon_pvp_party/Packet_DeathTower_Statistic_Playdata_Job.
struct Packet_DeathTower_Statistic_Playdata_Job : public PacketHeader {
    Packet_DeathTower_Statistic_Playdata_Job();

    uint8_t death_tower_type;            // +0x0a
    uint16_t character_level;            // +0x0b
    uint32_t character_job;              // +0x0d
    uint8_t grow_type;                   // +0x11
    uint32_t clear_stage;                // +0x12
};

// IDA unit: 09_dungeon_pvp_party/Packet_DeathTower_Statistic_Playdata_Party.
struct Packet_DeathTower_Statistic_Playdata_Party : public PacketHeader {
    Packet_DeathTower_Statistic_Playdata_Party();

    uint8_t death_tower_type;            // +0x0a
    uint8_t start_member_count;          // +0x0b
    uint32_t clear_stage;                // +0x0c
};

static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Job,
                       death_tower_type) == 0x0au,
              "Death Tower job statistic type must stay at +0x0a");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Job,
                       character_level) == 0x0bu,
              "Death Tower job statistic level must stay at +0x0b");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Job,
                       character_job) == 0x0du,
              "Death Tower job statistic job must stay at +0x0d");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Job,
                       grow_type) == 0x11u,
              "Death Tower job statistic grow type must stay at +0x11");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Job,
                       clear_stage) == 0x12u,
              "Death Tower job statistic clear stage must stay at +0x12");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Party,
                       death_tower_type) == 0x0au,
              "Death Tower party statistic type must stay at +0x0a");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Party,
                       start_member_count) == 0x0bu,
              "Death Tower party statistic member count must stay at +0x0b");
static_assert(offsetof(Packet_DeathTower_Statistic_Playdata_Party,
                       clear_stage) == 0x0cu,
              "Death Tower party statistic clear stage must stay at +0x0c");

// IDA unit: 09_dungeon_pvp_party/Packet_Pcroom_Response.
struct Packet_Pcroom_Response : public PacketHeader {
    Packet_Pcroom_Response();

    uint32_t dword_0a;
    uint32_t dword_0e;
    uint8_t raw_12[0x0c];
    uint8_t byte_1e;
};

// IDA unit: 09_dungeon_pvp_party/Packet_PvPResult.
struct Packet_PvPResult : public PacketHeader {
    struct TeamResult {
        // CLeagueMatch::OnReward @0x855EB22 proves the complete 56-byte
        // per-team payload instead of an opaque byte interval.
        uint8_t daily_badges[4][3];       // packet +0x10/+0x48
        uint8_t awarded_badges[4][3];     // packet +0x1c/+0x54
        uint32_t school_numbers[4];       // packet +0x28/+0x60
        uint32_t account_ids[4];          // packet +0x38/+0x70
    };

    Packet_PvPResult();

    int32_t school_match_elo_k2;          // packet +0x0a
    uint8_t rating_result;                // packet +0x0e
    uint8_t player_count_per_team;        // packet +0x0f
    TeamResult teams[2];                  // packet +0x10
};

static_assert(sizeof(Packet_PvPResult::TeamResult) == 0x38u,
              "Packet_PvPResult team payload must stay 56 bytes");
static_assert(offsetof(Packet_PvPResult, school_match_elo_k2) == 0x0au,
              "Packet_PvPResult elo field must stay at +0x0a");
static_assert(offsetof(Packet_PvPResult, rating_result) == 0x0eu,
              "Packet_PvPResult rating result must stay at +0x0e");
static_assert(offsetof(Packet_PvPResult, player_count_per_team) == 0x0fu,
              "Packet_PvPResult player count must stay at +0x0f");
static_assert(offsetof(Packet_PvPResult, teams) == 0x10u,
              "Packet_PvPResult team payload must stay at +0x10");
static_assert(sizeof(Packet_PvPResult) == 0x80u,
              "Packet_PvPResult must stay 128 bytes");

// IDA unit: 09_dungeon_pvp_party/Packet_Response_Add_PvP_Buddy.
struct Packet_Response_Add_PvP_Buddy : public PacketHeader {
    Packet_Response_Add_PvP_Buddy();

    uint32_t acc_id;
    uint32_t charac_no;
    char response_kind;
    char target_name[30];
};

// IDA unit: 09_dungeon_pvp_party/Packet_TowerOfDespair_Statistic_GTS.
struct Packet_TowerOfDespair_Statistic_GTS : public PacketHeader {
    Packet_TowerOfDespair_Statistic_GTS();

    TowerOfDespairStatistic_Key key;
    uint8_t byte_12;
    uint32_t dword_13;
};

#pragma pack(pop)


#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_PACKETS_H_
