#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_PACKAGE_OPEN_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_PACKAGE_OPEN_DISPATCHER_H_

#include "old_game/economy/mail/mail_trade_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD519.  The package redemption work belongs to CCeraShop's existing
// SpecialItemHandler; this derived object is only its original request,
// validation, inventory-consumption, and response boundary.  It stays out of
// the PacketDispatcher 606-slot root until the complete root transaction is
// recovered.
class Dispatcher_CeraPackageOpen
    : public IPacketDispatcher<MSG_OPEN_CERAPACKAGE,
                               stOpenCeraPackage,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597EA6
    // IDA_NAME: _ZN26Dispatcher_CeraPackageOpenC2Ev
    // DEMANGLED: Dispatcher_CeraPackageOpen::Dispatcher_CeraPackageOpen(void)
    Dispatcher_CeraPackageOpen();

    // IDA_EA: 0x8243B6A / 0x8243B9A
    // IDA_NAME: _ZN26Dispatcher_CeraPackageOpenD2Ev /
    //           _ZN26Dispatcher_CeraPackageOpenD0Ev
    virtual ~Dispatcher_CeraPackageOpen();

    // IDA_EA: 0x81E76FA / 0x81E788A
    // IDA_NAME: _ZN26Dispatcher_CeraPackageOpen4readER9PacketBufR8MSG_BASE /
    //           _ZN26Dispatcher_CeraPackageOpen7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E7CC2
    // IDA_NAME: _ZN26Dispatcher_CeraPackageOpen11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_CeraPackageOpen::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CeraPackageOpen) == 0x04u,
              "CMD519 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_PACKAGE_OPEN_DISPATCHER_H_
