#include "infra/security/policy/packet_dispatcher_security_scan.h"

#include "common/manager/global_tools.h"
#include "infra/security/policy/network_security_scan_runtime.h"

// IDA_EA: 0x8596F34
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScanC2Ev
// DEMANGLED: Dispatcher_Security_ProcessScan::Dispatcher_Security_ProcessScan(void)
// IDA_EA: 0x8240B46
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScanD2Ev
// DEMANGLED: Dispatcher_Security_ProcessScan::~Dispatcher_Security_ProcessScan()
// IDA_EA: 0x8240B76
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScanD0Ev
// DEMANGLED: Dispatcher_Security_ProcessScan::~Dispatcher_Security_ProcessScan()
// IDA_EA: 0x823406E
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScan4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_ProcessScan::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x8234078
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScan7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_ProcessScan::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_ProcessScan::Dispatcher_Security_ProcessScan() {}
Dispatcher_Security_ProcessScan::~Dispatcher_Security_ProcessScan() {}
int Dispatcher_Security_ProcessScan::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_ProcessScan::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8223CF6
// IDA_NAME: _ZN31Dispatcher_Security_ProcessScan12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_ProcessScan::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_ProcessScan::dispatch_sig(CUser* user, PacketBuf& packet) {
    const int status = _NS_ProcessScan(user, packet);
    switch (status) {
        case 1:
            return LineFunc(
                56964,
                "virtual int Dispatcher_Security_ProcessScan::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 2:
            return LineFunc(
                56966,
                "virtual int Dispatcher_Security_ProcessScan::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 3:
            return LineFunc(
                56968,
                "virtual int Dispatcher_Security_ProcessScan::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        default:
            return 0;
    }
}

// IDA_EA: 0x8596F50
// IDA_NAME: _ZN26Dispatcher_Security_IPScanC2Ev
// DEMANGLED: Dispatcher_Security_IPScan::Dispatcher_Security_IPScan(void)
// IDA_EA: 0x8240AF8
// IDA_NAME: _ZN26Dispatcher_Security_IPScanD2Ev
// DEMANGLED: Dispatcher_Security_IPScan::~Dispatcher_Security_IPScan()
// IDA_EA: 0x8240B28
// IDA_NAME: _ZN26Dispatcher_Security_IPScanD0Ev
// DEMANGLED: Dispatcher_Security_IPScan::~Dispatcher_Security_IPScan()
// IDA_EA: 0x8234082
// IDA_NAME: _ZN26Dispatcher_Security_IPScan4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_IPScan::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x823408C
// IDA_NAME: _ZN26Dispatcher_Security_IPScan7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_IPScan::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_IPScan::Dispatcher_Security_IPScan() {}
Dispatcher_Security_IPScan::~Dispatcher_Security_IPScan() {}
int Dispatcher_Security_IPScan::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_IPScan::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8223D96
// IDA_NAME: _ZN26Dispatcher_Security_IPScan12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_IPScan::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_IPScan::dispatch_sig(CUser* user, PacketBuf& packet) {
    const int status = _NS_IPScan(user, packet);
    if (status == 1) {
        return LineFunc(
            56980,
            "virtual int Dispatcher_Security_IPScan::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    return 0;
}

// IDA_EA: 0x8596F88
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrityC2Ev
// DEMANGLED: Dispatcher_Security_PacketIntegrity::Dispatcher_Security_PacketIntegrity(void)
// IDA_EA: 0x8240A5C
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrityD2Ev
// DEMANGLED: Dispatcher_Security_PacketIntegrity::~Dispatcher_Security_PacketIntegrity()
// IDA_EA: 0x8240A8C
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrityD0Ev
// DEMANGLED: Dispatcher_Security_PacketIntegrity::~Dispatcher_Security_PacketIntegrity()
// IDA_EA: 0x82340AA
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrity4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_PacketIntegrity::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x82340B4
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrity7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_PacketIntegrity::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_PacketIntegrity::Dispatcher_Security_PacketIntegrity() {}
Dispatcher_Security_PacketIntegrity::~Dispatcher_Security_PacketIntegrity() {}
int Dispatcher_Security_PacketIntegrity::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_PacketIntegrity::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x822449A
// IDA_NAME: _ZN35Dispatcher_Security_PacketIntegrity12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    const int status = _NS_PacketIntegrity(user, packet);
    switch (status) {
        case 1:
            return LineFunc(
                57047,
                "virtual int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 2:
            return LineFunc(
                57049,
                "virtual int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 3:
            return LineFunc(
                57051,
                "virtual int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 4:
            return LineFunc(
                57053,
                "virtual int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 5:
            return LineFunc(
                57055,
                "virtual int Dispatcher_Security_PacketIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        default:
            return 0;
    }
}

// IDA_EA: 0x8597030
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponseC2Ev
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::Dispatcher_Security_NonClient_GetResponse(void)
// IDA_EA: 0x8240888
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponseD2Ev
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::~Dispatcher_Security_NonClient_GetResponse()
// IDA_EA: 0x82408B8
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponseD0Ev
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::~Dispatcher_Security_NonClient_GetResponse()
// IDA_EA: 0x8234122
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponse4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x823412C
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponse7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_NonClient_GetResponse::
    Dispatcher_Security_NonClient_GetResponse() {}
Dispatcher_Security_NonClient_GetResponse::
    ~Dispatcher_Security_NonClient_GetResponse() {}
int Dispatcher_Security_NonClient_GetResponse::read(PacketBuf& packet,
                                                     MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_NonClient_GetResponse::process(CUser* user,
                                                        MSG_BASE& message,
                                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8224E64
// IDA_NAME: _ZN41Dispatcher_Security_NonClient_GetResponse12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_NonClient_GetResponse::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_NonClient_GetResponse::dispatch_sig(CUser* user,
                                                             PacketBuf& packet) {
    const int status = _NS_NonClient_GetResponse(user, packet);
    if (status == 1) {
        return LineFunc(
            57224,
            "virtual int Dispatcher_Security_NonClient_GetResponse::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    } else if (status == 2) {
        return LineFunc(
            57226,
            "virtual int Dispatcher_Security_NonClient_GetResponse::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    return 0;
}

// IDA_EA: 0x859704C
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrityC2Ev
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::Dispatcher_Security_MemoryIntegrity(void)
// IDA_EA: 0x824083A
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrityD2Ev
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::~Dispatcher_Security_MemoryIntegrity()
// IDA_EA: 0x824086A
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrityD0Ev
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::~Dispatcher_Security_MemoryIntegrity()
// IDA_EA: 0x8234136
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrity4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x8234140
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrity7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_MemoryIntegrity::Dispatcher_Security_MemoryIntegrity() {}
Dispatcher_Security_MemoryIntegrity::~Dispatcher_Security_MemoryIntegrity() {}
int Dispatcher_Security_MemoryIntegrity::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_MemoryIntegrity::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8224EDC
// IDA_NAME: _ZN35Dispatcher_Security_MemoryIntegrity12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_MemoryIntegrity::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_MemoryIntegrity::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    const int status = _NS_MI_RecvMemoryIntegrityModulation(user, packet);
    switch (status) {
        case 1:
            return LineFunc(
                57237,
                "virtual int Dispatcher_Security_MemoryIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 2:
            return LineFunc(
                57239,
                "virtual int Dispatcher_Security_MemoryIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 3:
            return LineFunc(
                57241,
                "virtual int Dispatcher_Security_MemoryIntegrity::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        default:
            return 0;
    }
}

// IDA_EA: 0x8597362
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2CC2Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::Dispatcher_Security_PI_HashModulation_S2C(void)
// IDA_EA: 0x824035A
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2CD2Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::~Dispatcher_Security_PI_HashModulation_S2C()
// IDA_EA: 0x824038A
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2CD0Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::~Dispatcher_Security_PI_HashModulation_S2C()
// IDA_EA: 0x8234276
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2C4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x8234280
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2C7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_PI_HashModulation_S2C::
    Dispatcher_Security_PI_HashModulation_S2C() {}
Dispatcher_Security_PI_HashModulation_S2C::
    ~Dispatcher_Security_PI_HashModulation_S2C() {}
int Dispatcher_Security_PI_HashModulation_S2C::read(PacketBuf& packet,
                                                     MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_PI_HashModulation_S2C::process(CUser* user,
                                                        MSG_BASE& message,
                                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82271B6
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_S2C12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_PI_HashModulation_S2C::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_PI_HashModulation_S2C::dispatch_sig(CUser* user,
                                                             PacketBuf& packet) {
    const int status =
        _NS_PI_2ND_RecvPacketIntegrityModulation_S2C(user, packet);
    switch (status) {
        case 1:
            return LineFunc(
                58142,
                "virtual int Dispatcher_Security_PI_HashModulation_S2C::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 2:
            return LineFunc(
                58144,
                "virtual int Dispatcher_Security_PI_HashModulation_S2C::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        case 3:
            return LineFunc(
                58146,
                "virtual int Dispatcher_Security_PI_HashModulation_S2C::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        default:
            return 0;
    }
}

// IDA_EA: 0x859737E
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2CC2Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::Dispatcher_Security_PI_HashModulation_C2C(void)
// IDA_EA: 0x824030C
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2CD2Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::~Dispatcher_Security_PI_HashModulation_C2C()
// IDA_EA: 0x824033C
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2CD0Ev
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::~Dispatcher_Security_PI_HashModulation_C2C()
// IDA_EA: 0x823428A
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2C4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x8234294
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2C7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_Security_PI_HashModulation_C2C::
    Dispatcher_Security_PI_HashModulation_C2C() {}
Dispatcher_Security_PI_HashModulation_C2C::
    ~Dispatcher_Security_PI_HashModulation_C2C() {}
int Dispatcher_Security_PI_HashModulation_C2C::read(PacketBuf& packet,
                                                     MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}
int Dispatcher_Security_PI_HashModulation_C2C::process(CUser* user,
                                                        MSG_BASE& message,
                                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8227256
// IDA_NAME: _ZN41Dispatcher_Security_PI_HashModulation_C2C12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Security_PI_HashModulation_C2C::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Security_PI_HashModulation_C2C::dispatch_sig(CUser* user,
                                                             PacketBuf& packet) {
    const int status =
        _NS_PI_2ND_RecvPacketIntegrityModulation_C2C(user, packet);
    if (status == 1) {
        return LineFunc(
            58159,
            "virtual int Dispatcher_Security_PI_HashModulation_C2C::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    } else if (status == 2) {
        return LineFunc(
            58161,
            "virtual int Dispatcher_Security_PI_HashModulation_C2C::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    return 0;
}
