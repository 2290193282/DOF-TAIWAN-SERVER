#include "old_game/progression/quest/packet_dispatcher_req_urgent_quest.h"
#include "infra/network/packet/PacketEnum.h"

#include <iterator>
#include <list>

#include "common/config.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"

namespace {

std::list<int>& selected_quests() {
    // IDA 0x81DF4C7..0x81DF533: function-local list with its matching
    // guard/atexit lifetime, shared by each urgent-quest request.
    static std::list<int> values;
    return values;
}

CMTRand& urgent_random_dice() {
    // IDA 0x81DF61A..0x81DF666: initialized only once for this dispatcher.
    static CMTRand dice;
    return dice;
}

}  // namespace

// IDA_EA: 0x8231A0C
// IDA_NAME: _ZN9ReqResultC2Ev
// DEMANGLED: ReqResult::ReqResult(void)
ReqResult::ReqResult()
    : ParamBase(), has_selected_quest(0), selected_quest_index(-1) {}

// IDA_EA: 0x8246A50
// IDA_NAME: _ZN9ReqResultD2Ev
// IDA_EA: 0x8246A80
// IDA_NAME: _ZN9ReqResultD0Ev
// DEMANGLED: ReqResult::~ReqResult()
ReqResult::~ReqResult() {}

// IDA_EA: 0x81DF422
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Req_Urgent_Quest::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Req_Urgent_Quest::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81DF42C
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Req_Urgent_Quest::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Req_Urgent_Quest::process(CUser* user,
                                          MSG_BASE&,
                                          ParamBase& param) {
    if (user->get_state() <= 2) {
        return 0;
    }

    ReqResult& result = static_cast<ReqResult&>(param);
    result.has_selected_quest = 0;
    result.selected_quest_index = -1;

    const int character_level = user->get_charac_level();
    UserQuest* const current_quest = user->getCurCharacQuestR();
    // 0627 admitted urgent quests through level 69 because level 70 was its
    // cap.  Preserve the original "below the current cap" policy for 0828:
    // this runtime caps characters at 85, so requests remain valid through
    // level 84 while the official Time Gate/Power Station urgent rows apply.
    if (character_level <= 26 ||
        character_level > oldsrv::config::kCharacterLevelBeforeMaximum ||
        current_quest->urgent_quest_index != static_cast<uint32_t>(-1)) {
        result.error_code = 1;
        return 0;
    }

    UserQuest* const writable_quest = user->getCurCharacQuestW();
    if (writable_quest->hasEpicQuest()) {
        result.error_code = 1;
        return 0;
    }

    std::list<int>& candidates = selected_quests();
    candidates.clear();
    // IDA copies this exact CQuestClear child before evaluating the list.
    const WongWork::CQuestClear clear_snapshot = current_quest->clear_quest;
    stSelectQuestParam select_parameter(user);
    QuestList* const quest_list = G_CDataManager()->quest_list_;
    const std::multimap<int, int>* const quest_npcs =
        quest_list->getQuestNPCList(static_cast<ENUM_QUEST_GRADE>(8));
    quest_list->allowable_questlist_as_npc(candidates,
                                           *quest_npcs,
                                           select_parameter,
                                           clear_snapshot,
                                           true);
    if (candidates.empty()) {
        result.error_code = 4;
        return 0;
    }

    const unsigned long last_index =
        static_cast<unsigned long>(candidates.size() - 1u);
    const unsigned int selected_index = urgent_random_dice().randInt(last_index);
    if (candidates.size() > selected_index) {
        std::list<int>::const_iterator selected = candidates.begin();
        std::advance(selected, selected_index);
        writable_quest->urgent_quest_index =
            static_cast<uint32_t>(*selected);
        result.selected_quest_index = *selected;
        result.has_selected_quest = 1;
    }
    return 0;
}

// IDA_EA: 0x81DF75A
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_Quest4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Req_Urgent_Quest::send(CUser *,ParamBase &)
void Dispatcher_Req_Urgent_Quest::send(CUser* user, ParamBase& param) {
    const ReqResult& result = static_cast<const ReqResult&>(param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_REQ_URGENT_QUEST);
    response.put_byte(result.has_selected_quest != 0);
    if (result.has_selected_quest) {
        response.put_short(static_cast<short>(result.selected_quest_index));
    } else {
        response.put_byte(static_cast<char>(result.error_code));
    }
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x8244440
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_QuestD2Ev
// IDA_EA: 0x8244470
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_QuestD0Ev
// DEMANGLED: Dispatcher_Req_Urgent_Quest::~Dispatcher_Req_Urgent_Quest()
Dispatcher_Req_Urgent_Quest::~Dispatcher_Req_Urgent_Quest() {}

// IDA_EA: 0x8597AD2
// IDA_NAME: _ZN27Dispatcher_Req_Urgent_QuestC2Ev
// DEMANGLED: Dispatcher_Req_Urgent_Quest::Dispatcher_Req_Urgent_Quest(void)
Dispatcher_Req_Urgent_Quest::Dispatcher_Req_Urgent_Quest()
    : IPacketDispatcher<MSG_BASE, ReqResult, static_cast<ch_state>(3)>() {}
