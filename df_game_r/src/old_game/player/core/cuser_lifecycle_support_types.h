#ifndef OLDSRV_OLD_GAME_USERINFO_CUSER_LIFECYCLE_SUPPORT_TYPES_H_
#define OLDSRV_OLD_GAME_USERINFO_CUSER_LIFECYCLE_SUPPORT_TYPES_H_

// Physical direct-child layouts used only by CUser's recovered C2/D2
// lifetime boundary.  Every non-trivial child recovered from the old C2/D2
// remains a real C++ member below; scalar-only portions are named separately
// instead of being used as a substitute for a container or another class.

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <queue>
#include <vector>

#include "infra/network/packet/PacketHeader.h"
#include "infra/security/policy/codehack_checksum.h"

class CUser;
struct MSG_CHECK_MAIL_RECEIVER;
struct MSG_MONSTER_DIE;

namespace WongWork {
enum ENUM_HACKTYPE : int;
}  // namespace WongWork


// 140828 uses the canonical packed CS_Header directly.  A full native build
// does not retain the pre-upgrade `_header` name-only mangled-symbol bridge.

struct CPacketTimeChecker {
    CPacketTimeChecker();
    ~CPacketTimeChecker();

    CPacketTimeChecker* reset();
    int check();

    // IDA data: 0x93ACE48=100, 0x93ACE4C=1.
    static int validCount_;
    static int validSecond_;

    uint32_t packet_count_;       // +0x00
    uint32_t last_packet_time_;   // +0x04
};

struct CPacketVerifier {
    CPacketVerifier();
    ~CPacketVerifier();

    CPacketVerifier* Init();
    int GetSequence();
    int UpdatePacket(const CS_Header& header);

    // IDA data: 0x93ACE38=5, 0x93ACE3C=1, 0x93ACE40=6.
    static int m_nVaridCount_;
    static int m_nVaridSecond_;
    static int m_nAuctionVaridCount_;

    uint32_t raw_00_;             // +0x00
    uint32_t last_packet_id_;     // +0x04
    uint32_t last_packet_value_;  // +0x08
    uint32_t duplicate_count_;    // +0x0c
    uint32_t duplicate_tick_;     // +0x10
    uint16_t sequence_;           // +0x14
    uint16_t pad_16_17_;
    uint32_t auction_count_;      // +0x18
    uint32_t auction_tick_;       // +0x1c
    CPacketTimeChecker time_checker_;  // +0x20
};

// IDA unit: 99_misc_global/CodeHackCheckStorage.  Its sole non-trivial
// member is the queue; the old D2 destroys this exact queue.  The old server
// uses the 0x28-byte i386 libstdc++ deque representation.  The immediately
// following expanded CUser+0x79974 dword is CUser::std_drop_rate_, proven independently
// by setStdDropRate/getStdDropRate; it is not deque tail storage.

// IDA unit: 99_misc_global/CodeHackCheckRangeEx.  This is the typed range
// payload sent by CUser::checkCodeChecksumRange: a MAX_PATH-sized file name
// followed by the exact two 32-bit range values.
struct CodeHackCheckRangeEx {
    CodeHackCheckRangeEx();

    char* SetFileName(const char* source);
    const char* GetFileName() const;
    char* setData(const char* source);

    char file_name_[0x104];  // +0x000
    uint32_t range_start_;   // +0x104
    uint32_t range_size_;    // +0x108
};

struct CodeHackCheckStorage {
    CodeHackCheckStorage();
    ~CodeHackCheckStorage();

    bool IsEmpty();
    bool IsFull();
    int PushData(const CodeHackChecksumEx* value);
    int PopData(char* file_name);
    int PopData();
    int IsRightChecksum(const CodeHackChecksum* checksum);
    int reset();

    std::queue<CodeHackChecksumEx> checksums_;  // +0x00 (target: 0x28)
};

// IDA unit: 02_login_account/Secu_AccountHacking.  The original C2/D2 are
// scalar-only and the direct next child establishes the 0x14-byte extent.
namespace AccountHackingType {
enum T : int {
    GOLD_70_PERCENT = 0,
    GOLD_50_PERCENT = 1,
    GOLD_30_PERCENT = 2,
    GOLD_DETECTION = 3,
    AVATAR_DETECTION = 4,
    AVATAR_REPEAT = 5,
};
}  // namespace AccountHackingType

struct Secu_AccountHacking {
    // IDA_EA: 0x8278A48
    // IDA_NAME: _ZN19Secu_AccountHacking27checkAccountHackingHackTypeEN8WongWork13ENUM_HACKTYPEE
    // DEMANGLED: Secu_AccountHacking::checkAccountHackingHackType(
    //     WongWork::ENUM_HACKTYPE)
    static int checkAccountHackingHackType(WongWork::ENUM_HACKTYPE hack_type);

    Secu_AccountHacking();
    ~Secu_AccountHacking();

    // IDA_EA: 0x8278A98
    // IDA_NAME: _ZN19Secu_AccountHacking4initEP5CUser
    // DEMANGLED: Secu_AccountHacking::init(CUser *)
    Secu_AccountHacking* init(CUser* user);

    Secu_AccountHacking* resetInfo();

    // IDA_EA: 0x8279020
    // IDA_NAME: _ZN19Secu_AccountHacking12setLoginTimeEv
    // DEMANGLED: Secu_AccountHacking::setLoginTime(void)
    Secu_AccountHacking* setLoginTime();

    // IDA_EA: 0x8278B48
    // IDA_NAME: _ZN19Secu_AccountHacking8sendMailEPKcP23MSG_CHECK_MAIL_RECEIVER
    // DEMANGLED: Secu_AccountHacking::sendMail(char const*,MSG_CHECK_MAIL_RECEIVER *)
    char sendMail(const char* charac_name,
                  MSG_CHECK_MAIL_RECEIVER* message);

    // IDA_EA: 0x8278CCE
    // IDA_NAME: _ZN19Secu_AccountHacking8sendGoldEPKcjjjhPc
    // DEMANGLED: Secu_AccountHacking::sendGold(char const*,unsigned int,unsigned int,unsigned int,unsigned char,char *)
    char sendGold(const char* charac_name,
                  uint32_t account_id,
                  uint32_t gold,
                  uint32_t current_gold,
                  uint8_t tracking_type,
                  char* ip);

    // IDA_EA: 0x827903C
    // IDA_NAME: _ZN19Secu_AccountHacking9setPattenEN18AccountHackingType1TEPKcjj
    char setPatten(AccountHackingType::T type,
                   const char* charac_name,
                   uint32_t account_id,
                   uint32_t gold);

    // IDA_EA: 0x827912A
    // IDA_NAME: _ZN19Secu_AccountHacking19getMatchingHacktypeEN18AccountHackingType1TE
    static int getMatchingHacktype(AccountHackingType::T type);

    // IDA_EA: 0x8279258
    // IDA_NAME: _ZN19Secu_AccountHacking12isGoldPattenEN18AccountHackingType1TE
    // DEMANGLED: Secu_AccountHacking::isGoldPatten(AccountHackingType::T)
    static int isGoldPatten(AccountHackingType::T type);

    // IDA_EA: 0x8279282
    // IDA_NAME: _ZN19Secu_AccountHacking14isAvatarPattenEN18AccountHackingType1TE
    // DEMANGLED: Secu_AccountHacking::isAvatarPatten(AccountHackingType::T)
    static int isAvatarPatten(AccountHackingType::T type);

    // IDA_EA: 0x8279182
    // IDA_NAME: _ZN19Secu_AccountHacking22isSameAccountCharacterEPKc
    int isSameAccountCharacter(const char* charac_name);

    // IDA_EA: 0x8278AEC / 0x8278F16 / 0x82792A0.
    int isAvatar(uint32_t item_id);
    char sendAvatar(const char* charac_name, uint32_t account_id);
    int isDetectionGoldPatten();

    uint8_t detection_flags_[6];  // +0x00
    uint8_t pad_06_07_[2];        // +0x06
    uint32_t avatar_count_;       // +0x08
    uint32_t login_time_;         // +0x0c
    CUser* owner_;                // +0x10
};

// IDA unit: 99_misc_global/STGuildAgitDBInfo.
struct STGuildAgitDBInfo {
    STGuildAgitDBInfo();

    uint8_t enabled_;  // +0x00
};

// IDA unit: 99_misc_global/stLookOuts and tyIPCounters.  They are vector
// element objects, so their concrete definitions are required before CUser
// can own the three vector members below.
struct stLookOuts {
    stLookOuts(uint32_t first = 0, uint32_t second = 0);
    ~stLookOuts();

    uint32_t first_;
    uint32_t second_;
};

struct tyIPCounters {
    tyIPCounters(uint32_t first = 0, uint32_t second = 0);
    ~tyIPCounters();

    uint32_t first_;
    uint32_t second_;
};

// IDA unit: 99_misc_global/Secu_MonsterCheck and
// 05_world_enter_village/Secu_HackLogCheck.  No hidden container is present;
// the records below are the exact scalar groups touched by Check223/224/225.
#pragma pack(push, 1)
struct SecuMonsterCheck223State {
    uint16_t monster_id_;       // +0x00
    uint16_t monster_x_;        // +0x02
    uint16_t monster_y_;        // +0x04
    uint16_t check_count_;      // +0x06
    uint16_t violation_state_;  // +0x08
};

struct SecuMonsterCheck224State {
    uint16_t monster_id_;       // +0x00
    uint32_t tick_;             // +0x02
    uint16_t check_count_;      // +0x06
    uint16_t violation_state_;  // +0x08
};

struct SecuMonsterCheck225State {
    uint16_t monster_id_;       // +0x00
    uint16_t pad_02_03_;
    uint32_t tick_;             // +0x04
    uint16_t check_count_;      // +0x08
    uint16_t violation_state_;  // +0x0a
};

struct SecuMonsterLastDieInfo {
    uint16_t monster_id_;       // +0x00
    uint16_t monster_x_;        // +0x02
    uint16_t monster_y_;        // +0x04
    uint16_t pad_06_07_;
    uint32_t tick_;             // +0x08
};
#pragma pack(pop)

struct Secu_MonsterCheck {
    Secu_MonsterCheck();
    ~Secu_MonsterCheck();

    // IDA_EA: 0x827ABEC
    // IDA_NAME: _ZN17Secu_MonsterCheck10SetUserPtrEP5CUser
    // DEMANGLED: Secu_MonsterCheck::SetUserPtr(CUser *)
    Secu_MonsterCheck* SetUserPtr(CUser* user);

    // IDA_EA: 0x827AC7A
    // IDA_NAME: _ZN17Secu_MonsterCheck5ResetEv
    // DEMANGLED: Secu_MonsterCheck::Reset(void)
    Secu_MonsterCheck* Reset();
    // IDA_EA: 0x827ABFA / 0x827AD98 / 0x827AF78 / 0x827B100.
    void SetMonsterInfo(const MSG_MONSTER_DIE* message);
    int Check223();
    int Check224();
    int Check225();
    void init223(const MSG_MONSTER_DIE* message);
    void init224(const MSG_MONSTER_DIE* message);
    void init225(const MSG_MONSTER_DIE* message);

    CUser* user_;                                // +0x00
    uint8_t check_223_enabled_;                  // +0x04
    uint8_t check_224_enabled_;                  // +0x05
    uint8_t check_225_enabled_;                  // +0x06
    uint8_t pad_07_;
    SecuMonsterCheck223State check_223_;         // +0x08
    SecuMonsterCheck224State check_224_;         // +0x12
    SecuMonsterCheck225State check_225_;         // +0x1c
    SecuMonsterLastDieInfo last_die_;            // +0x28
    CUser* init_user_;                           // +0x34
};

struct Secu_HackLogCheck {
    Secu_HackLogCheck();
    ~Secu_HackLogCheck();

    // IDA_EA: 0x827A7D0
    // IDA_NAME: _ZN17Secu_HackLogCheck4InitEP5CUser
    // DEMANGLED: Secu_HackLogCheck::Init(CUser *)
    Secu_MonsterCheck* Init(CUser* user);

    // IDA_EA: 0x827A7F4
    // IDA_NAME: _ZN17Secu_HackLogCheck7MoveMapEv
    // DEMANGLED: Secu_HackLogCheck::MoveMap(void)
    int MoveMap();

    // IDA_EA: 0x827A81C
    // IDA_NAME: _ZN17Secu_HackLogCheck6DieMobEPK15MSG_MONSTER_DIE
    int DieMob(const MSG_MONSTER_DIE* message);

    Secu_MonsterCheck monster_check_;  // +0x00
    uint8_t monster_enabled_;          // +0x38
    uint8_t log_enabled_;              // +0x39
    uint8_t audit_enabled_;            // +0x3a
    uint8_t pad_3b_;
};

// IDA unit: 99_misc_global/PIReceiverManager.  The vptr at +0 and rolling
// key at +4 are evidenced by the original C2; a virtual D2 retains the old
// object model rather than modelling it as a scalar blob.
class PIReceiverManager {
public:
    PIReceiverManager();
    virtual ~PIReceiverManager();

    bool CheckHashReceived(unsigned char* payload,
                           unsigned int length,
                           unsigned int hash,
                           bool reset);

private:
    uint8_t key_;                 // +0x04 on i386
    uint8_t pad_05_07_[3];
};

// IDA unit: 08_quest_skill_growth/GrowthWeaponEvent.  Its C2 writes the
// fields below directly and has no non-trivial child.
struct GrowthWeaponEvent {
    GrowthWeaponEvent();

    uint8_t enabled_;             // +0x00: event enable flag
    uint8_t pad_01_03_[3];
    uint32_t charac_no_;          // +0x04, checked by CMD505/CMD506
    uint32_t event_value_;        // +0x08, persisted by SIG_GROWTH_WEAPON_EVENT
    uint32_t infinity_weapon_id_; // +0x0c, persisted by SIG_GROWTH_WEAPON_EVENT
    uint16_t timepiece_;          // +0x10, incremented by CMD506
    uint16_t server_group_;       // +0x12, checked by CMD505/CMD506
};

// IDA unit: 02_login_account/CharacLevelUpGiftAccountOnce.  The old C2/D2
// construct and destroy the multimap at the object base.
struct CharacLevelUpGiftAccountOnce {
    CharacLevelUpGiftAccountOnce();
    ~CharacLevelUpGiftAccountOnce();

    std::multimap<int, int> gifts_;  // +0x00
};

#if defined(__i386__)
static_assert(sizeof(CPacketTimeChecker) == 0x08u,
              "CPacketTimeChecker must preserve old i386 size");
static_assert(sizeof(CS_Header) == 0x0du,
              "140828 C2S frame header must stay 13 bytes");
static_assert(sizeof(CPacketVerifier) == 0x28u,
              "CPacketVerifier must preserve old i386 size");
static_assert(sizeof(CodeHackChecksum) == 0x20u,
              "CodeHackChecksum must preserve old i386 size");
static_assert(sizeof(CodeHackChecksumData) == 0x0cu,
              "CodeHackChecksumData must preserve old i386 vector base");
static_assert(sizeof(CodeHackChecksumEx) == 0x118u,
               "CodeHackChecksumEx must preserve old i386 size");
static_assert(sizeof(CodeHackCheckRangeEx) == 0x10cu,
              "CodeHackCheckRangeEx must preserve old i386 size");
static_assert(sizeof(CodeHackCheckStorage) == 0x28u,
              "CodeHackCheckStorage must preserve old i386 queue extent");
static_assert(sizeof(Secu_AccountHacking) == 0x14u,
              "Secu_AccountHacking must preserve old i386 size");
static_assert(sizeof(STGuildAgitDBInfo) == 0x01u,
              "STGuildAgitDBInfo must preserve old i386 size");
static_assert(sizeof(stLookOuts) == 0x08u,
              "stLookOuts must preserve old i386 size");
static_assert(sizeof(tyIPCounters) == 0x08u,
              "tyIPCounters must preserve old i386 size");
static_assert(sizeof(SecuMonsterCheck223State) == 0x0au,
              "SecuMonsterCheck223State must preserve old i386 extent");
static_assert(sizeof(SecuMonsterCheck224State) == 0x0au,
              "SecuMonsterCheck224State must preserve old i386 extent");
static_assert(sizeof(SecuMonsterCheck225State) == 0x0cu,
              "SecuMonsterCheck225State must preserve old i386 extent");
static_assert(sizeof(SecuMonsterLastDieInfo) == 0x0cu,
              "SecuMonsterLastDieInfo must preserve old i386 extent");
static_assert(sizeof(Secu_MonsterCheck) == 0x38u,
              "Secu_MonsterCheck must preserve old i386 size");
static_assert(sizeof(Secu_HackLogCheck) == 0x3cu,
              "Secu_HackLogCheck must preserve old i386 size");
static_assert(sizeof(PIReceiverManager) == 0x08u,
              "PIReceiverManager must preserve old i386 size");
static_assert(sizeof(GrowthWeaponEvent) == 0x14u,
              "GrowthWeaponEvent must preserve old i386 size");
static_assert(sizeof(CharacLevelUpGiftAccountOnce) == 0x18u,
              "CharacLevelUpGiftAccountOnce must preserve old i386 size");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_CUSER_LIFECYCLE_SUPPORT_TYPES_H_
