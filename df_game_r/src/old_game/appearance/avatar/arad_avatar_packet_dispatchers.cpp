#include "old_game/appearance/avatar/arad_avatar_packet_dispatchers.h"

#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/economy/inventory/avatar_coin.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace arad {

// IDA_EA: 0x81A092E
// IDA_NAME: _ZN4arad17SigAvatarRouletteC2Ev
// DEMANGLED: arad::SigAvatarRoulette::SigAvatarRoulette(void)
SigAvatarRoulette::SigAvatarRoulette() {
    init();
}

// IDA_EA: 0x81A0942
// IDA_NAME: _ZN4arad17SigAvatarRoulette4initEv
// DEMANGLED: arad::SigAvatarRoulette::init(void)
SigAvatarRoulette* SigAvatarRoulette::init() {
    roulette_type = 0;
    select_no = 0;
    zeroed_unused_byte = 0;
    return this;
}

}  // namespace arad

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x8597BEA
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::Dispatcher_AvartarRoulette(void)
Dispatcher_AvartarRoulette::Dispatcher_AvartarRoulette()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0E3E
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::~Dispatcher_AvartarRoulette()
// IDA_EA: 0x81A0E6E
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRouletteD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::~Dispatcher_AvartarRoulette()
Dispatcher_AvartarRoulette::~Dispatcher_AvartarRoulette() {}

// IDA_EA: 0x819EF54
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AvartarRoulette::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user == 0) {
        return 786;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->CheckInTrade() ||
        G_GameWorld()->IsPVPChannel() || G_GameWorld()->IsFreePvPChannel()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 7);
        return 0;
    }

    arad::SigAvatarRoulette payload;
    if (packet.get_byte(payload.roulette_type) != 1) {
        return 815;
    }
    if (packet.get_byte(payload.select_no) != 1) {
        return 817;
    }
    if (payload.roulette_type > 10u) {
        return 820;
    }
    if (payload.select_no > 9u) {
        return 823;
    }
    if (checkWorkState(user, payload) != 1) {
        return 0;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 839),
                       true);
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(44), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x819EE6C
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette14checkWorkStateEP5CUserRN4arad17SigAvatarRouletteE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::checkWorkState(CUser *,arad::SigAvatarRoulette &)
int Dispatcher_AvartarRoulette::checkWorkState(
    CUser* user,
    arad::SigAvatarRoulette& payload) {
    (void)payload;
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(69));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE,
                                 static_cast<unsigned char>(protection_result));
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenR();
    AvatarCoin* const avatar_coin = &inventory->avatar_coin_;
    if (!avatar_coin->IsUsable()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 22);
        return 0;
    }
    if (inventory->GetRemainCapacity(static_cast<INVEN_TYPE>(2),
                                      static_cast<Inven_Item::ITEM_TYPE>(0)) <=
        0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 4);
        return 0;
    }
    return 1;
}

// IDA_EA: 0x81A0AE6
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AvartarRoulette::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0AF0
// IDA_NAME: _ZN4ARAD10DISPATCHER26Dispatcher_AvartarRoulette7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarRoulette::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AvartarRoulette::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597C06
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::Dispatcher_AvartarCoinCount(void)
Dispatcher_AvartarCoinCount::Dispatcher_AvartarCoinCount()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0DF0
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::~Dispatcher_AvartarCoinCount()
// IDA_EA: 0x81A0E20
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCountD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::~Dispatcher_AvartarCoinCount()
Dispatcher_AvartarCoinCount::~Dispatcher_AvartarCoinCount() {}

// IDA_EA: 0x819F1BC
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AvartarCoinCount::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    return AvatarCoin::SendSyncPacket(user) == 1 ? 0 : 851;
}

// IDA_EA: 0x819F1E2
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount14checkWorkStateEP5CUser
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::checkWorkState(CUser *)
int Dispatcher_AvartarCoinCount::checkWorkState(CUser* user) {
    (void)user;
    return 1;
}

// IDA_EA: 0x81A0AFA
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AvartarCoinCount::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B04
// IDA_NAME: _ZN4ARAD10DISPATCHER27Dispatcher_AvartarCoinCount7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarCoinCount::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AvartarCoinCount::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597C22
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::Dispatcher_AvartarHiddenOptionChange(void)
Dispatcher_AvartarHiddenOptionChange::Dispatcher_AvartarHiddenOptionChange()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0D54
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::~Dispatcher_AvartarHiddenOptionChange()
// IDA_EA: 0x81A0D84
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChangeD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::~Dispatcher_AvartarHiddenOptionChange()
Dispatcher_AvartarHiddenOptionChange::~Dispatcher_AvartarHiddenOptionChange() {}

// IDA_EA: 0x819F6F8
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AvartarHiddenOptionChange::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    if (user == 0) {
        return 1586;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4 ||
        G_GameWorld()->IsPVPChannel() || G_GameWorld()->IsFreePvPChannel() ||
        user->CheckInTrade()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                                 7);
        return 0;
    }

    arad::SigAvatarHiddenOptionChange payload;
    short material_slot = 0;
    if (packet.get_short(material_slot) != 1) {
        return 1613;
    }
    payload.material_slot = static_cast<uint16_t>(material_slot);
    short avatar_slot = 0;
    if (packet.get_short(avatar_slot) != 1) {
        return 1617;
    }
    payload.avatar_slot = static_cast<uint16_t>(avatar_slot);
    if (checkWorkState(user, payload) != 1) {
        return 0;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 1625),
                       true);
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(46), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x819F956
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange14checkWorkStateEP5CUserRN4arad27SigAvatarHiddenOptionChangeE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::checkWorkState(CUser *,arad::SigAvatarHiddenOptionChange &)
int Dispatcher_AvartarHiddenOptionChange::checkWorkState(
    CUser* user,
    arad::SigAvatarHiddenOptionChange& payload) {
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
            static_cast<unsigned char>(protection_result));
        return 0;
    }
    return ARAD::Singleton<AvatarFixedHiddenOptionServer>::Get()
                   ->CheckCondition(user, &payload) ==
           1;
}

// IDA_EA: 0x81A0B0E
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AvartarHiddenOptionChange::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B18
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_AvartarHiddenOptionChange7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AvartarHiddenOptionChange::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597C3E
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::Dispatcher_AradUseAvatarRecharge(void)
Dispatcher_AradUseAvatarRecharge::Dispatcher_AradUseAvatarRecharge()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0D06
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::~Dispatcher_AradUseAvatarRecharge()
// IDA_EA: 0x81A0D36
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRechargeD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::~Dispatcher_AradUseAvatarRecharge()
Dispatcher_AradUseAvatarRecharge::~Dispatcher_AradUseAvatarRecharge() {}

// IDA_EA: 0x819FA56
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AradUseAvatarRecharge::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    if (user == 0) {
        return 1692;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4 ||
        G_GameWorld()->IsPVPChannel() || G_GameWorld()->IsFreePvPChannel() ||
        user->CheckInTrade()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN, 7);
        return 0;
    }

    arad::SigAradUseAvatarRecharge payload;
    if (packet.get_short(payload.recharge_stone_slot) != 1) {
        return 1718;
    }
    if (packet.get_short(payload.avatar_slot) != 1) {
        return 1722;
    }
    if (checkWorkState(user, payload) == 0) {
        return 0;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 1730),
                       true);
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(48), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x819F9D6
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge14checkWorkStateEP5CUserRN4arad24SigAradUseAvatarRechargeE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::checkWorkState(CUser *,arad::SigAradUseAvatarRecharge &)
int Dispatcher_AradUseAvatarRecharge::checkWorkState(
    CUser* user,
    arad::SigAradUseAvatarRecharge& payload) {
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 static_cast<unsigned char>(protection_result));
        return 0;
    }
    return ARAD::Singleton<AvatarRechargeServer>::Get()
                   ->CheckConditionUseRecharge(user, &payload) ==
           1;
}

// IDA_EA: 0x81A0B36
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AradUseAvatarRecharge::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B40
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_AradUseAvatarRecharge7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AradUseAvatarRecharge::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597C76
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::Dispatcher_AradEmblemCompound(void)
Dispatcher_AradEmblemCompound::Dispatcher_AradEmblemCompound()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0CB8
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::~Dispatcher_AradEmblemCompound()
// IDA_EA: 0x81A0CE8
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompoundD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::~Dispatcher_AradEmblemCompound()
Dispatcher_AradEmblemCompound::~Dispatcher_AradEmblemCompound() {}

// IDA_EA: 0x819FD7E
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AradEmblemCompound::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    if (user == 0) {
        return 1777;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4 ||
        G_GameWorld()->IsPVPChannel() || G_GameWorld()->IsFreePvPChannel() ||
        user->CheckInTrade()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 7);
        return 0;
    }

    const int socket_action_result = user->isEnableAvatarSocketAction();
    if (socket_action_result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN,
            static_cast<unsigned char>(socket_action_result));
        return 0;
    }

    arad::SigAradEmblemCompound payload;
    if (packet.get_short(payload.materials[0].slot) != 1) {
        return 1811;
    }
    if (packet.get_short(payload.materials[1].slot) != 1) {
        return 1815;
    }
    if (packet.get_short(payload.select_no) != 1) {
        return 1819;
    }
    if (checkWorkState(user, payload) == 0) {
        return 0;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 1827),
                       true);
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(50), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x819FCB4
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound14checkWorkStateEP5CUserRN4arad21SigAradEmblemCompoundE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::checkWorkState(CUser *,arad::SigAradEmblemCompound &)
int Dispatcher_AradEmblemCompound::checkWorkState(
    CUser* user,
    arad::SigAradEmblemCompound& payload) {
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 21) != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0xd1u);
        return 0;
    }

    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN,
                                 static_cast<unsigned char>(protection_result));
        return 0;
    }
    return ARAD::Singleton<EmblemCompoundServer>::Get()
                   ->CheckCondition(user, &payload) ==
           1;
}

// IDA_EA: 0x81A0B4A
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AradEmblemCompound::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B54
// IDA_NAME: _ZN4ARAD10DISPATCHER29Dispatcher_AradEmblemCompound7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradEmblemCompound::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AradEmblemCompound::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597C5A
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::Dispatcher_AradAvatarConvert(void)
Dispatcher_AradAvatarConvert::Dispatcher_AradAvatarConvert()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0C6A
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::~Dispatcher_AradAvatarConvert()
// IDA_EA: 0x81A0C9A
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvertD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::~Dispatcher_AradAvatarConvert()
Dispatcher_AradAvatarConvert::~Dispatcher_AradAvatarConvert() {}

// IDA_EA: 0x81A00BE
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AradAvatarConvert::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    if (user == 0) {
        return 1866;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4) {
        return 0;
    }
    if (G_GameWorld()->IsPVPChannel() || G_GameWorld()->IsFreePvPChannel() ||
        user->CheckInTrade()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 7);
        return 0;
    }

    const int socket_action_result = user->isEnableAvatarSocketAction();
    if (socket_action_result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_AVATAR_CONVERT_JPN,
            static_cast<unsigned char>(socket_action_result));
        return 0;
    }

    arad::SigAradAvatarConvert payload;
    if (packet.get_short(payload.select_no) != 1) {
        return 1901;
    }
    if (packet.get_short(payload.avatar_slot) != 1) {
        return 1905;
    }
    unsigned long input_item_index = 0;
    if (packet.get_int(input_item_index) != 1) {
        return 1909;
    }
    payload.input_item_index = static_cast<uint32_t>(input_item_index);
    if (checkWorkState(user, payload) == 0) {
        return 0;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 1920),
                       true);
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(51), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x81A003E
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert14checkWorkStateEP5CUserRN4arad20SigAradAvatarConvertE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::checkWorkState(CUser *,arad::SigAradAvatarConvert &)
int Dispatcher_AradAvatarConvert::checkWorkState(
    CUser* user,
    arad::SigAradAvatarConvert& payload) {
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN,
                                 static_cast<unsigned char>(protection_result));
        return 0;
    }
    return ARAD::Singleton<AvatarConvertServer>::Get()
                   ->CheckCondition(user, &payload) ==
           1;
}

// IDA_EA: 0x81A0B5E
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AradAvatarConvert::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B68
// IDA_NAME: _ZN4ARAD10DISPATCHER28Dispatcher_AradAvatarConvert7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradAvatarConvert::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AradAvatarConvert::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
