#include "old_game/progression/quest/quest_state_runtime.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/concurrent/timer/time_gate.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

#include <cstdarg>
#include <cstring>
#include <new>


class CParty;

namespace WongWork {

// Imported A-owned protocol/security member used by this C-side consumer.
// ORIGINAL_SYMBOL: _ZN8WongWork13CHackAnalyzer16addServerHackCntEP5CUserNS_13ENUM_HACKTYPEEjjj
}  // namespace WongWork

typedef char WongWork_CQuestClear_Size_Must_Be_0x7534[
    (sizeof(WongWork::CQuestClear) == 0x7534) ? 1 : -1];
typedef char SIG_LOAD_QUEST_Size_Must_Be_30216[
    (sizeof(SIG_LOAD_QUEST) == 30216) ? 1 : -1];
typedef char UserQuest_Size_Must_Be_30236[
    (sizeof(UserQuest) == 30236) ? 1 : -1];

namespace WongWork {

// IDA_EA: 0x808BA78
// IDA_NAME: _ZN8WongWork11CQuestClear15setClearedQuestEj
// DEMANGLED: WongWork::CQuestClear::setClearedQuest(unsigned int)
int CQuestClear::setClearedQuest(unsigned int quest_index) {
    if (!quest_index || quest_index > 0x752Fu) {
        return 0;
    }
    save_flag = 1;
    clear_quest[quest_index].clear_flag = 1;
    return 1;
}

// IDA_EA: 0x808BAAC
// IDA_NAME: _ZN8WongWork11CQuestClear18resetClearedQuestsEj
// DEMANGLED: WongWork::CQuestClear::resetClearedQuests(unsigned int)
int CQuestClear::resetClearedQuests(unsigned int quest_index) {
    if (!quest_index || quest_index > 0x752Fu) {
        return 0;
    }
    save_flag = 1;
    clear_quest[quest_index].clear_flag = 0;
    return 1;
}

// IDA_EA: 0x808BAE0
// IDA_NAME: _ZNK8WongWork11CQuestClear14isClearedQuestEj
// DEMANGLED: WongWork::CQuestClear::isClearedQuest(unsigned int)const
int CQuestClear::isClearedQuest(unsigned int quest_index) const {
    if (quest_index && quest_index <= 0x752Fu) {
        return clear_quest[quest_index].clear_flag;
    }
    return 0;
}

// IDA_EA: 0x808BB06
// IDA_NAME: _ZN8WongWork11CQuestClear5clearEv
// DEMANGLED: WongWork::CQuestClear::clear(void)
CQuestClear* CQuestClear::clear() {
    std::memset(this, 0, 0x7530u);
    save_flag = 0;
    return this;
}

// IDA_EA: 0x808BB34
// IDA_NAME: _ZN8WongWork11CQuestClear10syncScriptEv
// DEMANGLED: WongWork::CQuestClear::syncScript(void)
int CQuestClear::syncScript() {
    return 1;
}

// IDA_EA: 0x808BB3E
// IDA_NAME: _ZNK8WongWork11CQuestClear13checkSaveFlagEv
// DEMANGLED: WongWork::CQuestClear::checkSaveFlag(void)const
int CQuestClear::checkSaveFlag() const {
    return save_flag;
}

// IDA_EA: 0x808BB4E
// IDA_NAME: _ZN8WongWork11CQuestClear14resetSaveFlagsEv
// DEMANGLED: WongWork::CQuestClear::resetSaveFlags(void)
CQuestClear* CQuestClear::resetSaveFlags() {
    save_flag = 0;
    return this;
}

// IDA_EA: 0x808BB5E
// IDA_NAME: _ZN8WongWork11CQuestClear15setClearedQuestEPKNS_14STClearedQuestE
// DEMANGLED: WongWork::CQuestClear::setClearedQuest(WongWork::STClearedQuest const*)
void* CQuestClear::setClearedQuest(const STClearedQuest* quest_clear) {
    return std::memcpy(clear_quest, quest_clear, 0x7530u);
}

// IDA_EA: 0x808BB80
// IDA_NAME: _ZNK8WongWork11CQuestClear15getClearedQuestEv
// DEMANGLED: WongWork::CQuestClear::getClearedQuest(void)const
const STClearedQuest* CQuestClear::getClearedQuest() const {
    return clear_quest;
}

// IDA_EA: 0x808BB88
// IDA_NAME: _ZNK8WongWork11CQuestClear14enumQuestClearEPFbjPcEz
// DEMANGLED: WongWork::CQuestClear::enumQuestClear(bool (*)(unsigned int,char *),...)const
bool CQuestClear::enumQuestClear(bool (*callback)(unsigned int, char*),
                                 ...) const {
    va_list arguments;
    va_start(arguments, callback);

    // The original i386 ABI hands each callback the unchanged address of the
    // first variadic argument.  Callers use it as a three-pointer context.
#if defined(__i386__)
    char* const argument_block = arguments;
#else
    char* const argument_block = reinterpret_cast<char*>(&arguments);
#endif
    for (unsigned int quest_index = 1; quest_index <= 29999u;
         ++quest_index) {
        callback(quest_index, argument_block);
    }

    va_end(arguments);
    return false;
}

// IDA_EA: 0x808BBC4
// IDA_NAME: _ZNK8WongWork11CQuestClear18getClearedQuestNumEv
// DEMANGLED: WongWork::CQuestClear::getClearedQuestNum(void)const
int CQuestClear::getClearedQuestNum() const {
    int count = 0;
    for (int index = 1; index <= 29999; ++index) {
        if (clear_quest[index].clear_flag) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x822E4CC
// IDA_NAME: _ZN8WongWork11CQuestClearD2Ev
// DEMANGLED: WongWork::CQuestClear::~CQuestClear()
CQuestClear::~CQuestClear() {}

// IDA_EA: 0x8694B92
// IDA_NAME: _ZN8WongWork11CQuestClearC2Ev
// DEMANGLED: WongWork::CQuestClear::CQuestClear(void)
CQuestClear::CQuestClear() {
    STClearedQuest* item = clear_quest;
    for (int index = 29999; index != -1; --index) {
        new (item) STClearedQuest();
        ++item;
    }
    clear();
}

}  // namespace WongWork

// IDA_EA: 0x8491E92
// IDA_NAME: _ZN14SIG_LOAD_QUESTC2Ev
// DEMANGLED: SIG_LOAD_QUEST::SIG_LOAD_QUEST(void)
SIG_LOAD_QUEST::SIG_LOAD_QUEST() {
    WongWork::STClearedQuest* item = clear_quest;
    for (int index = 29999; index != -1; --index) {
        new (item) WongWork::STClearedQuest();
        ++item;
    }
}

// IDA_EA: 0x8694F06
// IDA_NAME: _ZN9UserQuest10set_parentEP5CUser
// DEMANGLED: UserQuest::set_parent(CUser *)
UserQuest* UserQuest::set_parent(CUser* user) {
    parent = user;
    return this;
}

// IDA_EA: 0x8697B8A
// IDA_NAME: _ZN9UserQuestC2Ev
// DEMANGLED: UserQuest::UserQuest(void)
UserQuest::UserQuest()
    : clear_quest(),
      authen_data() {}

// IDA_EA: 0x8697BB0
// IDA_NAME: _ZN9UserQuestD2Ev
// DEMANGLED: UserQuest::~UserQuest()
UserQuest::~UserQuest() {}

// IDA_EA: 0x86AB894
// IDA_NAME: _ZN9UserQuest5resetEv
// DEMANGLED: UserQuest::reset(void)
void* UserQuest::reset() {
    parent = 0;
    clear_quest.clear();
    for (int index = 0; index <= 19; ++index) {
        play_quest[index] = 0;
        play_quest_trigger[index] = 0;
    }
    urgent_quest_index = static_cast<uint32_t>(-1);
    return std::memset(quest_notify, 0, 0x28u);
}

// IDA_EA: 0x86AB920
// IDA_NAME: _ZNK9UserQuest12isClearQuestEi
// DEMANGLED: UserQuest::isClearQuest(int)const
int UserQuest::isClearQuest(int quest_index) const {
    return clear_quest.isClearedQuest(static_cast<unsigned int>(quest_index));
}

// IDA_EA: 0x86AB93E
// IDA_NAME: _ZN9UserQuest15resetClearQuestEi
// DEMANGLED: UserQuest::resetClearQuest(int)
int UserQuest::resetClearQuest(int quest_index) {
    return clear_quest.resetClearedQuests(
        static_cast<unsigned int>(quest_index));
}

// IDA_EA: 0x86AB95C
// IDA_NAME: _ZN9UserQuest9set_questEPK14SIG_LOAD_QUEST
// DEMANGLED: UserQuest::set_quest(SIG_LOAD_QUEST const*)
int UserQuest::set_quest(const SIG_LOAD_QUEST* payload) {
    for (int index = 0; index <= 19; ++index) {
        play_quest[index] = payload->play_quest[index];
        play_quest_trigger[index] = payload->play_quest_trigger[index];
        Quest* quest = G_CDataManager()->find_quest(play_quest[index]);
        if (!quest && play_quest[index]) {
            cMyTrace trace("bool UserQuest::set_quest(const SIG_LOAD_QUEST*)",
                           66,
                           0);
            trace("VERIFY_QUEST : ABNOMAL QUEST INDEX (%d) : CHARAC_NO (%u)",
                  play_quest[index],
                  payload->charac_no);
            play_quest[index] = 0;
            play_quest_trigger[index] = 0;
        }
    }
    clear_quest.setClearedQuest(payload->clear_quest);
    urgent_quest_index = payload->urgent_quest_index;
    std::memcpy(quest_notify, payload->quest_notify, 0x28u);
    return 1;
}

// IDA_EA: 0x86ABAD6
// IDA_NAME: _ZNK9UserQuest9get_questEP14SIG_LOAD_QUEST
// DEMANGLED: UserQuest::get_quest(SIG_LOAD_QUEST *)const
int UserQuest::get_quest(SIG_LOAD_QUEST* payload) const {
    payload->error_code = 0;
    for (int index = 0; index <= 19; ++index) {
        payload->play_quest[index] = play_quest[index];
        payload->play_quest_trigger[index] = play_quest_trigger[index];
    }
    const WongWork::STClearedQuest* cleared = clear_quest.getClearedQuest();
    std::memcpy(payload->clear_quest, cleared, 0x7530u);
    payload->urgent_quest_index =
        parent->getCurCharacQuestR()->urgent_quest_index;
    std::memcpy(payload->quest_notify, quest_notify, 0x28u);
    return 1;
}

// IDA_EA: 0x86AC532
// IDA_NAME: _ZN9UserQuest12giveup_questEi
// DEMANGLED: UserQuest::giveup_quest(int)
int UserQuest::giveup_quest(int quest_index) {
    int index = 0;
    for (;; ++index) {
        if (index > 19) {
            return 19;
        }
        if (play_quest[index] == static_cast<uint32_t>(quest_index)) {
            break;
        }
    }
    play_quest[index] = 0;
    play_quest_trigger[index] = 0;
    return 0;
}

// IDA_EA: 0x86ACFFE
// IDA_NAME: _ZNK9UserQuest12IsDoingQuestEi
// DEMANGLED: UserQuest::IsDoingQuest(int)const
int UserQuest::IsDoingQuest(int quest_index) const {
    for (int index = 0; index <= 19; ++index) {
        if (play_quest[index] == static_cast<uint32_t>(quest_index) &&
            play_quest_trigger[index]) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x84C5054
// IDA_NAME: _ZN15Inter_LoadQuest12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadQuest::dispatch_sig(CUser *,char *,int)
int Inter_LoadQuest::dispatch_sig(CUser* user,
                                  char* payload,
                                  int payload_size) {
    (void)payload_size;

    SIG_LOAD_QUEST* sig = reinterpret_cast<SIG_LOAD_QUEST*>(payload);
    if (!user->get_state()) {
        return 6675;
    }
    if (sig->error_code == 1) {
        return 6682;
    }
    if (!user->getCurCharacR()) {
        return 6685;
    }
    UserQuest* quest = user->getCurCharacQuestR();
    quest->set_quest(sig);
    if (::GlobalData::s_timeGate_ && ::GlobalData::s_timeGate_->isOpen()) {
        const int quest_index = ::GlobalData::s_timeGate_->getQuestIndex();
        UserQuest* quest_r = user->getCurCharacQuestR();
        if (static_cast<unsigned char>(quest_r->IsDoingQuest(quest_index))) {
            const int quest_index_2 = ::GlobalData::s_timeGate_->getQuestIndex();
            UserQuest* quest_w = user->getCurCharacQuestW();
            quest_w->giveup_quest(quest_index_2);
        }
    }
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(3));
    return 0;
}

// IDA_EA: 0x84E9DF4
// IDA_NAME: _ZN15Inter_LoadQuestC2Ev
// DEMANGLED: Inter_LoadQuest::Inter_LoadQuest(void)
Inter_LoadQuest::Inter_LoadQuest() {}

// IDA_EA: 0x84E9E10
// IDA_NAME: _ZN15Inter_LoadQuestD2Ev
// DEMANGLED: Inter_LoadQuest::~Inter_LoadQuest()
// IDA_EA: 0x84E9E40
// IDA_NAME: _ZN15Inter_LoadQuestD0Ev
// DEMANGLED: Inter_LoadQuest::~Inter_LoadQuest() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_LoadQuest::~Inter_LoadQuest() {}

// IDA_EA: 0x81EE010
// IDA_NAME: _ZN22DisPatcher_QuestAction12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_QuestAction::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_QuestAction::dispatch_sig(CUser* user, PacketBuf& packet) {
    const CS_Header* const c2s_header =
        reinterpret_cast<const CS_Header*>(packet.get_packet(0));
    const unsigned char cmd = static_cast<unsigned char>(c2s_header->type);
    if (user->get_state() <= 2) {
        return 0;
    }
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        user->SendCmdErrorPacket(
            static_cast<ENUM_CMDPACKET>(cmd),
            1);
        return 0;
    }

    short first_quest_index = 0;
    if (packet.get_short(first_quest_index)) {
        if (cmd != static_cast<unsigned char>(first_quest_index)) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(819),
                1,
                0,
                0);
            user->SendCmdErrorPacket(
                static_cast<ENUM_CMDPACKET>(cmd),
                1);
            return 0;
        }
    }

    if (cmd < static_cast<unsigned char>(ENUM_CMDPACKET_ACCEPT_QUEST)) {
        return 0;
    }
    if (cmd <= static_cast<unsigned char>(ENUM_CMDPACKET_GIVEUP_QUEST)) {
        short quest_index = 0;
        if (packet.get_short(quest_index) != 1) {
            return LineFunc(
                29001,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        Quest* const dispatcher_quest =
            G_CDataManager()->find_quest(static_cast<int>(quest_index));
        LogManager::logFormat(
            1,
            "quest_world_enter_diag.cpp",
            "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
            0,
            "[WORLD_ENTER][QUEST_DISPATCH] uid=%d state=%d cmd=%u first=%d quest=%d lookup=%d",
            user->get_acc_id(),
            user->get_state(),
            static_cast<unsigned int>(cmd),
            static_cast<int>(first_quest_index),
            static_cast<int>(quest_index),
            dispatcher_quest ? 1 : 0);
        if (!dispatcher_quest) {
            return 0;
        }
        if (cmd == ENUM_CMDPACKET_ACCEPT_QUEST && ::GlobalData::s_timeGate_ &&
            ::GlobalData::s_timeGate_->setBlindTimeGateStateQuest(
                quest_index)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_ACCEPT_QUEST, 8);
        }
        user->quest_action(cmd, quest_index, 0, 0);
    } else if (cmd == ENUM_CMDPACKET_SET_QUEST_TRIGGER) {
        if (user->get_state() == 5) {
            CParty* party = user->GetParty();
            if (!party) {
                const short party_index =
                    static_cast<short>(user->GetPartyIndex());
                const unsigned int account_id =
                    static_cast<unsigned int>(user->get_acc_id());
                LogManager::logFormat(
                    1,
                    "PacketDispatcher_Impl_1.cpp",
                    "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                    29103,
                    "<IN_DUNGEON_NULL_PARTY> error m_id(%s), PartyIndex(%d)",
                    NumberToString(account_id, 0),
                    party_index);
                return 0;
            }
            if (party->battle_field().instance_dungeon_mode() == 1) {
                return 0;
            }
        }

        short quest_index = 0;
        char trigger_state = 0;
        if (packet.get_short(quest_index) != 1) {
            return LineFunc(
                29113,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (packet.get_byte(trigger_state) != 1) {
            return LineFunc(
                29114,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        char trigger_value = 0;
        if ((trigger_state == 16 || trigger_state == 32 ||
             trigger_state == 64) &&
            packet.get_byte(trigger_value) != 1) {
            return LineFunc(
                29124,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        user->quest_action(cmd,
                           quest_index,
                           trigger_state,
                           trigger_value);
    } else if (cmd == ENUM_CMDPACKET_FINISH_QUEST) {
        short quest_index = 0;
        short trigger_state = 0;
        short trigger_value = 0;
        if (packet.get_short(quest_index) != 1) {
            return LineFunc(
                29082,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (packet.get_short(trigger_state) != 1) {
            return LineFunc(
                29083,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (packet.get_short(trigger_value) != 1) {
            return LineFunc(
                29084,
                "virtual int DisPatcher_QuestAction::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (trigger_value > 0) {
            user->quest_action(cmd,
                               quest_index,
                               trigger_state,
                               trigger_value);
            user->setGmQuestFlag(false);
        }
    }
    return 0;
}

// IDA_EA: 0x8232D1A
// IDA_NAME: _ZN22DisPatcher_QuestAction4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_QuestAction::read(PacketBuf &,MSG_BASE &)
int DisPatcher_QuestAction::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232D24
// IDA_NAME: _ZN22DisPatcher_QuestAction7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_QuestAction::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_QuestAction::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8232D2E
// IDA_NAME: _ZN22DisPatcher_QuestActionD2Ev
// DEMANGLED: DisPatcher_QuestAction::~DisPatcher_QuestAction()
DisPatcher_QuestAction::~DisPatcher_QuestAction() {}

// IDA_EA: 0x8232D5E
// IDA_NAME: _ZN22DisPatcher_QuestActionD0Ev
// DEMANGLED: DisPatcher_QuestAction::~DisPatcher_QuestAction() deleting destructor
// Compiler-generated from the virtual destructor declaration in the canonical
// class header.  It emits the original D0 member and calls D2 then operator
// delete; a free compatibility wrapper would be the wrong ABI owner.

// IDA_EA: 0x85954A0
// IDA_NAME: _ZN22DisPatcher_QuestActionC2Ev
// DEMANGLED: DisPatcher_QuestAction::DisPatcher_QuestAction(void)
DisPatcher_QuestAction::DisPatcher_QuestAction()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x825ED06
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotify4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveQuestNotify::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveQuestNotify::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SAVE_QUEST_NOTIFY& request =
        static_cast<MSG_SAVE_QUEST_NOTIFY&>(message);
    char count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(
            691,
            "virtual int Dispatcher_SaveQuestNotify::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (static_cast<unsigned char>(count) >= 6u) {
        return LineFunc(
            694,
            "virtual int Dispatcher_SaveQuestNotify::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    for (int index = 0; count > index; ++index) {
        if (packet.get_int(request.quest_ids[index]) != 1) {
            return LineFunc(
                699,
                "virtual int Dispatcher_SaveQuestNotify::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }
    }
    return 0;
}

// IDA_EA: 0x825EDFC
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotify7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveQuestNotify::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveQuestNotify::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    saveQuestNotifyParam& response =
        static_cast<saveQuestNotifyParam&>(param);
    response.result = check_error(user, message);
    if (response.result > 0) {
        return 0;
    }
    if (response.result >= 0) {
        const MSG_SAVE_QUEST_NOTIFY& request =
            static_cast<const MSG_SAVE_QUEST_NOTIFY&>(message);
        UserQuest* quest = user->getCurCharacQuestW();
        std::memcpy(quest->quest_notify,
                    request.quest_ids,
                    sizeof(request.quest_ids));
        return 0;
    }

    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_2.cpp",
        "virtual int Dispatcher_SaveQuestNotify::process(CUser*, MSG_BASE&, ParamBase&)",
        715,
        "Dispatcher Check_Error No(%d)",
        response.result);
    return 716;
}

// IDA_EA: 0x825EEC4
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotify11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SaveQuestNotify::check_error(CUser *,MSG_BASE &)
int Dispatcher_SaveQuestNotify::check_error(CUser* user, MSG_BASE& message) {
    const MSG_SAVE_QUEST_NOTIFY& request =
        static_cast<const MSG_SAVE_QUEST_NOTIFY&>(message);

    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return -2;
    }
    for (int index = 0; index <= 9; ++index) {
        const int quest_index = static_cast<int>(request.quest_ids[index]);
        if (quest_index &&
            !G_CDataManager()->find_quest(quest_index)) {
            return -3;
        }
    }
    return 0;
}

// IDA_EA: 0x8260A8E
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotifyD2Ev
// DEMANGLED: Dispatcher_SaveQuestNotify::~Dispatcher_SaveQuestNotify()
Dispatcher_SaveQuestNotify::~Dispatcher_SaveQuestNotify() {}

// IDA_EA: 0x8260ABE
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotifyD0Ev
// DEMANGLED: Dispatcher_SaveQuestNotify::~Dispatcher_SaveQuestNotify()
// Compiler-generated from the virtual destructor declaration in the canonical
// class header.  It emits the original D0 member and calls D2 then operator
// delete; a free compatibility wrapper would be the wrong ABI owner.

// IDA_EA: 0x8595804
// IDA_NAME: _ZN26Dispatcher_SaveQuestNotifyC2Ev
// DEMANGLED: Dispatcher_SaveQuestNotify::Dispatcher_SaveQuestNotify(void)
Dispatcher_SaveQuestNotify::Dispatcher_SaveQuestNotify()
    : IPacketDispatcher<MSG_SAVE_QUEST_NOTIFY,
                        saveQuestNotifyParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82617C0 / 0x826191C / 0x8261926 / 0x82619BA /
//         0x8262482 / 0x82624B2 / 0x85982B2
template class IPacketDispatcher<MSG_SAVE_QUEST_NOTIFY,
                                 saveQuestNotifyParam,
                                 static_cast<ch_state>(3)>;
