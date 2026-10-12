#include "old_game/activity/schedule/event_packets.h"


typedef char Packet_Monitor_Event_Start_Size_Must_Be_0x12[
    (sizeof(Packet_Monitor_Event_Start) == 0x12) ? 1 : -1];
typedef char Packet_Monitor_Event_End_Size_Must_Be_0x0e[
    (sizeof(Packet_Monitor_Event_End) == 0x0e) ? 1 : -1];
typedef char Packet_Guild_Attendance_Info_Size_Must_Be_0x13[
    (sizeof(Packet_Guild_Attendance_Info) == 0x13) ? 1 : -1];
typedef char Packet_Event_UpgradeJar_Output_Statistic_GTS_Size_Must_Be_0x12[
    (sizeof(Packet_Event_UpgradeJar_Output_Statistic_GTS) == 0x12) ? 1 : -1];
typedef char GoldCardEventStatistic_Size_Must_Be_0x09[
    (sizeof(GoldCardEventStatistic) == 0x09) ? 1 : -1];
typedef char Packet_Goldcard_Event_Statistic_GTS_Size_Must_Be_0x13[
    (sizeof(Packet_Goldcard_Event_Statistic_GTS) == 0x13) ? 1 : -1];
typedef char Packet_Avater_Disjoint_Statistic_Size_Must_Be_0x28[
    (sizeof(Packet_Avater_Disjoint_Statistic) == 0x28) ? 1 : -1];
typedef char Packet_Emblem_Create_Statistic_Size_Must_Be_0x36[
    (sizeof(Packet_Emblem_Create_Statistic) == 0x36) ? 1 : -1];

// IDA_EA: 0x84B4184
// IDA_NAME: _ZN26Packet_Monitor_Event_StartC2Ev
// DEMANGLED: Packet_Monitor_Event_Start::Packet_Monitor_Event_Start(void)
Packet_Monitor_Event_Start::Packet_Monitor_Event_Start()
    : PacketHeader(0x044Fu, 0x0012u) {}

// IDA_EA: 0x826889A
// IDA_NAME: _ZN24Packet_Monitor_Event_EndC2Ev
// DEMANGLED: Packet_Monitor_Event_End::Packet_Monitor_Event_End(void)
Packet_Monitor_Event_End::Packet_Monitor_Event_End()
    : PacketHeader(0x0450u, 0x000Eu) {}

// IDA_EA: 0x822C384
// IDA_NAME: _ZN28Packet_Guild_Attendance_InfoC2Ev
// DEMANGLED: Packet_Guild_Attendance_Info::Packet_Guild_Attendance_Info(void)
Packet_Guild_Attendance_Info::Packet_Guild_Attendance_Info()
    : PacketHeader(0x1BC6u, 0x0013u),
      dword_0a(0),
      dword_0e(0),
      byte_12(0) {}

// IDA_EA: 0x8694528
// IDA_NAME: _ZN44Packet_Event_UpgradeJar_Output_Statistic_GTSC2Ev
// DEMANGLED: Packet_Event_UpgradeJar_Output_Statistic_GTS::Packet_Event_UpgradeJar_Output_Statistic_GTS(void)
Packet_Event_UpgradeJar_Output_Statistic_GTS::
    Packet_Event_UpgradeJar_Output_Statistic_GTS()
    : PacketHeader(0x106Eu, 0x0012u),
      dword_0a(0),
      dword_0e(0) {}

// IDA_EA: 0x85BEE76
// IDA_NAME: _ZN22GoldCardEventStatisticC2Ev
// DEMANGLED: GoldCardEventStatistic::GoldCardEventStatistic(void)
GoldCardEventStatistic::GoldCardEventStatistic()
    : byte_00(0),
      dword_01(0),
      dword_05(0) {}

// IDA_EA: 0x85BEE96
// IDA_NAME: _ZN35Packet_Goldcard_Event_Statistic_GTSC2Ev
// DEMANGLED: Packet_Goldcard_Event_Statistic_GTS::Packet_Goldcard_Event_Statistic_GTS(void)
Packet_Goldcard_Event_Statistic_GTS::Packet_Goldcard_Event_Statistic_GTS()
    : PacketHeader(0x1F46u, 0x0013u),
      statistic_0a() {}

// IDA_EA: 0x8110BAE
// IDA_NAME: _ZN32Packet_Avater_Disjoint_StatisticC2Ev
// DEMANGLED: Packet_Avater_Disjoint_Statistic::Packet_Avater_Disjoint_Statistic(void)
Packet_Avater_Disjoint_Statistic::Packet_Avater_Disjoint_Statistic()
    : PacketHeader(0x17A2u, 0x0028u),
      action_0a(0),
      first_grade_bucket_0e(0),
      first_item_type_12(0),
      first_succeeded_16(0),
      reserved_17(),
      second_grade_bucket_1b(0),
      second_item_type_1f(0),
      second_succeeded_23(0),
      reserved_24() {}

// IDA_EA: 0x8110BDC
// IDA_NAME: _ZN29Packet_Emblem_Create_StatisticC2Ev
// DEMANGLED: Packet_Emblem_Create_Statistic::Packet_Emblem_Create_Statistic(void)
Packet_Emblem_Create_Statistic::Packet_Emblem_Create_Statistic()
    : PacketHeader(0x17A4u, 0x0036u),
      emblem_count_0a(0),
      emblem_grades_0e(),
      emblem_counts_22() {}
