#include "old_game/economy/inventory/packet_dispatcher_item_actions.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/economy/equipment/equipment_growth_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85953F8
// IDA_NAME: _ZN19DisPatcher_CompoundC2Ev
// DEMANGLED: DisPatcher_Compound::DisPatcher_Compound(void)
DisPatcher_Compound::DisPatcher_Compound() {}

// IDA_EA: 0x8232AA6
// IDA_NAME: _ZN19DisPatcher_CompoundD2Ev
// DEMANGLED: DisPatcher_Compound::~DisPatcher_Compound()
// IDA_EA: 0x8232AD6
// IDA_NAME: _ZN19DisPatcher_CompoundD0Ev
// DEMANGLED: DisPatcher_Compound::~DisPatcher_Compound()
DisPatcher_Compound::~DisPatcher_Compound() {}

// IDA_EA: 0x81EAA5C
// IDA_NAME: _ZN19DisPatcher_Compound12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_Compound::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_Compound::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(
            user, RestrictCategory::AuthMobile, 9)) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPOUND_ITEM, 209));
        return 0;
    }

    unsigned long recipe_slot = 0;
    if (packet.get_int(recipe_slot) != 1) {
        return LineFunc(
            27706,
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    unsigned char avatar_recipe = 0;
    if (packet.get_byte(avatar_recipe) != 1) {
        return LineFunc(
            27707,
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    unsigned short compound_count = 0;
    if (packet.get_short(compound_count) != 1) {
        return LineFunc(
            27709,
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!compound_count) {
        return LineFunc(
            27712,
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    // 140828 CLIENT_IDA_EVIDENCE: sub_11997E0 writes two additional bytes
    // after compound_count, constants 0 and 1 at 0x1199890/0x119989E, then
    // immediately finalizes the packet.  The 130627 owner stopped after the
    // count and silently tolerated this native tail.
    uint8_t reserved_zero = 0xffu;
    uint8_t reserved_one = 0xffu;
    if (packet.get_byte(reserved_zero) != 1 ||
        packet.get_byte(reserved_one) != 1 ||
        reserved_zero != 0u || reserved_one != 1u) {
        return LineFunc(
            27713,
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    {
        cMyTrace trace(
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            27714,
            0);
        trace("[RECIPE][CMD25_INGRESS] charac_no=%d slot=%lu avatar=%u count=%u tail=%u/%u",
              user->get_charac_no(-1),
              recipe_slot,
              static_cast<unsigned int>(avatar_recipe),
              static_cast<unsigned int>(compound_count),
              static_cast<unsigned int>(reserved_zero),
              static_cast<unsigned int>(reserved_one));
    }
    if (avatar_recipe != 1 &&
        user->CheckItemLock(1, static_cast<int>(recipe_slot))) {
        cMyTrace trace(
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            27715,
            5);
        trace("[RECIPE][CMD25_RESULT] charac_no=%d slot=%lu avatar=%u count=%u result=213 stage=item_lock",
              user->get_charac_no(-1),
              recipe_slot,
              static_cast<unsigned int>(avatar_recipe),
              static_cast<unsigned int>(compound_count));
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPOUND_ITEM, 213));
        return 0;
    }

    const int result = user->compound_item(
        static_cast<int>(recipe_slot),
        avatar_recipe == 1,
        compound_count,
        static_cast<int>(ENUM_CMDPACKET_COMPOUND_ITEM));
    {
        cMyTrace trace(
            "virtual int DisPatcher_Compound::dispatch_sig(CUser*, PacketBuf&)",
            27716,
            result > 0 ? 5 : 0);
        trace("[RECIPE][CMD25_RESULT] charac_no=%d slot=%lu avatar=%u count=%u result=%d stage=owner_return",
              user->get_charac_no(-1),
              recipe_slot,
              static_cast<unsigned int>(avatar_recipe),
              static_cast<unsigned int>(compound_count),
              result);
    }
    if (result <= 0) {
        if (user && result == 0) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                user,
                static_cast<APSystem::_ActionGroupIndex>(6),
                0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
        return 0;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_COMPOUND_ITEM);
    response.put_byte(0);
    response.put_byte(result);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8232A92
// IDA_NAME: _ZN19DisPatcher_Compound4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_Compound::read(PacketBuf &,MSG_BASE &)
int DisPatcher_Compound::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232A9C
// IDA_NAME: _ZN19DisPatcher_Compound7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_Compound::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_Compound::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596540
// IDA_NAME: _ZN23Dispatcher_AgreeEnchantC2Ev
// DEMANGLED: Dispatcher_AgreeEnchant::Dispatcher_AgreeEnchant(void)
Dispatcher_AgreeEnchant::Dispatcher_AgreeEnchant() {}

// IDA_EA: 0x82420E8
// IDA_NAME: _ZN23Dispatcher_AgreeEnchantD2Ev
// DEMANGLED: Dispatcher_AgreeEnchant::~Dispatcher_AgreeEnchant()
// IDA_EA: 0x8242118
// IDA_NAME: _ZN23Dispatcher_AgreeEnchantD0Ev
// DEMANGLED: Dispatcher_AgreeEnchant::~Dispatcher_AgreeEnchant()
Dispatcher_AgreeEnchant::~Dispatcher_AgreeEnchant() {}

// IDA_EA: 0x820ADC6
// IDA_NAME: _ZN23Dispatcher_AgreeEnchant12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AgreeEnchant::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AgreeEnchant::dispatch_sig(CUser* user, PacketBuf& packet) {
    MSG_STATIC_COMMAND* const command =
        reinterpret_cast<MSG_STATIC_COMMAND*>(packet.get_packet(0));

    if (!user->getCurCharacR()) {
        return LineFunc(
            42737,
            "virtual int Dispatcher_AgreeEnchant::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->GetCurCharacExpertJob()) {
        return 0;
    }

    expert_job::CExpertJob* const expert_job =
        user->GetCurCharacExpertJob();
    if (expert_job->GetType() == 1) {
        expert_job::CEnchanter* const enchanter =
            static_cast<expert_job::CEnchanter*>(expert_job);
        static_cast<void>(enchanter->OnAgreeEnchant(user, command));
    }
    return 0;
}

// IDA_EA: 0x8233ABA
// IDA_NAME: _ZN23Dispatcher_AgreeEnchant4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AgreeEnchant::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AgreeEnchant::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233AC4
// IDA_NAME: _ZN23Dispatcher_AgreeEnchant7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AgreeEnchant::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AgreeEnchant::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859655C
// IDA_NAME: _ZN21Dispatcher_TryEnchantC2Ev
// DEMANGLED: Dispatcher_TryEnchant::Dispatcher_TryEnchant(void)
Dispatcher_TryEnchant::Dispatcher_TryEnchant() {}

// IDA_EA: 0x824209A
// IDA_NAME: _ZN21Dispatcher_TryEnchantD2Ev
// DEMANGLED: Dispatcher_TryEnchant::~Dispatcher_TryEnchant()
// IDA_EA: 0x82420CA
// IDA_NAME: _ZN21Dispatcher_TryEnchantD0Ev
// DEMANGLED: Dispatcher_TryEnchant::~Dispatcher_TryEnchant()
Dispatcher_TryEnchant::~Dispatcher_TryEnchant() {}

// IDA_EA: 0x820AE84
// IDA_NAME: _ZN21Dispatcher_TryEnchant12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_TryEnchant::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_TryEnchant::dispatch_sig(CUser* user, PacketBuf& packet) {
    char* const raw_command = packet.get_packet(0);

    if (!user->getCurCharacR()) {
        return LineFunc(
            42763,
            "virtual int Dispatcher_TryEnchant::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(31));
    if (security_result) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_TRY_ENCHANT,
            static_cast<unsigned char>(security_result)));
        return 0;
    }

    // CMD169 is byte-for-byte shared by the 0627 enchanter and the 0828
    // equipment-growth UI.  The growth owner claims it only when the u32
    // identity resolves to a parsed [equipment grow] prototype; every other
    // frame, including legacy action 2 close, remains on this original path.
    if (equipment_growth_runtime::TryHandleCommand(user, packet)) {
        return 0;
    }

    MSG_STATIC_COMMAND* const command =
        reinterpret_cast<MSG_STATIC_COMMAND*>(raw_command);
    const unsigned char mode = command->action_type;
    if (mode == 1) {
        if (!user->GetCurCharacExpertJob()) {
            static_cast<void>(user->SendCmdErrorPacket(
                ENUM_CMDPACKET_TRY_ENCHANT, 22));
            return 0;
        }
    } else if (mode == 2) {
        if (!user->GetCurCharacExpertJob()) {
            return 0;
        }
    } else {
        if (mode) {
            return LineFunc(
                42802,
                "virtual int Dispatcher_TryEnchant::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }

        expert_job::CExpertJob* const acquired =
            GlobalData::s_ExpertJobMgr->AcquireExpertJob(
                static_cast<ENUM_EXPERT_JOB_TYPE>(1));
        if (!acquired) {
            static_cast<void>(user->SendCmdErrorPacket(
                ENUM_CMDPACKET_TRY_ENCHANT, 22));
            return 0;
        }
        static_cast<void>(acquired->OnStartMission(user, user));
    }

    expert_job::CExpertJob* const expert_job =
        user->GetCurCharacExpertJob();
    if (expert_job->GetType() != 1) {
        return LineFunc(
            42807,
            "virtual int Dispatcher_TryEnchant::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    expert_job::CEnchanter* const enchanter =
        static_cast<expert_job::CEnchanter*>(expert_job);
    enchanter->OnTryEnchant(user, command);
    return 0;
}

// IDA_EA: 0x8233ACE
// IDA_NAME: _ZN21Dispatcher_TryEnchant4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TryEnchant::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TryEnchant::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233AD8
// IDA_NAME: _ZN21Dispatcher_TryEnchant7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TryEnchant::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TryEnchant::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596578
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchantC2Ev
// DEMANGLED: Dispatcher_PutItemForEnchant::Dispatcher_PutItemForEnchant(void)
Dispatcher_PutItemForEnchant::Dispatcher_PutItemForEnchant() {}

// IDA_EA: 0x824204C
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchantD2Ev
// DEMANGLED: Dispatcher_PutItemForEnchant::~Dispatcher_PutItemForEnchant()
// IDA_EA: 0x824207C
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchantD0Ev
// DEMANGLED: Dispatcher_PutItemForEnchant::~Dispatcher_PutItemForEnchant()
Dispatcher_PutItemForEnchant::~Dispatcher_PutItemForEnchant() {}

// IDA_EA: 0x820B09A
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchant12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PutItemForEnchant::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PutItemForEnchant::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    MSG_STATIC_COMMAND* const command =
        reinterpret_cast<MSG_STATIC_COMMAND*>(packet.get_packet(0));

    if (!user->getCurCharacR()) {
        return LineFunc(
            42820,
            "virtual int Dispatcher_PutItemForEnchant::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->GetCurCharacExpertJob()) {
        return 0;
    }

    expert_job::CExpertJob* const expert_job =
        user->GetCurCharacExpertJob();
    if (expert_job->GetType() != 1) {
        return LineFunc(
            42827,
            "virtual int Dispatcher_PutItemForEnchant::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    expert_job::CEnchanter* const enchanter =
        static_cast<expert_job::CEnchanter*>(expert_job);
    static_cast<void>(enchanter->OnPutItemForEnchant(user, command));
    return 0;
}

// IDA_EA: 0x8233AE2
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchant4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PutItemForEnchant::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PutItemForEnchant::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233AEC
// IDA_NAME: _ZN28Dispatcher_PutItemForEnchant7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PutItemForEnchant::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PutItemForEnchant::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596D04
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItemC2Ev
// DEMANGLED: Dispatcher_UseSharedEffectItem::Dispatcher_UseSharedEffectItem(void)
Dispatcher_UseSharedEffectItem::Dispatcher_UseSharedEffectItem() {}

// IDA_EA: 0x8240FD8
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItemD2Ev
// DEMANGLED: Dispatcher_UseSharedEffectItem::~Dispatcher_UseSharedEffectItem()
// IDA_EA: 0x8241008
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItemD0Ev
// DEMANGLED: Dispatcher_UseSharedEffectItem::~Dispatcher_UseSharedEffectItem()
Dispatcher_UseSharedEffectItem::~Dispatcher_UseSharedEffectItem() {}

// IDA_EA: 0x8220484
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UseSharedEffectItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UseSharedEffectItem::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    int item_id = 0;
    short slot = 0;
    if (packet.get_int(item_id) != 1) {
        return LineFunc(
            55577,
            "virtual int Dispatcher_UseSharedEffectItem::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            55578,
            "virtual int Dispatcher_UseSharedEffectItem::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    if (user->get_state() != 3 || !user->getCurCharacR()) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM, 1));
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const Inven_Item item = inventory->GetInvenSlot(1, slot);
    if (item.isEmpty() || item.get_add_info() <= 0) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM, 22));
        return 0;
    }
    if (item.getKey() != item_id) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM, 1));
        return 0;
    }

    CStackableItem* const stackable_item =
        static_cast<CStackableItem*>(G_CDataManager()->find_item(item_id));
    if (!stackable_item || stackable_item->IsSharedEffectItem() != 1) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM, 17));
        return 0;
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                        slot,
                                        1,
                                        static_cast<eItemDelReason>(3),
                                        true) != 1) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM, 1));
        return 0;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM);
    response.put_byte(1);
    response.put_short(slot);
    // The 140828 success reader consumes one reserved byte after the slot.
    response.put_byte(0);
    response.finalize(true);
    user->Send(response);

    response.clear();
    response.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                        ENUM_NOTIPACKET_USE_SHARED_EFFECT_ITEM);
    response.put_short(static_cast<short>(user->get_unique_id()));
    response.put_int(item_id);
    response.finalize(true);
    G_GameWorld()->send_all_with_state(response, 3);
    return 0;
}

// IDA_EA: 0x8233F2E
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseSharedEffectItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseSharedEffectItem::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F38
// IDA_NAME: _ZN30Dispatcher_UseSharedEffectItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseSharedEffectItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseSharedEffectItem::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
