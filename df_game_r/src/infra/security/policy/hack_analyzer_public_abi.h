#ifndef OLDSRV_GLOBAL_HACK_ANALYZER_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_HACK_ANALYZER_PUBLIC_ABI_H_

#include <stdint.h>

#include <map>
#include <vector>

class CUser;
class PacketBuf;

// IDA name: ENUM_REPORT_4_HACK_FLAG.  This is a byte-sized bit field stored
// at CHackAnalyzer+0x04; retain the named type so the member ABI identities
// below demangle exactly as the old executable.
enum ENUM_REPORT_4_HACK_FLAG : unsigned char {
    REPORT_4_HACK_SPEED = 0x01,
    REPORT_4_HACK_DIFFICULTY = 0x02,
    REPORT_4_HACK_EQUIPMENT = 0x04,
    REPORT_4_HACK_MEMORY = 0x08,
    REPORT_4_HACK_CLIENT_FILE = 0x10,
    REPORT_4_HACK_UNKNOWN_20 = 0x20,
    REPORT_4_HACK_INCORRECT = 0x40,
    REPORT_4_HACK_MONSTER_DIE = 0x80,
};

namespace WongWork {

// The old ABI uses this named enum in the CHackAnalyzer member signature.
// Its underlying i386 representation is int; consumers only pass exact old
// numeric hack categories through it.
enum ENUM_HACKTYPE : int;

// IDA unit: WongWork_stHackInfo_t.  This is the concrete vector element at
// CHackAnalyzer+0x08; keeping its typed layout is necessary for vector C2/D2
// rather than treating the embedded analyzer as opaque storage.
struct stHackInfo_t {
    // IDA exports only D2 for this POD-like vector element.  Leave its C2
    // implicit rather than inventing a second lifecycle identity.
    ~stHackInfo_t();

    uint32_t account_id_;     // +0x00
    uint32_t charac_no_;      // +0x04
    uint16_t hack_type_;      // +0x08
    uint16_t pad_0a_0b_;
    int32_t payload_;         // +0x0c
};

// IDA_EA: 0x80F8C7E
// IDA_NAME: _ZN8WongWork13CHackAnalyzer16addServerHackCntEP5CUserNS_13ENUM_HACKTYPEEjjj
// DEMANGLED: WongWork::CHackAnalyzer::addServerHackCnt(CUser *,WongWork::ENUM_HACKTYPE,unsigned int,unsigned int,unsigned int)
//
// The canonical provider remains src/global/hack_analyzer_public_abi.cpp.
// This is its typed consumer declaration, not a second implementation.
class CHackAnalyzer {
public:
    // IDA_EA: 0x80F8F62
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer17ReqDBSaveHackInfoEjjtii
    // DEMANGLED: WongWork::CHackAnalyzer::ReqDBSaveHackInfo(unsigned int,unsigned int,unsigned short,int,int)
    // The first stack word is the CHackAnalyzer* receiver.  The original
    // symbol retains an obsolete five-parameter spelling, so keep its exact
    // linker name while expressing the recovered this-call at the source ABI.
    void ReqDBSaveHackInfo(unsigned int charac_no,
                                  unsigned short hack_type,
                                  int etc,
                                  int hack_count);

    // IDA_EA: 0x86950D2
    // IDA_NAME: _ZN8WongWork13CHackAnalyzerC2Ev
    // DEMANGLED: WongWork::CHackAnalyzer::CHackAnalyzer(void)
    CHackAnalyzer();

    // IDA_EA: 0x86951EA
    // IDA_NAME: _ZN8WongWork13CHackAnalyzerD2Ev
    // DEMANGLED: WongWork::CHackAnalyzer::~CHackAnalyzer()
    ~CHackAnalyzer();

    // IDA_EA: 0x80F79C0
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer8setUserPEP5CUser
    // DEMANGLED: WongWork::CHackAnalyzer::setUserP(CUser *)
    CHackAnalyzer* setUserP(CUser* user);

    // IDA_EA: 0x80F79CE
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer8getUserPEv
    // DEMANGLED: WongWork::CHackAnalyzer::getUserP(void)
    CUser* getUserP();

    // IDA_EA: 0x80F79E8
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer20beginCollectHackInfoEP5CUser
    // DEMANGLED: WongWork::CHackAnalyzer::beginCollectHackInfo(CUser *)
    int beginCollectHackInfo(CUser* user);

    // IDA_EA: 0x80F7A22
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer17analyzeHackReportEP5CUserP9PacketBuf
    // DEMANGLED: WongWork::CHackAnalyzer::analyzeHackReport(CUser *,PacketBuf *)
    int analyzeHackReport(CUser* user, PacketBuf* packet);

    // IDA_EA: 0x80F8080
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer14reportHackInfoEv
    // DEMANGLED: WongWork::CHackAnalyzer::reportHackInfo(void)
    int reportHackInfo();

    // IDA_EA: 0x80F81A6
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer13resetHackInfoEv
    // DEMANGLED: WongWork::CHackAnalyzer::resetHackInfo(void)
    CHackAnalyzer* resetHackInfo();

    // IDA_EA: 0x80F9856
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer15setBaseHackTypeENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CHackAnalyzer::setBaseHackType(WongWork::ENUM_HACKTYPE)
    int setBaseHackType(ENUM_HACKTYPE hack_type);

    // IDA_EA: 0x80F9868
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer15getBaseHackTypeEv
    // DEMANGLED: WongWork::CHackAnalyzer::getBaseHackType(void)
    int getBaseHackType();

    char addServerHackCnt(CUser* user,
                          ENUM_HACKTYPE hack_type,
                          unsigned int count,
                          unsigned int value1,
                          unsigned int value2);

    // IDA_EA: 0x80F908C
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer20ValidateFixMobDiePosEP5CUserttt
    // DEMANGLED: WongWork::CHackAnalyzer::ValidateFixMobDiePos(CUser *,unsigned short,unsigned short,unsigned short)
    // The mangled signature has no analyzer-object argument: this is the
    // original static fixed-monster coordinate validator used by CMD42.
    static char ValidateFixMobDiePos(CUser* user,
                                     uint16_t monster_type,
                                     uint16_t reported_x,
                                     uint16_t reported_y);

    // IDA_EA: 0x8695086
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer16getServerHackCntENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CHackAnalyzer::getServerHackCnt(WongWork::ENUM_HACKTYPE)
    int getServerHackCnt(ENUM_HACKTYPE hack_type);

    // IDA_EA: 0x869509C
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer18resetServerHackCntENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CHackAnalyzer::resetServerHackCnt(WongWork::ENUM_HACKTYPE)
    CHackAnalyzer* resetServerHackCnt(ENUM_HACKTYPE hack_type);

    // IDA_EA: 0x822EEF6
    // IDA_NAME: _ZNK8WongWork13CHackAnalyzer27getServerHackAccumulatedCntENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CHackAnalyzer::getServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE)const
    int getServerHackAccumulatedCnt(ENUM_HACKTYPE hack_type) const;

    // IDA_EA: 0x84EB57C
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer27setServerHackAccumulatedCntENS_13ENUM_HACKTYPEEj
    // DEMANGLED: WongWork::CHackAnalyzer::setServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE,unsigned int)
    // The old body stores the supplied count and returns this in EAX.
    CHackAnalyzer* setServerHackAccumulatedCnt(ENUM_HACKTYPE hack_type,
                                               unsigned int count);

    // IDA_EA: 0x86950B6
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer29resetServerHackAccumulatedCntENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CHackAnalyzer::resetServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE)
    CHackAnalyzer* resetServerHackAccumulatedCnt(ENUM_HACKTYPE hack_type);

    // IDA_EA: 0x85DF98C
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer22setLastMonsterDeadTimeEl
    // DEMANGLED: WongWork::CHackAnalyzer::setLastMonsterDeadTime(long)
    // The old leaf stores the value and preserves this in EAX.
    CHackAnalyzer* setLastMonsterDeadTime(long current_sec);

private:
    // IDA_EA: 0x80F7CCA / 0x80F7D82 / 0x80F7DCE.
    int _analyzeSpeedHack(unsigned int elapsed_client_msec);
    int _analyzeDifficultyHack(char value);
    int _analyzeMemoryHack(unsigned int value);

    // IDA_EA: 0x80F7FD2 / 0x80F8032 / 0x80F9876.
    int _addHackInfo(ENUM_REPORT_4_HACK_FLAG flag,
                     ENUM_HACKTYPE hack_type,
                     int payload);
    int _reportIncurrectReport();
    bool checkCollectedHack(ENUM_REPORT_4_HACK_FLAG flag) const;

    // IDA_EA: 0x80F906A
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer23ReqDBSaveHackUserPunishEjj
    // DEMANGLED: WongWork::CHackAnalyzer::ReqDBSaveHackUserPunish(unsigned int,unsigned int)
    // As above, this ABI-namespaced sender has no object-state access.
    static void ReqDBSaveHackUserPunish(unsigned int account_id,
                                        unsigned int punish_value);

    // IDA_EA: 0x80F81FC
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer17CheckExceptionCntENS_13ENUM_HACKTYPEEi
    // DEMANGLED: WongWork::CHackAnalyzer::CheckExceptionCnt(WongWork::ENUM_HACKTYPE,int)
    int CheckExceptionCnt(ENUM_HACKTYPE hack_type, int count);

    // IDA_EA: 0x80F8320
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer25CheckHackRateByPartyCountENS_13ENUM_HACKTYPEEi
    // DEMANGLED: WongWork::CHackAnalyzer::CheckHackRateByPartyCount(WongWork::ENUM_HACKTYPE,int)
    int CheckHackRateByPartyCount(ENUM_HACKTYPE hack_type, int member_count);

    // IDA_EA: 0x80F842E
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer24CheckHackRateByDifficultENS_13ENUM_HACKTYPEEii
    // DEMANGLED: WongWork::CHackAnalyzer::CheckHackRateByDifficult(WongWork::ENUM_HACKTYPE,int,int)
    int CheckHackRateByDifficult(ENUM_HACKTYPE hack_type,
                                 int level_delta,
                                 int dungeon_diff);

    // IDA_EA: 0x80F8C4E
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer17onUpdateHackCountEP5CUserNS_13ENUM_HACKTYPEEijj
    // DEMANGLED: WongWork::CHackAnalyzer::onUpdateHackCount(CUser *,WongWork::ENUM_HACKTYPE,int,unsigned int,unsigned int)
    int onUpdateHackCount(CUser* user,
                          ENUM_HACKTYPE hack_type,
                          int count,
                          unsigned int value1,
                          unsigned int value2);

    // IDA_EA: 0x80F8550
    // IDA_NAME: _ZN8WongWork13CHackAnalyzer24AddCheckRateHackTypeListEv
    // DEMANGLED: WongWork::CHackAnalyzer::AddCheckRateHackTypeList(void)
    // This lifecycle-only provider is kept with CUser's unactivated typed
    // C2/D2 recovery because old CHackAnalyzer C2 invokes it directly.
    int AddCheckRateHackTypeList();

    CUser* user_;                                  // +0x0000
    uint8_t report_flags_;                         // +0x0004
    uint8_t pad_0005_0007_[3];
    std::vector<stHackInfo_t> reports_;            // +0x0008
    std::vector<int> pending_hack_types_;          // +0x0014
    int32_t collect_begin_sec_;                    // +0x0020
    int32_t last_monster_dead_sec_;                // +0x0024
    uint32_t server_hack_count_[3900];             // +0x0028
    uint32_t server_hack_accumulated_[3900];       // +0x3d18
    ENUM_HACKTYPE base_hack_type_;                 // +0x7a08
    std::map<int, int> exception_counts_;          // +0x7a0c
    std::vector<int> party_rate_hack_types_;       // +0x7a24
    std::vector<int> difficult_rate_hack_types_;   // +0x7a30
};

#if defined(__i386__)
static_assert(sizeof(stHackInfo_t) == 0x10u,
              "WongWork::stHackInfo_t must preserve its old i386 extent");
static_assert(sizeof(CHackAnalyzer) == 0x7a3cu,
              "WongWork::CHackAnalyzer must preserve its old i386 extent");
#endif

}  // namespace WongWork

#endif  // OLDSRV_GLOBAL_HACK_ANALYZER_PUBLIC_ABI_H_
