#include "old_game/progression/quest/packet_dispatcher_sr08_quest_skill_unclaimed.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <type_traits>

namespace {

const char kSelectThirdChronicleCheckState[] =
    "virtual int Dispatcher_Select_3rdChronicleItem_ForEnchant::check_state(CUser*, PacketBuf&)";
const char kBaseCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stEnchantParam, ch_state STATE = (ch_state)0u]";

}  // namespace

// IDA_EA: 0x823154C
// IDA_NAME: _ZN14stEnchantParamC2Ev
// DEMANGLED: stEnchantParam::stEnchantParam(void)
stEnchantParam::stEnchantParam() : ParamBase(), result(0) {}

// IDA_EA: 0x82470D8
// IDA_NAME: _ZN14stEnchantParamD2Ev
// DEMANGLED: stEnchantParam::~stEnchantParam()
// IDA_EA: 0x8247108
// IDA_NAME: _ZN14stEnchantParamD0Ev
// DEMANGLED: stEnchantParam::~stEnchantParam()
stEnchantParam::~stEnchantParam() {}

// The one old-server template instance is an ABI owner: do not make it a
// header-only implicit instantiation.  Its seven direct identities are kept
// here with the slot-370 concrete class.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// The raw 0x8252506 template intentionally does not call MSG_BASE::C2.  It
// treats the 13-byte C2S header as packet-decoder stack storage and hands that
// storage straight to read/process.  A normal automatic MSG_BASE object would add a
// constructor edge that is absent from the old binary, so retain this exact
// unconstructed, aligned storage boundary in the specialization.
// Declare the state specialization before dispatch_template mentions it.  This
// prevents the host compiler from implicitly instantiating the generic member
// first, while leaving the old server's specialized state behavior unchanged.
template <>
int IPacketDispatcher<MSG_BASE,
                      stEnchantParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_BASE,
                      stEnchantParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    typedef typename std::aligned_storage<sizeof(MSG_BASE),
                                          alignof(MSG_BASE)>::type
        RawMessageStorage;
    RawMessageStorage storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&storage);

    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stEnchantParam param;
    result = process(user, message, static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// The state-0 generic body is behaviorally shared, but its old diagnostic
// literal is part of the recovered evidence.  Keep it independent of the
// host compiler's spelling of __PRETTY_FUNCTION__.
template <>
int IPacketDispatcher<MSG_BASE,
                      stEnchantParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    const int packet_len = packet.get_len();
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kBaseCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet_len);
    return 149;
}

// IDA_EA: 0x8252506
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8252656
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x825D7F0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x8252660
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825C730
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EED2Ev
// IDA_EA: 0x825C760
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EED0Ev
// IDA_EA: 0x8598B56
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE14stEnchantParamL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_BASE,
                                 stEnchantParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597602
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantC2Ev
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::Dispatcher_Select_3rdChronicleItem_ForEnchant(void)
Dispatcher_Select_3rdChronicleItem_ForEnchant::
    Dispatcher_Select_3rdChronicleItem_ForEnchant()
    : IPacketDispatcher<MSG_BASE, stEnchantParam, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8245418
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantD2Ev
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::~Dispatcher_Select_3rdChronicleItem_ForEnchant()
// IDA_EA: 0x8245448
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantD0Ev
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::~Dispatcher_Select_3rdChronicleItem_ForEnchant()
Dispatcher_Select_3rdChronicleItem_ForEnchant::
    ~Dispatcher_Select_3rdChronicleItem_ForEnchant() {}

// IDA_EA: 0x81D3416
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Select_3rdChronicleItem_ForEnchant::read(PacketBuf& packet,
                                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D3420
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Select_3rdChronicleItem_ForEnchant::process(CUser* user,
                                                             MSG_BASE& message,
                                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8232854
// IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant11check_stateEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::check_state(CUser *,PacketBuf &)
int Dispatcher_Select_3rdChronicleItem_ForEnchant::check_state(CUser* user,
                                                                 PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(1505, kSelectThirdChronicleCheckState, 0, 0);
    }
    if (user->CheckInTrade()) {
        return LineFunc(1508, kSelectThirdChronicleCheckState, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x85C3D08
// IDA_NAME: _ZN20CPremiumLetheManagerC2Ev
// DEMANGLED: CPremiumLetheManager::CPremiumLetheManager(void)
CPremiumLetheManager::CPremiumLetheManager() {}

// IDA_EA: 0x85C3D0E
// IDA_NAME: _ZN20CPremiumLetheManagerD2Ev
// DEMANGLED: CPremiumLetheManager::~CPremiumLetheManager()
CPremiumLetheManager::~CPremiumLetheManager() {}

// IDA_EA: 0x85C3D14
// IDA_NAME: _ZN20CPremiumLetheManager13ResetSkillReqEP5CUser
// DEMANGLED: CPremiumLetheManager::ResetSkillReq(CUser *)
int CPremiumLetheManager::ResetSkillReq(CUser* user) {
    if (!user->getCurCharacR()) {
        return 0;
    }
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
        return 0;
    }
    _resetSkill(user);
    return 1;
}

// IDA_EA: 0x85C40AA
// IDA_NAME: _ZN20CPremiumLetheManager11_resetSkillEP5CUser
// DEMANGLED: CPremiumLetheManager::_resetSkill(CUser *)
void CPremiumLetheManager::_resetSkill(CUser* user) {
    const int skill_tree_index = user->GetCurCharacSkillTreeIndex();
    const uint8_t lethe_state =
        (skill_tree_index == -1 || skill_tree_index == 0)
            ? static_cast<uint8_t>(user->GetOneDayLetheFlag())
            : static_cast<uint8_t>(user->GetOneDayLetheFlag2ND());

    if (lethe_state == USE_LETHE_STATE) {
        if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
            InitSkill(user,
                      true,
                      static_cast<ENUM_SKILL_TREE_KIND>(skill_tree_index));
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_ONEDAY_LETHE);
            packet.put_byte(1);
            packet.finalize(true);
            user->Send(packet);
        }
        return;
    }

    cMyTrace trace("void CPremiumLetheManager::_resetSkill(CUser*)", 147, 0);
    trace("ONE_DAY_LETHE : CAN'T USE INIT STATE");
}
