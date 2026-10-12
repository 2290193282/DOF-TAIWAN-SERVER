#include "old_game/dungeon/instance/dungeon_pvp_packets.h"

#include <cstring>


typedef char Packet_Party_Result_Statistic_Size_Must_Be_0x0e[
    (sizeof(Packet_Party_Result_Statistic) == 0x0e) ? 1 : -1];
typedef char Packet_Party_Ping_Statistic_Size_Must_Be_0x0e[
    (sizeof(Packet_Party_Ping_Statistic) == 0x0e) ? 1 : -1];
typedef char Packet_Pvp_Ping_Statistic_Size_Must_Be_0x0e[
    (sizeof(Packet_Pvp_Ping_Statistic) == 0x0e) ? 1 : -1];
typedef char Packet_Fair_Pvp_Ping_Statistic_Size_Must_Be_0x0e[
    (sizeof(Packet_Fair_Pvp_Ping_Statistic) == 0x0e) ? 1 : -1];
typedef char Packet_PcRoomPlayTimeReward_Size_Must_Be_0x13[
    (sizeof(Packet_PcRoomPlayTimeReward) == 0x13) ? 1 : -1];
typedef char Packet_Dungeon_Statistic_Party_Size_Must_Be_0x40[
    (sizeof(Packet_Dungeon_Statistic_Party) == 0x40) ? 1 : -1];
typedef char Packet_Dungeon_Statistic_Party_Job_Size_Must_Be_0x1f[
    (sizeof(Packet_Dungeon_Statistic_Party_Job) == 0x1f) ? 1 : -1];
typedef char Packet_Blood_dungeon_statistic_Size_Must_Be_0x14[
    (sizeof(Packet_Blood_dungeon_statistic) == 0x14) ? 1 : -1];
typedef char Packet_HellParty_Statistic_Item_Size_Must_Be_0x2a[
    (sizeof(Packet_HellParty_Statistic_Item) == 0x2a) ? 1 : -1];
typedef char Packet_DeathTower_Statistic_Value_Size_Must_Be_0x15[
    (sizeof(Packet_DeathTower_Statistic_Value) == 0x15) ? 1 : -1];
typedef char Packet_DeathTower_Statistic_Playdata_Job_Size_Must_Be_0x16[
    (sizeof(Packet_DeathTower_Statistic_Playdata_Job) == 0x16) ? 1 : -1];
typedef char Packet_DeathTower_Statistic_Playdata_Party_Size_Must_Be_0x10[
    (sizeof(Packet_DeathTower_Statistic_Playdata_Party) == 0x10) ? 1 : -1];
typedef char Packet_Pcroom_Response_Size_Must_Be_0x1f[
    (sizeof(Packet_Pcroom_Response) == 0x1f) ? 1 : -1];
typedef char Packet_PvPResult_Size_Must_Be_0x80[
    (sizeof(Packet_PvPResult) == 0x80) ? 1 : -1];
typedef char Packet_Response_Add_PvP_Buddy_Size_Must_Be_0x31[
    (sizeof(Packet_Response_Add_PvP_Buddy) == 0x31) ? 1 : -1];
typedef char Packet_TowerOfDespair_Statistic_GTS_Size_Must_Be_0x17[
    (sizeof(Packet_TowerOfDespair_Statistic_GTS) == 0x17) ? 1 : -1];

// IDA_EA: 0x822BD60
// IDA_NAME: _ZN29Packet_Party_Result_StatisticC2Ev
// DEMANGLED: Packet_Party_Result_Statistic::Packet_Party_Result_Statistic(void)
Packet_Party_Result_Statistic::Packet_Party_Result_Statistic()
    : PacketHeader(0x0FA6u, 0x000Eu) {}

// IDA_EA: 0x822BD84
// IDA_NAME: _ZN27Packet_Party_Ping_StatisticC2Ev
// DEMANGLED: Packet_Party_Ping_Statistic::Packet_Party_Ping_Statistic(void)
Packet_Party_Ping_Statistic::Packet_Party_Ping_Statistic()
    : PacketHeader(0x0FA7u, 0x000Eu) {}

// IDA_EA: 0x822BDA8
// IDA_NAME: _ZN25Packet_Pvp_Ping_StatisticC2Ev
// DEMANGLED: Packet_Pvp_Ping_Statistic::Packet_Pvp_Ping_Statistic(void)
Packet_Pvp_Ping_Statistic::Packet_Pvp_Ping_Statistic()
    : PacketHeader(0x0FA8u, 0x000Eu) {}

// IDA_EA: 0x822C808
// IDA_NAME: _ZN30Packet_Fair_Pvp_Ping_StatisticC2Ev
// DEMANGLED: Packet_Fair_Pvp_Ping_Statistic::Packet_Fair_Pvp_Ping_Statistic(void)
Packet_Fair_Pvp_Ping_Statistic::Packet_Fair_Pvp_Ping_Statistic()
    : PacketHeader(0x27EDu, 0x000Eu),
      word_0a(0),
      word_0c(0) {}

// IDA_EA: 0x8110C0A
// IDA_NAME: _ZN27Packet_PcRoomPlayTimeRewardC2Ev
// DEMANGLED: Packet_PcRoomPlayTimeReward::Packet_PcRoomPlayTimeReward(void)
Packet_PcRoomPlayTimeReward::Packet_PcRoomPlayTimeReward()
    : PacketHeader(0x27ECu, 0x0013u),
      byte_0a(0),
      dword_0b(0),
      dword_0f(0) {}

// IDA_EA: 0x85BEDE6
// IDA_NAME: _ZN30Packet_Dungeon_Statistic_PartyC2Ev
// DEMANGLED: Packet_Dungeon_Statistic_Party::Packet_Dungeon_Statistic_Party(void)
Packet_Dungeon_Statistic_Party::Packet_Dungeon_Statistic_Party()
    : PacketHeader(0x0C35u, 0x0040u) {}

// IDA_EA: 0x85BEE0A
// IDA_NAME: _ZN34Packet_Dungeon_Statistic_Party_JobC2Ev
// DEMANGLED: Packet_Dungeon_Statistic_Party_Job::Packet_Dungeon_Statistic_Party_Job(void)
Packet_Dungeon_Statistic_Party_Job::Packet_Dungeon_Statistic_Party_Job()
    : PacketHeader(0x0C37u, 0x001Fu) {}

// IDA_EA: 0x85BEE52
// IDA_NAME: _ZN30Packet_Blood_dungeon_statisticC2Ev
// DEMANGLED: Packet_Blood_dungeon_statistic::Packet_Blood_dungeon_statistic(void)
Packet_Blood_dungeon_statistic::Packet_Blood_dungeon_statistic()
    : PacketHeader(0x1036u, 0x0014u) {}

// IDA_EA: 0x860FF9E
// IDA_NAME: _ZN31Packet_HellParty_Statistic_ItemC2Ev
// DEMANGLED: Packet_HellParty_Statistic_Item::Packet_HellParty_Statistic_Item(void)
Packet_HellParty_Statistic_Item::Packet_HellParty_Statistic_Item()
    : PacketHeader(0x0C31u, 0x002Au) {}

// IDA_EA: 0x860FFC2
// IDA_NAME: _ZN33Packet_DeathTower_Statistic_ValueC2Ev
// DEMANGLED: Packet_DeathTower_Statistic_Value::Packet_DeathTower_Statistic_Value(void)
Packet_DeathTower_Statistic_Value::Packet_DeathTower_Statistic_Value()
    : PacketHeader(0x0C3Bu, 0x0015u) {}

// IDA_EA: 0x860FFE6
// IDA_NAME: _ZN40Packet_DeathTower_Statistic_Playdata_JobC2Ev
// DEMANGLED: Packet_DeathTower_Statistic_Playdata_Job::Packet_DeathTower_Statistic_Playdata_Job(void)
Packet_DeathTower_Statistic_Playdata_Job::
    Packet_DeathTower_Statistic_Playdata_Job()
    : PacketHeader(0x0C3Du, 0x0016u) {}

// IDA_EA: 0x861000A
// IDA_NAME: _ZN42Packet_DeathTower_Statistic_Playdata_PartyC2Ev
// DEMANGLED: Packet_DeathTower_Statistic_Playdata_Party::Packet_DeathTower_Statistic_Playdata_Party(void)
Packet_DeathTower_Statistic_Playdata_Party::
    Packet_DeathTower_Statistic_Playdata_Party()
    : PacketHeader(0x0C3Fu, 0x0010u) {}

// IDA_EA: 0x822BE14
// IDA_NAME: _ZN22Packet_Pcroom_ResponseC2Ev
// DEMANGLED: Packet_Pcroom_Response::Packet_Pcroom_Response(void)
Packet_Pcroom_Response::Packet_Pcroom_Response()
    : PacketHeader(0x0FAFu, 0x001Fu),
      dword_0a(0),
      dword_0e(0),
      byte_1e(0) {}

// IDA_EA: 0x8568DC6
// IDA_NAME: _ZN16Packet_PvPResultC2Ev
// DEMANGLED: Packet_PvPResult::Packet_PvPResult(void)
Packet_PvPResult::Packet_PvPResult()
    : PacketHeader(0x0FA5u, 0x0080u) {}

// IDA_EA: 0x846CF90
// IDA_NAME: _ZN29Packet_Response_Add_PvP_BuddyC2Ev
// DEMANGLED: Packet_Response_Add_PvP_Buddy::Packet_Response_Add_PvP_Buddy(void)
Packet_Response_Add_PvP_Buddy::Packet_Response_Add_PvP_Buddy()
    : PacketHeader(0x1B5Eu, 0x0031u),
      acc_id(0),
      charac_no(0),
      response_kind(0) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x8374CBA
// IDA_NAME: _ZN35Packet_TowerOfDespair_Statistic_GTSC2Ev
// DEMANGLED: Packet_TowerOfDespair_Statistic_GTS::Packet_TowerOfDespair_Statistic_GTS(void)
Packet_TowerOfDespair_Statistic_GTS::Packet_TowerOfDespair_Statistic_GTS()
    : PacketHeader(0x271Cu, 0x0017u),
      key(),
      byte_12(0),
      dword_13(0) {}
