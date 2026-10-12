#ifndef OLDSRV_GLOBAL_DISCONNECT_DETECTER_ABI_H_
#define OLDSRV_GLOBAL_DISCONNECT_DETECTER_ABI_H_

#include <map>
#include <utility>

class CUser;

// The full detector container/map layout belongs to its unrecovered global
// owner. C gameplay needs only these two exact direct-call declarations.
namespace disconnect_detecter {

class CDisconnectDetecter {
public:
    // IDA_EA: 0x82348CC / 0x82348B8
    // IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecterC2Ev / _ZN19disconnect_detecter19CDisconnectDetecterD2Ev
    CDisconnectDetecter();
    ~CDisconnectDetecter();

    // IDA_EA: 0x8472272
    // IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter12RegisterUserEP5CUser
    // DEMANGLED: disconnect_detecter::CDisconnectDetecter::RegisterUser(CUser *)
    int RegisterUser(CUser* user);

    // IDA_EA: 0x847236A
    // IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter14UnregisterUserEP5CUser
    // DEMANGLED: disconnect_detecter::CDisconnectDetecter::UnregisterUser(CUser *)
    int UnregisterUser(CUser* user);

    // IDA_EA: 0x8472408
    // IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter15RefreshRecvTimeEP5CUser
    // DEMANGLED: disconnect_detecter::CDisconnectDetecter::RefreshRecvTime(CUser *)
    int RefreshRecvTime(CUser* user);

    // IDA_EA: 0x84724B0 / 0x84725BA
    // IDA_NAME: _ZN19disconnect_detecter19CDisconnectDetecter21DoDetectingDisconnectEv / _ZN19disconnect_detecter19CDisconnectDetecter12OnDisconnectEP5CUser
    int DoDetectingDisconnect();
    int OnDisconnect(CUser* user);

private:
    bool _IsTimeout(int receive_second) const;
    CUser* _GetUser(int uid, int unique_id) const;
    // IDA's only instance field is std::map<DetectedObjectKey, int>.  The
    // recovered key comparator orders (uid, unique_id) lexicographically, so
    // this canonical value-pair representation has the same key semantics
    // without exposing the detector's private key type cross-domain.
    typedef std::pair<int, int> DetectedObjectKey;
    typedef std::map<DetectedObjectKey, int> DetectTimeMap;
    DetectTimeMap detect_times_;
};

// IDA_EA: 0x82348DF
// IDA_NAME: _ZN19disconnect_detecter29GetInstanceDisconnectDetecterEv
// DEMANGLED: disconnect_detecter::GetInstanceDisconnectDetecter(void)
CDisconnectDetecter* GetInstanceDisconnectDetecter();

}  // namespace disconnect_detecter

#endif  // OLDSRV_GLOBAL_DISCONNECT_DETECTER_ABI_H_
