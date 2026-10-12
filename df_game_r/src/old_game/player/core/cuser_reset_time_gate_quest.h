#ifndef OLDSRV_OLD_GAME_USERINFO_CUSER_RESET_TIME_GATE_QUEST_H_
#define OLDSRV_OLD_GAME_USERINFO_CUSER_RESET_TIME_GATE_QUEST_H_

class CUser;

// IDA unit: 06_userinfo_stat_inventory/CUserModifier.  The old game-world
// iterator invokes vtable slot zero with (modifier, user).
class CUserModifier {
public:
    // IDA_EA: 0x84F0878
    // IDA_NAME: _ZN13CUserModifierC2Ev
    // DEMANGLED: CUserModifier::CUserModifier(void)
    CUserModifier();

    virtual int Modify(CUser* user) = 0;
};

// IDA unit: 06_userinfo_stat_inventory/CUserResetTimeGateQuest.
class CUserResetTimeGateQuest : public CUserModifier {
public:
    // IDA_EA: 0x84F0886
    // IDA_NAME: _ZN23CUserResetTimeGateQuestC2Ev
    // DEMANGLED: CUserResetTimeGateQuest::CUserResetTimeGateQuest(void)
    CUserResetTimeGateQuest();

    // IDA_EA: 0x84F07EC
    // IDA_NAME: _ZN23CUserResetTimeGateQuest6ModifyEP5CUser
    // DEMANGLED: CUserResetTimeGateQuest::Modify(CUser *)
    int Modify(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(CUserModifier) == 0x04u,
              "CUserModifier must retain its one-vptr ABI");
static_assert(sizeof(CUserResetTimeGateQuest) == 0x04u,
              "CUserResetTimeGateQuest must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_CUSER_RESET_TIME_GATE_QUEST_H_
