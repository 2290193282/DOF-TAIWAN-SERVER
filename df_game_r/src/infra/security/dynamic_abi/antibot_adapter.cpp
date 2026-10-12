#include "infra/security/dynamic_abi/antibot_adapter.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

namespace {

typedef IAntibotInterface* (*GetAntiBotInterfaceAllFn)();

// These are only the call-site-proven slots whose signatures are proven by old
// df_game_r call sites.  Do not turn this into a C++ virtual base class or add
// declarations for the unobserved slots: the object belongs to the plug-in.
enum AntibotCheckerVtableSlot {
    kAntibotCheckerInitializeSlot = 0,
    kAntibotCheckerAddClientSlot = 1,
    kAntibotCheckerDeleteClientSlot = 2,
    kAntibotCheckerProcessClientPacketSlot = 3,
    kAntibotCheckerProcessInputSlot = 4,
    kAntibotCheckerP2PAddClientSlot = 8,
    kAntibotCheckerRecvBufferForEncryptSlot = 10,
    kAntibotCheckerRecvBufferForDecryptSlot = 11,
    kAntibotCheckerIsCmdInUpListSlot = 12,
    kAntibotCheckerIsCmdInDownListSlot = 13,
    kAntibotCheckerAntiCheatPacketSlot = 15,
};

typedef int (*AntibotCheckerInitializeFn)(IAntibotInterface*, IBusiness*, char*);
typedef int (*AntibotCheckerAddClientFn)(IAntibotInterface*, int, int,
                                         uint32_t, int, uint8_t*);
typedef void (*AntibotCheckerDeleteClientFn)(IAntibotInterface*, int, int,
                                             uint32_t);
typedef int (*AntibotCheckerProcessClientPacketFn)(
    IAntibotInterface*, int, int, uint32_t, uint8_t*, uint16_t);
typedef void (*AntibotCheckerProcessInputFn)(IAntibotInterface*);
typedef int (*AntibotCheckerRecvBufferForDecryptFn)(
    IAntibotInterface*, uint32_t, uint8_t*, uint16_t, uint8_t*, uint16_t*);
typedef int (*AntibotCheckerRecvBufferForEncryptFn)(
    IAntibotInterface*, uint32_t, uint8_t*, uint32_t, uint8_t*, uint32_t*);
typedef int (*AntibotCheckerCommandFn)(IAntibotInterface*, uint32_t, int);
typedef int (*AntibotCheckerP2PAddClientFn)(IAntibotInterface*, uint32_t,
                                            int, int);

void** GetAntibotCheckerVtable(IAntibotInterface* checker) {
    if (!checker) {
        return 0;
    }
    return *reinterpret_cast<void***>(checker);
}

template <typename Function>
Function ResolveAntibotCheckerSlot(IAntibotInterface* checker, size_t slot) {
    void** const vtable = GetAntibotCheckerVtable(checker);
    if (!vtable || !vtable[slot]) {
        return 0;
    }
    return reinterpret_cast<Function*>(vtable)[slot];
}

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(void*) == 4,
              "the old AntibotInterface vtable evidence is i386");
static_assert(kAntibotCheckerInitializeSlot * sizeof(void*) == 0x00u,
              "old Initialize slot must stay at checker vtable +0x00");
static_assert(kAntibotCheckerAddClientSlot * sizeof(void*) == 0x04u,
              "old MonitorLoginOK AddClient slot must stay at +0x04");
static_assert(kAntibotCheckerDeleteClientSlot * sizeof(void*) == 0x08u,
              "old DeleteClient slot must stay at checker vtable +0x08");
static_assert(kAntibotCheckerProcessClientPacketSlot * sizeof(void*) ==
                  0x0Cu,
              "old CMD603 slot must stay at checker vtable +0x0C");
static_assert(kAntibotCheckerProcessInputSlot * sizeof(void*) == 0x10u,
              "old ProcessAntiBotInput slot must stay at checker vtable +0x10");
static_assert(kAntibotCheckerP2PAddClientSlot * sizeof(void*) == 0x20u,
              "old MonitorLoginOK P2PAddClient slot must stay at +0x20");
static_assert(kAntibotCheckerRecvBufferForEncryptSlot * sizeof(void*) ==
                  0x28u,
              "old DPROTO encrypt slot must stay at checker vtable +0x28");
static_assert(kAntibotCheckerRecvBufferForDecryptSlot * sizeof(void*) ==
                  0x2Cu,
              "old DPROTO decrypt slot must stay at checker vtable +0x2C");
static_assert(kAntibotCheckerIsCmdInUpListSlot * sizeof(void*) ==
                  0x30u,
              "old IsCmdInUpList slot must stay at +0x30");
static_assert(kAntibotCheckerIsCmdInDownListSlot * sizeof(void*) == 0x34u,
              "old IsCmdInDownList slot must stay at +0x34");
static_assert(kAntibotCheckerAntiCheatPacketSlot * sizeof(void*) == 0x3Cu,
              "old AntiCheatPacket slot must stay at +0x3C");
#endif

}  // namespace

// IDA_EA: 0x82A6539
// IDA_NAME: _Z19CreateAntiServerObjPKc
// DEMANGLED: CreateAntiServerObj(char const*)
IAntibotInterface* CreateAntiServerObj(const char* library_dir) {
    char path[260];
    memset(path, 0, sizeof(path));

    const size_t library_dir_length = strlen(library_dir);
    strncat(path, library_dir, library_dir_length);

    const size_t path_length = strlen(path);
    memcpy(&path[path_length], "libantisvrimport.so", 0x14u);

    void* handle = dlopen(path, 2);
    char* error = dlerror();
    if (error) {
        printf("Open Dll %s failed, %s\n", "libantisvrimport.so", error);
        return 0;
    }

    GetAntiBotInterfaceAllFn factory =
        reinterpret_cast<GetAntiBotInterfaceAllFn>(
            dlsym(handle, "GetAntiBotInterfaceAll"));
    error = dlerror();
    if (error) {
        dlclose(handle);
        printf("dlsym failed, %s\n", error);
        return 0;
    }
    return factory();
}

// OLD_IDA_CALLSITE: GlobalData::Init @0x829CC99..0x829CD71; checker vtable
// +0x00 is called as (checker, IBusiness*, decimal-process-sequence).
int InitializeAntiBotChecker(IAntibotInterface* checker,
                             IBusiness* business,
                             char* process_sequence_text) {
    if (!checker || !business || !process_sequence_text) {
        return 0;
    }
    const AntibotCheckerInitializeFn initialize =
        ResolveAntibotCheckerSlot<AntibotCheckerInitializeFn>(
            checker, kAntibotCheckerInitializeSlot);
    if (!initialize) {
        return 0;
    }
    return initialize(checker, business, process_sequence_text);
}

// OLD_IDA_CALLSITE: Inter_MonitorLoginOK::dispatch_sig @0x84C855A; checker
// vtable slot 1 / old i386 byte offset +0x04.  The raw caller clears exactly
// 0x4c bytes and passes that storage as the final argument.
int AntibotCheckerAddClient(IAntibotInterface* checker,
                            int process_sequence,
                            int reserved,
                            uint32_t account_id,
                            int client_version,
                            uint8_t* output_0x4c) {
    if (!checker || !output_0x4c) {
        return kAntibotCheckerUnavailable;
    }
    const AntibotCheckerAddClientFn add_client =
        ResolveAntibotCheckerSlot<AntibotCheckerAddClientFn>(
            checker, kAntibotCheckerAddClientSlot);
    if (!add_client) {
        return kAntibotCheckerUnavailable;
    }
    return add_client(checker,
                      process_sequence,
                      reserved,
                      account_id,
                      client_version,
                      output_0x4c);
}

// OLD_IDA_CALLSITE: Inter_MonitorLoginOK::dispatch_sig @0x84C8672; checker
// vtable slot 8 / old i386 byte offset +0x20.  Assembly writes only the four
// stack words checker/account_id/-1/client_version before `call esi`.
int AntibotCheckerP2PAddClient(IAntibotInterface* checker,
                               uint32_t account_id,
                               int reserved,
                               int client_version) {
    if (!checker) {
        return kAntibotCheckerUnavailable;
    }
    const AntibotCheckerP2PAddClientFn p2p_add_client =
        ResolveAntibotCheckerSlot<AntibotCheckerP2PAddClientFn>(
            checker, kAntibotCheckerP2PAddClientSlot);
    if (!p2p_add_client) {
        return kAntibotCheckerUnavailable;
    }
    return p2p_add_client(checker, account_id, reserved, client_version);
}

bool AntibotCheckerDeleteClient(IAntibotInterface* checker,
                                int process_sequence,
                                int reserved,
                                uint32_t account_id) {
    if (!checker) {
        return false;
    }
    const AntibotCheckerDeleteClientFn delete_client =
        ResolveAntibotCheckerSlot<AntibotCheckerDeleteClientFn>(
            checker, kAntibotCheckerDeleteClientSlot);
    if (!delete_client) {
        return false;
    }
    delete_client(checker, process_sequence, reserved, account_id);
    return true;
}

// OLD_IDA_CALLSITE: Dispatcher_Antibot::dispatch_sig / CMD603 @0x820B50C;
// checker +0x0C receives the original anti-bot request tuple.
bool TryAntibotCheckerProcessClientPacket(IAntibotInterface* checker,
                                          int process_sequence,
                                          int reserved,
                                          uint32_t account_id,
                                          uint8_t* client_buffer,
                                          uint16_t client_buffer_size,
                                          int* plugin_result) {
    // IDA 0x820B719..0x820B769 forwards the PacketBuf pointer verbatim to
    // checker slot +0x0C.  In particular, the original does not reject a
    // null pointer based on the advertised wire length; the plug-in owns
    // that validation and its return value is preserved for the caller.
    if (!checker || !plugin_result) {
        return false;
    }
    const AntibotCheckerProcessClientPacketFn process_packet =
        ResolveAntibotCheckerSlot<AntibotCheckerProcessClientPacketFn>(
            checker, kAntibotCheckerProcessClientPacketSlot);
    if (!process_packet) {
        return false;
    }
    *plugin_result = process_packet(checker,
                                    process_sequence,
                                    reserved,
                                    account_id,
                                    client_buffer,
                                    client_buffer_size);
    return true;
}

// OLD_IDA_CALLSITE: ProcessAntiBotInputWrapper @0x80EDDB0; checker +0x10.
bool AntibotCheckerProcessInput(IAntibotInterface* checker) {
    if (!checker) {
        return false;
    }
    const AntibotCheckerProcessInputFn process_input =
        ResolveAntibotCheckerSlot<AntibotCheckerProcessInputFn>(
            checker, kAntibotCheckerProcessInputSlot);
    if (!process_input) {
        return false;
    }
    process_input(checker);
    return true;
}

// OLD_IDA_CALLSITE: Dispatcher_Antibot_DP::dispatch_sig @0x820B322 and
// Dispatcher_Antibot_DPCallBack::dispatch_sig @0x820B17A; checker +0x2C.
int AntibotCheckerRecvBufferForDecrypt(IAntibotInterface* checker,
                                       uint32_t antibot_key,
                                       uint8_t* encrypted_packet,
                                       uint16_t encrypted_packet_len,
                                       uint8_t* decoded_packet,
                                       uint16_t* decoded_packet_len) {
    int plugin_result = 0;
    if (!TryAntibotCheckerRecvBufferForDecrypt(checker,
                                               antibot_key,
                                               encrypted_packet,
                                               encrypted_packet_len,
                                               decoded_packet,
                                               decoded_packet_len,
                                               &plugin_result)) {
        return kAntibotCheckerUnavailable;
    }
    return plugin_result;
}

bool TryAntibotCheckerRecvBufferForDecrypt(IAntibotInterface* checker,
                                           uint32_t antibot_key,
                                           uint8_t* encrypted_packet,
                                           uint16_t encrypted_packet_len,
                                           uint8_t* decoded_packet,
                                           uint16_t* decoded_packet_len,
                                           int* plugin_result) {
    if (!checker || !encrypted_packet || !decoded_packet ||
        !decoded_packet_len || !plugin_result) {
        return false;
    }
    const AntibotCheckerRecvBufferForDecryptFn decrypt =
        ResolveAntibotCheckerSlot<AntibotCheckerRecvBufferForDecryptFn>(
            checker, kAntibotCheckerRecvBufferForDecryptSlot);
    if (!decrypt) {
        return false;
    }
    *plugin_result = decrypt(checker,
                             antibot_key,
                             encrypted_packet,
                             encrypted_packet_len,
                             decoded_packet,
                             decoded_packet_len);
    return true;
}

// OLD_IDA_CALLSITE: PacketDispatcher::doDispatch @0x8594922; checker +0x30.
int AntibotCheckerIsCmdInUpList(IAntibotInterface* checker,
                                uint32_t antibot_key,
                                int command) {
    if (!checker) {
        return kAntibotCheckerUnavailable;
    }
    const AntibotCheckerCommandFn check =
        ResolveAntibotCheckerSlot<AntibotCheckerCommandFn>(
            checker, kAntibotCheckerIsCmdInUpListSlot);
    if (!check) {
        return kAntibotCheckerUnavailable;
    }
    return check(checker, antibot_key, command);
}

// OLD_IDA_CALLSITE: CUser::AntibotSend @0x86480D8..0x8648105; checker
// vtable slot 13 / old i386 byte offset +0x34.
int AntibotCheckerIsCmdInDownList(IAntibotInterface* checker,
                                  uint32_t antibot_key,
                                  int command) {
    if (!checker) {
        return kAntibotCheckerUnavailable;
    }
    const AntibotCheckerCommandFn check =
        ResolveAntibotCheckerSlot<AntibotCheckerCommandFn>(
            checker, kAntibotCheckerIsCmdInDownListSlot);
    if (!check) {
        return kAntibotCheckerUnavailable;
    }
    return check(checker, antibot_key, command);
}

// OLD_IDA_CALLSITE: PacketDispatcher::dispatch @0x8594E52; checker +0x3C.
int AntibotCheckerAntiCheatPacket(IAntibotInterface* checker,
                                  uint32_t antibot_key,
                                  int command) {
    if (!checker) {
        return kAntibotCheckerUnavailable;
    }
    const AntibotCheckerCommandFn reject =
        ResolveAntibotCheckerSlot<AntibotCheckerCommandFn>(
            checker, kAntibotCheckerAntiCheatPacketSlot);
    if (!reject) {
        return kAntibotCheckerUnavailable;
    }
    return reject(checker, antibot_key, command);
}

// OLD_IDA_CALLSITE: CUser::AntibotSend @0x8648260..0x86482C4; checker
// vtable slot 10 / old i386 byte offset +0x28.
int AntibotCheckerRecvBufferForEncrypt(IAntibotInterface* checker,
                                       uint32_t antibot_key,
                                       uint8_t* plain_packet,
                                       uint32_t plain_packet_len,
                                       uint8_t* encrypted_packet,
                                       uint32_t* encrypted_packet_len) {
    int plugin_result = 0;
    if (!TryAntibotCheckerRecvBufferForEncrypt(checker,
                                               antibot_key,
                                               plain_packet,
                                               plain_packet_len,
                                               encrypted_packet,
                                               encrypted_packet_len,
                                               &plugin_result)) {
        return kAntibotCheckerUnavailable;
    }
    return plugin_result;
}

bool TryAntibotCheckerRecvBufferForEncrypt(IAntibotInterface* checker,
                                           uint32_t antibot_key,
                                           uint8_t* plain_packet,
                                           uint32_t plain_packet_len,
                                           uint8_t* encrypted_packet,
                                           uint32_t* encrypted_packet_len,
                                           int* plugin_result) {
    if (!checker || !plain_packet || !encrypted_packet ||
        !encrypted_packet_len || !plugin_result) {
        return false;
    }
    const AntibotCheckerRecvBufferForEncryptFn encrypt =
        ResolveAntibotCheckerSlot<AntibotCheckerRecvBufferForEncryptFn>(
            checker, kAntibotCheckerRecvBufferForEncryptSlot);
    if (!encrypt) {
        return false;
    }
    *plugin_result = encrypt(checker,
                             antibot_key,
                             plain_packet,
                             plain_packet_len,
                             encrypted_packet,
                             encrypted_packet_len);
    return true;
}
