#include "infra/network/dispatcher/inter_dispatcher_arad_only.h"

#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

// IDA_EA: 0x84E7EB8
// IDA_NAME: _ZN24Inter_DispatcherAradOnlyC2Ev
// DEMANGLED: Inter_DispatcherAradOnly::Inter_DispatcherAradOnly(void)
Inter_DispatcherAradOnly::Inter_DispatcherAradOnly()
    : ISigDispatcher(), arad_dispatcher_() {
    // The original calls Init immediately after placement-constructing the
    // embedded map owner and ignores its boolean return.
    (void)arad_dispatcher_.Init();
}

// IDA_EA: 0x84E7F2C
// IDA_NAME: _ZN24Inter_DispatcherAradOnly12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DispatcherAradOnly::dispatch_sig(CUser *,char *,int)
int Inter_DispatcherAradOnly::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    // Old df_game_r loads this dword before any local pointer or length test.
    // Its caller therefore owns the invariant that this ARAD reply has the
    // four-byte internal-code prefix.  Do not add a synthetic malformed-frame
    // policy at this inner adapter boundary.
    int inner_code = 0;
    std::memcpy(&inner_code, payload, sizeof(inner_code));

    ARAD::DISPATCHER::Arad_IInterDispatcher* const dispatcher =
        arad_dispatcher_.GetDispatcher(
            static_cast<ARAD::ENUM_INTERNALPACKET_JPN>(inner_code));
    if (dispatcher) {
        // vtable +0x08 in the old i386 binary: dispatch_sig(user, payload+4,
        // payload_size-4).  Forwarding through the typed interface preserves
        // that virtual slot without a raw vtable overlay.
        return dispatcher->dispatch_sig(user, payload + sizeof(inner_code),
                                        payload_size - sizeof(inner_code));
    }

    // This is the C-owner's narrow, typed ABI declaration of the exact old
    // CUserCharacInfo::getCurCharacNo symbol.  The adapter does not reopen
    // CUser's large concrete layout merely to emit the diagnostic.
    const int charac_no = user->getCurCharacNo();
    cMyTrace trace(
        "virtual int Inter_DispatcherAradOnly::dispatch_sig(CUser*, char*, int)",
        27554,
        5);
    trace("[DISPATCHER_ARAD] Invalid Internal Packet Code. (code:%d, charac:%u)",
          inner_code,
          static_cast<unsigned int>(charac_no));
    return 27555;
}

// IDA_EA: 0x84F5A1C
// IDA_NAME: _ZN24Inter_DispatcherAradOnlyD2Ev
// DEMANGLED: Inter_DispatcherAradOnly::~Inter_DispatcherAradOnly()
// IDA_EA: 0x84F5A7E
// IDA_NAME: _ZN24Inter_DispatcherAradOnlyD0Ev
// DEMANGLED: Inter_DispatcherAradOnly::~Inter_DispatcherAradOnly()
Inter_DispatcherAradOnly::~Inter_DispatcherAradOnly() {
    // C++ destruction order is the observed D2 sequence: embedded ARAD map
    // owner first, then ISigDispatcher.  The compiler emits the D0 deleting
    // wrapper exactly as the old ABI requires.
}
