#ifndef OLDSRV_SECURITY_NETWORK_SECURITY_SCAN_RUNTIME_H_
#define OLDSRV_SECURITY_NETWORK_SECURITY_SCAN_RUNTIME_H_

#include <stdint.h>

class CUser;
class PacketBuf;

// IDA_EA: 0x8274BC5
// IDA_NAME: _Z21_NS_NonClient_RequestP5CUser
// DEMANGLED: _NS_NonClient_Request(CUser *)
// Emits the CMD272 random challenge after monitor-login completion and arms
// its exact message-140 response timer.
void _NS_NonClient_Request(CUser* user);

// IDA_EA: 0x8274523
// IDA_NAME: _Z15_NS_ProcessScanP5CUserR9PacketBuf
// DEMANGLED: _NS_ProcessScan(CUser *,PacketBuf &)
int _NS_ProcessScan(CUser* user, PacketBuf& packet);

// IDA_EA: 0x8274676
// IDA_NAME: _Z10_NS_IPScanP5CUserR9PacketBuf
// DEMANGLED: _NS_IPScan(CUser *,PacketBuf &)
int _NS_IPScan(CUser* user, PacketBuf& packet);

// IDA_EA: 0x8274975
// IDA_NAME: _Z19_NS_PacketIntegrityP5CUserR9PacketBuf
// DEMANGLED: _NS_PacketIntegrity(CUser *,PacketBuf &)
int _NS_PacketIntegrity(CUser* user, PacketBuf& packet);

// IDA_EA: 0x8274CC3
// IDA_NAME: _Z25_NS_NonClient_GetResponseP5CUserR9PacketBuf
// DEMANGLED: _NS_NonClient_GetResponse(CUser *,PacketBuf &)
int _NS_NonClient_GetResponse(CUser* user, PacketBuf& packet);

// IDA_EA: 0x8274B09
// IDA_NAME: _Z36_NS_MI_RecvMemoryIntegrityModulationP5CUserR9PacketBuf
// DEMANGLED: _NS_MI_RecvMemoryIntegrityModulation(CUser *,PacketBuf &)
int _NS_MI_RecvMemoryIntegrityModulation(CUser* user, PacketBuf& packet);

// IDA_EA: 0x8275140
// IDA_NAME: _Z44_NS_PI_2ND_RecvPacketIntegrityModulation_S2CP5CUserR9PacketBuf
// DEMANGLED: _NS_PI_2ND_RecvPacketIntegrityModulation_S2C(CUser *,PacketBuf &)
int _NS_PI_2ND_RecvPacketIntegrityModulation_S2C(CUser* user,
                                                  PacketBuf& packet);

// IDA_EA: 0x8275290
// IDA_NAME: _Z44_NS_PI_2ND_RecvPacketIntegrityModulation_C2CP5CUserR9PacketBuf
// DEMANGLED: _NS_PI_2ND_RecvPacketIntegrityModulation_C2C(CUser *,PacketBuf &)
int _NS_PI_2ND_RecvPacketIntegrityModulation_C2C(CUser* user,
                                                  PacketBuf& packet);

// The three mapping leaves are part of the same original security closure.
// Their exact native identities are retained because external packet paths
// call them directly in df_game_r.
int HP_GetHackType(uint8_t packet_type);       // IDA_EA: 0x82756CF
int HIP_GetHackType(uint8_t packet_type);      // IDA_EA: 0x827585C
int MI_GetHackType(char packet_type);          // IDA_EA: 0x8274AD7

#endif  // OLDSRV_SECURITY_NETWORK_SECURITY_SCAN_RUNTIME_H_
