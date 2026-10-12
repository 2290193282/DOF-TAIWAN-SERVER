#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include "common/manager/global_data.h"
#include "infra/log/format/log_manager.h"
#include "old_data/channel/arad_internal_server_state_packets.h"
#include "old_data/channel/arad_server_state.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A4E0
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kSaveServerState::Arad_INTER_kSaveServerState(void)
Arad_INTER_kSaveServerState::Arad_INTER_kSaveServerState()
    : Arad_IInterDispatcher(kInternalPacketSaveServerState) {}

// IDA_EA: 0x819A504
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateD2Ev
// IDA_EA: 0x819A534
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kSaveServerState::~Arad_INTER_kSaveServerState()
Arad_INTER_kSaveServerState::~Arad_INTER_kSaveServerState() {}

// IDA_EA: 0x8199278
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerState12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kSaveServerState::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kSaveServerState::dispatch_sig(CUser*, char* payload, int) {
    if (!payload) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kSaveServerState::dispatch_sig(CUser*, char*, int)",
            0x521,
            "Packet_Arad_ApplyEffect is null.");
        return 1314;
    }

    const Packet_Arad_ApplyEffect& input =
        *reinterpret_cast<const Packet_Arad_ApplyEffect*>(payload);
    AradServerStateMessage message;
    // The original C2 clears the remaining six message bytes, then overwrites
    // only active (+0) and end_sec (+2) from the control packet.
    message.active = 1;
    message.end_sec = input.end_sec;
    GlobalData::s_serverStateManager_->insertState(input.effect, message);
    return 0;
}

// IDA_EA: 0x819A552
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadServerState::Arad_INTER_kLoadServerState(void)
Arad_INTER_kLoadServerState::Arad_INTER_kLoadServerState()
    : Arad_IInterDispatcher(kInternalPacketLoadServerState) {}

// IDA_EA: 0x819A576
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateD2Ev
// IDA_EA: 0x819A5A6
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadServerState::~Arad_INTER_kLoadServerState()
Arad_INTER_kLoadServerState::~Arad_INTER_kLoadServerState() {}

// IDA_EA: 0x81992FE
// IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerState12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadServerState::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kLoadServerState::dispatch_sig(CUser*, char* payload, int) {
    if (!payload) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kLoadServerState::dispatch_sig(CUser*, char*, int)",
            0x533,
            "SigServerState is null.");
        return 1332;
    }

    GlobalData::s_serverStateManager_->insertState(
        reinterpret_cast<arad::SigServerState*>(payload));
    return 0;
}

// IDA_EA: 0x819A5C4
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kDeleteServerState::Arad_INTER_kDeleteServerState(void)
Arad_INTER_kDeleteServerState::Arad_INTER_kDeleteServerState()
    : Arad_IInterDispatcher(kInternalPacketDeleteServerState) {}

// IDA_EA: 0x819A5E8
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateD2Ev
// IDA_EA: 0x819A618
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kDeleteServerState::~Arad_INTER_kDeleteServerState()
Arad_INTER_kDeleteServerState::~Arad_INTER_kDeleteServerState() {}

// IDA_EA: 0x819935E
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerState12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kDeleteServerState::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kDeleteServerState::dispatch_sig(CUser*, char* payload, int) {
    if (!payload) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kDeleteServerState::dispatch_sig(CUser*, char*, int)",
            0x540,
            "Packet_Arad_DeleteEffect is null.");
        return 1345;
    }

    const Packet_Arad_DeleteEffect& input =
        *reinterpret_cast<const Packet_Arad_DeleteEffect*>(payload);
    GlobalData::s_serverStateManager_->deleteState(input.effect);
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
