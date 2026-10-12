#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_USE_LIMIT_CUBE_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_USE_LIMIT_CUBE_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CItem;
class CUser;
class PacketBuf;
struct Inven_Item;

// IDA unit: 99_misc_global/MSG_USE_LIMIT_CUBE.  The old reader consumes the
// three fields at +0x0d, +0x0f and +0x13 after the 13-byte C2S header.
#pragma pack(push, 1)
struct MSG_USE_LIMIT_CUBE : public MSG_BASE {
    // IDA_EA: 0x822DD4E
    // IDA_NAME: _ZN18MSG_USE_LIMIT_CUBEC2Ev
    // DEMANGLED: MSG_USE_LIMIT_CUBE::MSG_USE_LIMIT_CUBE(void)
    MSG_USE_LIMIT_CUBE();

    int16_t material_slot;                // +0x0d
    uint32_t expected_material_item_id;   // +0x0f
    int16_t limit_cube_slot;              // +0x13
};
#pragma pack(pop)

static_assert(sizeof(MSG_USE_LIMIT_CUBE) == 0x15u,
              "CMD339 request must retain its 21-byte i386 ABI");
static_assert(offsetof(MSG_USE_LIMIT_CUBE, material_slot) == 0x0du,
              "CMD339 material slot must stay at +0x0d");
static_assert(offsetof(MSG_USE_LIMIT_CUBE, limit_cube_slot) == 0x13u,
              "CMD339 cube slot must stay at +0x13");

// IDA unit: 99_misc_global/stUseLimitCube.  The final u16 is initialized by
// the old C2 but never read by this handler; retain it for the exact object
// extent instead of shrinking the virtual parameter object.
struct stUseLimitCube : public ParamBase {
    // IDA_EA: 0x8231598
    // IDA_NAME: _ZN14stUseLimitCubeC2Ev
    // DEMANGLED: stUseLimitCube::stUseLimitCube(void)
    stUseLimitCube();

    // IDA_EA: 0x824703C / 0x824706C
    // IDA_NAME: _ZN14stUseLimitCubeD2Ev / _ZN14stUseLimitCubeD0Ev
    virtual ~stUseLimitCube();

    uint8_t error_code;        // +0x04
    uint8_t reserved_05_07[3];
    uint32_t result_item_id;   // +0x08
    uint32_t result_count;     // +0x0c (only low u16 is serialized)
    uint16_t reserved_10;      // +0x10
};

#if defined(__i386__)
static_assert(sizeof(stUseLimitCube) == 0x14u,
              "CMD339 result parameter must retain its i386 ABI");
static_assert(offsetof(stUseLimitCube, error_code) == 0x04u,
              "CMD339 result code must stay at +0x04");
static_assert(offsetof(stUseLimitCube, result_item_id) == 0x08u,
              "CMD339 result item must stay at +0x08");
static_assert(offsetof(stUseLimitCube, result_count) == 0x0cu,
              "CMD339 result count must stay at +0x0c");
#endif

// PacketDispatcher slot 339.  This is deliberately a feature-local provider:
// no part of the business path belongs in packet_dispatcher_sr08_remaining_derived.cpp.
class Dispatcher_UseLimitCube
    : public IPacketDispatcher<MSG_USE_LIMIT_CUBE,
                               stUseLimitCube,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597426
    // IDA_NAME: _ZN23Dispatcher_UseLimitCubeC2Ev
    // DEMANGLED: Dispatcher_UseLimitCube::Dispatcher_UseLimitCube(void)
    Dispatcher_UseLimitCube();

    // IDA_EA: 0x824532E / 0x824535E
    // IDA_NAME: _ZN23Dispatcher_UseLimitCubeD2Ev / _ZN23Dispatcher_UseLimitCubeD0Ev
    virtual ~Dispatcher_UseLimitCube();

    // IDA_EA: 0x81D3BBC
    // IDA_NAME: _ZN23Dispatcher_UseLimitCube11check_errorEijiR10Inven_ItemS1_PK5CItem
    // DEMANGLED: Dispatcher_UseLimitCube::check_error(int,unsigned int,int,Inven_Item &,Inven_Item &,CItem const*)
    int check_error(int material_slot,
                    unsigned int expected_material_item_id,
                    int limit_cube_slot,
                    Inven_Item& material,
                    Inven_Item& limit_cube,
                    const CItem* limit_cube_prototype);

    // IDA_EA: 0x81D3C62 / 0x81D3D38 / 0x81D4784
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseLimitCube) == 0x04u,
              "CMD339 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_USE_LIMIT_CUBE_H_
