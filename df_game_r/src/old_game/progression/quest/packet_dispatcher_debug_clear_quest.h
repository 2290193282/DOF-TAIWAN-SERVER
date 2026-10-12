#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DEBUG_CLEAR_QUEST_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DEBUG_CLEAR_QUEST_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/ReqDebugClearQuestResult.
struct ReqDebugClearQuestResult : public ParamBase {
    // IDA_EA: 0x8231A72
    // IDA_NAME: _ZN24ReqDebugClearQuestResultC2Ev
    // DEMANGLED: ReqDebugClearQuestResult::ReqDebugClearQuestResult(void)
    ReqDebugClearQuestResult();

    // IDA_EA: 0x82469D0 / 0x8246A32
    // IDA_NAME: _ZN24ReqDebugClearQuestResultD2Ev / D0Ev
    // DEMANGLED: ReqDebugClearQuestResult::~ReqDebugClearQuestResult()
    virtual ~ReqDebugClearQuestResult();

    MsgClearQuestIndex completed_requests;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(ReqDebugClearQuestResult) == 0x24u,
              "CMD457 parameter must retain its i386 ABI size");
#endif

// This exact generic instantiation belongs to the A protocol source.
extern template class IPacketDispatcher<MsgClearQuestIndex,
                                        ReqDebugClearQuestResult,
                                        static_cast<ch_state>(0)>;

// IDA unit: 08_quest_skill_growth/Dispatcher_Debug_Clear_Quest.
class Dispatcher_Debug_Clear_Quest
    : public IPacketDispatcher<MsgClearQuestIndex,
                               ReqDebugClearQuestResult,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597AEE
    // IDA_NAME: _ZN28Dispatcher_Debug_Clear_QuestC2Ev
    // DEMANGLED: Dispatcher_Debug_Clear_Quest::Dispatcher_Debug_Clear_Quest(void)
    Dispatcher_Debug_Clear_Quest();

    // IDA_EA: 0x82443F2 / 0x8244422
    // IDA_NAME: _ZN28Dispatcher_Debug_Clear_QuestD2Ev / D0Ev
    // DEMANGLED: Dispatcher_Debug_Clear_Quest::~Dispatcher_Debug_Clear_Quest()
    virtual ~Dispatcher_Debug_Clear_Quest();

    // IDA_EA: 0x81DF840
    // IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Debug_Clear_Quest::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DF91C
    // IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Debug_Clear_Quest::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DFB40
    // IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Debug_Clear_Quest::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Debug_Clear_Quest) == 0x04u,
              "CMD457 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DEBUG_CLEAR_QUEST_H_
