#include "old_game/player/core/packet_dispatcher_rename_creature.h"

#include <cstring>
#include <string>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/restrict_inputting_name.h"
#include "common/manager/slang_filter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_RenameCreature::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_RenameCreature::process(CUser*, MSG_BASE&, ParamBase&)";

// IDA 0x81CD19C references s2_ at 0x8BC17BF.  The original executable stores
// this sentinel in CP949, not UTF-8: C0 CC B8 A7 BE F8 C0 BD ("이름없음").
const char kUnnamedCreature[] = "\xC0\xCC\xB8\xA7\xBE\xF8\xC0\xBD";

}  // namespace

// IDA_EA: 0x81CCFDC
// IDA_NAME: _ZN25Dispatcher_RenameCreature4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RenameCreature::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RenameCreature::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_RENAME_CREATURE& message = static_cast<MSG_RENAME_CREATURE&>(base_message);
    if (packet.get_short(message.creature_slot) != 1) {
        return LineFunc(9396, kReadSignature, 0, 0);
    }
    if (packet.get_byte(message.rename_mode) != 1) {
        return LineFunc(9399, kReadSignature, 0, 0);
    }
    if (packet.get_int(message.name_length) != 1) {
        return LineFunc(9402, kReadSignature, 0, 0);
    }
    if (packet.get_str(message.name, sizeof(message.name), message.name_length) ==
        1) {
        return 0;
    }
    return LineFunc(9405, kReadSignature, 0, 0);
}

// IDA_EA: 0x81CD110
// IDA_NAME: _ZN25Dispatcher_RenameCreature7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RenameCreature::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RenameCreature::process(CUser* user,
                                       MSG_BASE& base_message,
                                       ParamBase& base_param) {
    MSG_RENAME_CREATURE& message = static_cast<MSG_RENAME_CREATURE&>(base_message);
    stRenameCreature& param = static_cast<stRenameCreature&>(base_param);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result < 0) {
        return LineFunc(9427,
                        kProcessSignature,
                        param.result,
                        user->get_acc_id());
    }

    if (std::strcmp(message.name, kUnnamedCreature) == 0) {
        std::memset(message.name, 0, sizeof(message.name));
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    inventory->GetCreatureMgrW()->RenameCreature(
        message.name,
        static_cast<int>(static_cast<short>(message.creature_slot)),
        static_cast<int>(static_cast<signed char>(message.rename_mode)));
    return 0;
}

// IDA_EA: 0x81CD21C
// IDA_NAME: _ZN25Dispatcher_RenameCreature4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RenameCreature::send(CUser *,ParamBase &)
void Dispatcher_RenameCreature::send(CUser* user, ParamBase& base_param) {
    stRenameCreature& param = static_cast<stRenameCreature&>(base_param);
    if (param.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RENAME_CREATURE,
            static_cast<unsigned char>(param.result));
    }
}

// IDA_EA: 0x81CD254
// IDA_NAME: _ZN25Dispatcher_RenameCreature11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RenameCreature::check_error(CUser *,MSG_BASE &)
int Dispatcher_RenameCreature::check_error(CUser* user,
                                           MSG_BASE& base_message) {
    MSG_RENAME_CREATURE& message = static_cast<MSG_RENAME_CREATURE&>(base_message);
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }

    const std::string name(message.name);
    if (restrict_inputting_name::isUtf8StrInUnicodeRange(name.c_str()) != 1) {
        return 159;
    }

    CSyncSlangFilter* const slang_filter = CSyncSlangFilter::GetInstance();
    if (slang_filter->HasSlangName(name) ||
        G_CDataManager()->hasSpecialCharacter(message.name)) {
        return 159;
    }
    return slang_filter->HasSlang(name) ? 159 : 0;
}

// IDA_EA: 0x8245F5E
// IDA_NAME: _ZN25Dispatcher_RenameCreatureD2Ev
// IDA_EA: 0x8245F8E
// IDA_NAME: _ZN25Dispatcher_RenameCreatureD0Ev
// DEMANGLED: Dispatcher_RenameCreature::~Dispatcher_RenameCreature()
Dispatcher_RenameCreature::~Dispatcher_RenameCreature() {}

// IDA_EA: 0x8595F20
// IDA_NAME: _ZN25Dispatcher_RenameCreatureC2Ev
// DEMANGLED: Dispatcher_RenameCreature::Dispatcher_RenameCreature(void)
Dispatcher_RenameCreature::Dispatcher_RenameCreature()
    : IPacketDispatcher<MSG_RENAME_CREATURE,
                        stRenameCreature,
                        static_cast<ch_state>(0)>() {}

// The seven raw identity providers below belong exclusively to CMD103's
// typed state-0 template specialization.
// IDA_EA: 0x8256470
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x82565CC
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x82565D6
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825D0F0
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EED2Ev
// IDA_EA: 0x825D120
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EED0Ev
// IDA_EA: 0x825D97E
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598616
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_RENAME_CREATURE16stRenameCreatureL8ch_state0EEC2Ev
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
template class IPacketDispatcher<MSG_RENAME_CREATURE,
                                stRenameCreature,
                                static_cast<ch_state>(0)>;
