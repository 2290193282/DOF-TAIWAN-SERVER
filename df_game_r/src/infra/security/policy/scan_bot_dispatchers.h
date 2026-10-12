#ifndef OLDSRV_SECURITY_SCAN_BOT_DISPATCHERS_H_
#define OLDSRV_SECURITY_SCAN_BOT_DISPATCHERS_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD338's stack message has no named derived C2.  It is the packed legacy
// C2S base followed by the three fields read by Dispatcher_ScanBotByDll.
#pragma pack(push, 1)
struct MSG_SCAN_DLL : MSG_BASE {
    int code;
    char value;
    uint16_t extra;
};

// CMD367's stack message likewise has no named derived C2.
struct MSG_SCAN_DRV : MSG_BASE {
    int code;
    char value;
};
#pragma pack(pop)

static_assert(sizeof(MSG_SCAN_DLL) == 0x14u,
              "CMD338 MSG_SCAN_DLL must retain its 20-byte ABI");
static_assert(sizeof(MSG_SCAN_DRV) == 0x12u,
              "CMD367 MSG_SCAN_DRV must retain its 18-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_SCAN_DLL, code) == 0x0du &&
                  offsetof(MSG_SCAN_DLL, value) == 0x11u &&
                  offsetof(MSG_SCAN_DLL, extra) == 0x12u,
              "CMD338 MSG_SCAN_DLL offsets changed");
static_assert(offsetof(MSG_SCAN_DRV, code) == 0x0du &&
                  offsetof(MSG_SCAN_DRV, value) == 0x11u,
              "CMD367 MSG_SCAN_DRV offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class ScanBotByDllParam : public ParamBase {
public:
    // IDA_EA: 0x82653C0
    // IDA_NAME: _ZN17ScanBotByDllParamC2Ev
    // DEMANGLED: ScanBotByDllParam::ScanBotByDllParam(void)
    ScanBotByDllParam();

    // IDA_EA: 0x8265984
    // IDA_NAME: _ZN17ScanBotByDllParamD2Ev
    // DEMANGLED: ScanBotByDllParam::~ScanBotByDllParam()
    // IDA_EA: 0x82659B4
    // IDA_NAME: _ZN17ScanBotByDllParamD0Ev
    // DEMANGLED: ScanBotByDllParam::~ScanBotByDllParam()
    virtual ~ScanBotByDllParam();

    int raw_04;
    char raw_08;
    uint16_t raw_0a;
};

class ScanBotByDrvParam : public ParamBase {
public:
    // IDA_EA: 0x82653F6
    // IDA_NAME: _ZN17ScanBotByDrvParamC2Ev
    // DEMANGLED: ScanBotByDrvParam::ScanBotByDrvParam(void)
    ScanBotByDrvParam();

    // IDA_EA: 0x8265936
    // IDA_NAME: _ZN17ScanBotByDrvParamD2Ev
    // DEMANGLED: ScanBotByDrvParam::~ScanBotByDrvParam()
    // IDA_EA: 0x8265966
    // IDA_NAME: _ZN17ScanBotByDrvParamD0Ev
    // DEMANGLED: ScanBotByDrvParam::~ScanBotByDrvParam()
    virtual ~ScanBotByDrvParam();

    int raw_04;
    char raw_08;
};

#if defined(__i386__)
static_assert(sizeof(ScanBotByDllParam) == 0x0cu,
              "CMD338 parameter must retain its 12-byte i386 ABI");
static_assert(sizeof(ScanBotByDrvParam) == 0x0cu,
              "CMD367 parameter must retain its 12-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(ScanBotByDllParam, raw_04) == 0x04u &&
                  offsetof(ScanBotByDllParam, raw_08) == 0x08u &&
                  offsetof(ScanBotByDllParam, raw_0a) == 0x0au,
              "CMD338 parameter offsets changed");
static_assert(offsetof(ScanBotByDrvParam, raw_04) == 0x04u &&
                  offsetof(ScanBotByDrvParam, raw_08) == 0x08u,
              "CMD367 parameter offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

class Dispatcher_ScanBotByDll
    : public IPacketDispatcher<MSG_SCAN_DLL,
                               ScanBotByDllParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859740A
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDllC2Ev
    // DEMANGLED: Dispatcher_ScanBotByDll::Dispatcher_ScanBotByDll(void)
    Dispatcher_ScanBotByDll();

    // IDA_EA: 0x8265714
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDllD2Ev
    // DEMANGLED: Dispatcher_ScanBotByDll::~Dispatcher_ScanBotByDll()
    // IDA_EA: 0x8265744
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDllD0Ev
    // DEMANGLED: Dispatcher_ScanBotByDll::~Dispatcher_ScanBotByDll()
    virtual ~Dispatcher_ScanBotByDll();

    // IDA_EA: 0x82643B2
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDll4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ScanBotByDll::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8264488
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDll7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ScanBotByDll::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8264538
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDll11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ScanBotByDll::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

class Dispatcher_ScanBotByDrv
    : public IPacketDispatcher<MSG_SCAN_DRV,
                               ScanBotByDrvParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85975AE
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrvC2Ev
    // DEMANGLED: Dispatcher_ScanBotByDrv::Dispatcher_ScanBotByDrv(void)
    Dispatcher_ScanBotByDrv();

    // IDA_EA: 0x8265678
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrvD2Ev
    // DEMANGLED: Dispatcher_ScanBotByDrv::~Dispatcher_ScanBotByDrv()
    // IDA_EA: 0x82656A8
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrvD0Ev
    // DEMANGLED: Dispatcher_ScanBotByDrv::~Dispatcher_ScanBotByDrv()
    virtual ~Dispatcher_ScanBotByDrv();

    // IDA_EA: 0x82646AE
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrv4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ScanBotByDrv::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8264740
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrv7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ScanBotByDrv::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8264806
    // IDA_NAME: _ZN23Dispatcher_ScanBotByDrv11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ScanBotByDrv::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ScanBotByDll) == 0x04u,
              "CMD338 dispatcher must retain its four-byte i386 ABI");
static_assert(sizeof(Dispatcher_ScanBotByDrv) == 0x04u,
              "CMD367 dispatcher must retain its four-byte i386 ABI");
#endif

#endif  // OLDSRV_SECURITY_SCAN_BOT_DISPATCHERS_H_
