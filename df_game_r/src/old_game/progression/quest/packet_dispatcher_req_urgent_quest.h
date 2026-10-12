#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQ_URGENT_QUEST_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQ_URGENT_QUEST_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/ReqResult.
struct ReqResult : public ParamBase {
    // IDA_EA: 0x8231A0C
    // IDA_NAME: _ZN9ReqResultC2Ev
    // DEMANGLED: ReqResult::ReqResult(void)
    ReqResult();

    // IDA_EA: 0x8246A50 / 0x8246A80
    // IDA_NAME: _ZN9ReqResultD2Ev / D0Ev
    // DEMANGLED: ReqResult::~ReqResult()
    virtual ~ReqResult();

    uint8_t has_selected_quest;  // +0x04
    int selected_quest_index;    // +0x08; C2 initializes to -1.
    int error_code;              // +0x0c; C2 intentionally leaves it unwritten.
};

#if defined(__i386__)
static_assert(sizeof(ReqResult) == 0x10u,
              "ReqResult must retain its original i386 ABI size");
static_assert(offsetof(ReqResult, has_selected_quest) == 0x04u &&
                  offsetof(ReqResult, selected_quest_index) == 0x08u &&
                  offsetof(ReqResult, error_code) == 0x0cu,
              "ReqResult field offsets must retain the original ABI");
#endif

// The full generic triplet is emitted once by the A protocol source.
extern template class IPacketDispatcher<MSG_BASE,
                                        ReqResult,
                                        static_cast<ch_state>(3)>;

// IDA unit: 08_quest_skill_growth/Dispatcher_Req_Urgent_Quest.
class Dispatcher_Req_Urgent_Quest
    : public IPacketDispatcher<MSG_BASE, ReqResult, static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597AD2
    // IDA_NAME: _ZN27Dispatcher_Req_Urgent_QuestC2Ev
    // DEMANGLED: Dispatcher_Req_Urgent_Quest::Dispatcher_Req_Urgent_Quest(void)
    Dispatcher_Req_Urgent_Quest();

    // IDA_EA: 0x8244440 / 0x8244470
    // IDA_NAME: _ZN27Dispatcher_Req_Urgent_QuestD2Ev / D0Ev
    // DEMANGLED: Dispatcher_Req_Urgent_Quest::~Dispatcher_Req_Urgent_Quest()
    virtual ~Dispatcher_Req_Urgent_Quest();

    // IDA_EA: 0x81DF422
    // IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Req_Urgent_Quest::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DF42C
    // IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Req_Urgent_Quest::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DF75A
    // IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Req_Urgent_Quest::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Req_Urgent_Quest) == 0x04u,
              "CMD454 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQ_URGENT_QUEST_H_
