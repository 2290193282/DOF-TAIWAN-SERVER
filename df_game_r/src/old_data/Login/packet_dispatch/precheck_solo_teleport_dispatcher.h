#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PRECHECK_SOLO_TELEPORT_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PRECHECK_SOLO_TELEPORT_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
class NPC_Teleport;
class Teleport;
class User_Teleport;

// IDA unit: 99_misc_global/stPartyTelePort.  This is the plain twelve-byte
// destination record used by CMD470/471; the concrete-destination C2S wire
// supplies village/area/position/direction while teleport_kind stays at its
// local reset value.  It is unrelated to the polymorphic stPartyTeleport
// result parameter used by party teleport.
struct stPartyTelePort {
    uint8_t teleport_kind;       // +0x00
    uint8_t teleport_village;    // +0x01
    uint8_t teleport_area_index; // +0x02
    uint8_t reserved_03;         // +0x03
    uint16_t teleport_posx;      // +0x04
    uint16_t teleport_posy;      // +0x06
    uint8_t teleport_direction;  // +0x08
    uint8_t reserved_09[3];      // +0x09

    // IDA_EA: 0x822D764
    // IDA_NAME: _ZN15stPartyTelePort5resetEv
    // DEMANGLED: stPartyTelePort::reset(void)
    void reset();
};

// IDA unit: 99_misc_global/MSG_SOLO_TELEPORT.
struct MSG_SOLO_TELEPORT : public MSG_BASE {
    // IDA_EA: 0x822E10C
    // IDA_NAME: _ZN17MSG_SOLO_TELEPORTC2Ev
    // DEMANGLED: MSG_SOLO_TELEPORT::MSG_SOLO_TELEPORT(void)
    MSG_SOLO_TELEPORT();

    char target_charac_name[0x1e];  // +0x0d
    uint32_t target_name_length;    // +0x2c
    int32_t target_charac_no;       // +0x30
    int32_t teleport_data_index;    // +0x34, C2 = -1
    stPartyTelePort party_teleport; // +0x38
    Teleport* resolved_teleport;    // +0x44
};

// IDA unit: 99_misc_global/stSoloTeleportParam.
struct stSoloTeleportParam : public ParamBase {
    // IDA_EA: 0x8231B56
    // IDA_NAME: _ZN19stSoloTeleportParamC2Ev
    // DEMANGLED: stSoloTeleportParam::stSoloTeleportParam(void)
    stSoloTeleportParam();

    // IDA_EA: 0x82468B4 / 0x82468E4
    virtual ~stSoloTeleportParam();

    int result_code;              // +0x04
    int target_charac_no;         // +0x08
    Teleport* resolved_teleport;  // +0x0c
};

#if defined(__i386__)
static_assert(sizeof(stPartyTelePort) == 0x0cu,
              "solo-teleport destination must retain the old 12-byte ABI");
static_assert(sizeof(MSG_SOLO_TELEPORT) == 0x48u,
              "CMD470/471 request must retain the old 0x48-byte ABI");
static_assert(sizeof(stSoloTeleportParam) == 0x10u,
              "solo-teleport result must retain the old 0x10-byte ABI");
#endif

// IDA unit: 99_misc_global/STCheckRuleData.  Its original C2 is an empty
// five-byte body; CMD471 writes both words before virtual dispatch.
struct STCheckRuleData {
    STCheckRuleData();

    int32_t target_charac_no;
    CUser* requesting_user;
};

// IDA unit: 99_misc_global/STTeleportData.  The retail C2 clears only the
// target word; CMD471 fills the complete embedded destination immediately
// afterwards.
struct STTeleportData {
    STTeleportData();

    int32_t target_charac_no;
    stPartyTelePort party_teleport;
};

#if defined(__i386__)
static_assert(sizeof(STCheckRuleData) == 0x08u,
              "solo-teleport rule data must retain the old 8-byte ABI");
static_assert(sizeof(STTeleportData) == 0x10u,
              "solo-teleport transfer data must retain the old 16-byte ABI");
#endif

// IDA unit: 99_misc_global/Teleport.  CMD470 calls the static CUser rule;
// CMD471 owns the polymorphic destination rule/data path below.
class Teleport {
public:
    // IDA_EA: 0x8283C5A
    // IDA_NAME: _ZN8Teleport10check_ruleEP5CUser
    // DEMANGLED: Teleport::check_rule(CUser *)
    static int check_rule(CUser* user);

    // IDA_EA: 0x8283C30 / 0x82843B0 / 0x82843D6
    // IDA_NAME: _ZN8TeleportC2Ev / _ZN8TeleportD2Ev / _ZN8TeleportD0Ev
    Teleport();
    virtual ~Teleport();

    // IDA_EA: 0x8283D82 / 0x8283D98
    // The virtual order is significant: vtable slots +8/+12 are used by
    // Dispatcher_SoloTeleport::process/check_error.
    virtual int check_rule(const STCheckRuleData& rule);
    virtual Teleport* set_teleport_data(const STTeleportData& data) = 0;

    Teleport* set_party_teleport(const stPartyTelePort& destination);
    stPartyTelePort* get_party_teleport();
    void set_end_cool_time(CUser* user);
    int get_end_cool_time() const;
    void update_next_end_cool_time(CUser* user);
    void send_teleport_effect(CUser* user, int phase);
    int move_area(CUser* user);

protected:
    stPartyTelePort party_teleport_;
    int end_cool_time_;
};

class NPC_Teleport : public Teleport {
public:
    NPC_Teleport();
    virtual ~NPC_Teleport();

    virtual int check_rule(const STCheckRuleData& rule);
    virtual Teleport* set_teleport_data(const STTeleportData& data);
};

class User_Teleport : public Teleport {
public:
    User_Teleport();
    virtual ~User_Teleport();

    virtual int check_rule(const STCheckRuleData& rule);
    virtual Teleport* set_teleport_data(const STTeleportData& data);
};

// IDA_DATA: unnamed BSS dword at 0x941F81C.  The original GlobalData::Init
// creates this User_Teleport immediately after GlobalData::g_pTeleport.
extern User_Teleport* g_user_solo_teleport __asm__("dword_941F81C");

// CMD470.  This class is intentionally only a concrete provider: its slot is
// not registered until the complete 606-slot PacketDispatcher transaction is
// restored.
class Dispatcher_PrecheckSoloTeleport
    : public IPacketDispatcher<MSG_SOLO_TELEPORT,
                               stSoloTeleportParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597B7A
    // IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleportC2Ev
    Dispatcher_PrecheckSoloTeleport();

    // IDA_EA: 0x824421E / 0x824424E
    virtual ~Dispatcher_PrecheckSoloTeleport();

    // IDA_EA: 0x81E1920 / 0x81E19CA / 0x81E1ACC
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E1A9E
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PrecheckSoloTeleport) == 0x04u,
              "CMD470 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PRECHECK_SOLO_TELEPORT_DISPATCHER_H_
