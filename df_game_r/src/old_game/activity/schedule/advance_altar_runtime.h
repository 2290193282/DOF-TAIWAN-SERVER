#ifndef OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_RUNTIME_H_

class CUser;
struct MSG_CHARAC_SELECT;
struct MSG_MONSTER_DIE;

namespace advancealtar {

namespace EplpType {
// IDA ABI: advancealtar::EplpType::T is the four-byte value read from CMD75.
// The original routine only accepts values 0, 1 and 2, so retain the opaque
// ordinal representation rather than assigning speculative user-facing names.
enum T {
    VALUE_0 = 0,
    VALUE_1 = 1,
    VALUE_2 = 2
};
}  // namespace EplpType

struct _CharacAdvanceAltarDbData;

// IDA unit: 99_misc_global/advancealtar/DBAndExchangeServerProc.
// The two parameters in the original mangled name prove this is a static
// class operation: the user is the first ABI argument, not a fabricated
// DBAndExchangeServerProc receiver.
class DBAndExchangeServerProc {
public:
    // IDA_EA: 0x8133784
    // IDA_NAME: _ZN12advancealtar23DBAndExchangeServerProc22requestLoadFromDBAndESER5CUserR17MSG_CHARAC_SELECT
    // DEMANGLED: advancealtar::DBAndExchangeServerProc::requestLoadFromDBAndES(CUser &,MSG_CHARAC_SELECT &)
    static int requestLoadFromDBAndES(CUser& user,
                                      MSG_CHARAC_SELECT& select);

    // IDA_EA: 0x813394C
    // IDA_NAME: _ZN12advancealtar23DBAndExchangeServerProc13getFromMemoryERK5CUserRNS_25_CharacAdvanceAltarDbDataE
    // DEMANGLED: advancealtar::DBAndExchangeServerProc::getFromMemory(CUser const&,advancealtar::_CharacAdvanceAltarDbData &)
    // The old body has no return-value consumer and leaves EAX untouched after
    // its final side effect, so this is a void operation rather than IDA's
    // speculative int return inference.
    static void getFromMemory(const CUser& user,
                              _CharacAdvanceAltarDbData& data);
};

// IDA unit: 99_misc_global/advancealtar/DB_Update.  The cache-export path
// needs only this real request producer; dispatcher construction and SQL
// update handling remain separate identities.
class DB_Update {
public:
    // IDA_EA: 0x813EB38
    // IDA_NAME: _ZN12advancealtar9DB_Update11makeRequestEiRKNS_25_CharacAdvanceAltarDbDataE
    // DEMANGLED: advancealtar::DB_Update::makeRequest(int,advancealtar::_CharacAdvanceAltarDbData const&)
    static void makeRequest(int user_id,
                            const _CharacAdvanceAltarDbData& data);
};

class Manager {
public:
    // Original data symbol 0x09370094.  The production image initializes it
    // to one; the old GM command may raise it only within 1..50.
    static int gmTimeSpeed_;

    // IDA_EA: 0x813090E / 0x81309F0
    // IDA_NAME: _ZN12advancealtar7Manager11selectStageEP5CUseriNS_15StageDifficulty1TE /
    //           _ZN12advancealtar7Manager20procErrorSelectStageEP5CUser10ENUM_ERROR
    // DEMANGLED: advancealtar::Manager::selectStage(CUser *,int,advancealtar::StageDifficulty::T) /
    //            advancealtar::Manager::procErrorSelectStage(CUser *,ENUM_ERROR)
    static int selectStage(CUser* user, int stage_index, int difficulty);
    static void procErrorSelectStage(CUser* user, int error);

    // IDA_EA: 0x8130AB2
    // IDA_NAME: _ZN12advancealtar7Manager9dieCharacEP5CUser
    // DEMANGLED: advancealtar::Manager::dieCharac(CUser *)
    static void dieCharac(CUser* user);

    // IDA_EA: 0x8130BB8
    // IDA_NAME: _ZN12advancealtar7Manager6dieMobEP5CUserttRK15MSG_MONSTER_DIE
    static int dieMob(CUser* user,
                      unsigned short monster_id,
                      unsigned short monster_type,
                      const MSG_MONSTER_DIE& message);

    // IDA_EA: 0x8130EB2
    // IDA_NAME: _ZN12advancealtar7Manager10finishLoadEP5CUser
    // DEMANGLED: advancealtar::Manager::finishLoad(CUser *)
    static int finishLoad(CUser* user);

    // IDA_EA: 0x8130FA8
    // IDA_NAME: _ZN12advancealtar7Manager10giveUpGameEP5CUser
    // DEMANGLED: advancealtar::Manager::giveUpGame(CUser *)
    static void giveUpGame(CUser* user);

    // IDA_EA: 0x813101E
    // IDA_NAME: _ZN12advancealtar7Manager4eplpEP5CUserNS_8EplpType1TE
    // DEMANGLED: advancealtar::Manager::eplp(CUser *,advancealtar::EplpType::T)
    static void eplp(CUser* user, EplpType::T command);

    // IDA_EA: 0x813105C
    // IDA_NAME: _ZN12advancealtar7Manager5pauseEP5CUserb
    // DEMANGLED: advancealtar::Manager::pause(CUser *,bool)
    static int pause(CUser* user, bool paused);

    // IDA_EA: 0x81310B2
    // IDA_NAME: _ZN12advancealtar7Manager9leaveUserEP5CUser
    // DEMANGLED: advancealtar::Manager::leaveUser(CUser *)
    static void leaveUser(CUser* user);
};

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_RUNTIME_H_
