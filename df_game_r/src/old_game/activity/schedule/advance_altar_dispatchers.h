#ifndef OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DISPATCHERS_H_
#define OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DISPATCHERS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8141D2E in the companion source.


#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

namespace CMDPacketStruct {

// IDA unit: 04_character_select_create/CMDPacketStruct/_SetSlotData.
struct _SetSlotData {
    uint32_t dword_00;
    uint8_t byte_04;
    uint8_t raw_05[3];
    uint32_t dword_08;
};

// STBaseRequest is intentionally storage-free in the active packet models:
// their original C2 clears the entire wire image, including MSG_BASE.  The
// nominal inheritance preserves the member signatures without moving packet
// bytes beyond their documented offsets.
struct STBaseRequest {};

// IDA_EA: 0x812c0be / 0x812c028 / 0x812c058.
// Responses, unlike requests, are a real ParamBase polymorphic family.  This
// four-byte base is what gives every response its original D2/D0 vtable pair.
struct STBaseResponse : public ParamBase {
    STBaseResponse();
    virtual ~STBaseResponse();
};

#if defined(__i386__)
static_assert(sizeof(STBaseResponse) == 0x04u,
              "CMDPacketStruct::STBaseResponse must retain its i386 ABI");
#endif

// IDA unit:
// 04_character_select_create/CMDPacketStruct/_ReqAdvanceAltarSetSlot.
struct _ReqAdvanceAltarSetSlot : public STBaseRequest {
    _ReqAdvanceAltarSetSlot();

    uint8_t raw_00[16];
    uint32_t context_id_10;
    uint8_t byte_14;
    uint8_t raw_15[3];
    _SetSlotData slot_18;
};

static_assert(sizeof(_ReqAdvanceAltarSetSlot) == 0x24u &&
                  offsetof(_ReqAdvanceAltarSetSlot, context_id_10) == 0x10u &&
                  offsetof(_ReqAdvanceAltarSetSlot, byte_14) == 0x14u &&
                  offsetof(_ReqAdvanceAltarSetSlot, slot_18) == 0x18u,
              "advance-altar CMD643 request ABI changed");

// IDA unit:
// 04_character_select_create/CMDPacketStruct/_ResAdvanceAltarSetSlot.
struct _ResAdvanceAltarSetSlot : public STBaseResponse {
    _ResAdvanceAltarSetSlot();
    ~_ResAdvanceAltarSetSlot();

    uint32_t result_04;
    uint32_t context_id_08;
    uint8_t byte_0c;
    uint8_t raw_0d[3];
    _SetSlotData slot_10;
};

// IDA unit:
// 04_character_select_create/CMDPacketStruct/_ReqAdvanceAltarExchangeSlot.
struct _ReqAdvanceAltarExchangeSlot : public STBaseRequest {
    _ReqAdvanceAltarExchangeSlot();

    uint8_t raw_00[16];
    uint32_t context_id_10;
    uint8_t raw_14[4];
    _SetSlotData slot_18;
    _SetSlotData slot_24;
};

static_assert(sizeof(_ReqAdvanceAltarExchangeSlot) == 0x30u &&
                  offsetof(_ReqAdvanceAltarExchangeSlot, context_id_10) ==
                      0x10u &&
                  offsetof(_ReqAdvanceAltarExchangeSlot, slot_18) == 0x18u &&
                  offsetof(_ReqAdvanceAltarExchangeSlot, slot_24) == 0x24u,
              "advance-altar CMD646 request ABI changed");

// IDA unit:
// 04_character_select_create/CMDPacketStruct/_ResAdvanceAltarExchangeSlot.
struct _ResAdvanceAltarExchangeSlot : public STBaseResponse {
    _ResAdvanceAltarExchangeSlot();
    ~_ResAdvanceAltarExchangeSlot();

    uint32_t result_04;
    uint32_t context_id_08;
    _SetSlotData slot_0c;
    _SetSlotData slot_18;
};

// IDA_EA: 0x8141dfa / 0x8141e28 / 0x81425b8.
// Constructors clear the full wire image after their base C2 calls.  The
// first dword of the response is therefore a zeroed protocol field, not a
// C++ object pointer exposed to the client packet.
struct _ReqAdvanceAltarUpgradeGage : public STBaseRequest {
    _ReqAdvanceAltarUpgradeGage();

    uint8_t raw_00[0x18];
};

struct _ResAdvanceAltarUpgradeGage : public STBaseResponse {
    _ResAdvanceAltarUpgradeGage();
    ~_ResAdvanceAltarUpgradeGage();

    uint32_t result_04;
    uint32_t before_level_08;
    uint32_t after_level_0c;
    uint32_t gage_10;
};

static_assert(sizeof(_ReqAdvanceAltarUpgradeGage) == 0x18u,
              "advance-altar upgrade-gage request must retain its 24-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarUpgradeGage) == 0x14u,
              "advance-altar upgrade-gage response must retain its 20-byte wire extent");

// IDA_EA: 0x8141e60 / 0x8141e8e / 0x8142606.
struct _ReqAdvanceAltarSummonUnit : public STBaseRequest {
    _ReqAdvanceAltarSummonUnit();

    uint8_t raw_00[0x0d];
    uint8_t slot_0d;
    uint8_t raw_0e[0x02];
    uint32_t item_id_10;
    uint32_t expected_gage_level_14;
};

struct _ResAdvanceAltarSummonUnit : public STBaseResponse {
    _ResAdvanceAltarSummonUnit();
    ~_ResAdvanceAltarSummonUnit();

    uint32_t result_04;
    uint8_t slot_08;
    uint8_t raw_09[0x03];
    uint32_t item_id_0c;
    uint32_t gage_10;
};

static_assert(sizeof(_ReqAdvanceAltarSummonUnit) == 0x18u,
              "advance-altar summon-unit request must retain its 24-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarSummonUnit) == 0x14u,
              "advance-altar summon-unit response must retain its 20-byte wire extent");

// IDA_EA: 0x8141f2c / 0x8141f5a / 0x81426f0.
struct _ReqAdvanceAltarGetAchievementReward : public STBaseRequest {
    _ReqAdvanceAltarGetAchievementReward();

    uint8_t raw_00[0x14];
};

struct _ResAdvanceAltarGetAchievementReward : public STBaseResponse {
    _ResAdvanceAltarGetAchievementReward();
    ~_ResAdvanceAltarGetAchievementReward();

    uint32_t result_04;
    uint32_t achievement_id_08;
};

static_assert(sizeof(_ReqAdvanceAltarGetAchievementReward) == 0x14u,
              "advance-altar achievement request must retain its 20-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarGetAchievementReward) == 0x0cu,
              "advance-altar achievement response must retain its 12-byte wire extent");

// IDA_EA: 0x8141FFC / 0x814202A / 0x814205C.
struct _ReqAdvanceAltarResetStar : public STBaseRequest {
    _ReqAdvanceAltarResetStar();

    uint8_t raw_00[0x10];
    uint32_t reset_star_type_10;
};

struct _ResAdvanceAltarResetStar : public STBaseResponse {
    _ResAdvanceAltarResetStar();
    ~_ResAdvanceAltarResetStar();

    uint32_t result_04;
    uint8_t raw_08[0x04];
};

static_assert(sizeof(_ReqAdvanceAltarResetStar) == 0x14u,
              "advance-altar reset-star request must retain its 20-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarResetStar) == 0x0cu,
              "advance-altar reset-star response must retain its 12-byte wire extent");

}  // namespace CMDPacketStruct

namespace NOTIPacketStruct {

// IDA_EA: 0x8141FF8.  A successful reset-star operation prefixes the ridable
// update with this one-byte notification object before serialising the
// ordinary AdvanceAltarData body.
struct _NotiAdvanceAltarUpdateRidable {
    _NotiAdvanceAltarUpdateRidable();

    uint8_t value_00;
};

static_assert(sizeof(_NotiAdvanceAltarUpdateRidable) == 0x01u,
              "ridable notice must retain its one-byte ABI");

}  // namespace NOTIPacketStruct

namespace advancealtar {

// IDA unit: 04_character_select_create/advancealtar/Dispatcher_SetSlot.
class Dispatcher_SetSlot
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SetSlot();
    virtual ~Dispatcher_SetSlot();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SetSlot) == 0x04u,
              "advance-altar set-slot dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/advancealtar/Dispatcher_ExchangeSlot.
class Dispatcher_ExchangeSlot
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ExchangeSlot();
    virtual ~Dispatcher_ExchangeSlot();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ExchangeSlot) == 0x04u,
              "advance-altar exchange-slot dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_UpgradeGage.
// Its public dispatcher ABI is the ordinary state-0 PacketDispatcher base,
// but dispatch_sig owns the wire-level request/response transaction.
class Dispatcher_UpgradeGage
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_UpgradeGage();
    virtual ~Dispatcher_UpgradeGage();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpgradeGage) == 0x04u,
              "advance-altar upgrade-gage dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_SummonUnit.
class Dispatcher_SummonUnit
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SummonUnit();
    virtual ~Dispatcher_SummonUnit();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SummonUnit) == 0x04u,
              "advance-altar summon-unit dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 11_event_runtime/advancealtar/Dispatcher_GetAchievementReward.
class Dispatcher_GetAchievementReward
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_GetAchievementReward();
    virtual ~Dispatcher_GetAchievementReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GetAchievementReward) == 0x04u,
              "advance-altar achievement dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_ResetStar.
class Dispatcher_ResetStar
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ResetStar();
    virtual ~Dispatcher_ResetStar();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user,
                     CMDPacketStruct::STBaseResponse& response);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ResetStar) == 0x04u,
              "advance-altar reset-star dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace advancealtar

#endif  // OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DISPATCHERS_H_
