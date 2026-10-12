#include "old_game/player/core/packet_dispatcher_hatch_creature.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_HatchCreature::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_HatchCreature::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x81CD3AE
// IDA_NAME: _ZN24Dispatcher_HatchCreature4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_HatchCreature::read(PacketBuf &,MSG_BASE &)
int Dispatcher_HatchCreature::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_HATCH_CREATURE& message =
        static_cast<MSG_HATCH_CREATURE&>(base_message);
    if (packet.get_byte(message.item_space) != 1) {
        return LineFunc(9496, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.creature_slot) == 1) {
        return 0;
    }
    return LineFunc(9499, kReadSignature, 0, 0);
}

// IDA_EA: 0x81CD446
// IDA_NAME: _ZN24Dispatcher_HatchCreature7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_HatchCreature::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_HatchCreature::process(CUser* user,
                                      MSG_BASE& base_message,
                                      ParamBase& base_param) {
    MSG_HATCH_CREATURE& message =
        static_cast<MSG_HATCH_CREATURE&>(base_message);
    stHatchCreature& param = static_cast<stHatchCreature&>(base_param);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result < 0) {
        return LineFunc(9516,
                        kProcessSignature,
                        param.result,
                        user->get_acc_id());
    }

    const int slot = static_cast<int>(static_cast<int16_t>(
        message.creature_slot));
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(message.item_space));
    CInventory* const inventory = user->getCurCharacInvenR();
    Inven_Item* const item = inventory->GetInvenRef(inventory_type, slot);
    if (!item) {
        return LineFunc(9523, kProcessSignature, 0, user->get_acc_id());
    }

    user->getCurCharacInvenW()->GetCreatureMgrW()->Hatch(item->get_add_info());
    return 0;
}

// IDA_EA: 0x81CD572
// IDA_NAME: _ZN24Dispatcher_HatchCreature4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_HatchCreature::send(CUser *,ParamBase &)
void Dispatcher_HatchCreature::send(CUser* user, ParamBase& base_param) {
    stHatchCreature& param = static_cast<stHatchCreature&>(base_param);
    if (param.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_HATCH_CREATURE,
            static_cast<unsigned char>(param.result));
    }
}

// IDA_EA: 0x81CD5AA
// IDA_NAME: _ZN24Dispatcher_HatchCreature11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_HatchCreature::check_error(CUser *,MSG_BASE &)
int Dispatcher_HatchCreature::check_error(CUser* user, MSG_BASE& base_message) {
    MSG_HATCH_CREATURE& message =
        static_cast<MSG_HATCH_CREATURE&>(base_message);
    if (!user->getCurCharacR()) {
        return -1;
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(20));
    if (security_result != 0) {
        return security_result;
    }

    const int slot = static_cast<int>(static_cast<int16_t>(
        message.creature_slot));
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(message.item_space));
    return user->CheckItemLock(inventory_type, slot) ? 213 : 0;
}

// IDA_EA: 0x8245F10
// IDA_NAME: _ZN24Dispatcher_HatchCreatureD2Ev
// IDA_EA: 0x8245F40
// IDA_NAME: _ZN24Dispatcher_HatchCreatureD0Ev
// DEMANGLED: Dispatcher_HatchCreature::~Dispatcher_HatchCreature()
Dispatcher_HatchCreature::~Dispatcher_HatchCreature() {}

// IDA_EA: 0x8595F58
// IDA_NAME: _ZN24Dispatcher_HatchCreatureC2Ev
// DEMANGLED: Dispatcher_HatchCreature::Dispatcher_HatchCreature(void)
Dispatcher_HatchCreature::Dispatcher_HatchCreature()
    : IPacketDispatcher<MSG_HATCH_CREATURE,
                        stHatchCreature,
                        static_cast<ch_state>(3)>() {}

// The seven raw identity providers below belong exclusively to CMD105's
// typed state-3 template specialization.
// IDA_EA: 0x8256282
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x82563D2
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x82563DC
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825D0A2
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EED2Ev
// IDA_EA: 0x825D0D2
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EED0Ev
// IDA_EA: 0x825D978
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598632
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_HATCH_CREATURE15stHatchCreatureL8ch_state3EEC2Ev
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
template class IPacketDispatcher<MSG_HATCH_CREATURE,
                                stHatchCreature,
                                static_cast<ch_state>(3)>;
