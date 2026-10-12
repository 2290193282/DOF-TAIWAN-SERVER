#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_ARAD_INTERNAL_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_ARAD_INTERNAL_DISPATCHER_H_

// The DB-facing ARAD dispatcher owns a different map and lifecycle.  This
// header deliberately does not alter it: this type is the in-world, inner
// ARAD return dispatcher recovered from localjapan/Arad_InterDispatcher.cpp.
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"

#include <map>
#include <utility>

class CUser;

namespace ARAD {
namespace DISPATCHER {

// IDA unit: 13_database_dispatch/ARAD/Arad_InterDispatcher.
//
// i386 layout (0x08): [0x00] virtual-table pointer, [0x04] inner packet
// code.  The third virtual-table entry is dispatch_sig(CUser *,char *,int).
class Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A36A
    // IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherC2ENS_23ENUM_INTERNALPACKET_JPNE
    // DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::Arad_IInterDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
    explicit Arad_IInterDispatcher(ENUM_INTERNALPACKET_JPN code);

    // IDA_EA: 0x819A382
    // IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherD2Ev
    // DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::~Arad_IInterDispatcher()
    // IDA_EA: 0x819A3A8
    // IDA_NAME: _ZN4ARAD10DISPATCHER21Arad_IInterDispatcherD0Ev
    // DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::~Arad_IInterDispatcher()
    virtual ~Arad_IInterDispatcher();

    // Slot +8 of the recovered vtable.  Inter_DispatcherAradOnly strips the
    // four-byte inner code and calls this with (user, payload + 4, size - 4).
    virtual int dispatch_sig(CUser* user, char* payload, int length) = 0;

    // IDA_EA: 0x819A3C6
    // IDA_NAME: _ZNK4ARAD10DISPATCHER21Arad_IInterDispatcher8get_codeEv
    // DEMANGLED: ARAD::DISPATCHER::Arad_IInterDispatcher::get_code(void)const
    ENUM_INTERNALPACKET_JPN get_code() const;

private:
    ENUM_INTERNALPACKET_JPN code_;
};

typedef char Arad_IInterDispatcher_Size_Must_Be_0x08[
    (sizeof(Arad_IInterDispatcher) == 0x08u) ? 1 : -1];

// Forward declarations keep the B-owned map independent of the C-owned
// business implementations.  The concrete declarations live in
// old_game/creature_avatar/arad_internal_return_dispatchers.h and are only
// included by the .cpp that performs recovered Init() allocation.
class Arad_INTER_kLoadRewardEventItem;
class Arad_INTER_kSaveServerState;
class Arad_INTER_kLoadServerState;
class Arad_INTER_kDeleteServerState;
class Arad_INTER_kAvatarRoulette;
class Arad_INTER_kAradJumpingCharacter;
class Arad_INTER_kAvatarHiddenOptionChange;
class Arad_INTER_kUseAvatarRecharge;
class Arad_INTER_kEmblemCompound;
class Arad_INTER_kAvatarConvert;

class Arad_InternalDispatcher {
public:
    // IDA_EA: 0x81984D4
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcherC2Ev
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::Arad_InternalDispatcher(void)
    Arad_InternalDispatcher();

    // IDA_EA: 0x8198516
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcherD2Ev
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::~Arad_InternalDispatcher()
    ~Arad_InternalDispatcher();

    // IDA_EA: 0x8198598
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher4InitEv
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::Init(void)
    bool Init();

    // IDA_EA: 0x819899E
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher13GetDispatcherENS_23ENUM_INTERNALPACKET_JPNE
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::GetDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
    Arad_IInterDispatcher* GetDispatcher(ENUM_INTERNALPACKET_JPN code);

private:
    class DeleteDispatcher {
    public:
        // IDA_EA: 0x819A3D2
        // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher16DeleteDispatcherclERSt4pairIKNS_23ENUM_INTERNALPACKET_JPNEPNS0_21Arad_IInterDispatcherEE
        // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::DeleteDispatcher::operator()(std::pair<ARAD::ENUM_INTERNALPACKET_JPN const,ARAD::DISPATCHER::Arad_IInterDispatcher *> &)
        void operator()(std::pair<const ENUM_INTERNALPACKET_JPN,
                                  Arad_IInterDispatcher*>& entry);
    };

    // IDA_EA: 0x8198A02
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher17_registDispatcherENS_23ENUM_INTERNALPACKET_JPNEPNS0_21Arad_IInterDispatcherE
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::_registDispatcher(ARAD::ENUM_INTERNALPACKET_JPN,ARAD::DISPATCHER::Arad_IInterDispatcher *)
    bool _registDispatcher(ENUM_INTERNALPACKET_JPN code,
                           Arad_IInterDispatcher* dispatcher);

    // IDA_EA: 0x8198A48
    // IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_InternalDispatcher19_unregistDispatcherENS_23ENUM_INTERNALPACKET_JPNE
    // DEMANGLED: ARAD::DISPATCHER::Arad_InternalDispatcher::_unregistDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
    bool _unregistDispatcher(ENUM_INTERNALPACKET_JPN code);

    typedef std::map<ENUM_INTERNALPACKET_JPN, Arad_IInterDispatcher*>
        DispatcherMap;
    DispatcherMap dispatchers_;
};

// The recovered object contains only the libstdc++ i386 map (0x18 bytes);
// it intentionally has no root-dispatcher pointer or lifecycle state.
typedef char Arad_InternalDispatcher_Size_Must_Be_0x18[
    (sizeof(Arad_InternalDispatcher) == 0x18u) ? 1 : -1];

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_ARAD_INTERNAL_DISPATCHER_H_
