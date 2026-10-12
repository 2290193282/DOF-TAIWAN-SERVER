#ifndef OLDSRV_GLOBAL_HADES_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_HADES_PUBLIC_ABI_H_

#include <stdint.h>

#include <map>

class CUser;

namespace XNuclear {

// IDA preserves this as an int-backed enum.  Keep the original spellings and
// values because they participate in the GoldMinus mangled signature.
enum ENUM_GOLD_CONTROL_REASON {
    ENUM_GOLD_CONTROL_REASON_AUCTION = 0,
    ENUM_GOLD_CONTROL_REASON_USER = 1,
    ENUM_GOLD_CONTROL_REASON_GOLD = 2,
};

// IDA exposes only the named, int-backed enum type at this boundary.  The
// old-server evidence does not recover enumerator values, so retain it as an
// opaque fixed-underlying-type declaration rather than fabricating them.
enum ENUM_HADES_SECURITY : int;

// Public typed surface for the CHades object embedded in CUser.  The complete
// 0x2c i386 layout is recovered from its C2/D2 and the immediately following
// Secu_GoldControl child, so callers do not need a speculative offset facade.
class CHades {
public:
    // IDA_EA: 0x8697BC6
    // IDA_NAME: _ZN8XNuclear6CHadesC2Ev
    // DEMANGLED: XNuclear::CHades::CHades(void)
    CHades();

    // IDA_EA: 0x8694F54
    // IDA_NAME: _ZN8XNuclear6CHadesD2Ev
    // DEMANGLED: XNuclear::CHades::~CHades()
    ~CHades();

    // IDA_EA: 0x84B91E8
    // IDA_NAME: _ZN8XNuclear6CHades4InitEP5CUser
    // DEMANGLED: XNuclear::CHades::Init(CUser *)
    CHades* Init(CUser* user);

    void Send_ReturnToVillage();
    int needLogin();

    // IDA_EA: 0x84B953E
    // IDA_NAME: _ZN8XNuclear6CHades5ExpUpEj
    // DEMANGLED: XNuclear::CHades::ExpUp(unsigned int)
    int ExpUp(unsigned int level);

    int Send_Login();

    // IDA_EA: 0x84B92EA
    // IDA_NAME: _ZN8XNuclear6CHades11Send_LogoutEv
    // DEMANGLED: XNuclear::CHades::Send_Logout(void)
    int Send_Logout();

    char* GetIP(char* out);
    int Send_UserStatus();
    int StdCateUpdate(uint16_t category, uint16_t value);

    // IDA_EA: 0x84B9328
    // IDA_NAME: _ZN8XNuclear6CHades14AddHackTypeCntEmmmm
    // DEMANGLED: XNuclear::CHades::AddHackTypeCnt(unsigned long,unsigned long,unsigned long,unsigned long)
    int AddHackTypeCnt(unsigned long hack_type,
                       unsigned long count,
                       unsigned long value1,
                       unsigned long value2);

    // IDA_EA: 0x84B9552
    // IDA_NAME: _ZN8XNuclear6CHades10UseFatigueEj
    // DEMANGLED: XNuclear::CHades::UseFatigue(unsigned int)
    CHades* UseFatigue(unsigned int fatigue);

    // IDA_EA: 0x84B93EE
    // IDA_NAME: _ZN8XNuclear6CHades15Send_ChangeGoldEv
    // DEMANGLED: XNuclear::CHades::Send_ChangeGold(void)
    void Send_ChangeGold();

    // IDA_EA: 0x84B93FA
    // IDA_NAME: _ZN8XNuclear6CHades8GoldPlusEjNS_24ENUM_GOLD_CONTROL_REASONE
    // DEMANGLED: XNuclear::CHades::GoldPlus(unsigned int,XNuclear::ENUM_GOLD_CONTROL_REASON)
    int GoldPlus(unsigned int money, ENUM_GOLD_CONTROL_REASON reason);

    int GoldMinus(unsigned int money, ENUM_GOLD_CONTROL_REASON reason);

    // IDA_EA: 0x84B949A
    // IDA_NAME: _ZN8XNuclear6CHades12GoldTrackingEhjPcjj
    // DEMANGLED: XNuclear::CHades::GoldTracking(unsigned char,unsigned int,char *,unsigned int,unsigned int)
    int GoldTracking(unsigned char tracking_type,
                     unsigned int account_id,
                     char* ip,
                     unsigned int money,
                     unsigned int used_fatigue);

    // IDA_EA: 0x84B95A8
    // IDA_NAME: _ZN8XNuclear6CHades14ChangeSecurityENS_19ENUM_HADES_SECURITYEj
    // DEMANGLED: XNuclear::CHades::ChangeSecurity(XNuclear::ENUM_HADES_SECURITY,unsigned int)
    int ChangeSecurity(ENUM_HADES_SECURITY security, unsigned int value);

    // IDA_EA: 0x84B95BC
    // IDA_NAME: _ZN8XNuclear6CHades14RunHadesActionEjjjjPc
    // DEMANGLED: XNuclear::CHades::RunHadesAction(unsigned int,unsigned int,unsigned int,unsigned int,char *)
    unsigned int RunHadesAction(unsigned int action_flags,
                                unsigned int punish_days,
                                unsigned int apply_switch,
                                unsigned int reserved,
                                char* reason);

private:
    friend class ::CUser;

    CHades(const CHades&);
    CHades& operator=(const CHades&);

    struct SecurityState {
        uint8_t reward_enabled;       // CHades +0x28 / CUser +0x08eccc
        uint8_t security_type;        // CHades +0x29 / CUser +0x08eccd
        uint8_t reward_type;          // CHades +0x2a / CUser +0x08ecce
        int8_t security_grade;        // CHades +0x2b / CUser +0x08eccf
    };
    static_assert(sizeof(SecurityState) == 0x4u,
                  "XNuclear::CHades security state must stay four bytes");

    std::map<uint16_t, uint16_t> standard_category_counts_;  // +0x00
    uint32_t raw_18_;                                         // +0x18
    uint32_t fatigue_counter_;                                // +0x1c
    CUser* owner_;                                            // +0x20
    int32_t login_conn_sec_;                                  // +0x24
    SecurityState security_state_;                            // +0x28
};

#if defined(__i386__)
static_assert(sizeof(CHades) == 0x2cu,
              "XNuclear::CHades must preserve its old i386 extent");
#endif

}  // namespace XNuclear

#endif  // OLDSRV_GLOBAL_HADES_PUBLIC_ABI_H_
