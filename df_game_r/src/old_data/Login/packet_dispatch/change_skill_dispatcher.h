#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CHANGE_SKILL_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CHANGE_SKILL_DISPATCHER_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD30.  The original handler is a vptr-only derivative of the state-3
// MSG_CHANGE_SKILLSLOT template.  Its slot mutation is owned by SkillSlot;
// this class only performs the old packet and character-tree gates.
class DisPatcher_ChangeSkill
    : public IPacketDispatcher<MSG_CHANGE_SKILLSLOT,
                               stOnChangeSkillParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85950E0
    // IDA_NAME: _ZN22DisPatcher_ChangeSkillC2Ev
    // DEMANGLED: DisPatcher_ChangeSkill::DisPatcher_ChangeSkill(void)
    DisPatcher_ChangeSkill();

    // IDA_EA: 0x823202C / 0x823205C
    // IDA_NAME: _ZN22DisPatcher_ChangeSkillD2Ev /
    //           _ZN22DisPatcher_ChangeSkillD0Ev
    // DEMANGLED: DisPatcher_ChangeSkill::~DisPatcher_ChangeSkill()
    virtual ~DisPatcher_ChangeSkill();

    // IDA_EA: 0x81BFEE6
    // IDA_NAME: _ZN22DisPatcher_ChangeSkill4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_ChangeSkill::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81BFFBC
    // IDA_NAME: _ZN22DisPatcher_ChangeSkill7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_ChangeSkill::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C00AC
    // IDA_NAME: _ZN22DisPatcher_ChangeSkill4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_ChangeSkill::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81C0196
    // IDA_NAME: _ZN22DisPatcher_ChangeSkill11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_ChangeSkill::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ChangeSkill) == 0x04u,
              "CMD30 dispatcher must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CHANGE_SKILL_DISPATCHER_H_
