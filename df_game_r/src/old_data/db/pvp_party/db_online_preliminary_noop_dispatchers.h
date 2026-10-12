#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_ONLINE_PRELIMINARY_NOOP_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_ONLINE_PRELIMINARY_NOOP_DISPATCHERS_H_

#include "old_data/db/core/db_dispatcher.h"

// These DBTask entries have no payload logic in the old binary: each raw
// dispatch body returns success after the virtual call reaches the class.
class DB_LoadPowerWarStatueRanker : public DBDispatcher {
public:
    DB_LoadPowerWarStatueRanker();
    virtual ~DB_LoadPowerWarStatueRanker();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_LoadPowerWarInfo : public DBDispatcher {
public:
    DB_LoadPowerWarInfo();
    virtual ~DB_LoadPowerWarInfo();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_LoadOnlinePreliminaryMember : public DBDispatcher {
public:
    DB_LoadOnlinePreliminaryMember();
    virtual ~DB_LoadOnlinePreliminaryMember();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateOnlinePreliminaryMember : public DBDispatcher {
public:
    DB_UpdateOnlinePreliminaryMember();
    virtual ~DB_UpdateOnlinePreliminaryMember();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateOnlinePreliminaryMemberMatchList : public DBDispatcher {
public:
    DB_UpdateOnlinePreliminaryMemberMatchList();
    virtual ~DB_UpdateOnlinePreliminaryMemberMatchList();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_LoadOnlinePreliminaryTeam : public DBDispatcher {
public:
    DB_LoadOnlinePreliminaryTeam();
    virtual ~DB_LoadOnlinePreliminaryTeam();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateOnlinePreliminaryTeam : public DBDispatcher {
public:
    DB_UpdateOnlinePreliminaryTeam();
    virtual ~DB_UpdateOnlinePreliminaryTeam();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_LoadOnlinePreliminaryTeamMatchList : public DBDispatcher {
public:
    DB_LoadOnlinePreliminaryTeamMatchList();
    virtual ~DB_LoadOnlinePreliminaryTeamMatchList();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateOnlinePreliminaryTeamMatchList : public DBDispatcher {
public:
    DB_UpdateOnlinePreliminaryTeamMatchList();
    virtual ~DB_UpdateOnlinePreliminaryTeamMatchList();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_TOD_Random_Select_UserAPC : public DBDispatcher {
public:
    DB_TOD_Random_Select_UserAPC();
    virtual ~DB_TOD_Random_Select_UserAPC();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DB_LoadPowerWarStatueRanker) == 0x04u,
              "DB_LoadPowerWarStatueRanker must retain the i386 base ABI");
static_assert(sizeof(DB_LoadPowerWarInfo) == 0x04u,
              "DB_LoadPowerWarInfo must retain the i386 base ABI");
static_assert(sizeof(DB_LoadOnlinePreliminaryMember) == 0x04u,
              "DB_LoadOnlinePreliminaryMember must retain the i386 base ABI");
static_assert(sizeof(DB_UpdateOnlinePreliminaryMember) == 0x04u,
              "DB_UpdateOnlinePreliminaryMember must retain the i386 base ABI");
static_assert(sizeof(DB_UpdateOnlinePreliminaryMemberMatchList) == 0x04u,
              "DB_UpdateOnlinePreliminaryMemberMatchList ABI changed");
static_assert(sizeof(DB_LoadOnlinePreliminaryTeam) == 0x04u,
              "DB_LoadOnlinePreliminaryTeam must retain the i386 base ABI");
static_assert(sizeof(DB_UpdateOnlinePreliminaryTeam) == 0x04u,
              "DB_UpdateOnlinePreliminaryTeam must retain the i386 base ABI");
static_assert(sizeof(DB_LoadOnlinePreliminaryTeamMatchList) == 0x04u,
              "DB_LoadOnlinePreliminaryTeamMatchList ABI changed");
static_assert(sizeof(DB_UpdateOnlinePreliminaryTeamMatchList) == 0x04u,
              "DB_UpdateOnlinePreliminaryTeamMatchList ABI changed");
static_assert(sizeof(DB_TOD_Random_Select_UserAPC) == 0x04u,
              "DB_TOD_Random_Select_UserAPC must retain the i386 base ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_ONLINE_PRELIMINARY_NOOP_DISPATCHERS_H_
