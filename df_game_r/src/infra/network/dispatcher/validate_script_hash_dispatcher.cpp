#include "infra/network/dispatcher/validate_script_hash_dispatcher.h"

#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"


namespace {

const char kValidateScriptHashReadSignature[] =
    "virtual int Dispatcher_ValidateScriptHash::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x82667C2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8266912
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x826691C
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82669AE
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8266FC0
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266FF0
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x85989EA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_SCRIPT_HASH15ScriptHashParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCRIPT_HASH,ScriptHashParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SCRIPT_HASH,
                                 ScriptHashParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x85973D2
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHashC2Ev
// DEMANGLED: Dispatcher_ValidateScriptHash::Dispatcher_ValidateScriptHash(void)
Dispatcher_ValidateScriptHash::Dispatcher_ValidateScriptHash() {}

// IDA_EA: 0x8265762
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHashD2Ev
// DEMANGLED: Dispatcher_ValidateScriptHash::~Dispatcher_ValidateScriptHash()
// IDA_EA: 0x8265792
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHashD0Ev
// DEMANGLED: Dispatcher_ValidateScriptHash::~Dispatcher_ValidateScriptHash() deleting destructor
Dispatcher_ValidateScriptHash::~Dispatcher_ValidateScriptHash() {}

// IDA_EA: 0x82641E2
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHash4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ValidateScriptHash::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ValidateScriptHash::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SCRIPT_HASH& request = static_cast<MSG_SCRIPT_HASH&>(message);
    if (packet.get_byte(request.script_type) != 1) {
        return LineFunc(893, kValidateScriptHashReadSignature, 0, 0);
    }
    if (packet.get_byte(request.script_index) != 1) {
        return LineFunc(898, kValidateScriptHashReadSignature, 0, 0);
    }
    if (packet.get_int(request.script_hash) != 1) {
        return LineFunc(903, kValidateScriptHashReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x826435A
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHash11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ValidateScriptHash::check_error(CUser *,MSG_BASE &)
int Dispatcher_ValidateScriptHash::check_error(CUser*, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8264364
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHash13get_hack_typeEc
// DEMANGLED: Dispatcher_ValidateScriptHash::get_hack_type(char)
WongWork::ENUM_HACKTYPE Dispatcher_ValidateScriptHash::get_hack_type(
    char script_type) {
    // The old default branch returns the uninitialized stack local.  Do not
    // replace it with a fabricated category, rejection, or safe default.
    WongWork::ENUM_HACKTYPE hack_type;
    switch (script_type) {
    case 1:
        hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x344);
        break;
    case 2:
        hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x345);
        break;
    case 3:
        hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x346);
        break;
    case 4:
        hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x347);
        break;
    case 5:
        hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x348);
        break;
    default:
        return hack_type;
    }
    return hack_type;
}

// IDA_EA: 0x82642B8
// IDA_NAME: _ZN29Dispatcher_ValidateScriptHash7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ValidateScriptHash::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ValidateScriptHash::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)param;
    const MSG_SCRIPT_HASH& request =
        static_cast<const MSG_SCRIPT_HASH&>(message);
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        get_hack_type(static_cast<char>(request.script_type)),
        1,
        request.script_index,
        request.script_hash);
    return 0;
}
