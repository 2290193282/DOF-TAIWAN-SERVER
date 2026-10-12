#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SECURITY_CARD_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SECURITY_CARD_DISPATCHERS_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 164 / SECURITY_CARD_ISSUE.  The original is a base-only state-0
// dispatcher: all request state stays in CUser's already-restored security
// card and passpad subobjects.
class Dispatcher_SecurityCardIssue
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596444
    // IDA_NAME: _ZN28Dispatcher_SecurityCardIssueC2Ev
    Dispatcher_SecurityCardIssue();

    // IDA_EA: 0x82422BC / 0x82422EC
    // IDA_NAME: _ZN28Dispatcher_SecurityCardIssueD2Ev /
    //           _ZN28Dispatcher_SecurityCardIssueD0Ev
    virtual ~Dispatcher_SecurityCardIssue();

    // IDA_EA: 0x8209C8C
    // IDA_NAME: _ZN28Dispatcher_SecurityCardIssue12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A42 / 0x8233A4C
    // IDA_NAME: _ZN28Dispatcher_SecurityCardIssue4readER9PacketBufR8MSG_BASE /
    //           _ZN28Dispatcher_SecurityCardIssue7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardIssue) == 0x04u,
              "CMD164 dispatcher must retain its i386 base-only ABI");
#endif

// CMD 202 / SECURITY_CARD_RETRANSFER.  This is the neighbouring base-only
// handler and shares the same security-card object graph as CMD164.
class Dispatcher_SecurityCardRetransfer
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85964EC
    // IDA_NAME: _ZN33Dispatcher_SecurityCardRetransferC2Ev
    Dispatcher_SecurityCardRetransfer();

    // IDA_EA: 0x8242136 / 0x8242166
    // IDA_NAME: _ZN33Dispatcher_SecurityCardRetransferD2Ev /
    //           _ZN33Dispatcher_SecurityCardRetransferD0Ev
    virtual ~Dispatcher_SecurityCardRetransfer();

    // IDA_EA: 0x820A9A0
    // IDA_NAME: _ZN33Dispatcher_SecurityCardRetransfer12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233AA6 / 0x8233AB0
    // IDA_NAME: _ZN33Dispatcher_SecurityCardRetransfer4readER9PacketBufR8MSG_BASE /
    //           _ZN33Dispatcher_SecurityCardRetransfer7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardRetransfer) == 0x04u,
              "CMD202 dispatcher must retain its i386 base-only ABI");
#endif

// CMD 165 / SECURITY_CARD_DISUSE.  The old registration entry constructs the
// state-0 IPacketDispatcher base only; this class adds no data members.
class Dispatcher_SecurityCardDisuse
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596460
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseC2Ev
    Dispatcher_SecurityCardDisuse();

    // IDA_EA: 0x824226E
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseD2Ev
    // IDA_EA: 0x824229E
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseD0Ev
    virtual ~Dispatcher_SecurityCardDisuse();

    // IDA_EA: 0x820A0F6
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A56
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233A60
    // IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardDisuse) == 0x04u,
              "CMD165 dispatcher must retain its i386 base-only ABI");
#endif

// CMD 166 / SECURITY_CARD_AUTH_REQ.  The raw constructor selects the same
// state-0 IPacketDispatcher base and introduces no concrete data members.
class Dispatcher_SecurityCardAuthReq
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859647C
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqC2Ev
    Dispatcher_SecurityCardAuthReq();

    // IDA_EA: 0x8242220
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqD2Ev
    // IDA_EA: 0x8242250
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqD0Ev
    virtual ~Dispatcher_SecurityCardAuthReq();

    // IDA_EA: 0x820A2C4
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A6A
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233A74
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardAuthReq) == 0x04u,
              "CMD166 dispatcher must retain its i386 base-only ABI");
#endif

// CMD 168 / SECURITY_CARD_CERT_KEY.  The raw constructor selects the same
// state-0 IPacketDispatcher base and introduces no concrete data members.
class Dispatcher_SecurityCardCertKey
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85964B4
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyC2Ev
    Dispatcher_SecurityCardCertKey();

    // IDA_EA: 0x8242184
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyD2Ev
    // IDA_EA: 0x82421B4
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyD0Ev
    virtual ~Dispatcher_SecurityCardCertKey();

    // IDA_EA: 0x820A6BC
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A7E
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233A88
    // IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardCertKey) == 0x04u,
              "CMD168 dispatcher must retain its i386 base-only ABI");
#endif

// CMD 175 / SECURITY_CARD_AUTH_CANCEL. The raw class has the same state-0
// dispatcher base and one non-virtual process(CUser*) helper overload.
class Dispatcher_SecurityCardAuthCancel
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85964D0
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelC2Ev
    Dispatcher_SecurityCardAuthCancel();

    // IDA_EA: 0x82421D2
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelD2Ev
    // IDA_EA: 0x8242202
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelD0Ev
    virtual ~Dispatcher_SecurityCardAuthCancel();

    // IDA_EA: 0x820A42C
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x820A580
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel7processEP5CUser
    static int process(CUser* user);

    // IDA_EA: 0x8233A92
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233A9C
    // IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardAuthCancel) == 0x04u,
              "CMD175 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: MSG_SECU_RPY.  The raw state-2 template reserves exactly 0x1014
// bytes and does not zero them; the reader below initializes only the five
// protocol fields it consumes.  MSG_BASE remains an empty-C2 base at +0x00.
#pragma pack(push, 1)
struct MSG_SECU_RPY : MSG_BASE {
    uint8_t security_service;    // +0x0d: 0=card, 1=Goblin Pad
    uint8_t cipher_sequence;     // +0x0e
    uint8_t encrypted[4096];     // +0x0f
    uint32_t encrypted_length;   // +0x100f
    uint8_t modification_type;   // +0x1013
};
#pragma pack(pop)

static_assert(sizeof(MSG_SECU_RPY) == 0x1014u,
              "CMD167 input must retain the old 0x1014-byte extent");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_SECU_RPY, security_service) == 0x0du &&
                  offsetof(MSG_SECU_RPY, cipher_sequence) == 0x0eu &&
                  offsetof(MSG_SECU_RPY, encrypted) == 0x0fu &&
                  offsetof(MSG_SECU_RPY, encrypted_length) == 0x100fu &&
                  offsetof(MSG_SECU_RPY, modification_type) == 0x1013u,
              "CMD167 input field offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// IDA unit: SecuCardRpyParam.  Its packed six-byte stack object is a virtual
// ParamBase plus the success/error bytes written by the card and Pad branches.
#pragma pack(push, 1)
struct SecuCardRpyParam : ParamBase {
    bool success;     // +0x04
    uint8_t error;    // +0x05

    // IDA_EA: 0x8265362
    // IDA_NAME: _ZN16SecuCardRpyParamC2Ev
    // DEMANGLED: SecuCardRpyParam::SecuCardRpyParam(void)
    SecuCardRpyParam();

    // IDA_EA: 0x8265A20 / 0x8265A50
    // IDA_NAME: _ZN16SecuCardRpyParamD2Ev / _ZN16SecuCardRpyParamD0Ev
    // DEMANGLED: SecuCardRpyParam::~SecuCardRpyParam()
    virtual ~SecuCardRpyParam();
};
#pragma pack(pop)

#if defined(__i386__)
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(SecuCardRpyParam) == 0x06u &&
                  offsetof(SecuCardRpyParam, success) == 0x04u &&
                  offsetof(SecuCardRpyParam, error) == 0x05u,
              "CMD167 parameter must retain its packed i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

// CMD 167 / SECURITY_CARD_AUTH_RPY.  This is the common encrypted answer
// path for both the physical card and Goblin Pad; it uses CMD116's recovered
// replay routine rather than a parallel Pad state machine.
class Dispatcher_SecurityCardAuthRpy
    : public IPacketDispatcher<MSG_SECU_RPY, SecuCardRpyParam,
                               static_cast<ch_state>(2)> {
public:
    // IDA_EA: 0x8596498
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpyC2Ev
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::Dispatcher_SecurityCardAuthRpy(void)
    Dispatcher_SecurityCardAuthRpy();

    // IDA_EA: 0x82657FE / 0x826582E
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpyD2Ev /
    //           _ZN30Dispatcher_SecurityCardAuthRpyD0Ev
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::~Dispatcher_SecurityCardAuthRpy()
    virtual ~Dispatcher_SecurityCardAuthRpy();

    // IDA_EA: 0x8262658
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82627CA
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8262F4E
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x826314C
    // IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_SecurityCardAuthRpy::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityCardAuthRpy) == 0x04u,
              "CMD167 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SECURITY_CARD_DISPATCHERS_H_
