#include "infra/security/policy/scan_bot_dispatchers.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDllReadSignature[] =
    "virtual int Dispatcher_ScanBotByDll::read(PacketBuf&, MSG_BASE&)";
const char kDrvReadSignature[] =
    "virtual int Dispatcher_ScanBotByDrv::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x82653C0
// IDA_NAME: _ZN17ScanBotByDllParamC2Ev
// DEMANGLED: ScanBotByDllParam::ScanBotByDllParam(void)
ScanBotByDllParam::ScanBotByDllParam()
    : ParamBase(), raw_04(0), raw_08(0), raw_0a(0) {}

// IDA_EA: 0x8265984
// IDA_NAME: _ZN17ScanBotByDllParamD2Ev
// DEMANGLED: ScanBotByDllParam::~ScanBotByDllParam()
// IDA_EA: 0x82659B4
// IDA_NAME: _ZN17ScanBotByDllParamD0Ev
// DEMANGLED: ScanBotByDllParam::~ScanBotByDllParam()
ScanBotByDllParam::~ScanBotByDllParam() {}

// IDA_EA: 0x82653F6
// IDA_NAME: _ZN17ScanBotByDrvParamC2Ev
// DEMANGLED: ScanBotByDrvParam::ScanBotByDrvParam(void)
ScanBotByDrvParam::ScanBotByDrvParam()
    : ParamBase(), raw_04(0), raw_08(0) {}

// IDA_EA: 0x8265936
// IDA_NAME: _ZN17ScanBotByDrvParamD2Ev
// DEMANGLED: ScanBotByDrvParam::~ScanBotByDrvParam()
// IDA_EA: 0x8265966
// IDA_NAME: _ZN17ScanBotByDrvParamD0Ev
// DEMANGLED: ScanBotByDrvParam::~ScanBotByDrvParam()
ScanBotByDrvParam::~ScanBotByDrvParam() {}

// The two old state-0 specializations have no derived-message C2 identities.
// Keep their generic vtables and function symbols owned by this one source.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x82665D0
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8266720
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x826672A
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82667BC
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8266F72
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266FA2
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598A06
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DLL17ScanBotByDllParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DLL,ScanBotByDllParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SCAN_DLL,
                                 ScanBotByDllParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x82661EC
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x826633C
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8266346
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82663D8
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8266ED6
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266F06
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598B1E
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SCAN_DRV17ScanBotByDrvParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SCAN_DRV,ScanBotByDrvParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SCAN_DRV,
                                 ScanBotByDrvParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859740A
// IDA_NAME: _ZN23Dispatcher_ScanBotByDllC2Ev
// DEMANGLED: Dispatcher_ScanBotByDll::Dispatcher_ScanBotByDll(void)
Dispatcher_ScanBotByDll::Dispatcher_ScanBotByDll() {}

// IDA_EA: 0x8265714
// IDA_NAME: _ZN23Dispatcher_ScanBotByDllD2Ev
// DEMANGLED: Dispatcher_ScanBotByDll::~Dispatcher_ScanBotByDll()
// IDA_EA: 0x8265744
// IDA_NAME: _ZN23Dispatcher_ScanBotByDllD0Ev
// DEMANGLED: Dispatcher_ScanBotByDll::~Dispatcher_ScanBotByDll()
Dispatcher_ScanBotByDll::~Dispatcher_ScanBotByDll() {}

// IDA_EA: 0x82643B2
// IDA_NAME: _ZN23Dispatcher_ScanBotByDll4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ScanBotByDll::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ScanBotByDll::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SCAN_DLL& input = static_cast<MSG_SCAN_DLL&>(message);
    if (packet.get_int(input.code) != 1) {
        return LineFunc(982, kDllReadSignature, 0, 0);
    }
    if (packet.get_byte(input.value) != 1) {
        return LineFunc(987, kDllReadSignature, 0, 0);
    }
    if (packet.get_short(input.extra) == 1) {
        return 0;
    }
    return LineFunc(992, kDllReadSignature, 0, 0);
}

// IDA_EA: 0x8264488
// IDA_NAME: _ZN23Dispatcher_ScanBotByDll7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ScanBotByDll::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ScanBotByDll::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)param;

    MSG_SCAN_DLL& input = static_cast<MSG_SCAN_DLL&>(message);
    if (check_error(user, message) == 0) {
        const int hack_type = input.code + 850;
        const int stored_value = user->SetETC(
            static_cast<short>(hack_type), static_cast<int>(input.value));
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
            1u,
            static_cast<unsigned int>(stored_value),
            static_cast<unsigned int>(input.extra));
    }
    return 0;
}

// IDA_EA: 0x8264538
// IDA_NAME: _ZN23Dispatcher_ScanBotByDll11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ScanBotByDll::check_error(CUser *,MSG_BASE &)
int Dispatcher_ScanBotByDll::check_error(CUser* user, MSG_BASE& message) {
    (void)user;

    const MSG_SCAN_DLL& input = static_cast<const MSG_SCAN_DLL&>(message);
    const int hack_type = input.code + 850;
    if (hack_type > 849 && hack_type <= 899) {
        return 0;
    }
    return 1031;
}

// IDA_EA: 0x85975AE
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrvC2Ev
// DEMANGLED: Dispatcher_ScanBotByDrv::Dispatcher_ScanBotByDrv(void)
Dispatcher_ScanBotByDrv::Dispatcher_ScanBotByDrv() {}

// IDA_EA: 0x8265678
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrvD2Ev
// DEMANGLED: Dispatcher_ScanBotByDrv::~Dispatcher_ScanBotByDrv()
// IDA_EA: 0x82656A8
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrvD0Ev
// DEMANGLED: Dispatcher_ScanBotByDrv::~Dispatcher_ScanBotByDrv()
Dispatcher_ScanBotByDrv::~Dispatcher_ScanBotByDrv() {}

// IDA_EA: 0x82646AE
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrv4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ScanBotByDrv::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ScanBotByDrv::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SCAN_DRV& input = static_cast<MSG_SCAN_DRV&>(message);
    if (packet.get_int(input.code) != 1) {
        return LineFunc(1068, kDrvReadSignature, 0, 0);
    }
    if (packet.get_byte(input.value) == 1) {
        return 0;
    }
    return LineFunc(1073, kDrvReadSignature, 0, 0);
}

// IDA_EA: 0x8264740
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrv7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ScanBotByDrv::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ScanBotByDrv::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)param;

    MSG_SCAN_DRV& input = static_cast<MSG_SCAN_DRV&>(message);
    if (check_error(user, message) == 0) {
        const int hack_type = input.code + 1800;
        user->getHackAnalyzer()->setBaseHackType(
            static_cast<WongWork::ENUM_HACKTYPE>(1800));
        const int stored_value = user->SetETC(
            static_cast<short>(hack_type), static_cast<int>(input.value));
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
            1u,
            static_cast<unsigned int>(stored_value),
            0u);
    }
    return 0;
}

// IDA_EA: 0x8264806
// IDA_NAME: _ZN23Dispatcher_ScanBotByDrv11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ScanBotByDrv::check_error(CUser *,MSG_BASE &)
int Dispatcher_ScanBotByDrv::check_error(CUser* user, MSG_BASE& message) {
    (void)user;
    (void)message;
    return 0;
}
