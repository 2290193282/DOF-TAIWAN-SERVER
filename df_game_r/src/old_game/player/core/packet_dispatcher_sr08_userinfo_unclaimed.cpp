#include "old_game/player/core/packet_dispatcher_sr08_userinfo_unclaimed.h"

#include "old_data/db/game_option/db_save_game_option_1.h"
#include "old_data/db/game_option/db_save_game_option_chatting_emoticon.h"
#include "old_data/db/game_option/db_save_game_option_quick_chatting.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <string>


// These four generic triplets exist as concrete old-server vtables, but this
// file intentionally does not insert them into PacketDispatcher's incomplete
// root table.  Registration remains gated by the full 606-slot constructor.

// IDA_EA: 0x8255270
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82553CC
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82553D6
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825546A
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x825CE32
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CE62
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x859874A
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_19ParamBaseL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,ParamBase,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,
                                 ParamBase,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x8255040
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82551AA
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82551B4
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8255248
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x825CDE4
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CE14
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x8598766
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_SAVE_GAME_OPTION_29ParamBaseL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,ParamBase,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,
                                 ParamBase,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x8254E10
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8254F7A
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254F84
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8255018
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x825CD96
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CDC6
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x8598B3A
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUICK_CHAT16stQuickChatParamL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_CHAT,stQuickChatParam,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_QUICK_CHAT,
                                 stQuickChatParam,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x8254BF4
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8254D72
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8254D7C
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D960
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x825CD48
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CD78
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x8598FEE
// IDA_NAME: _ZN17IPacketDispatcherI38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON9ParamBaseL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,ParamBase,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,
                                 ParamBase,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x825524E
// IDA_NAME: _ZN22MSG_SAVE_GAME_OPTION_1C2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_1::MSG_SAVE_GAME_OPTION_1(void)
MSG_SAVE_GAME_OPTION_1::MSG_SAVE_GAME_OPTION_1()
    : MSG_BASE(),
      option_length(0),
      option() {}

// IDA_EA: 0x825501E
// IDA_NAME: _ZN22MSG_SAVE_GAME_OPTION_2C2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_2::MSG_SAVE_GAME_OPTION_2(void)
MSG_SAVE_GAME_OPTION_2::MSG_SAVE_GAME_OPTION_2()
    : MSG_BASE(),
      option_length(0),
      option() {}

// IDA_EA: 0x822DC66
// IDA_NAME: _ZN14MSG_QUICK_CHATC2Ev
// DEMANGLED: MSG_QUICK_CHAT::MSG_QUICK_CHAT(void)
MSG_QUICK_CHAT::MSG_QUICK_CHAT()
    : MSG_BASE(),
      option_length(0),
      quick_chat_type(0),
      option() {}

// IDA_EA: 0x822DC98
// IDA_NAME: _ZN14MSG_QUICK_CHAT13isLastMessageEv
// DEMANGLED: MSG_QUICK_CHAT::isLastMessage(void)
bool MSG_QUICK_CHAT::isLastMessage() {
    return quick_chat_type == 1;
}

// IDA_EA: 0x8231262
// IDA_NAME: _ZN16stQuickChatParamC2Ev
// DEMANGLED: stQuickChatParam::stQuickChatParam(void)
stQuickChatParam::stQuickChatParam()
    : ParamBase() {}

// IDA_EA: 0x82476A2
// IDA_NAME: _ZN16stQuickChatParamD2Ev
// DEMANGLED: stQuickChatParam::~stQuickChatParam()
// IDA_EA: 0x82476D2
// IDA_NAME: _ZN16stQuickChatParamD0Ev
// DEMANGLED: stQuickChatParam::~stQuickChatParam()
stQuickChatParam::~stQuickChatParam() {}

// IDA_EA: 0x8254BBC
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICONC2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON(void)
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::
    MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON()
    : MSG_BASE(),
      slots() {}

// IDA_EA: 0x8254BDE
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICOND2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::~MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON()
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::
    ~MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON() {}

// IDA_EA: 0x85967E0
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_1C2Ev
// DEMANGLED: Dispatcher_SaveGameOption_1::Dispatcher_SaveGameOption_1(void)
Dispatcher_SaveGameOption_1::Dispatcher_SaveGameOption_1()
    : IPacketDispatcher<MSG_SAVE_GAME_OPTION_1,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8245C04
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_1D2Ev
// DEMANGLED: Dispatcher_SaveGameOption_1::~Dispatcher_SaveGameOption_1()
// IDA_EA: 0x8245C34
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_1D0Ev
// DEMANGLED: Dispatcher_SaveGameOption_1::~Dispatcher_SaveGameOption_1()
Dispatcher_SaveGameOption_1::~Dispatcher_SaveGameOption_1() {}

// IDA_EA: 0x81CEA7A
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_14readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_1::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveGameOption_1::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SAVE_GAME_OPTION_1& input =
        static_cast<MSG_SAVE_GAME_OPTION_1&>(message);
    if (packet.get_int(input.option_length) != 1 ||
        input.option_length != sizeof(input.option)) {
        return LineFunc(
            10336,
            "virtual int Dispatcher_SaveGameOption_1::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_binary(reinterpret_cast<char*>(&input.option),
                          static_cast<int>(input.option_length)) == 1) {
        return 0;
    }
    return LineFunc(
        10338,
        "virtual int Dispatcher_SaveGameOption_1::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CEB38
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_17processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveGameOption_1::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveGameOption_1::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    const int result = check_error(user, message);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return -1;
    }

    MSG_SAVE_GAME_OPTION_1& input =
        static_cast<MSG_SAVE_GAME_OPTION_1&>(message);
    static_assert(sizeof(CETCOption) == sizeof(CGameEtcOptionPayload),
                  "runtime and packet ETC-option layouts must match");
    CETCOption options;
    std::memcpy(&options,
                &input.option,
                sizeof(input.option));
    const int16_t grow_avatar =
        static_cast<int16_t>(options.getETCOption(1));
    if (grow_avatar != user->get_charac_visible_grow_avatar()) {
        user->set_charac_visible_grow_avatar(grow_avatar != 0);
        user->send_charac_visible_data();
        user->character_view().enableSaveCharacView();
    }

    CGameOption* const game_option = user->GetGameOptionRef();
    if (game_option->IsEtcOptionChanged(options) == 1) {
        game_option->SetEtcOption(options);
    }

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           10393),
        true);
    guard << 304;
    guard << user->GetUID();
    SIG_SAVE_GAME_OPTION_1* const saved =
        guard.GetInBuffer<SIG_SAVE_GAME_OPTION_1>();
    saved->m_id = user->get_acc_id();
    std::memcpy(&saved->option_1,
                &input.option,
                sizeof(saved->option_1));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81CED4E
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_111check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_1::check_error(CUser *,MSG_BASE &)
int Dispatcher_SaveGameOption_1::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return LineFunc(
            10413,
            "int Dispatcher_SaveGameOption_1::check_error(CUser*, MSG_BASE&)",
            0,
            0);
    }
    return user->get_state() > 1 ? 0 : -1;
}

// IDA_EA: 0x85967FC
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_2C2Ev
// DEMANGLED: Dispatcher_SaveGameOption_2::Dispatcher_SaveGameOption_2(void)
Dispatcher_SaveGameOption_2::Dispatcher_SaveGameOption_2()
    : IPacketDispatcher<MSG_SAVE_GAME_OPTION_2,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8245BB6
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_2D2Ev
// DEMANGLED: Dispatcher_SaveGameOption_2::~Dispatcher_SaveGameOption_2()
// IDA_EA: 0x8245BE6
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_2D0Ev
// DEMANGLED: Dispatcher_SaveGameOption_2::~Dispatcher_SaveGameOption_2()
Dispatcher_SaveGameOption_2::~Dispatcher_SaveGameOption_2() {}

// IDA_EA: 0x81CEDA4
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_24readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_2::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveGameOption_2::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SAVE_GAME_OPTION_2& input =
        static_cast<MSG_SAVE_GAME_OPTION_2&>(message);
    const int index_before_length = packet.get_index();
    const int packet_length = packet.get_bound_length();
    const int length_read_result = packet.get_int(input.option_length);
    if (length_read_result != 1) {
        LogManager::logFormat(
            1, "hotkey_option_diag.cpp",
            "Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)", 0,
            "[HOTKEY198][READ_FAIL] stage=length_field result=%d "
            "index_before=%d index_after=%d packet_len=%d remaining_before=%d "
            "last_error=%d",
            length_read_result, index_before_length, packet.get_index(),
            packet_length, packet_length - index_before_length,
            packet.getLastError());
        return LineFunc(
            10428,
            "virtual int Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (input.option_length > sizeof(input.option.bytes_)) {
        LogManager::logFormat(
            1, "hotkey_option_diag.cpp",
            "Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)", 0,
            "[HOTKEY198][READ_FAIL] stage=declared_length length=%u cap=%u "
            "index_after=%d packet_len=%d payload_remaining=%d last_error=%d",
            static_cast<unsigned int>(input.option_length),
            static_cast<unsigned int>(sizeof(input.option.bytes_)),
            packet.get_index(), packet_length,
            packet_length - packet.get_index(), packet.getLastError());
        return LineFunc(
            10428,
            "virtual int Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    input.option.clear();
    if (input.option_length == 0) {
        return 0;
    }
    const int payload_read_result = packet.get_binary(input.option.bytes_,
        static_cast<int>(input.option_length));
    const bool text_valid =
        payload_read_result == 1 && input.option.isValidText(input.option_length);
    if (text_valid) {
        return 0;
    }
    LogManager::logFormat(
        1, "hotkey_option_diag.cpp",
        "Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)", 0,
        "[HOTKEY198][READ_FAIL] stage=%s length=%u result=%d index_after=%d "
        "packet_len=%d remaining=%d last_error=%d",
        payload_read_result == 1 ? "text_validation" : "payload",
        static_cast<unsigned int>(input.option_length), payload_read_result,
        packet.get_index(), packet_length, packet_length - packet.get_index(),
        packet.getLastError());
    return LineFunc(
        10429,
        "virtual int Dispatcher_SaveGameOption_2::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CEE64
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_27processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveGameOption_2::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveGameOption_2::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    const int result = check_error(user, message);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return -1;
    }

    MSG_SAVE_GAME_OPTION_2& input =
        static_cast<MSG_SAVE_GAME_OPTION_2&>(message);
    CGameOption* const game_option = user->GetGameOptionRef();
    const unsigned char key_type = static_cast<unsigned char>(
        user->GetCharacHotKeyType(user->get_charac_job()));
    if (game_option->IsHotKeyOptionChanged(key_type, input.option) != 1) {
        return 0;
    }
    game_option->SetHotKeyOption(key_type, input.option);

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           10465),
        true);
    guard << 305;
    guard << user->GetUID();
    SIG_LOAD_CHARAC_HOTKEY_OPTION* const saved =
        guard.GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>();
    saved->m_id = user->get_acc_id();
    saved->key_type = static_cast<unsigned char>(
        user->GetCharacHotKeyType(user->get_charac_job()));
    saved->option_length = input.option_length;
    saved->hotkey_option = input.option;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81CF050
// IDA_NAME: _ZN27Dispatcher_SaveGameOption_211check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_2::check_error(CUser *,MSG_BASE &)
int Dispatcher_SaveGameOption_2::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return LineFunc(
            10490,
            "int Dispatcher_SaveGameOption_2::check_error(CUser*, MSG_BASE&)",
            0,
            0);
    }
    return user->get_state() > 1 ? 0 : -1;
}

// IDA_EA: 0x85975E6
// IDA_NAME: _ZN39Dispatcher_SaveGameOption_QuickChattingC2Ev
// DEMANGLED: Dispatcher_SaveGameOption_QuickChatting::Dispatcher_SaveGameOption_QuickChatting(void)
Dispatcher_SaveGameOption_QuickChatting::
    Dispatcher_SaveGameOption_QuickChatting()
    : IPacketDispatcher<MSG_QUICK_CHAT,
                        stQuickChatParam,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8245B68
// IDA_NAME: _ZN39Dispatcher_SaveGameOption_QuickChattingD2Ev
// DEMANGLED: Dispatcher_SaveGameOption_QuickChatting::~Dispatcher_SaveGameOption_QuickChatting()
// IDA_EA: 0x8245B98
// IDA_NAME: _ZN39Dispatcher_SaveGameOption_QuickChattingD0Ev
// DEMANGLED: Dispatcher_SaveGameOption_QuickChatting::~Dispatcher_SaveGameOption_QuickChatting()
Dispatcher_SaveGameOption_QuickChatting::
    ~Dispatcher_SaveGameOption_QuickChatting() {}

// IDA_EA: 0x81CF0A6
// IDA_NAME: _ZN39Dispatcher_SaveGameOption_QuickChatting4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_QuickChatting::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveGameOption_QuickChatting::read(PacketBuf& packet,
                                                   MSG_BASE& message) {
    MSG_QUICK_CHAT& input = static_cast<MSG_QUICK_CHAT&>(message);
    if (packet.get_byte(input.quick_chat_type) != 1) {
        return LineFunc(
            10506,
            "virtual int Dispatcher_SaveGameOption_QuickChatting::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(input.option_length) != 1 ||
        input.option_length > sizeof(input.option)) {
        return LineFunc(
            10507,
            "virtual int Dispatcher_SaveGameOption_QuickChatting::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_binary(reinterpret_cast<char*>(&input.option),
                          static_cast<int>(input.option_length)) == 1) {
        return 0;
    }
    return LineFunc(
        10508,
        "virtual int Dispatcher_SaveGameOption_QuickChatting::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CF1AC
// IDA_NAME: _ZN39Dispatcher_SaveGameOption_QuickChatting7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveGameOption_QuickChatting::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveGameOption_QuickChatting::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    (void)param;
    MSG_QUICK_CHAT& input = static_cast<MSG_QUICK_CHAT&>(message);
    CGameOption* const game_option = user->GetGameOptionRef();
    game_option->SetQuickChatOption(input.option, input.quick_chat_type);

    if (input.isLastMessage() &&
        game_option->isChanged_QuickChatData()) {
        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               10523),
            true);
        guard << 569;
        guard << user->GetUID();
        SIG_SAVE_GAME_OPTION_QUICKCHAT* const saved =
            guard.GetInBuffer<SIG_SAVE_GAME_OPTION_QUICKCHAT>();
        saved->m_id = user->get_acc_id();
        CQuickChattingOption* const first =
            reinterpret_cast<CQuickChattingOption*>(saved->option_3);
        game_option->CopyQuickChatting(*first, 0);
        game_option->CopyQuickChatting(
            *reinterpret_cast<CQuickChattingOption*>(
                saved->option_3 + sizeof(CQuickChattingOption)),
            1);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        game_option->resetFlag_QuickChatChnage();
    }
    return 0;
}

// IDA_EA: 0x8597BB2
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticonC2Ev
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::Dispatcher_SaveGameOption_ChattingEmoticon(void)
Dispatcher_SaveGameOption_ChattingEmoticon::
    Dispatcher_SaveGameOption_ChattingEmoticon()
    : IPacketDispatcher<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8245B1A
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticonD2Ev
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::~Dispatcher_SaveGameOption_ChattingEmoticon()
// IDA_EA: 0x8245B4A
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticonD0Ev
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::~Dispatcher_SaveGameOption_ChattingEmoticon()
Dispatcher_SaveGameOption_ChattingEmoticon::
    ~Dispatcher_SaveGameOption_ChattingEmoticon() {}

// IDA_EA: 0x81CF354
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticon4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf& packet,
                                                      MSG_BASE& message) {
    MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON& input =
        static_cast<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON&>(message);
    if (packet.get_short(input.slot_count) != 1) {
        return LineFunc(
            10548,
            "virtual int Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }

    for (int index = 0;
         static_cast<int16_t>(input.slot_count) > index;
         ++index) {
        MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String slot;
        if (packet.get_short(slot.word_00) != 1) {
            return LineFunc(
                10554,
                "virtual int Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }

        int text_length;
        if (packet.get_int(text_length) != 1) {
            return LineFunc(
                10560,
                "virtual int Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }

        char text[20];
        if (packet.get_str(text, sizeof(text), text_length) != 1) {
            return LineFunc(
                10563,
                "virtual int Dispatcher_SaveGameOption_ChattingEmoticon::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }
        *reinterpret_cast<std::string*>(slot.text_02) = text;
        input.slots.push_back(slot);
    }
    return 0;
}

// IDA_EA: 0x81CF52A
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticon7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveGameOption_ChattingEmoticon::process(CUser* user,
                                                         MSG_BASE& message,
                                                         ParamBase& param) {
    (void)param;
    const MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON& input =
        static_cast<const MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON&>(message);
    CGameOption* const game_option = user->GetGameOptionRef();
    CChattingEmoticonConfig config = game_option->getEmoticonInfo();

    for (std::vector<MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String>::
             const_iterator it = input.slots.begin();
         it != input.slots.end();
         ++it) {
        const std::string& text =
            *reinterpret_cast<const std::string*>(it->text_02);
        // The observed handler intentionally trusts the client slot number.
        std::strncpy(config.rows[it->word_00].text,
                     text.c_str(),
                     sizeof(config.rows[it->word_00].text));
    }

    if (game_option->isEqualEmoticon(config)) {
        return 0;
    }
    game_option->SetEmoticonInfo(config);
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           10585),
        true);
    guard << 727;
    guard << user->GetUID();
    SIG_SAVE_CHATTING_EMOTICON* const saved =
        guard.GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>();
    saved->m_id = user->get_acc_id();
    std::memcpy(saved->shortcut_emoticon,
                &config,
                sizeof(saved->shortcut_emoticon));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81CF71E
// IDA_NAME: _ZN42Dispatcher_SaveGameOption_ChattingEmoticon4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SaveGameOption_ChattingEmoticon::send(CUser *,ParamBase &)
void Dispatcher_SaveGameOption_ChattingEmoticon::send(CUser* user,
                                                       ParamBase& param) {
    (void)param;
    user->SendCmdOkPacket(ENUM_CMDPACKET_SAVE_GAME_OPTION_CHATTING_EMOTICON);
    user->ReCalcChattingEmoticon();
    user->SendChattingEmoticon();
}
