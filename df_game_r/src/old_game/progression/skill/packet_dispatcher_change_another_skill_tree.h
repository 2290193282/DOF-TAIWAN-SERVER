#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CHANGE_ANOTHER_SKILL_TREE_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CHANGE_ANOTHER_SKILL_TREE_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD264.  The old class is a vptr-only derivative of its one concrete,
// C-owned MSG_CHANGE_ANOTHER_SKILL_TREE state-0 template specialization.
class Dispatcher_ChangeAnotherSkillTree
    : public IPacketDispatcher<MSG_CHANGE_ANOTHER_SKILL_TREE,
                               stChangeAnotherSkillTree,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596E00
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeC2Ev
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::Dispatcher_ChangeAnotherSkillTree(void)
    Dispatcher_ChangeAnotherSkillTree();

    // IDA_EA: 0x82456D6
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeD2Ev
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::~Dispatcher_ChangeAnotherSkillTree()
    // IDA_EA: 0x8245706
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTreeD0Ev
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::~Dispatcher_ChangeAnotherSkillTree()
    virtual ~Dispatcher_ChangeAnotherSkillTree();

    // IDA_EA: 0x81D1F74
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D1FCA
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D2094
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81D2134
    // IDA_NAME: _ZN33Dispatcher_ChangeAnotherSkillTree4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_ChangeAnotherSkillTree::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeAnotherSkillTree) == 0x04u,
              "CMD264 must retain the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CHANGE_ANOTHER_SKILL_TREE_H_
