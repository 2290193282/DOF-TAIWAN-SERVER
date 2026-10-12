#include "old_game/progression/quest/packet_dispatcher_debug_clear_quest.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_tools.h"
#include <stdint.h>

#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_Debug_Clear_Quest::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_Debug_Clear_Quest::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8231A72
// IDA_NAME: _ZN24ReqDebugClearQuestResultC2Ev
// DEMANGLED: ReqDebugClearQuestResult::ReqDebugClearQuestResult(void)
ReqDebugClearQuestResult::ReqDebugClearQuestResult()
    : ParamBase(), completed_requests() {}

// IDA_EA: 0x82469D0
// IDA_NAME: _ZN24ReqDebugClearQuestResultD2Ev
// IDA_EA: 0x8246A32
// IDA_NAME: _ZN24ReqDebugClearQuestResultD0Ev
// DEMANGLED: ReqDebugClearQuestResult::~ReqDebugClearQuestResult()
ReqDebugClearQuestResult::~ReqDebugClearQuestResult() {}

// IDA_EA: 0x81DF840
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Debug_Clear_Quest::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Debug_Clear_Quest::read(PacketBuf& packet, MSG_BASE& message) {
    MsgClearQuestIndex& request = static_cast<MsgClearQuestIndex&>(message);
    int16_t requested_count;
    if (packet.get_short(requested_count) != 1) {
        return LineFunc(20961, kReadSignature, 0, 0);
    }
    request.quest_count = requested_count;
    for (int index = 0; request.quest_count > index; ++index) {
        int16_t quest_index;
        if (packet.get_short(quest_index) != 1) {
            return LineFunc(20968, kReadSignature, 0, 0);
        }
        request.quest_indexes_14.push_back(quest_index);
    }
    return 0;
}

// IDA_EA: 0x81DF91C
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Debug_Clear_Quest::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Debug_Clear_Quest::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    if (user->isGMUser() != 1) {
        return LineFunc(20979, kProcessSignature, 0, 0);
    }

    const MsgClearQuestIndex& request =
        static_cast<const MsgClearQuestIndex&>(message);
    ReqDebugClearQuestResult& result =
        static_cast<ReqDebugClearQuestResult&>(param);
    result.completed_requests = request;

    int cleared_count = 0;
    for (int index = 0; request.quest_count > index; ++index) {
        const int quest_index = request.quest_indexes_14[index];
        Quest* const quest = G_CDataManager()->find_quest(quest_index);
        if (!quest) {
            return 1;
        }

        stSelectQuestParam select_parameter(user);
        if (quest->exposeQuest() == 1 &&
            quest->check_possible(select_parameter) == 1) {
            user->getCurCharacQuestW()->clear_quest.setClearedQuest(
                static_cast<unsigned int>(quest_index));
            if (quest->quest_point > 0) {
                user->incQuestPoint(quest->quest_point, quest_index, true);
            }
            if (quest->quest_piece > 0) {
                user->incQuestPiece(quest->quest_piece, quest_index, true);
            }
            ++cleared_count;
        }
    }

    // The original writes only the count at +0x14; the copied vector remains
    // in its original order, including entries that did not meet the test.
    result.completed_requests.quest_count = cleared_count;
    user->sendCharacQp();
    user->sendCharacQuestPiece();
    return 0;
}

// IDA_EA: 0x81DFB40
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_Quest4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Debug_Clear_Quest::send(CUser *,ParamBase &)
void Dispatcher_Debug_Clear_Quest::send(CUser* user, ParamBase& param) {
    const ReqDebugClearQuestResult& result =
        static_cast<const ReqDebugClearQuestResult&>(param);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CLEAR_QUEST);
    response.put_byte(1);
    response.put_short(static_cast<short>(result.completed_requests.quest_count));
    for (int index = 0;
         result.completed_requests.quest_count > index;
         ++index) {
        response.put_short(static_cast<short>(
            result.completed_requests.quest_indexes_14[index]));
    }
    response.finalize(true);
    user->Send(response);
    user->send_clear_quest_list();

    PacketGuard quest_info;
    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&quest_info));
    user->Send(quest_info);
}

// IDA_EA: 0x82443F2
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_QuestD2Ev
// IDA_EA: 0x8244422
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_QuestD0Ev
// DEMANGLED: Dispatcher_Debug_Clear_Quest::~Dispatcher_Debug_Clear_Quest()
Dispatcher_Debug_Clear_Quest::~Dispatcher_Debug_Clear_Quest() {}

// IDA_EA: 0x8597AEE
// IDA_NAME: _ZN28Dispatcher_Debug_Clear_QuestC2Ev
// DEMANGLED: Dispatcher_Debug_Clear_Quest::Dispatcher_Debug_Clear_Quest(void)
Dispatcher_Debug_Clear_Quest::Dispatcher_Debug_Clear_Quest()
    : IPacketDispatcher<MsgClearQuestIndex,
                        ReqDebugClearQuestResult,
                        static_cast<ch_state>(0)>() {}
