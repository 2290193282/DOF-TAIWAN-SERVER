#ifndef OLDSRV_11_EVENT_RUNTIME_EVENT_PACKETS_H_
#define OLDSRV_11_EVENT_RUNTIME_EVENT_PACKETS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x84B4184 in the companion source.


#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 11_event_runtime/Packet_Monitor_Event_Start.
struct Packet_Monitor_Event_Start : public PacketHeader {
    Packet_Monitor_Event_Start();

    uint8_t raw_0a[0x08];
};

// IDA unit: 11_event_runtime/Packet_Monitor_Event_End.
struct Packet_Monitor_Event_End : public PacketHeader {
    Packet_Monitor_Event_End();

    uint8_t raw_0a[0x04];
};

// IDA unit: 11_event_runtime/Packet_Guild_Attendance_Info.
struct Packet_Guild_Attendance_Info : public PacketHeader {
    Packet_Guild_Attendance_Info();

    uint32_t dword_0a;
    uint32_t dword_0e;
    uint8_t byte_12;
};

// IDA unit: 11_event_runtime/Packet_Event_UpgradeJar_Output_Statistic_GTS.
struct Packet_Event_UpgradeJar_Output_Statistic_GTS : public PacketHeader {
    Packet_Event_UpgradeJar_Output_Statistic_GTS();

    uint32_t dword_0a;
    uint32_t dword_0e;
};

// IDA unit: 11_event_runtime/GoldCardEventStatistic.
struct GoldCardEventStatistic {
    GoldCardEventStatistic();

    uint8_t byte_00;
    uint32_t dword_01;
    uint32_t dword_05;
};

// IDA unit: 11_event_runtime/Packet_Goldcard_Event_Statistic_GTS.
struct Packet_Goldcard_Event_Statistic_GTS : public PacketHeader {
    Packet_Goldcard_Event_Statistic_GTS();

    GoldCardEventStatistic statistic_0a;
};

// IDA unit: 99_misc_global/Packet_Avater_Disjoint_Statistic.
// The original spelling "Avater" is part of the exported packet type.
struct Packet_Avater_Disjoint_Statistic : public PacketHeader {
    Packet_Avater_Disjoint_Statistic();

    int32_t action_0a;
    int32_t first_grade_bucket_0e;
    int32_t first_item_type_12;
    uint8_t first_succeeded_16;
    uint8_t reserved_17[0x04];
    int32_t second_grade_bucket_1b;
    int32_t second_item_type_1f;
    uint8_t second_succeeded_23;
    uint8_t reserved_24[0x04];
};

// IDA unit: 99_misc_global/Packet_Emblem_Create_Statistic.
struct Packet_Emblem_Create_Statistic : public PacketHeader {
    Packet_Emblem_Create_Statistic();

    int32_t emblem_count_0a;
    int32_t emblem_grades_0e[5];
    int32_t emblem_counts_22[5];
};

#pragma pack(pop)

#if defined(__i386__)
static_assert(sizeof(Packet_Avater_Disjoint_Statistic) == 0x28u,
              "Packet_Avater_Disjoint_Statistic IDA ABI size must stay 0x28");
static_assert(sizeof(Packet_Emblem_Create_Statistic) == 0x36u,
              "Packet_Emblem_Create_Statistic IDA ABI size must stay 0x36");
#endif


#endif  // OLDSRV_11_EVENT_RUNTIME_EVENT_PACKETS_H_
