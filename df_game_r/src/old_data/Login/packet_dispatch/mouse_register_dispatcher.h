#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_MOUSE_REGISTER_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_MOUSE_REGISTER_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: MSG_TEMP_STRUCT.  This is a genuine MSG_BASE-only stack message
// used by the state-2 CMD116 template; its C2 still matters because the raw
// template calls it before Dispatcher_MouseRegister::read().
struct MSG_TEMP_STRUCT : MSG_BASE {
    // IDA_EA: 0x8265276
    // IDA_NAME: _ZN15MSG_TEMP_STRUCTC2Ev
    // DEMANGLED: MSG_TEMP_STRUCT::MSG_TEMP_STRUCT(void)
    MSG_TEMP_STRUCT();
};

// IDA unit: TempParam.  The raw stack object is a ParamBase vptr followed by
// the one four-byte result written by CMD116's process/send pair.
struct TempParam : ParamBase {
    int result;

    // IDA_EA: 0x826533C
    // IDA_NAME: _ZN9TempParamC2Ev
    // DEMANGLED: TempParam::TempParam(void)
    TempParam();

    // IDA_EA: 0x8265A6E / 0x8265A9E
    // IDA_NAME: _ZN9TempParamD2Ev / _ZN9TempParamD0Ev
    // DEMANGLED: TempParam::~TempParam()
    virtual ~TempParam();
};

#if defined(__i386__)
static_assert(sizeof(MSG_TEMP_STRUCT) == 0x0du,
              "CMD116 message must retain the old 13-byte MSG_BASE extent");
static_assert(sizeof(TempParam) == 0x08u,
              "CMD116 parameter must retain its i386 ParamBase-plus-int ABI");
#endif

// IDA unit: Dispatcher_MouseRegister / CMD116.  It owns the Goblin Pad
// register/change/delete request path and the shared replay helper used by
// CMD167.  Both commands therefore use the same typed state-2 closure.
class Dispatcher_MouseRegister
    : public IPacketDispatcher<MSG_TEMP_STRUCT, TempParam,
                               static_cast<ch_state>(2)> {
public:
    // IDA_EA: 0x8596054
    // IDA_NAME: _ZN24Dispatcher_MouseRegisterC2Ev
    // DEMANGLED: Dispatcher_MouseRegister::Dispatcher_MouseRegister(void)
    Dispatcher_MouseRegister();

    // IDA_EA: 0x82657B0 / 0x82657E0
    // IDA_NAME: _ZN24Dispatcher_MouseRegisterD2Ev /
    //           _ZN24Dispatcher_MouseRegisterD0Ev
    // DEMANGLED: Dispatcher_MouseRegister::~Dispatcher_MouseRegister()
    virtual ~Dispatcher_MouseRegister();

    // IDA_EA: 0x8263210
    // IDA_NAME: _ZN24Dispatcher_MouseRegister4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MouseRegister::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8263224
    // IDA_NAME: _ZN24Dispatcher_MouseRegister7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MouseRegister::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x82632A0
    // IDA_NAME: _ZN24Dispatcher_MouseRegister11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_MouseRegister::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x82632B4
    // IDA_NAME: _ZN24Dispatcher_MouseRegister4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_MouseRegister::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x82637FE
    // IDA_NAME: _ZN24Dispatcher_MouseRegister12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_MouseRegister::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82632CA / 0x826333A.
    // IDA_NAME: _ZN24Dispatcher_MouseRegister19checkValiedPasswordEPKc /
    //           _ZN24Dispatcher_MouseRegister15decryptPasswordEPKcPcj
    static int checkValiedPassword(const char* password);
    static int decryptPassword(const char* encrypted,
                               char* password,
                               uint32_t length);

    // IDA_EA: 0x82633C6
    // IDA_NAME: _ZN24Dispatcher_MouseRegister15doPassPadReplayEP5CUserPccRbRh
    // DEMANGLED: Dispatcher_MouseRegister::doPassPadReplay(CUser *,char *,char,bool &,unsigned char &)
    static void doPassPadReplay(CUser* user, char* answer,
                                char modification_type, bool& success,
                                unsigned char& error);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MouseRegister) == 0x04u,
              "CMD116 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_MOUSE_REGISTER_DISPATCHER_H_
