#include "old_data/Login/packet_dispatch/change_skill_dispatcher.h"

#include <stdint.h>

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kChangeSkillReadSignature[] =
    "virtual int DisPatcher_ChangeSkill::read(PacketBuf&, MSG_BASE&)";
const char kChangeSkillProcessSignature[] =
    "virtual int DisPatcher_ChangeSkill::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// CMD30 owns the one concrete template/vtable provider.  Registration remains
// deliberately untouched until the root PacketDispatcher transaction closes.
// IDA_EA: 0x825A76E
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x825A8BE
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x825A8C8
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8236354
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8236384
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825AF14
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8597FDA
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHANGE_SKILLSLOT20stOnChangeSkillParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_SKILLSLOT,stOnChangeSkillParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CHANGE_SKILLSLOT,
                                 stOnChangeSkillParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x85950E0
// IDA_NAME: _ZN22DisPatcher_ChangeSkillC2Ev
// DEMANGLED: DisPatcher_ChangeSkill::DisPatcher_ChangeSkill(void)
DisPatcher_ChangeSkill::DisPatcher_ChangeSkill()
    : IPacketDispatcher<MSG_CHANGE_SKILLSLOT,
                        stOnChangeSkillParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x823202C
// IDA_NAME: _ZN22DisPatcher_ChangeSkillD2Ev
// DEMANGLED: DisPatcher_ChangeSkill::~DisPatcher_ChangeSkill()
// IDA_EA: 0x823205C
// IDA_NAME: _ZN22DisPatcher_ChangeSkillD0Ev
// DEMANGLED: DisPatcher_ChangeSkill::~DisPatcher_ChangeSkill()
DisPatcher_ChangeSkill::~DisPatcher_ChangeSkill() {}

// IDA_EA: 0x81BFEE6
// IDA_NAME: _ZN22DisPatcher_ChangeSkill4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_ChangeSkill::read(PacketBuf &,MSG_BASE &)
int DisPatcher_ChangeSkill::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHANGE_SKILLSLOT& request =
        static_cast<MSG_CHANGE_SKILLSLOT&>(message);
    char value = 0;
    if (packet.get_byte(value) != 1) {
        return LineFunc(1780, kChangeSkillReadSignature, 0, 0);
    }
    request.skill_tree_index = static_cast<uint8_t>(value);
    if (packet.get_byte(value) != 1) {
        return LineFunc(1782, kChangeSkillReadSignature, 0, 0);
    }
    request.source_slot = static_cast<uint8_t>(value);
    if (packet.get_byte(value) != 1) {
        return LineFunc(1784, kChangeSkillReadSignature, 0, 0);
    }
    request.destination_slot = static_cast<uint8_t>(value);
    return 0;
}

// IDA_EA: 0x81BFFBC
// IDA_NAME: _ZN22DisPatcher_ChangeSkill7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_ChangeSkill::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_ChangeSkill::process(CUser* user, MSG_BASE& message,
                                    ParamBase& param) {
    MSG_CHANGE_SKILLSLOT& request =
        static_cast<MSG_CHANGE_SKILLSLOT&>(message);
    stOnChangeSkillParam& result = static_cast<stOnChangeSkillParam&>(param);

    int error = check_error(user, message);
    if (error <= 0) {
        result.field_04 = static_cast<int8_t>(request.source_slot);
        result.field_05 = static_cast<int8_t>(request.destination_slot);
        error = user->getCurCharacSkillW()->skill_move(
            request.source_slot,
            request.destination_slot,
            user->get_charac_job());
        if (error <= 0) {
            return error;
        }
    }
    user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_SKILLSLOT,
                             static_cast<uint8_t>(error));
    return -1;
}

// IDA_EA: 0x81C00AC
// IDA_NAME: _ZN22DisPatcher_ChangeSkill4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_ChangeSkill::send(CUser *,ParamBase &)
void DisPatcher_ChangeSkill::send(CUser* user, ParamBase& param) {
    const stOnChangeSkillParam& result =
        static_cast<const stOnChangeSkillParam&>(param);
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_CHANGE_SKILLSLOT);
    response.put_byte(1);
    response.put_byte(static_cast<int8_t>(user->GetCurCharacSkillTreeIndex()));
    response.put_byte(result.field_04);
    response.put_byte(result.field_05);
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x81C0196
// IDA_NAME: _ZN22DisPatcher_ChangeSkill11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_ChangeSkill::check_error(CUser *,MSG_BASE &)
int DisPatcher_ChangeSkill::check_error(CUser* user, MSG_BASE& message) {
    const MSG_CHANGE_SKILLSLOT& request =
        static_cast<const MSG_CHANGE_SKILLSLOT&>(message);
    const int skill_tree = user->GetCurCharacSkillTreeIndex();
    if (skill_tree != -1 && skill_tree != 0) {
        return request.skill_tree_index == 1u ? 0 : 19;
    }
    return request.skill_tree_index == 1u ? 19 : 0;
}

// IDA_EA: 0x8604428
// IDA_NAME: _ZN9SkillSlot10skill_moveEhhi
// DEMANGLED: SkillSlot::skill_move(unsigned char,unsigned char,int)
int SkillSlot::skill_move(unsigned char source_slot,
                          unsigned char destination_slot,
                          int charac_job) {
    if (!parent_) {
        return -1;
    }
    if (!checkMoveComboSkillSlot(source_slot, destination_slot)) {
        return 19;
    }

    const ENUM_SKILL_TREE_KIND skill_tree =
        static_cast<ENUM_SKILL_TREE_KIND>(
            parent_->GetCurCharacSkillTreeIndex());
    if (skill_tree != ENUM_SKILL_TREE_CURRENT &&
        skill_tree != ENUM_SKILL_TREE_0 &&
        skill_tree != ENUM_SKILL_TREE_1) {
        return -1;
    }
    _Mastered_skill* const slots = get_mastered_skill_tree(skill_tree);
    const uint8_t source_skill_index =
        static_cast<uint8_t>(slots[source_slot].skill_index);
    const uint8_t destination_skill_index =
        static_cast<uint8_t>(slots[destination_slot].skill_index);
    if (source_skill_index == 0u && destination_skill_index == 0u) {
        return 21;
    }

    CSkill* const source_skill =
        G_CDataManager()->find_skill(charac_job, source_skill_index);
    CSkill* const destination_skill =
        G_CDataManager()->find_skill(charac_job, destination_skill_index);
    if (!source_skill && !destination_skill) {
        return 1;
    }

    const int source_group = get_skillslot_group(source_slot);
    if (source_group == 6) {
        return 1;
    }
    const int destination_group = get_skillslot_group(destination_slot);
    if (destination_group == 6) {
        return 1;
    }
    if (source_group == 4 || destination_group == 4) {
        return 19;
    }

    bool valid_move = false;
    if (source_skill_index == 0u) {
        // The both-empty case returned above, so this is a destination-only
        // move.  The old code permits an active quick-slot skill or a skill
        // assigned to the source slot group.
        if (destination_skill) {
            valid_move = source_group == -1
                ? destination_skill->is_active_skill()
                : destination_skill->get_group() == source_group;
        }
    } else if (destination_skill_index == 0u) {
        if (source_skill) {
            valid_move = destination_group == -1
                ? source_skill->is_active_skill()
                : source_skill->get_group() == destination_group;
        }
    } else if (source_skill && destination_skill) {
        if (source_group == -1 && destination_group == -1) {
            valid_move = true;
        } else if (source_group == -1 &&
                   source_skill->get_group() == destination_group &&
                   destination_skill->is_active_skill()) {
            valid_move = true;
        } else if (destination_skill->get_group() == source_group &&
                   destination_group == -1 &&
                   source_skill->is_active_skill()) {
            valid_move = true;
        } else if (destination_skill->get_group() == source_group &&
                   source_skill->get_group() == destination_group) {
            valid_move = true;
        }
    }

    if (!valid_move) {
        return 19;
    }

    const _Mastered_skill source_entry = slots[source_slot];
    slots[source_slot] = slots[destination_slot];
    slots[destination_slot] = source_entry;
    return 0;
}
