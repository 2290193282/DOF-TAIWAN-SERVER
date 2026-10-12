#include "old_data/db/legacy_dispatch/arad_internal_dispatcher.h"

#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include <algorithm>

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A36A
// IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherC2ENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::Arad_IInterDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
Arad_IInterDispatcher::Arad_IInterDispatcher(ENUM_INTERNALPACKET_JPN code)
    : code_(code) {}

// IDA_EA: 0x819A382
// IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::~Arad_IInterDispatcher()
// IDA_EA: 0x819A3A8
// IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::~Arad_IInterDispatcher()
Arad_IInterDispatcher::~Arad_IInterDispatcher() {}

// IDA_EA: 0x819A3C6
// IDA_NAME: _ZNK4ARAD10DISPATCHER21Arad_IInterDispatcher8get_codeEv
// DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::get_code(void)const
ENUM_INTERNALPACKET_JPN Arad_IInterDispatcher::get_code() const {
    return code_;
}

// IDA_EA: 0x81984D4
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcherC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::Arad_InternalDispatcher(void)
Arad_InternalDispatcher::Arad_InternalDispatcher() : dispatchers_() {
    // The original performs an explicit clear immediately after std::map C2.
    dispatchers_.clear();
}

// IDA_EA: 0x8198516
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcherD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::~Arad_InternalDispatcher()
Arad_InternalDispatcher::~Arad_InternalDispatcher() {
    std::for_each(dispatchers_.begin(), dispatchers_.end(),
                  DeleteDispatcher());
}

// IDA_EA: 0x8198598
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher4InitEv
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::Init(void)
bool Arad_InternalDispatcher::Init() {
    // Allocation and registration order is exact.  This remains a provider
    // only: no root dispatcher or global lifecycle registration is performed
    // here.
    Arad_IInterDispatcher* dispatcher = new Arad_INTER_kLoadRewardEventItem();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kSaveServerState();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kLoadServerState();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kDeleteServerState();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kAvatarRoulette();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kAradJumpingCharacter();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kAvatarHiddenOptionChange();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kUseAvatarRecharge();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kEmblemCompound();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }

    dispatcher = new Arad_INTER_kAvatarConvert();
    return !dispatcher || _registDispatcher(dispatcher->get_code(), dispatcher);
}

// IDA_EA: 0x819899E
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher13GetDispatcherENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::GetDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
Arad_IInterDispatcher* Arad_InternalDispatcher::GetDispatcher(
    ENUM_INTERNALPACKET_JPN code) {
    DispatcherMap::iterator it = dispatchers_.find(code);
    return it != dispatchers_.end() ? it->second : 0;
}

// IDA_EA: 0x8198A02
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher17_registDispatcherENS_23ENUM_INTERNALPACKET_JPNEPNS0_21Arad_IInterDispatcherE
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::_registDispatcher(ARAD::ENUM_INTERNALPACKET_JPN,ARAD::DISPATCHER::Arad_IInterDispatcher *)
bool Arad_InternalDispatcher::_registDispatcher(
    ENUM_INTERNALPACKET_JPN code,
    Arad_IInterDispatcher* dispatcher) {
    if (GetDispatcher(code)) {
        return false;
    }
    dispatchers_[code] = dispatcher;
    return true;
}

// IDA_EA: 0x8198A48
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher19_unregistDispatcherENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::_unregistDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
bool Arad_InternalDispatcher::_unregistDispatcher(
    ENUM_INTERNALPACKET_JPN code) {
    if (!GetDispatcher(code)) {
        return false;
    }
    dispatchers_.erase(code);
    return true;
}

// IDA_EA: 0x819A3D2
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher16DeleteDispatcherclERSt4pairIKNS_23ENUM_INTERNALPACKET_JPNEPNS0_21Arad_IInterDispatcherEE
// DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::DeleteDispatcher::operator()(std::pair<ARAD::ENUM_INTERNALPACKET_JPN const,ARAD::DISPATCHER::Arad_IInterDispatcher *> &)
void Arad_InternalDispatcher::DeleteDispatcher::operator()(
    std::pair<const ENUM_INTERNALPACKET_JPN,
              Arad_IInterDispatcher*>& entry) {
    if (entry.second) {
        delete entry.second;
    }
}

}  // namespace DISPATCHER
}  // namespace ARAD
