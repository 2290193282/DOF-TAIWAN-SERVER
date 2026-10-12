#include "old_data/Login/packet_dispatch/use_right_of_change_grow_type_dispatcher.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_UseRightOfChangeGrowType::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_UseRightOfChangeGrowType::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x826031A
// IDA_NAME: _ZN29MSG_RIGHT_OF_CHANGE_GROW_TYPEC2Ev
// DEMANGLED: MSG_RIGHT_OF_CHANGE_GROW_TYPE::MSG_RIGHT_OF_CHANGE_GROW_TYPE(void)
MSG_RIGHT_OF_CHANGE_GROW_TYPE::MSG_RIGHT_OF_CHANGE_GROW_TYPE()
    : MSG_BASE(), item_slot(0), first_grow_type(0) {}

// IDA_EA: 0x82620E0
// IDA_NAME: _ZN23stRightOfChangeGrowTypeC2Ev
// DEMANGLED: stRightOfChangeGrowType::stRightOfChangeGrowType(void)
stRightOfChangeGrowType::stRightOfChangeGrowType() : ParamBase() {}

// IDA_EA: 0x82620FC
// IDA_NAME: _ZN23stRightOfChangeGrowTypeD2Ev
// IDA_EA: 0x826212C
// IDA_NAME: _ZN23stRightOfChangeGrowTypeD0Ev
// DEMANGLED: stRightOfChangeGrowType::~stRightOfChangeGrowType()
stRightOfChangeGrowType::~stRightOfChangeGrowType() {}

// IDA_EA: 0x826214A
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x82622A6
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x82622B0
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x82625BA / 0x82625EA
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EED0Ev
// IDA_EA: 0x826263C
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x85982CE
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_RIGHT_OF_CHANGE_GROW_TYPE23stRightOfChangeGrowTypeL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_RIGHT_OF_CHANGE_GROW_TYPE,
                                 stRightOfChangeGrowType,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8595820
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowTypeC2Ev
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::Dispatcher_UseRightOfChangeGrowType(void)
Dispatcher_UseRightOfChangeGrowType::Dispatcher_UseRightOfChangeGrowType()
    : IPacketDispatcher<MSG_RIGHT_OF_CHANGE_GROW_TYPE,
                        stRightOfChangeGrowType,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8260BC6
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowTypeD2Ev
// IDA_EA: 0x8260BF6
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowTypeD0Ev
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::~Dispatcher_UseRightOfChangeGrowType()
Dispatcher_UseRightOfChangeGrowType::~Dispatcher_UseRightOfChangeGrowType() {}

// IDA_EA: 0x825DBCC
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseRightOfChangeGrowType::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    MSG_RIGHT_OF_CHANGE_GROW_TYPE& request =
        static_cast<MSG_RIGHT_OF_CHANGE_GROW_TYPE&>(message);
    if (packet.get_short(request.item_slot) != 1) {
        return LineFunc(129, kReadSignature, 0, 0);
    }
    if (packet.get_byte(request.first_grow_type) == 1) {
        return 0;
    }
    return LineFunc(132, kReadSignature, 0, 0);
}

// IDA_EA: 0x825DD84
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::check_error(CUser *,MSG_BASE &)
int Dispatcher_UseRightOfChangeGrowType::check_error(CUser* user,
                                                      MSG_BASE& message) {
    const MSG_RIGHT_OF_CHANGE_GROW_TYPE& request =
        static_cast<const MSG_RIGHT_OF_CHANGE_GROW_TYPE&>(message);
    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 30;
    }
    if (user->get_charac_job() ==
            oldsrv::config::kCharacterJobDemonicSwordman ||
        user->get_charac_job() ==
            oldsrv::config::kCharacterJobCreatorMage) {
        return 234;
    }
    if (!user->getCurCharFirstGrowType()) {
        return 19;
    }
    if (user->CheckInTrade()) {
        return 216;
    }
    if (request.first_grow_type >= 6u) {
        return 1;
    }

    const Inven_Item inventory_item = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), static_cast<short>(request.item_slot));
    CItem* const item = G_CDataManager()->find_item(inventory_item.getKey());
    if (!item) {
        return 17;
    }
    if (user->getCurCharacQuestRefR()->isDoingJobChangeQuest()) {
        return 22;
    }
    return user->verifyStackableItemActionPlace(
               static_cast<StackableAction::Type>(46), item)
               ? 0
               : 23;
}

// IDA_EA: 0x825DC64
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseRightOfChangeGrowType::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    stRightOfChangeGrowType& response =
        static_cast<stRightOfChangeGrowType&>(param);
    response.result = check_error(user, message);
    if (response.result > 0) {
        return 0;
    }
    if (response.result < 0) {
        LogManager::logFormat(1,
                              "PacketDispatcher_Impl_2.cpp",
                              kProcessSignature,
                              147,
                              "Dispatcher Check_Error No(%d)",
                              response.result);
        return LineFunc(148, kProcessSignature, 0, 0);
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_error != 0) {
        response.result = protection_error;
        return 0;
    }

    const MSG_RIGHT_OF_CHANGE_GROW_TYPE& request =
        static_cast<const MSG_RIGHT_OF_CHANGE_GROW_TYPE&>(message);
    response.result = user->ChangeGrowType_Item(
        static_cast<int>(request.first_grow_type),
        static_cast<short>(request.item_slot));
    user->_SaveCharacData();
    return 0;
}

// IDA_EA: 0x825DF24
// IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_UseRightOfChangeGrowType::send(CUser *,ParamBase &)
void Dispatcher_UseRightOfChangeGrowType::send(CUser* user, ParamBase& param) {
    const stRightOfChangeGrowType& response =
        static_cast<const stRightOfChangeGrowType&>(param);
    if (response.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_RIGHT_OF_CHANGE_GROW_TYPE,
            static_cast<uint8_t>(response.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_USE_RIGHT_OF_CHANGE_GROW_TYPE);
}
