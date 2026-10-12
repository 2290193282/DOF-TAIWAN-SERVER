#include "old_game/player/setting/character_option_dispatchers.h"

#include "common/config.h"
#include "old_game/player/setting/character_option.h"
#include "old_data/db/character/db_req_query_charac_info.h"
#include "old_data/db/mail/db_req_query_charac_info_mailbox.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

extern char* trim(char* text)
    __asm__("_Z4trimPc");

// IDA_EA: 0x81CE8BC
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SaveCharacterOption::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SaveCharacterOption::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    MSG_SAVE_CHARACTER_OPTION& request =
        static_cast<MSG_SAVE_CHARACTER_OPTION&>(message);
    unsigned int option_size = 0;
    if (packet.get_int(option_size) != 1 ||
        option_size > sizeof(request.option_11)) {
        return LineFunc(
            10281,
            "virtual int Dispatcher_SaveCharacterOption::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    request.option_size = option_size;
    if (packet.get_binary(reinterpret_cast<char*>(&request.option_11),
                          static_cast<int>(option_size)) == 1) {
        return 0;
    }
    return LineFunc(
        10283,
        "virtual int Dispatcher_SaveCharacterOption::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81CE97C
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SaveCharacterOption::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SaveCharacterOption::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    int32_t& result = static_cast<stSaveCharacOption&>(param).dword_04;
    result = check_error(user, message);
    if (result > 0) {
        return 0;
    }
    if (result >= 0) {
        user->setSaveCharacOption(true);
        MSG_SAVE_CHARACTER_OPTION& request =
            static_cast<MSG_SAVE_CHARACTER_OPTION&>(message);
        user->SetCurCharacOption(request.option_11);
        return 0;
    }

    return LineFunc(
        10301,
        "virtual int Dispatcher_SaveCharacterOption::process(CUser*, MSG_BASE&, ParamBase&)",
        result,
        user->get_acc_id());
}

// IDA_EA: 0x81CEA28
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOption11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SaveCharacterOption::check_error(CUser *,MSG_BASE &)
int Dispatcher_SaveCharacterOption::check_error(CUser* user,
                                                MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 1) {
        return -2;
    }
    if (user->getCurCharacNo()) {
        return 0;
    }
    return -3;
}

// IDA_EA: 0x8596818
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOptionC2Ev
// DEMANGLED: Dispatcher_SaveCharacterOption::Dispatcher_SaveCharacterOption(void)
Dispatcher_SaveCharacterOption::Dispatcher_SaveCharacterOption()
    : IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                        stSaveCharacOption,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8245C52
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOptionD2Ev
// DEMANGLED: Dispatcher_SaveCharacterOption::~Dispatcher_SaveCharacterOption()
// IDA_EA: 0x8245C82
// IDA_NAME: _ZN30Dispatcher_SaveCharacterOptionD0Ev
// DEMANGLED: Dispatcher_SaveCharacterOption::~Dispatcher_SaveCharacterOption()
Dispatcher_SaveCharacterOption::~Dispatcher_SaveCharacterOption() {}

// IDA_EA: 0x825F492
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotReset4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ComboSkillExtensionQuickSlotReset::read(PacketBuf& packet,
                                                       MSG_BASE& message) {
    MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET& request =
        static_cast<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET&>(message);
    unsigned char skill_tree = 0;
    if (packet.get_byte(skill_tree) == 1) {
        request.byte_0d = skill_tree;
        return 0;
    }
    return LineFunc(
        882,
        "virtual int Dispatcher_ComboSkillExtensionQuickSlotReset::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x825F4E8
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotReset7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ComboSkillExtensionQuickSlotReset::process(CUser* user,
                                                          MSG_BASE& message,
                                                          ParamBase& param) {
    int32_t& result =
        static_cast<stComboSkillExtensionQuickSlotReset&>(param).dword_04;
    result = check_error(user, message);
    if (result > 0) {
        return 0;
    }
    if (result < 0) {
        return LineFunc(
            900,
            "virtual int Dispatcher_ComboSkillExtensionQuickSlotReset::process(CUser*, MSG_BASE&, ParamBase&)",
            result,
            0);
    }

    const MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET& request =
        static_cast<const MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET&>(message);
    const int skill_tree = static_cast<signed char>(request.byte_0d);
    SkillSlot* skill_slot = user->getCurCharacSkillW();
    skill_slot->pullComboSkillExtensionQuickSlot(
        static_cast<ENUM_SKILL_TREE_KIND>(skill_tree));
    user->send_skill_info();
    return 0;
}

// IDA_EA: 0x825F594
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotReset11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::check_error(CUser *,MSG_BASE &)
int Dispatcher_ComboSkillExtensionQuickSlotReset::check_error(
    CUser* user,
    MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return -1;
    }
    if (user->get_charac_job() !=
        oldsrv::config::kCharacterJobDemonicSwordman) {
        return -1;
    }
    const MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET& request =
        static_cast<const MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET&>(message);
    if (request.byte_0d < 2u) {
        return 0;
    }
    return 19;
}

// IDA_EA: 0x825F60A
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotReset4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::send(CUser *,ParamBase &)
void Dispatcher_ComboSkillExtensionQuickSlotReset::send(CUser* user,
                                                        ParamBase& param) {
    const int32_t result =
        static_cast<stComboSkillExtensionQuickSlotReset&>(param).dword_04;
    if (result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
            static_cast<unsigned char>(result));
        return;
    }
    user->SendCmdOkPacket(
        ENUM_CMDPACKET_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET);
}

// IDA_EA: 0x8597E6E
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotResetC2Ev
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::Dispatcher_ComboSkillExtensionQuickSlotReset(void)
Dispatcher_ComboSkillExtensionQuickSlotReset::
    Dispatcher_ComboSkillExtensionQuickSlotReset()
    : IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                        stComboSkillExtensionQuickSlotReset,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82609F2
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotResetD2Ev
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::~Dispatcher_ComboSkillExtensionQuickSlotReset()
// IDA_EA: 0x8260A22
// IDA_NAME: _ZN44Dispatcher_ComboSkillExtensionQuickSlotResetD0Ev
// DEMANGLED: Dispatcher_ComboSkillExtensionQuickSlotReset::~Dispatcher_ComboSkillExtensionQuickSlotReset()
Dispatcher_ComboSkillExtensionQuickSlotReset::
    ~Dispatcher_ComboSkillExtensionQuickSlotReset() {}

// IDA_EA: 0x8205F68
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_QueryCharacInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_QueryCharacInfo::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            40800,
            "virtual int Dispatcher_QueryCharacInfo::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    int name_len = 0;
    char name[30];
    std::memset(name, 0, sizeof(name));
    if (packet.get_int(name_len) != 1) {
        return LineFunc(
            40805,
            "virtual int Dispatcher_QueryCharacInfo::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_str(name, 30, name_len) != 1) {
        return LineFunc(
            40806,
            "virtual int Dispatcher_QueryCharacInfo::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    const char* cur_name = user->getCurCharacName();
    if (!std::strncmp(name, cur_name, 0x1du)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_QUERY_CHARAC_INFO, 7);
    } else {
        DB_ReqQueryCharacInfo::makeRequest(user->GetUID(), name);
    }
    return 0;
}

// IDA_EA: 0x823389E
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_QueryCharacInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_QueryCharacInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82338A8
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_QueryCharacInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_QueryCharacInfo::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85961C0
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfoC2Ev
// DEMANGLED: Dispatcher_QueryCharacInfo::Dispatcher_QueryCharacInfo(void)
Dispatcher_QueryCharacInfo::Dispatcher_QueryCharacInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242922
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfoD2Ev
// DEMANGLED: Dispatcher_QueryCharacInfo::~Dispatcher_QueryCharacInfo()
// IDA_EA: 0x8242952
// IDA_NAME: _ZN26Dispatcher_QueryCharacInfoD0Ev
// DEMANGLED: Dispatcher_QueryCharacInfo::~Dispatcher_QueryCharacInfo()
Dispatcher_QueryCharacInfo::~Dispatcher_QueryCharacInfo() {}

// IDA_EA: 0x8226BDA
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailbox12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_QueryCharacInfoMailbox::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            57993,
            "virtual int Dispatcher_QueryCharacInfoMailbox::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    int name_len = 0;
    char name[30];
    std::memset(name, 0, sizeof(name));
    if (packet.get_int(name_len) != 1) {
        return LineFunc(
            57998,
            "virtual int Dispatcher_QueryCharacInfoMailbox::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_str(name, 30, name_len) != 1) {
        return LineFunc(
            57999,
            "virtual int Dispatcher_QueryCharacInfoMailbox::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    const char* cur_name = user->getCurCharacName();
    const char* trimmed_name = trim(name);
    if (!std::strncmp(trimmed_name, cur_name, 0x1du)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_QUERY_CHARAC_INFO_MAILBOX, 7);
    } else {
        DB_ReqQueryCharacInfoMailbox::makeRequest(user->GetUID(), name);
    }
    return 0;
}

// IDA_EA: 0x823423A
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailbox4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::read(PacketBuf &,MSG_BASE &)
int Dispatcher_QueryCharacInfoMailbox::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234244
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailbox7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_QueryCharacInfoMailbox::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85972F2
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailboxC2Ev
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::Dispatcher_QueryCharacInfoMailbox(void)
Dispatcher_QueryCharacInfoMailbox::Dispatcher_QueryCharacInfoMailbox()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240444
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailboxD2Ev
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::~Dispatcher_QueryCharacInfoMailbox()
// IDA_EA: 0x8240474
// IDA_NAME: _ZN33Dispatcher_QueryCharacInfoMailboxD0Ev
// DEMANGLED: Dispatcher_QueryCharacInfoMailbox::~Dispatcher_QueryCharacInfoMailbox()
Dispatcher_QueryCharacInfoMailbox::~Dispatcher_QueryCharacInfoMailbox() {}

// IDA_EA: 0x82251F4
// IDA_NAME: _ZN29Dispatcher_Change_Charac_Slot12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Change_Charac_Slot::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Change_Charac_Slot::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (user->get_state() != 2) {
        return 0;
    }

    unsigned int src_slot = 0;
    unsigned int dst_slot = 0;
    if (packet.get_int(src_slot) != 1 || packet.get_int(dst_slot) != 1) {
        return LineFunc(
            57333,
            "virtual int Dispatcher_Change_Charac_Slot::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    bool locked = static_cast<unsigned char>(user->isLocked4DataLoad()) ||
                  static_cast<unsigned char>(user->isLockedCreateCharac());
    locked = locked ||
             static_cast<unsigned char>(user->isLockedDeleteCharac());
    if (!locked) {
        user->ChangeCharacSlot(src_slot, dst_slot);
    }
    return 0;
}

// IDA_EA: 0x823414A
// IDA_NAME: _ZN29Dispatcher_Change_Charac_Slot4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Change_Charac_Slot::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Change_Charac_Slot::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234154
// IDA_NAME: _ZN29Dispatcher_Change_Charac_Slot7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Change_Charac_Slot::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Change_Charac_Slot::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597084
// IDA_NAME: _ZN29Dispatcher_Change_Charac_SlotC2Ev
// DEMANGLED: Dispatcher_Change_Charac_Slot::Dispatcher_Change_Charac_Slot(void)
Dispatcher_Change_Charac_Slot::Dispatcher_Change_Charac_Slot()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82406B4
// IDA_NAME: _ZN29Dispatcher_Change_Charac_SlotD2Ev
// DEMANGLED: Dispatcher_Change_Charac_Slot::~Dispatcher_Change_Charac_Slot()
// IDA_EA: 0x82406E4
// IDA_NAME: _ZN29Dispatcher_Change_Charac_SlotD0Ev
// DEMANGLED: Dispatcher_Change_Charac_Slot::~Dispatcher_Change_Charac_Slot()
Dispatcher_Change_Charac_Slot::~Dispatcher_Change_Charac_Slot() {}

// IDA_EA: 0x81D7FAC
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffect4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CharacSlotExtendEffect::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D7FB6
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffect7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CharacSlotExtendEffect::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)message;
    stCharacSlotExtendEffect& result =
        static_cast<stCharacSlotExtendEffect&>(param);
    if (user->get_state() != 2) {
        result.result_code = 7;
        return 0;
    }

    const unsigned char effect_count = user->getSlotEffectCount();
    const unsigned char slot_limit = user->getCharacSlotLimit();
    if (effect_count >= slot_limit) {
        const char* acc_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace(
            "virtual int Dispatcher_CharacSlotExtendEffect::process(CUser*, MSG_BASE&, ParamBase&)",
            17041,
            0);
        trace("CharacSlotExtendEffect m_id(%s), SlotEffectCount(%u), "
              "CharacSlotLimit(%u)",
              acc_id_text,
              static_cast<unsigned int>(user->getSlotEffectCount()),
              static_cast<unsigned int>(user->getCharacSlotLimit()));
        result.result_code = 18;
        return 0;
    }

    user->setSlotEffectCount(user->getCharacSlotLimit());
    result.slot_effect_count = user->getSlotEffectCount();
    result.charac_slot_limit = user->getCharacSlotLimit();

    Stream* stream = GlobalData::stream_pool()->Acquire(
        "PacketDispatcher_Impl_1.cpp", 17050);
    CStreamGuard guard(stream, true);
    *guard << 650;
    *guard << -1;
    SIG_EXTEND_CHARAC_SLOT* const payload =
        guard.GetInBuffer<SIG_EXTEND_CHARAC_SLOT>();
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->slot_effect_count = user->getSlotEffectCount();
    payload->charac_slot_limit = user->getCharacSlotLimit();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81D81E6
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffect4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::send(CUser *,ParamBase &)
void Dispatcher_CharacSlotExtendEffect::send(CUser* user, ParamBase& param) {
    const stCharacSlotExtendEffect& result =
        static_cast<const stCharacSlotExtendEffect&>(param);
    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_CHARAC_SLOT_EXTEND_EFFECT);
    if (result.result_code) {
        response.put_byte(0);
        response.put_byte(result.result_code);
    } else {
        response.put_byte(1);
        response.put_byte(result.charac_slot_limit);
        response.put_byte(result.slot_effect_count);
    }
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x8597886
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffectC2Ev
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::Dispatcher_CharacSlotExtendEffect(void)
Dispatcher_CharacSlotExtendEffect::Dispatcher_CharacSlotExtendEffect()
    : IPacketDispatcher<MSG_BASE,
                        stCharacSlotExtendEffect,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244B90
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffectD2Ev
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::~Dispatcher_CharacSlotExtendEffect()
// IDA_EA: 0x8244BC0
// IDA_NAME: _ZN33Dispatcher_CharacSlotExtendEffectD0Ev
// DEMANGLED: Dispatcher_CharacSlotExtendEffect::~Dispatcher_CharacSlotExtendEffect()
Dispatcher_CharacSlotExtendEffect::~Dispatcher_CharacSlotExtendEffect() {}
