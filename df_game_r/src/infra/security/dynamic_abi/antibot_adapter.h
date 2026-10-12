#ifndef OLDSRV_SECURITY_ANTIBOT_ADAPTER_H_
#define OLDSRV_SECURITY_ANTIBOT_ADAPTER_H_

#include <stdint.h>

class IAntibotInterface;
class IBusiness;

// The plug-in object returned by GetAntiBotInterfaceAll() remains opaque.  It
// is the dynamically supplied IAntibotInterface implementation from
// libantisvrimport.so; its complete C++ class declaration must not be copied
// into df_game_r.  Consumers use only the named, call-site-proven helpers
// below instead of opening the plug-in vtable themselves.
enum {
    // Adapter-only failure value.  A caller must treat it as an unavailable
    // checker, never as an ordinary "not flagged" result.
    kAntibotCheckerUnavailable = -1,
};

// IDA_EA: 0x82A6539
// IDA_NAME: _Z19CreateAntiServerObjPKc
// DEMANGLED: CreateAntiServerObj(char const*)
//
// The old binary returns GetAntiBotInterfaceAll()'s EAX pointer.  The dynamic
// target is nominally typed here but its storage and complete vtable remain
// foreign to df_game_r.
IAntibotInterface* CreateAntiServerObj(const char* library_dir);

// OLD_IDA_CALLSITE: GlobalData::Init @0x829CC99..0x829CD71, checker vtable
// slot 0 / old i386 byte offset +0x00.  The raw caller treats a zero result as
// initialization failure only in the retained, unreachable block: the actual
// old i386 flow unconditionally jumps over it after calling Initialize.  Null
// adapter arguments still return zero and must not be mistaken for a live
// checker result.
int InitializeAntiBotChecker(IAntibotInterface* checker,
                             IBusiness* business,
                             char* process_sequence_text);

// OLD_IDA_CALLSITE: Inter_MonitorLoginOK::dispatch_sig @0x84C855A, checker
// vtable slot 1 / old i386 byte offset +0x04.  The checker owns the 0x4c-byte
// output image and returns the anti-bot key used by the same login reply.
// kAntibotCheckerUnavailable means the dynamic checker boundary was absent;
// it is never translated to a success result.
int AntibotCheckerAddClient(IAntibotInterface* checker,
                            int process_sequence,
                            int reserved,
                            uint32_t account_id,
                            int client_version,
                            uint8_t* output_0x4c);

// OLD_IDA_CALLSITE: Inter_MonitorLoginOK::dispatch_sig @0x84C8672, checker
// vtable slot 8 / old i386 byte offset +0x20.  Raw i386 assembly passes only
// (checker, account_id, -1, client_version); it does not pass the preceding
// AddClient result or output buffer into this call.
int AntibotCheckerP2PAddClient(IAntibotInterface* checker,
                               uint32_t account_id,
                               int reserved,
                               int client_version);

// OLD_IDA_CALLSITE: CUser::prepareDisconnect @0x8648952.  The old plug-in
// vtable slot +0x08 receives (checker, process_sequence, 0, account_id) when
// the game socket leaves.  It is a side-effect-only notification; false only
// means that the foreign plug-in boundary was unavailable.
bool AntibotCheckerDeleteClient(IAntibotInterface* checker,
                                int process_sequence,
                                int reserved,
                                uint32_t account_id);

// OLD_IDA_CALLSITE: Dispatcher_Antibot::dispatch_sig / CMD603 at
// 0x820B50C, checker vtable slot 3 / old i386 byte offset +0x0C.  The
// checker receives the raw anti-bot client buffer after its 32-bit length
// prefix, together with the process/account tuple.  The observed plug-in
// export is AntibotInterface::OnRecvBufferFromClient(int, int, unsigned int,
// unsigned char*, unsigned short); character serial is maintained by the
// caller but is not an argument of checker slot +0x0C.
// A false return means the foreign boundary was unavailable; when true,
// plugin_result is the original checker EAX result.
bool TryAntibotCheckerProcessClientPacket(IAntibotInterface* checker,
                                          int process_sequence,
                                          int reserved,
                                          uint32_t account_id,
                                          uint8_t* client_buffer,
                                          uint16_t client_buffer_size,
                                          int* plugin_result);

// OLD_IDA_CALLSITE: ProcessAntiBotInputWrapper @0x80EDDB0, checker vtable
// slot 4 / old i386 byte offset +0x10. The foreign call has no result; false
// means the plug-in boundary was unavailable and no input was processed.
bool AntibotCheckerProcessInput(IAntibotInterface* checker);

// OLD_IDA_CALLSITE: Dispatcher_Antibot_DP::dispatch_sig @0x820B322 and
// Dispatcher_Antibot_DPCallBack::dispatch_sig @0x820B17A, checker vtable
// slot 11 / old i386 byte offset +0x2C (decimal +44).  decoded_packet_len is
// the original unsigned-short in/out capacity/result field.  A null checker
// or buffer returns kAntibotCheckerUnavailable so DPROTO cannot be accepted.
int AntibotCheckerRecvBufferForDecrypt(IAntibotInterface* checker,
                                       uint32_t antibot_key,
                                       uint8_t* encrypted_packet,
                                       uint16_t encrypted_packet_len,
                                       uint8_t* decoded_packet,
                                       uint16_t* decoded_packet_len);

// This status-preserving form is for callers such as CMD_DPROTO whose raw
// control flow distinguishes a genuine nonzero plug-in result (including
// -1) from an unavailable foreign checker boundary. It returns false only
// when the checker, buffers, result storage, vtable, or slot 11 is absent.
// When it returns true, plugin_result is the unmodified plug-in EAX result.
// The legacy int helper above remains the compatible sentinel-mapping entry
// point for existing callers.
bool TryAntibotCheckerRecvBufferForDecrypt(IAntibotInterface* checker,
                                           uint32_t antibot_key,
                                           uint8_t* encrypted_packet,
                                           uint16_t encrypted_packet_len,
                                           uint8_t* decoded_packet,
                                           uint16_t* decoded_packet_len,
                                           int* plugin_result);

// OLD_IDA_CALLSITE: CUser::AntibotSend @0x864809C, checker vtable slot 13 /
// old i386 byte offset +0x34.  The old caller treats any nonzero result as a
// request to wrap the outgoing S2C packet in NOTI_DPROTO.
int AntibotCheckerIsCmdInDownList(IAntibotInterface* checker,
                                  uint32_t antibot_key,
                                  int command);

// OLD_IDA_CALLSITE: PacketDispatcher::doDispatch @0x8594922, checker vtable
// slot 12 / old i386 byte offset +0x30 (decimal +48).  The original root
// caller logs a nonzero result but does not itself reject the command.
int AntibotCheckerIsCmdInUpList(IAntibotInterface* checker,
                                uint32_t antibot_key,
                                int command);

// OLD_IDA_CALLSITE: PacketDispatcher::dispatch @0x8594E52, checker vtable
// slot 15 / old i386 byte offset +0x3C (decimal +60).  A nonzero result is
// the old anti-cheat deny/history branch.
int AntibotCheckerAntiCheatPacket(IAntibotInterface* checker,
                                  uint32_t antibot_key,
                                  int command);

// OLD_IDA_CALLSITE: CUser::AntibotSend @0x8648260..0x86482C4, checker vtable
// slot 10 / old i386 byte offset +0x28.  The capacity/result field is the
// original unsigned-int in/out value.
int AntibotCheckerRecvBufferForEncrypt(IAntibotInterface* checker,
                                       uint32_t antibot_key,
                                       uint8_t* plain_packet,
                                       uint32_t plain_packet_len,
                                       uint8_t* encrypted_packet,
                                       uint32_t* encrypted_packet_len);

bool TryAntibotCheckerRecvBufferForEncrypt(IAntibotInterface* checker,
                                           uint32_t antibot_key,
                                           uint8_t* plain_packet,
                                           uint32_t plain_packet_len,
                                           uint8_t* encrypted_packet,
                                           uint32_t* encrypted_packet_len,
                                           int* plugin_result);

#endif  // OLDSRV_SECURITY_ANTIBOT_ADAPTER_H_
