#include "old_game/player/core/character_packets.h"

#include <cstring>


typedef char Pakcet_FindUserName_use_Charac_no_Size_Must_Be_0x34[
    (sizeof(Pakcet_FindUserName_use_Charac_no) == 0x34) ? 1 : -1];
typedef char Packet_Monitor_Request_Charac_Info_Size_Must_Be_0x23[
    (sizeof(Packet_Monitor_Request_Charac_Info) == 0x23) ? 1 : -1];
typedef char Packet_Monitor_Notice_Charac_Live_On_Ten_Min_Size_Must_Be_0x0e[
    (sizeof(Packet_Monitor_Notice_Charac_Live_On_Ten_Min) == 0x0e) ? 1 : -1];
typedef char Packet_Monitor_Charac_Delete_Size_Must_Be_0x16[
    (sizeof(Packet_Monitor_Charac_Delete) == 0x16) ? 1 : -1];
typedef char Packet_Dungeon_Statistic_Party_Charac_Size_Must_Be_0x47[
    (sizeof(Packet_Dungeon_Statistic_Party_Charac) == 0x47) ? 1 : -1];
typedef char Packet_Request_Charac_Tower_Update_Ranking_Size_Must_Be_0x1a[
    (sizeof(Packet_Request_Charac_Tower_Update_Ranking) == 0x1a) ? 1 : -1];

// IDA_EA: 0x822C2E2
// IDA_NAME: _ZN33Pakcet_FindUserName_use_Charac_noC2Ev
// DEMANGLED: Pakcet_FindUserName_use_Charac_no::Pakcet_FindUserName_use_Charac_no(void)
Pakcet_FindUserName_use_Charac_no::Pakcet_FindUserName_use_Charac_no()
    : PacketHeader(0x1F45u, 0x0034u),
      requester_uid(0),
      charac_no(0),
      charac_name_length(0) {
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x822C5D2
// IDA_NAME: _ZN34Packet_Monitor_Request_Charac_InfoC2Ev
// DEMANGLED: Packet_Monitor_Request_Charac_Info::Packet_Monitor_Request_Charac_Info(void)
Packet_Monitor_Request_Charac_Info::Packet_Monitor_Request_Charac_Info()
    : PacketHeader(0x27DEu, 0x0023u),
      requester_charac_no(0) {
    std::memset(target_charac_name, 0, sizeof(target_charac_name));
}

// IDA_EA: 0x84715E6
// IDA_NAME: _ZN44Packet_Monitor_Notice_Charac_Live_On_Ten_MinC2Ev
// DEMANGLED: Packet_Monitor_Notice_Charac_Live_On_Ten_Min::Packet_Monitor_Notice_Charac_Live_On_Ten_Min(void)
Packet_Monitor_Notice_Charac_Live_On_Ten_Min::
    Packet_Monitor_Notice_Charac_Live_On_Ten_Min()
    : PacketHeader(0x0640u, 0x000Eu) {}

// IDA_EA: 0x84E9318
// IDA_NAME: _ZN28Packet_Monitor_Charac_DeleteC2Ev
// DEMANGLED: Packet_Monitor_Charac_Delete::Packet_Monitor_Charac_Delete(void)
Packet_Monitor_Charac_Delete::Packet_Monitor_Charac_Delete()
    : PacketHeader(0x04C2u, 0x0016u) {}

// IDA_EA: 0x85BEE2E
// IDA_NAME: _ZN37Packet_Dungeon_Statistic_Party_CharacC2Ev
// DEMANGLED: Packet_Dungeon_Statistic_Party_Charac::Packet_Dungeon_Statistic_Party_Charac(void)
Packet_Dungeon_Statistic_Party_Charac::
    Packet_Dungeon_Statistic_Party_Charac()
    : PacketHeader(0x0C39u, 0x0047u) {}

// IDA_EA: 0x84716D6
// IDA_NAME: _ZN42Packet_Request_Charac_Tower_Update_RankingC2Ev
// DEMANGLED: Packet_Request_Charac_Tower_Update_Ranking::Packet_Request_Charac_Tower_Update_Ranking(void)
Packet_Request_Charac_Tower_Update_Ranking::
    Packet_Request_Charac_Tower_Update_Ranking()
    : PacketHeader(0x04CEu, 0x001Au) {}
