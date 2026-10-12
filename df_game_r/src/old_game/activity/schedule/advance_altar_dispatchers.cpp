#include "old_game/activity/schedule/advance_altar_dispatchers.h"

#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/character_messages.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <new>


namespace advancealtar {
bool isOpenAdvanceAltar();
}

namespace CMDPacketStruct {

typedef char SetSlotData_Size_Must_Be_0x0c[
    (sizeof(_SetSlotData) == 0x0c) ? 1 : -1];
typedef char ReqAdvanceAltarSetSlot_Size_Must_Be_0x24[
    (sizeof(_ReqAdvanceAltarSetSlot) == 0x24) ? 1 : -1];
typedef char ResAdvanceAltarSetSlot_Size_Must_Be_0x1c[
    (sizeof(_ResAdvanceAltarSetSlot) == 0x1c) ? 1 : -1];
typedef char ReqAdvanceAltarExchangeSlot_Size_Must_Be_0x30[
    (sizeof(_ReqAdvanceAltarExchangeSlot) == 0x30) ? 1 : -1];
typedef char ReqAdvanceAltarUpgradeGage_Size_Must_Be_0x18[
    (sizeof(_ReqAdvanceAltarUpgradeGage) == 0x18) ? 1 : -1];
typedef char ResAdvanceAltarUpgradeGage_Size_Must_Be_0x14[
    (sizeof(_ResAdvanceAltarUpgradeGage) == 0x14) ? 1 : -1];
typedef char ReqAdvanceAltarSummonUnit_Size_Must_Be_0x18[
    (sizeof(_ReqAdvanceAltarSummonUnit) == 0x18) ? 1 : -1];
typedef char ResAdvanceAltarSummonUnit_Size_Must_Be_0x14[
    (sizeof(_ResAdvanceAltarSummonUnit) == 0x14) ? 1 : -1];
typedef char ReqAdvanceAltarGetAchievementReward_Size_Must_Be_0x14[
    (sizeof(_ReqAdvanceAltarGetAchievementReward) == 0x14) ? 1 : -1];
typedef char ResAdvanceAltarGetAchievementReward_Size_Must_Be_0x0c[
    (sizeof(_ResAdvanceAltarGetAchievementReward) == 0x0c) ? 1 : -1];
typedef char ReqAdvanceAltarResetStar_Size_Must_Be_0x14[
    (sizeof(_ReqAdvanceAltarResetStar) == 0x14) ? 1 : -1];
typedef char ResAdvanceAltarResetStar_Size_Must_Be_0x0c[
    (sizeof(_ResAdvanceAltarResetStar) == 0x0c) ? 1 : -1];
typedef char ResAdvanceAltarExchangeSlot_Size_Must_Be_0x24[
    (sizeof(_ResAdvanceAltarExchangeSlot) == 0x24) ? 1 : -1];

// IDA_EA: 0x812c0be
// IDA_NAME: _ZN15CMDPacketStruct14STBaseResponseC2Ev
// DEMANGLED: CMDPacketStruct::STBaseResponse::STBaseResponse(void)
STBaseResponse::STBaseResponse() : ParamBase() {}

// IDA_EA: 0x812c028
// IDA_NAME: _ZN15CMDPacketStruct14STBaseResponseD2Ev
// IDA_EA: 0x812c058
// IDA_NAME: _ZN15CMDPacketStruct14STBaseResponseD0Ev
// DEMANGLED: CMDPacketStruct::STBaseResponse::~STBaseResponse()
STBaseResponse::~STBaseResponse() {}

// IDA_EA: 0x8141D2E
// IDA_NAME: _ZN15CMDPacketStruct23_ReqAdvanceAltarSetSlotC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarSetSlot::_ReqAdvanceAltarSetSlot(void)
_ReqAdvanceAltarSetSlot::_ReqAdvanceAltarSetSlot()
    : STBaseRequest() {
    std::memset(this, 0, 0x24u);
}

// IDA_EA: 0x8141D5C
// IDA_NAME: _ZN15CMDPacketStruct23_ResAdvanceAltarSetSlotC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarSetSlot::_ResAdvanceAltarSetSlot(void)
_ResAdvanceAltarSetSlot::_ResAdvanceAltarSetSlot()
    : STBaseResponse() {
    std::memset(this, 0, 0x1cu);
}

// IDA_EA: 0x814256A
// IDA_NAME: _ZN15CMDPacketStruct23_ResAdvanceAltarSetSlotD2Ev
// IDA_EA: 0x814259A
// IDA_NAME: _ZN15CMDPacketStruct23_ResAdvanceAltarSetSlotD0Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarSetSlot::~_ResAdvanceAltarSetSlot()
_ResAdvanceAltarSetSlot::~_ResAdvanceAltarSetSlot() {}

// IDA_EA: 0x8141D94
// IDA_NAME: _ZN15CMDPacketStruct28_ReqAdvanceAltarExchangeSlotC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarExchangeSlot::_ReqAdvanceAltarExchangeSlot(void)
_ReqAdvanceAltarExchangeSlot::_ReqAdvanceAltarExchangeSlot()
    : STBaseRequest() {
    std::memset(this, 0, 0x30u);
}

// IDA_EA: 0x8141DC2
// IDA_NAME: _ZN15CMDPacketStruct28_ResAdvanceAltarExchangeSlotC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarExchangeSlot::_ResAdvanceAltarExchangeSlot(void)
_ResAdvanceAltarExchangeSlot::_ResAdvanceAltarExchangeSlot()
    : STBaseResponse() {
    std::memset(this, 0, 0x24u);
}

// IDA_EA: 0x8142654
// IDA_NAME: _ZN15CMDPacketStruct28_ResAdvanceAltarExchangeSlotD2Ev
// IDA_EA: 0x8142684
// IDA_NAME: _ZN15CMDPacketStruct28_ResAdvanceAltarExchangeSlotD0Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarExchangeSlot::~_ResAdvanceAltarExchangeSlot()
_ResAdvanceAltarExchangeSlot::~_ResAdvanceAltarExchangeSlot() {}

}  // namespace CMDPacketStruct

namespace advancealtar {

// MIGRATED_TO: src/old_game/event/advance_altar_db_exchange_runtime.cpp
// The original static advancealtar::DBAndExchangeServerProc member is no
// longer provided through a two-argument free __asm__ facade in this legacy
// source.

// IDA_EA: 0x813FF38
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlot12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_SetSlot::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SetSlot::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 ||
        !user->getCurCharacR()) {
        return 1020;
    }

    CMDPacketStruct::_ReqAdvanceAltarSetSlot request;
    CMDPacketStruct::_ResAdvanceAltarSetSlot response;
    read_packet(packet, request);
    const int result = process_packet(user, request, response);
    (void)result;
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x8140020
// IDA_NAME: _ZNK12advancealtar18Dispatcher_SetSlot11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_SetSlot::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_SetSlot::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 36);
    return 0;
}

// IDA_EA: 0x814004E
// IDA_NAME: _ZNK12advancealtar18Dispatcher_SetSlot11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_SetSlot::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_SetSlot::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return !isOpenAdvanceAltar();
}

// IDA_EA: 0x814006E
// IDA_NAME: _ZNK12advancealtar18Dispatcher_SetSlot14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_SetSlot::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_SetSlot::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request,
    CMDPacketStruct::STBaseResponse& response) const {
    CMDPacketStruct::_ReqAdvanceAltarSetSlot& req =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarSetSlot&>(request);
    CMDPacketStruct::_ResAdvanceAltarSetSlot& res =
        static_cast<CMDPacketStruct::_ResAdvanceAltarSetSlot&>(response);

    res.byte_0c = req.byte_14;
    // 0828 DNF.exe correlates the slot mutation with request +0x10 and reads
    // the echoed value from response +0x08.
    res.context_id_08 = req.context_id_10;
    std::memcpy(&res.slot_10, &req.slot_18, 0x0cu);
    res.result_04 = check_error(user, request);
    if (res.result_04) {
        return 0;
    }
    if (req.slot_18.dword_00 == 2 || req.slot_18.dword_00 == 1) {
        CharacAdvanceAltarManager* manager =
            &user->advance_altar_manager_;
        res.result_04 = manager->setSlot(req.byte_14 != 0, req.slot_18);
    } else {
        res.result_04 = 2;
    }
    return 0;
}

// IDA_EA: 0x8140148
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlot11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_SetSlot::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_SetSlot::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_SET_SLOT);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&response), 28);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x814223A
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlotD2Ev
// IDA_EA: 0x814226A
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlotD0Ev
// DEMANGLED: advancealtar::Dispatcher_SetSlot::~Dispatcher_SetSlot()
Dispatcher_SetSlot::~Dispatcher_SetSlot() {}

// IDA_EA: 0x8142288
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlot4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_SetSlot::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SetSlot::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8142292
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlot7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_SetSlot::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SetSlot::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595708
// IDA_NAME: _ZN12advancealtar18Dispatcher_SetSlotC2Ev
// DEMANGLED: advancealtar::Dispatcher_SetSlot::Dispatcher_SetSlot(void)
Dispatcher_SetSlot::Dispatcher_SetSlot()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x814076E
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlot12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ExchangeSlot::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 ||
        !user->getCurCharacR()) {
        return 1256;
    }

    CMDPacketStruct::_ReqAdvanceAltarExchangeSlot request;
    CMDPacketStruct::_ResAdvanceAltarExchangeSlot response;
    read_packet(packet, request);
    const int result = process_packet(user, request, response);
    (void)result;
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x8140856
// IDA_NAME: _ZNK12advancealtar23Dispatcher_ExchangeSlot11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_ExchangeSlot::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 48);
    return 0;
}

// IDA_EA: 0x8140884
// IDA_NAME: _ZNK12advancealtar23Dispatcher_ExchangeSlot11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_ExchangeSlot::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return !isOpenAdvanceAltar();
}

// IDA_EA: 0x81408A4
// IDA_NAME: _ZNK12advancealtar23Dispatcher_ExchangeSlot14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_ExchangeSlot::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request,
    CMDPacketStruct::STBaseResponse& response) const {
    CMDPacketStruct::_ReqAdvanceAltarExchangeSlot& req =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarExchangeSlot&>(request);
    CMDPacketStruct::_ResAdvanceAltarExchangeSlot& res =
        static_cast<CMDPacketStruct::_ResAdvanceAltarExchangeSlot&>(response);

    // CMD646 uses the same +0x10 -> +0x08 correlation field as CMD643.
    res.context_id_08 = req.context_id_10;
    res.result_04 = check_error(user, request);
    if (!res.result_04) {
        CharacAdvanceAltarManager* manager =
            &user->advance_altar_manager_;
        res.result_04 = manager->exchangeSlot(req.slot_18, req.slot_24);
        std::memcpy(&res.slot_0c, &req.slot_18, 0x0cu);
        std::memcpy(&res.slot_18, &req.slot_24, 0x0cu);
    }
    return 0;
}

// IDA_EA: 0x8140966
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlot11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_ExchangeSlot::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_EXCHANGE_SLOT);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&response), 36);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8142360
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlotD2Ev
// IDA_EA: 0x8142390
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlotD0Ev
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::~Dispatcher_ExchangeSlot()
Dispatcher_ExchangeSlot::~Dispatcher_ExchangeSlot() {}

// IDA_EA: 0x81423AE
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlot4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ExchangeSlot::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81423B8
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlot7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ExchangeSlot::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859575C
// IDA_NAME: _ZN12advancealtar23Dispatcher_ExchangeSlotC2Ev
// DEMANGLED: advancealtar::Dispatcher_ExchangeSlot::Dispatcher_ExchangeSlot(void)
Dispatcher_ExchangeSlot::Dispatcher_ExchangeSlot()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar

namespace CMDPacketStruct {

// IDA_EA: 0x8141dfa
// IDA_NAME: _ZN15CMDPacketStruct27_ReqAdvanceAltarUpgradeGageC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarUpgradeGage::_ReqAdvanceAltarUpgradeGage(void)
_ReqAdvanceAltarUpgradeGage::_ReqAdvanceAltarUpgradeGage() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8141e28
// IDA_NAME: _ZN15CMDPacketStruct27_ResAdvanceAltarUpgradeGageC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarUpgradeGage::_ResAdvanceAltarUpgradeGage(void)
_ResAdvanceAltarUpgradeGage::_ResAdvanceAltarUpgradeGage() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x81425b8
// IDA_NAME: _ZN15CMDPacketStruct27_ResAdvanceAltarUpgradeGageD2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarUpgradeGage::~_ResAdvanceAltarUpgradeGage()
_ResAdvanceAltarUpgradeGage::~_ResAdvanceAltarUpgradeGage() {}

// IDA_EA: 0x8141e60
// IDA_NAME: _ZN15CMDPacketStruct26_ReqAdvanceAltarSummonUnitC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarSummonUnit::_ReqAdvanceAltarSummonUnit(void)
_ReqAdvanceAltarSummonUnit::_ReqAdvanceAltarSummonUnit() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8141e8e
// IDA_NAME: _ZN15CMDPacketStruct26_ResAdvanceAltarSummonUnitC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarSummonUnit::_ResAdvanceAltarSummonUnit(void)
_ResAdvanceAltarSummonUnit::_ResAdvanceAltarSummonUnit() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8142606
// IDA_NAME: _ZN15CMDPacketStruct26_ResAdvanceAltarSummonUnitD2Ev
// IDA_EA: 0x8142636
// IDA_NAME: _ZN15CMDPacketStruct26_ResAdvanceAltarSummonUnitD0Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarSummonUnit::~_ResAdvanceAltarSummonUnit()
_ResAdvanceAltarSummonUnit::~_ResAdvanceAltarSummonUnit() {}

// IDA_EA: 0x8141F2C
// IDA_NAME: _ZN15CMDPacketStruct36_ReqAdvanceAltarGetAchievementRewardC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarGetAchievementReward::_ReqAdvanceAltarGetAchievementReward(void)
_ReqAdvanceAltarGetAchievementReward::_ReqAdvanceAltarGetAchievementReward()
    : STBaseRequest() {
    std::memset(this, 0, 0x14u);
}

// IDA_EA: 0x8141F5A
// IDA_NAME: _ZN15CMDPacketStruct36_ResAdvanceAltarGetAchievementRewardC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarGetAchievementReward::_ResAdvanceAltarGetAchievementReward(void)
_ResAdvanceAltarGetAchievementReward::_ResAdvanceAltarGetAchievementReward()
    : STBaseResponse() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x81426F0
// IDA_EA: 0x8142720
// IDA_NAME: _ZN15CMDPacketStruct36_ResAdvanceAltarGetAchievementRewardD2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarGetAchievementReward::~_ResAdvanceAltarGetAchievementReward()
_ResAdvanceAltarGetAchievementReward::~_ResAdvanceAltarGetAchievementReward() {}

// IDA_EA: 0x8141FFC
// IDA_NAME: _ZN15CMDPacketStruct25_ReqAdvanceAltarResetStarC2Ev
_ReqAdvanceAltarResetStar::_ReqAdvanceAltarResetStar()
    : STBaseRequest() {
    std::memset(this, 0, 0x14u);
}

// IDA_EA: 0x814202A
// IDA_NAME: _ZN15CMDPacketStruct25_ResAdvanceAltarResetStarC2Ev
_ResAdvanceAltarResetStar::_ResAdvanceAltarResetStar()
    : STBaseResponse() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x814205C / 0x814208C
// IDA_NAME: _ZN15CMDPacketStruct25_ResAdvanceAltarResetStarD2Ev
//           _ZN15CMDPacketStruct25_ResAdvanceAltarResetStarD0Ev
_ResAdvanceAltarResetStar::~_ResAdvanceAltarResetStar() {}

}  // namespace CMDPacketStruct

namespace NOTIPacketStruct {

// IDA_EA: 0x8141FF8
// IDA_NAME: _ZN16NOTIPacketStruct30_NotiAdvanceAltarUpdateRidableC2Ev
_NotiAdvanceAltarUpdateRidable::_NotiAdvanceAltarUpdateRidable()
    : value_00(0) {}

}  // namespace NOTIPacketStruct

namespace advancealtar {

// IDA_EA: 0x81401fe
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGage12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeGage::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 1094;
    }

    CMDPacketStruct::_ReqAdvanceAltarUpgradeGage request;
    CMDPacketStruct::_ResAdvanceAltarUpgradeGage response;
    read_packet(packet, request);
    (void)process_packet(user, request, response);
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x81402e6
// IDA_NAME: _ZNK12advancealtar22Dispatcher_UpgradeGage11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_UpgradeGage::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& base_request) const {
    packet.get_binary(reinterpret_cast<char*>(&base_request), 0x18);
    return 0;
}

// IDA_EA: 0x8140314
// IDA_NAME: _ZNK12advancealtar22Dispatcher_UpgradeGage11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_UpgradeGage::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return 0;
}

// IDA_EA: 0x814031e
// IDA_NAME: _ZNK12advancealtar22Dispatcher_UpgradeGage14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_UpgradeGage::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& base_request,
    CMDPacketStruct::STBaseResponse& base_response) const {
    (void)base_request;
    CMDPacketStruct::_ResAdvanceAltarUpgradeGage& response =
        static_cast<CMDPacketStruct::_ResAdvanceAltarUpgradeGage&>(base_response);

    response.result_04 = check_error(user, base_request);
    if (response.result_04 != 0) {
        return 0;
    }

    StageControl* const altar = user->getAdvanceAltar();
    if (!altar) {
        return 0;
    }

    CharacData& character_data = altar->charac_data;
    response.before_level_08 = character_data.getGageLevel();
    response.result_04 = character_data.upgreadGage();
    response.after_level_0c = character_data.getGageLevel();
    response.gage_10 = character_data.getGage();
    return 0;
}

// IDA_EA: 0x81403e2
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGage11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_UpgradeGage::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& base_response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_UPGRADE_GAGE);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&base_response), 0x14);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x814229c
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGageD2Ev
// IDA_EA: 0x81422cc
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGageD0Ev
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::~Dispatcher_UpgradeGage()
Dispatcher_UpgradeGage::~Dispatcher_UpgradeGage() {}

// IDA_EA: 0x81422ea
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGage4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeGage::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81422f4
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeGage::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595724
// IDA_NAME: _ZN12advancealtar22Dispatcher_UpgradeGageC2Ev
// DEMANGLED: advancealtar::Dispatcher_UpgradeGage::Dispatcher_UpgradeGage(void)
Dispatcher_UpgradeGage::Dispatcher_UpgradeGage()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8140498
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnit12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SummonUnit::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 1174;
    }

    CMDPacketStruct::_ReqAdvanceAltarSummonUnit request;
    CMDPacketStruct::_ResAdvanceAltarSummonUnit response;
    read_packet(packet, request);
    (void)process_packet(user, request, response);
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x8140580
// IDA_NAME: _ZNK12advancealtar21Dispatcher_SummonUnit11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_SummonUnit::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& base_request) const {
    packet.get_binary(reinterpret_cast<char*>(&base_request), 0x18);
    return 0;
}

// IDA_EA: 0x81405ae
// IDA_NAME: _ZNK12advancealtar21Dispatcher_SummonUnit11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_SummonUnit::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return 0;
}

// IDA_EA: 0x81405b8
// IDA_NAME: _ZNK12advancealtar21Dispatcher_SummonUnit14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_SummonUnit::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& base_request,
    CMDPacketStruct::STBaseResponse& base_response) const {
    CMDPacketStruct::_ReqAdvanceAltarSummonUnit& request =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarSummonUnit&>(base_request);
    CMDPacketStruct::_ResAdvanceAltarSummonUnit& response =
        static_cast<CMDPacketStruct::_ResAdvanceAltarSummonUnit&>(base_response);

    response.slot_08 = request.slot_0d;
    response.item_id_0c = request.item_id_10;
    response.result_04 = check_error(user, base_request);
    if (response.result_04 != 0) {
        return 0;
    }

    StageControl* const altar = user->getAdvanceAltar();
    if (!altar) {
        return 0;
    }

    CharacData& character_data = altar->charac_data;
    if (request.expected_gage_level_14 != character_data.getGageLevel()) {
        response.result_04 = 3;
        return 0;
    }

    response.result_04 =
        character_data.summonSlotUnit(request.slot_0d, request.item_id_10);
    response.gage_10 = character_data.getGage();
    return 0;
}

// IDA_EA: 0x81406b8
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnit11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_SummonUnit::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& base_response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_SUMMON_UNIT);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&base_response), 0x14);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81422fe
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnitD2Ev
// IDA_EA: 0x814232e
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnitD0Ev
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::~Dispatcher_SummonUnit()
Dispatcher_SummonUnit::~Dispatcher_SummonUnit() {}

// IDA_EA: 0x814234c
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnit4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SummonUnit::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8142356
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnit7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SummonUnit::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595740
// IDA_NAME: _ZN12advancealtar21Dispatcher_SummonUnitC2Ev
// DEMANGLED: advancealtar::Dispatcher_SummonUnit::Dispatcher_SummonUnit(void)
Dispatcher_SummonUnit::Dispatcher_SummonUnit()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8140F6E
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStar12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_ResetStar::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ResetStar::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 1472;
    }

    CMDPacketStruct::_ReqAdvanceAltarResetStar request;
    CMDPacketStruct::_ResAdvanceAltarResetStar response;
    read_packet(packet, request);
    (void)process_packet(user, request, response);
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x814105C
// IDA_NAME: _ZNK12advancealtar20Dispatcher_ResetStar11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_ResetStar::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_ResetStar::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 0x14);
    return 0;
}

// IDA_EA: 0x814108A
// IDA_NAME: _ZNK12advancealtar20Dispatcher_ResetStar11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_ResetStar::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_ResetStar::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return !isOpenAdvanceAltar();
}

// IDA_EA: 0x81410AA
// IDA_NAME: _ZNK12advancealtar20Dispatcher_ResetStar14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_ResetStar::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_ResetStar::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& base_request,
    CMDPacketStruct::STBaseResponse& base_response) const {
    CMDPacketStruct::_ReqAdvanceAltarResetStar& request =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarResetStar&>(base_request);
    CMDPacketStruct::_ResAdvanceAltarResetStar& response =
        static_cast<CMDPacketStruct::_ResAdvanceAltarResetStar&>(base_response);

    response.result_04 = check_error(user, base_request);
    if (response.result_04 != 0) {
        return 0;
    }

    CharacAdvanceAltarManager* const manager =
        &user->advance_altar_manager_;
    response.result_04 = manager->resetStar(
        user, static_cast<CMDPacketStruct::ResetStarType::T>(
                  request.reset_star_type_10));
    if (response.result_04 == 0) {
        AdvanceAltarData* const altar_data =
            manager->getCurrentAdvanceAltarData();
        if (altar_data) {
            NOTIPacketStruct::_NotiAdvanceAltarUpdateRidable notice;
            PacketGuard packet;
            packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                              ENUM_NOTIPACKET_ADVANCE_ALTAR_UPDATE_RIDABLE);
            packet.put_binary(reinterpret_cast<char*>(&notice), 0x01);
            putRidableInfoToPacket(packet, *altar_data);
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 0;
}

// IDA_EA: 0x81411FC
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStar11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_ResetStar::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_ResetStar::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_RESET_STAR);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&response), 0x0c);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8142486
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStarD2Ev
// DEMANGLED: advancealtar::Dispatcher_ResetStar::~Dispatcher_ResetStar()
// IDA_EA: 0x81424B6
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStarD0Ev
// DEMANGLED: advancealtar::Dispatcher_ResetStar::~Dispatcher_ResetStar()
Dispatcher_ResetStar::~Dispatcher_ResetStar() {}

// IDA_EA: 0x81424D4
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStar4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_ResetStar::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ResetStar::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81424DE
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStar7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_ResetStar::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ResetStar::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85957B0
// IDA_NAME: _ZN12advancealtar20Dispatcher_ResetStarC2Ev
// DEMANGLED: advancealtar::Dispatcher_ResetStar::Dispatcher_ResetStar(void)
Dispatcher_ResetStar::Dispatcher_ResetStar()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar

namespace advancealtar {

// IDA_EA: 0x8140C64
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementReward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GetAchievementReward::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 1393;
    }

    CMDPacketStruct::_ReqAdvanceAltarGetAchievementReward request;
    CMDPacketStruct::_ResAdvanceAltarGetAchievementReward response;
    read_packet(packet, request);
    (void)process_packet(user, request, response);
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x8140D52
// IDA_NAME: _ZNK12advancealtar31Dispatcher_GetAchievementReward11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_GetAchievementReward::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 0x14);
    return 0;
}

// IDA_EA: 0x8140D80
// IDA_NAME: _ZNK12advancealtar31Dispatcher_GetAchievementReward11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_GetAchievementReward::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return !isOpenAdvanceAltar();
}

// IDA_EA: 0x8140DA0
// IDA_NAME: _ZNK12advancealtar31Dispatcher_GetAchievementReward14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_GetAchievementReward::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request,
    CMDPacketStruct::STBaseResponse& response) const {
    // The original is a five-byte return-zero function.  In particular, it
    // does not rewrite the twelve-byte response before send_packet performs
    // the reward transaction.
    (void)user;
    (void)request;
    (void)response;
    return 0;
}

// IDA_EA: 0x8140DAA
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementReward11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_GetAchievementReward::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& base_response) {
    CMDPacketStruct::_ResAdvanceAltarGetAchievementReward& response =
        static_cast<CMDPacketStruct::_ResAdvanceAltarGetAchievementReward&>(
            base_response);
    const std::vector<_RewardItem>* reward_items = 0;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_GET_ACHIEVEMENT_REWARD);
    packet.put_byte(1);
    response.result_04 = user->advance_altar_manager_.sendAchievementReward(
        user, static_cast<int>(response.achievement_id_08), &reward_items);
    packet.put_binary(reinterpret_cast<char*>(&response), 0x0c);
    if (response.result_04 || !reward_items) {
        packet.put_int(0);
    } else {
        packet.put_int(static_cast<int>(reward_items->size()));
        for (std::vector<_RewardItem>::const_iterator it =
                 reward_items->begin();
             it != reward_items->end(); ++it) {
            packet.put_binary(reinterpret_cast<const char*>(&*it), 0x0c);
        }
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8142424
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementRewardD2Ev
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::~Dispatcher_GetAchievementReward()
// IDA_EA: 0x8142454
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementRewardD0Ev
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::~Dispatcher_GetAchievementReward() deleting destructor
Dispatcher_GetAchievementReward::~Dispatcher_GetAchievementReward() {}

// IDA_EA: 0x8142472
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GetAchievementReward::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x814247C
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GetAchievementReward::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595794
// IDA_NAME: _ZN12advancealtar31Dispatcher_GetAchievementRewardC2Ev
// DEMANGLED: advancealtar::Dispatcher_GetAchievementReward::Dispatcher_GetAchievementReward(void)
Dispatcher_GetAchievementReward::Dispatcher_GetAchievementReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar
