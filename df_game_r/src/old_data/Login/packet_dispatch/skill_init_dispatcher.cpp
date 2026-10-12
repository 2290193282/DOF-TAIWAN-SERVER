#include "old_data/Login/packet_dispatch/skill_init_dispatcher.h"

#include "common/manager/global_tools.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kSkillInitReadSignature[] =
    "virtual int Dispatcher_SkillInit::read(PacketBuf&, MSG_BASE&)";
const char kSkillInitProcessSignature[] =
    "virtual int Dispatcher_SkillInit::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// The old specialization is the only provider for CMD492's template/vtable.
// PacketDispatcher's 606-slot root registration intentionally remains absent.
// IDA_EA: 0x82498B0
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8249A0C
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8249A16
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B2C6
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B2F6
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D5A6
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x859833E
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_SKILL_INIT11stSkillInitL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SKILL_INIT,stSkillInit,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SKILL_INIT,
                                 stSkillInit,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595890
// IDA_NAME: _ZN20Dispatcher_SkillInitC2Ev
// DEMANGLED: Dispatcher_SkillInit::Dispatcher_SkillInit(void)
Dispatcher_SkillInit::Dispatcher_SkillInit()
    : IPacketDispatcher<MSG_SKILL_INIT,
                        stSkillInit,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243D3E
// IDA_NAME: _ZN20Dispatcher_SkillInitD2Ev
// DEMANGLED: Dispatcher_SkillInit::~Dispatcher_SkillInit()
// IDA_EA: 0x8243D6E
// IDA_NAME: _ZN20Dispatcher_SkillInitD0Ev
// DEMANGLED: Dispatcher_SkillInit::~Dispatcher_SkillInit()
Dispatcher_SkillInit::~Dispatcher_SkillInit() {}

// IDA_EA: 0x81E5934
// IDA_NAME: _ZN20Dispatcher_SkillInit4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SkillInit::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SkillInit::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SKILL_INIT& request = static_cast<MSG_SKILL_INIT&>(message);
    if (packet.get_byte(request.skill_tree_index) == 1) {
        return 0;
    }
    return LineFunc(24358, kSkillInitReadSignature, 0, 0);
}

// IDA_EA: 0x81E5A2C
// IDA_NAME: _ZN20Dispatcher_SkillInit11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SkillInit::check_error(CUser *,MSG_BASE &)
int Dispatcher_SkillInit::check_error(CUser* user, MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    MSG_SKILL_INIT& request = static_cast<MSG_SKILL_INIT&>(message);
    const int active_tree = user->GetCurCharacSkillTreeIndex();
    if (active_tree != -1 && active_tree != 0) {
        if (request.skill_tree_index != 1) {
            return 19;
        }
        request.skill_tree_index = 1;
    } else {
        if (request.skill_tree_index == 1) {
            return 19;
        }
        request.skill_tree_index = 0;
    }
    if (G_GameWorld()->IsPvPSkilTreeChannel() && user->get_pvp_grade() <= 9) {
        return 12;
    }
    return 0;
}

// IDA_EA: 0x81E598A
// IDA_NAME: _ZN20Dispatcher_SkillInit7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SkillInit::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SkillInit::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    stSkillInit& result = static_cast<stSkillInit&>(param);
    result.dword_04 = check_error(user, message);
    if (result.dword_04 > 0) {
        return 0;
    }
    if (result.dword_04 < 0) {
        return LineFunc(24381, kSkillInitProcessSignature, result.dword_04, 0);
    }
    const MSG_SKILL_INIT& request = static_cast<const MSG_SKILL_INIT&>(message);
    result.byte_08 = request.skill_tree_index;
    process_skill_init(user, param);
    return 0;
}

// IDA_EA: 0x81E5BDC
// IDA_NAME: _ZN20Dispatcher_SkillInit18process_skill_initEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SkillInit::process_skill_init(CUser *,ParamBase &)
void Dispatcher_SkillInit::process_skill_init(CUser* user, ParamBase& param) {
    if (!user) {
        return;
    }
    const stSkillInit& result = static_cast<const stSkillInit&>(param);
    const ENUM_SKILL_TREE_KIND skill_tree =
        static_cast<ENUM_SKILL_TREE_KIND>(static_cast<int8_t>(result.byte_08));
    SkillSlot* const skill_slot = user->getCurCharacSkillW();
    skill_slot->clear_all_skills(skill_tree);
    skill_slot->set_parent(user);
    skill_slot->InitSkill(0, skill_tree);
    skill_slot->resetComboSkill(skill_tree);
    user->setComboSkillSet(true);
}

// IDA_EA: 0x81E5AEC
// IDA_NAME: _ZN20Dispatcher_SkillInit4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SkillInit::send(CUser *,ParamBase &)
void Dispatcher_SkillInit::send(CUser* user, ParamBase& param) {
    const stSkillInit& result = static_cast<const stSkillInit&>(param);
    if (result.dword_04) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SKILL_INIT,
                                 static_cast<unsigned char>(result.dword_04));
        return;
    }
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SKILL_INIT);
    packet.put_byte(1);
    packet.put_byte(static_cast<int8_t>(result.byte_08));
    packet.finalize(true);
    user->Send(packet);
    user->send_skill_info();
}
