#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_SKILL_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_SKILL_DISPATCHER_H_

#include <stdint.h>

#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CSkill;
class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/CHackLog_InvalidUsingSkill.
// The old symbols are static validators; Hex-Rays displayed their first
// explicit parameter as a synthetic this pointer.
class CHackLog_InvalidUsingSkill {
public:
    // IDA_EA: 0x82864FC
    // IDA_NAME: _ZN26CHackLog_InvalidUsingSkill17IsValidUsingSkillEPK25SKILL_USING_VALIDATE_INFO
    static bool IsValidUsingSkill(const SKILL_USING_VALIDATE_INFO* info);

    // IDA_EA: 0x82865CA
    // IDA_NAME: _ZN26CHackLog_InvalidUsingSkill25IsSkillMatchFirstGrowTypeEPK6CSkillc
    static bool IsSkillMatchFirstGrowType(const CSkill* skill,
                                          char first_grow_type);

    // IDA_EA: 0x8286612
    // IDA_NAME: _ZN26CHackLog_InvalidUsingSkill26IsSkillMatchSecondGrowTypeEPK6CSkillcc
    static bool IsSkillMatchSecondGrowType(const CSkill* skill,
                                           char first_grow_type,
                                           char second_grow_type);

    // IDA_EA: 0x8286656
    // IDA_NAME: _ZN26CHackLog_InvalidUsingSkill11GetHackTypeEv
    static int GetHackType();
};

// IDA's reader consumes this exact packed 19-byte payload after the base
// header.  The old type carried no recovered constructor identity.
#pragma pack(push, 1)
struct MSG_USE_SKILL : public MSG_BASE {
    uint8_t request_kind;      // +0x0d, read but not otherwise consumed
    uint8_t skill_slot;        // +0x0e
    uint16_t skill_index;      // +0x0f
    uint16_t request_value;    // +0x11, read for ABI parity
};
#pragma pack(pop)

class DisPatcher_UseSkill
    : public IPacketDispatcher<MSG_USE_SKILL, stUseSkillParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595310
    // IDA_NAME: _ZN19DisPatcher_UseSkillC2Ev
    DisPatcher_UseSkill();

    // IDA_EA: 0x82327B8 / 0x82327E8
    // IDA_NAME: _ZN19DisPatcher_UseSkillD2Ev /
    //           _ZN19DisPatcher_UseSkillD0Ev
    virtual ~DisPatcher_UseSkill();

    // IDA_EA: 0x81CAC5E
    // IDA_NAME: _ZN19DisPatcher_UseSkill4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CADAA
    // IDA_NAME: _ZN19DisPatcher_UseSkill7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CAF28
    // IDA_NAME: _ZN19DisPatcher_UseSkill4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CAD80
    // IDA_NAME: _ZN19DisPatcher_UseSkill11check_errorEP5CUserR8MSG_BASER9ParamBase
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(MSG_USE_SKILL) == 0x13u,
              "CMD41 request must retain the old 19-byte ABI");
static_assert(sizeof(DisPatcher_UseSkill) == 0x04u,
              "CMD41 handler must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_SKILL_DISPATCHER_H_
