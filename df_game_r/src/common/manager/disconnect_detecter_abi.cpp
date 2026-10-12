#include "common/manager/disconnect_detecter_abi.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/timer_assault_lifecycle.h"
#include "common/manager/system_time.h"


namespace disconnect_detecter {

// IDA_EA: 0x82348CC
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecterC2Ev
CDisconnectDetecter::CDisconnectDetecter() : detect_times_() {}

// IDA_EA: 0x82348B8
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecterD2Ev
CDisconnectDetecter::~CDisconnectDetecter() {}

// IDA_EA: 0x82348DF
// IDA_NAME: _ZN19disconnect_detecter29GetInstanceDisconnectDetecterEv
// DEMANGLED: disconnect_detecter::GetInstanceDisconnectDetecter(void)
CDisconnectDetecter* GetInstanceDisconnectDetecter() {
    static CDisconnectDetecter instance;
    return &instance;
}

// IDA_EA: 0x84721F4
// IDA_NAME: _ZNK19disconnect_detecter19CDisconnectDetecter10_IsTimeoutEi
bool CDisconnectDetecter::_IsTimeout(int receive_second) const {
    return GlobalData::system_time()->getCurSec() - receive_second > 30;
}

// IDA_EA: 0x8472222
// IDA_NAME: _ZNK19disconnect_detecter19CDisconnectDetecter8_GetUserEii
CUser* CDisconnectDetecter::_GetUser(int uid, int unique_id) const {
    CUser* const user = G_CGameManager()->getUser(uid, 0);
    if (!user || static_cast<uint16_t>(user->get_unique_id()) !=
                     static_cast<uint16_t>(unique_id)) {
        return 0;
    }
    return user;
}

// IDA_EA: 0x8472272
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter12RegisterUserEP5CUser
// DEMANGLED: disconnect_detecter::CDisconnectDetecter::RegisterUser(CUser *)
int CDisconnectDetecter::RegisterUser(CUser* user) {
    const size_t old_size = detect_times_.size();
    const DetectedObjectKey key(user->GetUID(),
                                static_cast<uint16_t>(user->get_unique_id()));
    const std::pair<DetectTimeMap::iterator, bool> inserted =
        detect_times_.insert(std::make_pair(
            key, GlobalData::system_time()->getCurSec()));
    if (!inserted.second) {
        cMyTrace trace("bool disconnect_detecter::CDisconnectDetecter::RegisterUser(CUser*)",
                       37,
                       0);
        trace("RegisterUser() wrong");
        return 0;
    }
    if (old_size == 0u) {
        TimerDetectDisconnect::registNextTimer();
    }
    return 1;
}

// IDA_EA: 0x847236A
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter14UnregisterUserEP5CUser
// DEMANGLED: disconnect_detecter::CDisconnectDetecter::UnregisterUser(CUser *)
int CDisconnectDetecter::UnregisterUser(CUser* user) {
    const DetectedObjectKey key(user->GetUID(),
                                static_cast<uint16_t>(user->get_unique_id()));
    DetectTimeMap::iterator found = detect_times_.find(key);
    if (found == detect_times_.end()) {
        return 0;
    }
    detect_times_.erase(found);
    return 1;
}

// IDA_EA: 0x8472408
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter15RefreshRecvTimeEP5CUser
// DEMANGLED: disconnect_detecter::CDisconnectDetecter::RefreshRecvTime(CUser *)
int CDisconnectDetecter::RefreshRecvTime(CUser* user) {
    const DetectedObjectKey key(user->GetUID(),
                                static_cast<uint16_t>(user->get_unique_id()));
    DetectTimeMap::iterator found = detect_times_.find(key);
    if (found == detect_times_.end()) {
        return 0;
    }
    found->second = GlobalData::system_time()->getCurSec();
    return 1;
}

// IDA_EA: 0x84724B0
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter21DoDetectingDisconnectEv
// DEMANGLED: disconnect_detecter::CDisconnectDetecter::DoDetectingDisconnect(void)
int CDisconnectDetecter::DoDetectingDisconnect() {
    DetectTimeMap::iterator it = detect_times_.begin();
    while (it != detect_times_.end()) {
        if (!_IsTimeout(it->second)) {
            ++it;
            continue;
        }
        CUser* const user = _GetUser(it->first.first, it->first.second);
        if (user) {
            OnDisconnect(user);
        }
        detect_times_.erase(it++);
    }
    return !detect_times_.empty();
}

// IDA_EA: 0x84725BA
// IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter12OnDisconnectEP5CUser
// DEMANGLED: disconnect_detecter::CDisconnectDetecter::OnDisconnect(CUser *)
int CDisconnectDetecter::OnDisconnect(CUser* user) {
    return user->OnDisconnect();
}

}  // namespace disconnect_detecter
