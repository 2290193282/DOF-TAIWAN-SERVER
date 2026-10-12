#ifndef OLDSRV_DB_ARAD_DISPATCHER_H_
#define OLDSRV_DB_ARAD_DISPATCHER_H_

#include "old_data/db/core/db_dispatcher.h"

#include <map>

class CStreamGuard;
class Stream;

namespace ARAD {

// IDA type: ARAD::ENUM_INTERNALPACKET_JPN.  These are inner ARAD dispatch
// codes; all enter the DB queue through the distinct outer signal 0x2DC.
enum ENUM_INTERNALPACKET_JPN {
    kInternalPacketAradEventItemLog = 32,
    kInternalPacketSetCharacCreateTime = 36,
    kInternalPacketLoadRewardEventItem = 38,
    kInternalPacketSaveRewardEventItem = 39,
    kInternalPacketLoadServerState = 41,
    kInternalPacketSaveServerState = 42,
    // This is an in-world return code only.  It is emitted for the
    // server-state delete control packet and intentionally has no DB-side
    // Arad_IDatabaseDispatcher counterpart.
    kInternalPacketDeleteServerState = 43,
    kInternalPacketAvatarHiddenOptionSave = 47,
    kInternalPacketAvatarEnduranceSave = 49,
    kInternalPacketResetLimitedCreateCharacPerMid = 55,
};

namespace DISPATCHER {

// IDA_EA: 0x81977B6
// IDA_NAME: _ZN4ARAD10DISPATCHER24make_internal_stream_jpnER12CStreamGuardNS_23ENUM_INTERNALPACKET_JPNEi
// DEMANGLED: ARAD::DISPATCHER::make_internal_stream_jpn(CStreamGuard &,ARAD::ENUM_INTERNALPACKET_JPN,int)
int make_internal_stream_jpn(CStreamGuard& guard,
                             ENUM_INTERNALPACKET_JPN packet,
                             int uid);

class Arad_IDatabaseDispatcher {
public:
    explicit Arad_IDatabaseDispatcher(ENUM_INTERNALPACKET_JPN code);
    virtual ~Arad_IDatabaseDispatcher();

    virtual bool dispatch(int signal, int user_idx, Stream* stream) = 0;

    ENUM_INTERNALPACKET_JPN get_code() const;

private:
    ENUM_INTERNALPACKET_JPN code_;
};

class Arad_DB_kAradEventItemLog : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kAradEventItemLog();
    ~Arad_DB_kAradEventItemLog();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kSetCharacCreateTime : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kSetCharacCreateTime();
    ~Arad_DB_kSetCharacCreateTime();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kLoadRewardEventItem : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kLoadRewardEventItem();
    ~Arad_DB_kLoadRewardEventItem();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kSaveRewardEventItem : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kSaveRewardEventItem();
    ~Arad_DB_kSaveRewardEventItem();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kSaveServerState : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kSaveServerState();
    ~Arad_DB_kSaveServerState();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kLoadServerState : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kLoadServerState();
    ~Arad_DB_kLoadServerState();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kAvatarHiddenOptionSave : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kAvatarHiddenOptionSave();
    ~Arad_DB_kAvatarHiddenOptionSave();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kAvatarEnduranceSave : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kAvatarEnduranceSave();
    ~Arad_DB_kAvatarEnduranceSave();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DB_kResetLimitedCreateCharacPerMid
    : public Arad_IDatabaseDispatcher {
public:
    Arad_DB_kResetLimitedCreateCharacPerMid();
    ~Arad_DB_kResetLimitedCreateCharacPerMid();
    bool dispatch(int signal, int user_idx, Stream* stream);
};

class Arad_DatabaseDispatcher {
public:
    Arad_DatabaseDispatcher();
    ~Arad_DatabaseDispatcher();

    bool Init();
    Arad_IDatabaseDispatcher* GetDispatcher(ENUM_INTERNALPACKET_JPN code);

private:
    class DeleteDispatcher {
    public:
        void operator()(std::pair<const ENUM_INTERNALPACKET_JPN,
                                  Arad_IDatabaseDispatcher*>& entry);
    };

    bool _registDispatcher(ENUM_INTERNALPACKET_JPN code,
                           Arad_IDatabaseDispatcher* dispatcher);
    bool _unregistDispatcher(ENUM_INTERNALPACKET_JPN code);

    typedef std::map<ENUM_INTERNALPACKET_JPN, Arad_IDatabaseDispatcher*>
        DispatcherMap;
    DispatcherMap dispatchers_;
};

}  // namespace DISPATCHER
}  // namespace ARAD

// IDA unit: 13_database_dispatch/DB_AradDatabaseDispatcher.
class DB_AradDatabaseDispatcher : public DBDispatcher {
public:
    DB_AradDatabaseDispatcher();
    ~DB_AradDatabaseDispatcher();

    bool dispatch(int signal, int user_idx, Stream* stream);

private:
    ARAD::DISPATCHER::Arad_DatabaseDispatcher dispatcher_;
};

#endif  // OLDSRV_DB_ARAD_DISPATCHER_H_
