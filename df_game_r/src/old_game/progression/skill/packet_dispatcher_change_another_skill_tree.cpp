#include "old_game/progression/skill/packet_dispatcher_change_another_skill_tree.h"

#include <stdint.h>

#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kChangeAnotherSkillTreeReadSignature[] =
    "virtual int Dispatcher_ChangeAnotherSkillTree::read(PacketBuf&, MSG_BASE&)";
const char kChangeAnotherSkillTreeProcessSignature[] =
    "virtual int Dispatcher_ChangeAnotherSkillTree::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// This C TU is the sole owner of the concrete state-0 template/vtable.
// IDA_EA: 0x82530AC
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8253208
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8253212
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C904
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C934
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D89A
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598926
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_CHANGE_ANOTHER_SKILL_TREE24stChangeAnotherSkillTreeL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,stChangeAnotherSkillTree,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,
                                 stChangeAnotherSkillTree,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596E00
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeC2Ev
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::Dispatcher_ChangeAnotherSkillTree(void)
Dispatcher_ChangeAnotherSkillTree::Dispatcher_ChangeAnotherSkillTree()
    : IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,
                        stChangeAnotherSkillTree,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82456D6
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeD2Ev
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::~Dispatcher_ChangeAnotherSkillTree()
// IDA_EA: 0x8245706
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeD0Ev
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::~Dispatcher_ChangeAnotherSkillTree()
Dispatcher_ChangeAnotherSkillTree::~Dispatcher_ChangeAnotherSkillTree() {}

// IDA_EA: 0x81D1F74
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeAnotherSkillTree::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    MSG_CHANGE_ANOTHER_SKILL_TREE& request =
        static_cast<MSG_CHANGE_ANOTHER_SKILL_TREE&>(message);
    char requested_tree = 0;
    if (packet.get_byte(requested_tree) == 1) {
        request.skill_tree_index = static_cast<uint8_t>(requested_tree);
        return 0;
    }
    return LineFunc(11970, kChangeAnotherSkillTreeReadSignature, 0, 0);
}

// IDA_EA: 0x81D2094
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::check_error(CUser *,MSG_BASE &)
int Dispatcher_ChangeAnotherSkillTree::check_error(CUser* user,
                                                   MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3 && user->get_state() != 6) {
        return 19;
    }

    const MSG_CHANGE_ANOTHER_SKILL_TREE& request =
        static_cast<const MSG_CHANGE_ANOTHER_SKILL_TREE&>(message);
    const int requested_tree =
        static_cast<int>(static_cast<int8_t>(request.skill_tree_index));
    if (requested_tree != user->GetCurCharacSkillTreeIndex()) {
        return -2;
    }
    if (user->GetCurCharacSkillTreeIndex() == -1) {
        return -3;
    }
    return 0;
}

// IDA_EA: 0x81D1FCA
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeAnotherSkillTree::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    stChangeAnotherSkillTree& result =
        static_cast<stChangeAnotherSkillTree&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(11983,
                        kChangeAnotherSkillTreeProcessSignature,
                        result.result,
                        0);
    }

    const MSG_CHANGE_ANOTHER_SKILL_TREE& request =
        static_cast<const MSG_CHANGE_ANOTHER_SKILL_TREE&>(message);
    if (request.skill_tree_index != 0) {
        if (request.skill_tree_index == 1) {
            user->SetCurCharacSkillTreeIndex(0);
            result.skill_tree_index = 0;
        }
    } else {
        user->SetCurCharacSkillTreeIndex(1);
        result.skill_tree_index = 1;
    }
    return 0;
}

// IDA_EA: 0x81D2134
// IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ChangeAnotherSkillTree::send(CUser *,ParamBase &)
void Dispatcher_ChangeAnotherSkillTree::send(CUser* user, ParamBase& param) {
    const stChangeAnotherSkillTree& result =
        static_cast<const stChangeAnotherSkillTree&>(param);
    if (result.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHANGE_ANOTHER_SKILL_TREE,
            static_cast<unsigned char>(result.result));
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_CHANGE_ANOTHER_SKILL_TREE);
    packet.put_byte(1);
    packet.put_byte(static_cast<int8_t>(result.skill_tree_index));
    packet.finalize(true);
    user->Send(packet);
}
