#ifndef OLDSRV_GLOBAL_FATIGUE_BATTERY_HANDLE_ABI_H_
#define OLDSRV_GLOBAL_FATIGUE_BATTERY_HANDLE_ABI_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

class CUser;

#pragma pack(push, 1)
// IDA unit: 99_misc_global/Packet_Fatigue_Battery_Money_Statistic.  The
// constructor produces exactly the 17-byte statistic frame submitted when a
// non-end-level fatigue battery is activated.
struct Packet_Fatigue_Battery_Money_Statistic {
    // IDA_EA: 0x84A84EE
    // IDA_NAME: _ZN38Packet_Fatigue_Battery_Money_StatisticC2Ev
    Packet_Fatigue_Battery_Money_Statistic();

    PacketHeader header_;                       // +0x00
    uint8_t server_group_;                      // +0x0a
    uint32_t reserved_0b_;                      // +0x0b
    uint16_t grownup_buff_count_;               // +0x0f
};
#pragma pack(pop)

static_assert(sizeof(Packet_Fatigue_Battery_Money_Statistic) == 0x11u,
              "fatigue-battery statistic packet must stay 17 bytes");

// IDA unit: 99_misc_global/CFatigueBatteryHandle.  The original object is a
// stateless one-byte service allocated by GlobalData::Init.
class CFatigueBatteryHandle {
public:
    // IDA_EA: 0x84A7FF8
    // IDA_NAME: _ZN21CFatigueBatteryHandleC2Ev
    CFatigueBatteryHandle();

    // IDA_EA: 0x84A7FFE
    // IDA_NAME: _ZN21CFatigueBatteryHandleD2Ev
    ~CFatigueBatteryHandle();

    // IDA_EA: 0x84A8004
    // IDA_NAME: _ZN21CFatigueBatteryHandle29ComputeFatigueBatteryChargingEiii
    // DEMANGLED: CFatigueBatteryHandle::ComputeFatigueBatteryCharging(int,int,int)
    short ComputeFatigueBatteryCharging(int remaining_fatigue,
                                        int charac_level,
                                        int special_level_charac_count);

    // IDA_EA: 0x84A8094
    // IDA_NAME: _ZN21CFatigueBatteryHandle22ChargingFatigueBatteryEP5CUseri
    // DEMANGLED: CFatigueBatteryHandle::ChargingFatigueBattery(CUser*,int)
    short ChargingFatigueBattery(CUser* user, int unconnected_days);

    // IDA_EA: 0x84A81F4
    // IDA_NAME: _ZN21CFatigueBatteryHandle29ActiveFatigueBattery_EndLevelEP5CUser
    // DEMANGLED: CFatigueBatteryHandle::ActiveFatigueBattery_EndLevel(CUser*)
    short ActiveFatigueBattery_EndLevel(CUser* user);

    // IDA_EA: 0x84A829E
    // IDA_NAME: _ZN21CFatigueBatteryHandle32ActiveFatigueBattery_NotEndLevelEP5CUser
    // DEMANGLED: CFatigueBatteryHandle::ActiveFatigueBattery_NotEndLevel(CUser*)
    short ActiveFatigueBattery_NotEndLevel(CUser* user);

    // IDA_EA: 0x84A83DA
    // IDA_NAME: _ZN21CFatigueBatteryHandle20ActiveFatigueBatteryEP5CUser
    // DEMANGLED: CFatigueBatteryHandle::ActiveFatigueBattery(CUser*)
    short ActiveFatigueBattery(CUser* user);
};

static_assert(sizeof(CFatigueBatteryHandle) == 1u,
              "CFatigueBatteryHandle old ABI size must stay one byte");

#endif  // OLDSRV_GLOBAL_FATIGUE_BATTERY_HANDLE_ABI_H_
