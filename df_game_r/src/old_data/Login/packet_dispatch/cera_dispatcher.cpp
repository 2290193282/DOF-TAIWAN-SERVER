#include "old_data/Login/packet_dispatch/cera_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8595B30
// IDA_NAME: _ZN15DisPatcher_CeraC2Ev
// DEMANGLED: DisPatcher_Cera::DisPatcher_Cera(void)
DisPatcher_Cera::DisPatcher_Cera() {}

// IDA_EA: 0x82436D8
// IDA_NAME: _ZN15DisPatcher_CeraD2Ev
// DEMANGLED: DisPatcher_Cera::~DisPatcher_Cera()
// IDA_EA: 0x8243708
// IDA_NAME: _ZN15DisPatcher_CeraD0Ev
// DEMANGLED: DisPatcher_Cera::~DisPatcher_Cera()
DisPatcher_Cera::~DisPatcher_Cera() {}

// IDA_EA: 0x81FD360
// IDA_NAME: _ZN15DisPatcher_Cera12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_Cera::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_Cera::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    // +0x2c in the original helper vtable is IPGQuery(CUser *), the twelfth
    // virtual method of the recovered no-virtual-destructor interface.
    GlobalData::s_pIPGHelper->IPGQuery(user);
    user->LoadMileage();
    return 0;
}

// IDA_EA: 0x82334A4
// IDA_NAME: _ZN15DisPatcher_Cera4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_Cera::read(PacketBuf &,MSG_BASE &)
int DisPatcher_Cera::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82334AE
// IDA_NAME: _ZN15DisPatcher_Cera7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_Cera::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_Cera::process(CUser* user, MSG_BASE& message,
                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
