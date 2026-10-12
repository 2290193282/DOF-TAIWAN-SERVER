// Deliberately not listed by any active manifest.  This pre-ABI provider
// contains only the newly recovered typed lifecycle identities; it is kept
// separate from active gameplay translation units until ownership/activation
// is explicitly approved.

#include "old_game/player/core/cuser.h"

#include <ctime>
#include <cstring>
#include <map>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/thread/mutex.h"
#include "common/manager/system_time.h"
#include "infra/security/crypto/cipher.h"
#include "infra/security/policy/packet_integrity.h"
#include "infra/security/policy/pi_sender_manager.h"
#include "old_game/social/party/party.h"

// ABI_IMPORT_EA: 0x84B6EE4
// ABI_IMPORT_NAME: _Z14NumberToStringji
extern const char* NumberToString(unsigned int value, int slot)
    __asm__("_Z14NumberToStringji");

namespace {

struct TutorialPacketBudget {
    TutorialPacketBudget() : count(0), second(0) {}
    uint32_t count;
    uint32_t second;
};

typedef std::map<const CPacketVerifier*, TutorialPacketBudget>
    TutorialPacketBudgetMap;

Mutex& TutorialPacketBudgetMutex() {
    static Mutex mutex;
    return mutex;
}

TutorialPacketBudgetMap& TutorialPacketBudgets() {
    static TutorialPacketBudgetMap budgets;
    return budgets;
}

void ResetTutorialPacketBudget(const CPacketVerifier* verifier) {
    MutexGuard guard(&TutorialPacketBudgetMutex());
    TutorialPacketBudgets().erase(verifier);
}

bool CheckTutorialPacketBudget(const CPacketVerifier* verifier) {
    const uint32_t now = static_cast<uint32_t>(std::time(0));
    MutexGuard guard(&TutorialPacketBudgetMutex());
    TutorialPacketBudget& budget = TutorialPacketBudgets()[verifier];
    if (now - budget.second >= 1u) {
        budget.second = now;
        budget.count = 1u;
        return true;
    }
    ++budget.count;
    return budget.count <= 512u;
}

bool IsExactTutorialLevelUpFrame(const CS_Header& c2s) {
    return c2s.cmd == ENUM_PACKETCLASS_CMDPACKET &&
           c2s.type == ENUM_CMDPACKET_TUTORIAL_LEVEL_UP &&
           c2s.length == sizeof(CS_Header);
}

}  // namespace

// This translation unit intentionally remains outside every active manifest.
// It is the sole provider for the newly recovered CUser/CUserCharacInfo
// lifecycle identities and their direct lifecycle-only children.

// IDA_EA: 0x864E15E
// IDA_NAME: _ZN15CUserCharacInfoC2Ev
// DEMANGLED: CUserCharacInfo::CUserCharacInfo(void)
//
// C2 order is represented by the physical CUserCharacInfo declaration:
// PvpResultType, battle-record map, then CDungeonClear.  The two character
// pointers are the scalar slots initialized by the old constructor.
CUserCharacInfo::CUserCharacInfo()
    : pvp_result_(),
      battle_records_(),
      dungeon_clear_() {
    // Old C2 constructs the three non-trivial children first, then writes the
    // two pointer slots at +0x10/+0x14 before resetSaveFlags().
    current_charac_ = 0;
    tag_charac_ = 0;
    resetSaveFlags();
}

// IDA_EA: 0x8697AC0
// IDA_NAME: _ZN15CUserCharacInfoD2Ev
// DEMANGLED: CUserCharacInfo::~CUserCharacInfo()
// The compiler emits the old reverse member lifetime: CDungeonClear,
// map<int, GameResultType>, then PvpResultType.
CUserCharacInfo::~CUserCharacInfo() {}

// IDA_EA: 0x8697B3A
// IDA_NAME: _ZN20UserMercenaryInfoMgrC2Ev
// DEMANGLED: UserMercenaryInfoMgr::UserMercenaryInfoMgr(void)
UserMercenaryInfoMgr::UserMercenaryInfoMgr() : mercenaries_() {}

// IDA_EA: 0x8697B4E
// IDA_NAME: _ZN20UserMercenaryInfoMgrD2Ev
// DEMANGLED: UserMercenaryInfoMgr::~UserMercenaryInfoMgr()
UserMercenaryInfoMgr::~UserMercenaryInfoMgr() {}

// IDA_EA: 0x86AD9E8
// IDA_NAME: _ZN8WongWork12CUserPremium11InitPremiumEv
// DEMANGLED: WongWork::CUserPremium::InitPremium(void)
int WongWork::CUserPremium::InitPremium() {
    std::memset(premium_info_, 0, sizeof(premium_info_));
    advantage_premium_count_ = 0;
    normal_premium_count_ = 0;
    advantage_fatigue_rate_ = 0;
    advantage_coin_ = 0;
    advantage_exp_rate_ = 0;
    unlimited_fatigue_ = false;
    over_skill_level_ = 0;
    cera_shop_discount_rate_ = 0;
    need_check_everyday_ = false;
    std::memset(over_equipable_level_, 0, sizeof(over_equipable_level_));
    gold_bonus_ = false;
    advantage_additional_info_.clear();
    calculated_additional_info_.clear();
    return_item_rates_.clear();
    return 0;
}

// IDA_EA: 0x8694B0E
// IDA_NAME: _ZN8WongWork12CUserPremiumC2Ev
// DEMANGLED: WongWork::CUserPremium::CUserPremium(void)
WongWork::CUserPremium::CUserPremium()
    : advantage_additional_info_(),
      calculated_additional_info_(),
      return_item_rates_() {
    InitPremium();
}

// IDA_EA: 0x8694B7A
// IDA_NAME: _ZN8WongWork12CUserPremiumD2Ev
// DEMANGLED: WongWork::CUserPremium::~CUserPremium()
WongWork::CUserPremium::~CUserPremium() {}

// IDA_EA: 0x8696D5C
// IDA_NAME: _ZN18CPacketTimeChecker5resetEv
// DEMANGLED: CPacketTimeChecker::reset(void)
CPacketTimeChecker* CPacketTimeChecker::reset() {
    packet_count_ = 0;
    last_packet_time_ = 0;
    return this;
}

// IDA data: 0x93ACE48 / 0x93ACE4C.
int CPacketTimeChecker::validCount_ = 100;
int CPacketTimeChecker::validSecond_ = 1;

// IDA_EA: 0x8696D74
// IDA_NAME: _ZN18CPacketTimeChecker5checkEv
// DEMANGLED: CPacketTimeChecker::check(void)
int CPacketTimeChecker::check() {
    ++packet_count_;
    const uint32_t now = static_cast<uint32_t>(std::time(0));
    if (now - last_packet_time_ >= static_cast<uint32_t>(validSecond_)) {
        last_packet_time_ = now;
        packet_count_ = 0;
    } else if (packet_count_ > static_cast<uint32_t>(validCount_)) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x8696DD8
// IDA_NAME: _ZN18CPacketTimeCheckerC2Ev
// DEMANGLED: CPacketTimeChecker::CPacketTimeChecker(void)
CPacketTimeChecker::CPacketTimeChecker() {}

// IDA_EA: 0x8696DDE
// IDA_NAME: _ZN18CPacketTimeCheckerD2Ev
// DEMANGLED: CPacketTimeChecker::~CPacketTimeChecker()
CPacketTimeChecker::~CPacketTimeChecker() {}

// IDA_EA: 0x8696DE4
// IDA_NAME: _ZN15CPacketVerifier4InitEv
// DEMANGLED: CPacketVerifier::Init(void)
CPacketVerifier* CPacketVerifier::Init() {
    raw_00_ = 0;
    last_packet_id_ = 0;
    last_packet_value_ = 0;
    duplicate_count_ = 0;
    duplicate_tick_ = 0;
    sequence_ = 0;
    auction_count_ = 0;
    auction_tick_ = 0;
    time_checker_.reset();
    ResetTutorialPacketBudget(this);
    return this;
}

// IDA data: 0x93ACE38 / 0x93ACE3C / 0x93ACE40.
int CPacketVerifier::m_nVaridCount_ = 5;
int CPacketVerifier::m_nVaridSecond_ = 1;
int CPacketVerifier::m_nAuctionVaridCount_ = 6;

// IDA_EA: 0x8696E48
// IDA_NAME: _ZN15CPacketVerifier11GetSequenceEv
// DEMANGLED: CPacketVerifier::GetSequence(void)
int CPacketVerifier::GetSequence() {
    return sequence_;
}

namespace {

int UpdatePacketWithRatePolicy(CPacketVerifier* verifier,
                               const CS_Header& c2s,
                               bool allow_tutorial_burst) {
    // Female-slayer tutorial room 61024 can emit ~360 empty CMD481 frames in
    // one second while its level-up animation converges.  Keep this exact
    // command on an independent budget; every business/state check remains in
    // Dispatcher_TutorialLevelUp and all other commands retain the old 100/s.
    const bool valid_rate = allow_tutorial_burst &&
                                IsExactTutorialLevelUpFrame(c2s)
        ? CheckTutorialPacketBudget(verifier)
        : verifier->time_checker_.check() == 1;
    if (!valid_rate) {
        return 3;
    }

    if (c2s.type >= ENUM_CMDPACKET_AUCTION_ASK_AVERAGE_PRICE &&
        c2s.type <= ENUM_CMDPACKET_AUCTION_MY_AUCTION_HISTORY &&
        ++verifier->auction_count_ >
            static_cast<uint32_t>(CPacketVerifier::m_nAuctionVaridCount_)) {
        const uint32_t now = static_cast<uint32_t>(std::time(0));
        if (now - verifier->auction_tick_ <
            static_cast<uint32_t>(CPacketVerifier::m_nVaridSecond_)) {
            return 4;
        }
        verifier->auction_count_ = 0;
        verifier->auction_tick_ = now;
    }

    if (c2s.cmd == verifier->raw_00_ &&
        c2s.type == verifier->last_packet_id_ &&
        c2s.checksum == verifier->last_packet_value_) {
        if (++verifier->duplicate_count_ >
            static_cast<uint32_t>(CPacketVerifier::m_nVaridCount_)) {
            const uint32_t now = static_cast<uint32_t>(std::time(0));
            if (now - verifier->duplicate_tick_ <
                static_cast<uint32_t>(CPacketVerifier::m_nVaridSecond_)) {
                return 1;
            }
            verifier->duplicate_count_ = 0;
            verifier->duplicate_tick_ = now;
        }
    } else {
        verifier->raw_00_ = c2s.cmd;
        verifier->last_packet_id_ = c2s.type;
        verifier->last_packet_value_ = c2s.checksum;
        verifier->duplicate_count_ = 0;
        verifier->duplicate_tick_ = static_cast<uint32_t>(std::time(0));
    }
    return 0;
}

}  // namespace

// 140828 native owner: the wire header is passed by its canonical type.
int CPacketVerifier::UpdatePacket(const CS_Header& c2s) {
    return UpdatePacketWithRatePolicy(this, c2s, false);
}

// IDA_EA: 0x8696FE0
// IDA_NAME: _ZN15CPacketVerifierC2Ev
// DEMANGLED: CPacketVerifier::CPacketVerifier(void)
CPacketVerifier::CPacketVerifier() : time_checker_() {
    Init();
}

// IDA_EA: 0x8697002
// IDA_NAME: _ZN15CPacketVerifierD2Ev
// DEMANGLED: CPacketVerifier::~CPacketVerifier()
CPacketVerifier::~CPacketVerifier() {
    ResetTutorialPacketBudget(this);
}

// IDA_EA: 0x8575E74
// IDA_NAME: _ZN20CodeHackCheckStorage5resetEv
// DEMANGLED: CodeHackCheckStorage::reset(void)
int CodeHackCheckStorage::reset() {
    while (!checksums_.empty()) {
        checksums_.pop();
    }
    return 0;
}

// IDA_EA: 0x8575C22
// IDA_NAME: _ZN20CodeHackCheckStorageC2Ev
// DEMANGLED: CodeHackCheckStorage::CodeHackCheckStorage(void)
CodeHackCheckStorage::CodeHackCheckStorage()
    : checksums_() {
    reset();
}

// IDA_EA: 0x8697B76
// IDA_NAME: _ZN20CodeHackCheckStorageD2Ev
// DEMANGLED: CodeHackCheckStorage::~CodeHackCheckStorage()
CodeHackCheckStorage::~CodeHackCheckStorage() {}

// IDA_EA: 0x8695AA4
// IDA_NAME: _ZN19Secu_AccountHackingC2Ev
// DEMANGLED: Secu_AccountHacking::Secu_AccountHacking(void)
Secu_AccountHacking::Secu_AccountHacking() {}

// IDA_EA: 0x8695AAA
// IDA_NAME: _ZN19Secu_AccountHackingD2Ev
// DEMANGLED: Secu_AccountHacking::~Secu_AccountHacking()
Secu_AccountHacking::~Secu_AccountHacking() {}

// IDA_EA: 0x86940C6
// IDA_NAME: _ZN17STGuildAgitDBInfoC2Ev
// DEMANGLED: STGuildAgitDBInfo::STGuildAgitDBInfo(void)
STGuildAgitDBInfo::STGuildAgitDBInfo() : enabled_(0) {}

// IDA_EA: 0x869380E
// IDA_NAME: _ZN10stLookOutsC2Ejj
// DEMANGLED: stLookOuts::stLookOuts(unsigned int,unsigned int)
stLookOuts::stLookOuts(uint32_t first, uint32_t second)
    : first_(first), second_(second) {}

// IDA_EA: 0x86A9254
// IDA_NAME: _ZN10stLookOutsD2Ev
// DEMANGLED: stLookOuts::~stLookOuts()
stLookOuts::~stLookOuts() {}

// IDA_EA: 0x8693824
// IDA_NAME: _ZN12tyIPCountersC2Ejj
// DEMANGLED: tyIPCounters::tyIPCounters(unsigned int,unsigned int)
tyIPCounters::tyIPCounters(uint32_t first, uint32_t second)
    : first_(first), second_(second) {}

// IDA_EA: 0x86A9446
// IDA_NAME: _ZN12tyIPCountersD2Ev
// DEMANGLED: tyIPCounters::~tyIPCounters()
tyIPCounters::~tyIPCounters() {}

// IDA_EA: 0x8695AB0
// IDA_NAME: _ZN17Secu_MonsterCheckC2Ev
// DEMANGLED: Secu_MonsterCheck::Secu_MonsterCheck(void)
Secu_MonsterCheck::Secu_MonsterCheck()
    : check_223_enabled_(0),
      check_224_enabled_(0),
      check_225_enabled_(0) {}

// IDA_EA: 0x8695ACA
// IDA_NAME: _ZN17Secu_MonsterCheckD2Ev
// DEMANGLED: Secu_MonsterCheck::~Secu_MonsterCheck()
Secu_MonsterCheck::~Secu_MonsterCheck() {}

// IDA_EA: 0x8695AD0
// IDA_NAME: _ZN17Secu_HackLogCheckC2Ev
// DEMANGLED: Secu_HackLogCheck::Secu_HackLogCheck(void)
Secu_HackLogCheck::Secu_HackLogCheck()
    : monster_check_(), monster_enabled_(1), log_enabled_(1), audit_enabled_(1) {}

// IDA_EA: 0x8695AF8
// IDA_NAME: _ZN17Secu_HackLogCheckD2Ev
// DEMANGLED: Secu_HackLogCheck::~Secu_HackLogCheck()
Secu_HackLogCheck::~Secu_HackLogCheck() {}

// IDA_EA: 0x808BF68
// IDA_NAME: _ZN17PIReceiverManagerC2Ev
// DEMANGLED: PIReceiverManager::PIReceiverManager(void)
PIReceiverManager::PIReceiverManager() : key_(39) {}

// IDA_EA: 0x808BF7E
// IDA_NAME: _ZN17PIReceiverManager17CheckHashReceivedEPhjjb
// DEMANGLED: PIReceiverManager::CheckHashReceived(unsigned char *,unsigned int,unsigned int,bool)
bool PIReceiverManager::CheckHashReceived(unsigned char* payload,
                                          unsigned int length,
                                          unsigned int hash,
                                          bool reset) {
    if (reset) {
        key_ = 39;
    }
    const uint8_t previous_key = key_;
    const uint8_t second_hash_byte = static_cast<uint8_t>(hash >> 8);
    key_ = static_cast<uint8_t>((((hash >> 8) & 0x300u) >> 4) |
                                ((second_hash_byte & 0x30u) >> 2) |
                                ((((hash >> 8) & 0x3000u) >> 12)) |
                                (second_hash_byte << 6));
    const uint8_t expected = static_cast<uint8_t>(
        (4u * (second_hash_byte & 0x0cu)) |
        ((((hash >> 8) & 0xc000u) >> 12)) |
        ((second_hash_byte & 0xc0u) >> 6) |
        ((((hash >> 8) & 0x0c00u) >> 4)));
    return length == 0 ||
           static_cast<uint8_t>(MakeHash(payload, length, previous_key)) ==
               expected;
}

// IDA_EA: 0x808C130
// IDA_NAME: _ZN17PIReceiverManagerD2Ev
// DEMANGLED: PIReceiverManager::~PIReceiverManager()
PIReceiverManager::~PIReceiverManager() {}

// IDA_EA: 0x8697068
// IDA_NAME: _ZN17GrowthWeaponEventC2Ev
// DEMANGLED: GrowthWeaponEvent::GrowthWeaponEvent(void)
GrowthWeaponEvent::GrowthWeaponEvent()
    : enabled_(0),
      charac_no_(0),
      event_value_(0),
      infinity_weapon_id_(0),
      timepiece_(0),
      server_group_(0) {}

// IDA_EA: 0x86970E6
// IDA_NAME: _ZN28CharacLevelUpGiftAccountOnceC2Ev
// DEMANGLED: CharacLevelUpGiftAccountOnce::CharacLevelUpGiftAccountOnce(void)
CharacLevelUpGiftAccountOnce::CharacLevelUpGiftAccountOnce() : gifts_() {
    gifts_.clear();
}

// IDA_EA: 0x8697C2A
// IDA_NAME: _ZN28CharacLevelUpGiftAccountOnceD2Ev
// DEMANGLED: CharacLevelUpGiftAccountOnce::~CharacLevelUpGiftAccountOnce()
CharacLevelUpGiftAccountOnce::~CharacLevelUpGiftAccountOnce() {}

// IDA_EA: 0x8647890
// IDA_NAME: _ZN5CUser7ParsingEi
// DEMANGLED: CUser::Parsing(int)
int CUser::Parsing(int received_bytes) {
    const int header_size = static_cast<int>(sizeof(CS_Header));
    int parsing_length = received_bytes + network_.remain_recv_len_;
    network_.reserved_04_ = GlobalData::s_systemTime_.getCurTickCount();
    network_.reserved_08_ = 0;

    if (parsing_length <= header_size - 1) {
        network_.remain_recv_len_ += received_bytes;
        network_.recv_cursor_ += received_bytes;
        network_.recv_flag_ = 1;
        return 1;
    }

    do {
        if (network_.remain_recv_len_) {
            network_.recv_cursor_ -= network_.remain_recv_len_;
        }

        CS_Header c2s;
        std::memcpy(&c2s, network_.recv_cursor_, sizeof(c2s));
        const int packet_size = static_cast<int>(c2s.length);
        if (packet_size <= header_size - 1 || packet_size > 0x578) {
            LogManager::logFormat(
                1, "user.cpp", "bool CUser::Parsing(int)", 0x6CE,
                "Recv Size[%d], Parsing Packet Size[%d], recvbuf_offset[%d] is Too Large",
                received_bytes, packet_size,
                static_cast<int>(network_.recv_cursor_ - network_.recv_buffer_));
            return 0;
        }
        if (parsing_length < packet_size) {
            break;
        }

        bool allow_tutorial_burst = false;
        if (IsExactTutorialLevelUpFrame(c2s) && get_state() == 5 &&
            get_charac_job() == oldsrv::config::kCharacterJobAtSwordman &&
            get_charac_level() >= 1 && get_charac_level() <= 2) {
            CParty* const party = GetParty();
            allow_tutorial_burst =
                party && party->battle_field().get_cur_map() == 61024;
        }
        const int verify_result =
            UpdatePacketWithRatePolicy(
                &packet_verifier_, c2s, allow_tutorial_burst);
        if (verify_result != 0) {
            LogManager::logFormat(
                1, "user.cpp", "bool CUser::Parsing(int)", 0x6F4,
                "[Packet Varifier Error %d][classification %d][packet id %d][ServerSequence: %d][ClientSequence: %d](m_id: %s)",
                verify_result, static_cast<int>(c2s.cmd),
                static_cast<int>(c2s.type), packet_verifier_.GetSequence(),
                static_cast<int>(c2s.seq),
                NumberToString(static_cast<unsigned int>(get_acc_id()), 0));
            if (verify_result != 4) {
                return 0;
            }
        } else {
            _NS_PI_2ND_RecvHash_CS(network_.recv_cursor_, this,
                                   static_cast<uint32_t>(packet_size));
            if (c2s.type != ENUM_CMDPACKET_DPROTO) {
                unsigned char decrypted[1400];
                // CMD_NCC is the first graph-0 protected frame after the
                // monitor-login path.  Keep only a local copy of its cipher
                // bytes long enough to emit correlation metadata below; do
                // not write packet or key material to the log.
                unsigned char encrypted_ncc[1400];
                const bool is_ncc = c2s.type == ENUM_CMDPACKET_NCC;
                if (is_ncc) {
                    std::memcpy(encrypted_ncc,
                                network_.recv_cursor_ + header_size,
                                static_cast<size_t>(packet_size - header_size));
                }
                int decrypted_size = static_cast<int>(sizeof(decrypted));
                const int payload_size = packet_size - header_size;
                const bool decrypt_ok = G_Cipher()->Decrypt(
                    c2s.type,
                    reinterpret_cast<unsigned char*>(
                        network_.recv_cursor_ + header_size),
                    payload_size, decrypted, &decrypted_size);
                // CMD 1 is the first 140828 login frame. Its first two
                // payload dwords are structural fields (the reference trace
                // has 0x00000000 and 0x00000158); logging only these lets us
                // distinguish a cipher/key mismatch from a CRC mismatch
                // without recording account, token, or session material.
                if (decrypt_ok && c2s.type == 1u &&
                    decrypted_size >= static_cast<int>(2u * sizeof(uint32_t))) {
                    uint32_t cmd1_prefix0 = 0u;
                    uint32_t cmd1_prefix1 = 0u;
                    std::memcpy(&cmd1_prefix0, decrypted, sizeof(cmd1_prefix0));
                    std::memcpy(&cmd1_prefix1, decrypted + sizeof(cmd1_prefix0),
                                sizeof(cmd1_prefix1));
                    LogManager::logFormat(
                        1, "user_diag.cpp", "bool CUser::Parsing(int)", 0,
                        "[DIAG0828][CMD1_SHAPE] prefix0=0x%08X prefix1=0x%08X",
                        static_cast<unsigned int>(cmd1_prefix0),
                        static_cast<unsigned int>(cmd1_prefix1));
                }
                if (is_ncc && decrypt_ok && decrypted_size >= 5) {
                    // The original CMD_NCC reader consumes exactly a four-byte
                    // NCC response hash and one response-type byte.  Checking
                    // that first semantic field before the generic CRC tells
                    // us whether graph-0 decoded with the session key or if a
                    // framing/padding difference is the remaining issue.
                    int decoded_response_hash = 0;
                    std::memcpy(&decoded_response_hash, decrypted,
                                sizeof(decoded_response_hash));
                    const unsigned int response_type =
                        static_cast<unsigned int>(decrypted[4]);
                    int expected_response_hash = 0;
                    int challenge = GetNonClientRandInt();
                    const int hash_ready = _NS_MakeHash_(
                        reinterpret_cast<char*>(&challenge), 4,
                        expected_response_hash);
                    uint32_t wire_response_hash = 0;
                    std::memcpy(&wire_response_hash, encrypted_ncc,
                                sizeof(wire_response_hash));
                    const uint32_t wire_fingerprint = Encryption::crc32N(
                        encrypted_ncc, payload_size);
                    const uint32_t plain_fingerprint = Encryption::crc32N(
                        decrypted, decrypted_size);
                    LogManager::logFormat(
                        1, "user_diag.cpp", "bool CUser::Parsing(int)", 0,
                        "[DIAG0828][CMD_NCC_CRYPT] payload=%d hash_ready=%d "
                        "wire_hash_match=%d plain_hash_match=%d "
                        "response_type=%u wire_fingerprint=0x%08X "
                        "plain_fingerprint=0x%08X",
                        payload_size, hash_ready,
                        (hash_ready == 1 &&
                         wire_response_hash ==
                             static_cast<uint32_t>(expected_response_hash)) ? 1 : 0,
                        (hash_ready == 1 &&
                         decoded_response_hash == expected_response_hash) ? 1 : 0,
                        response_type,
                        static_cast<unsigned int>(wire_fingerprint),
                        static_cast<unsigned int>(plain_fingerprint));
                }
                if (decrypted_size > 0) {
                    std::memcpy(network_.recv_cursor_ + header_size,
                                decrypted,
                                static_cast<size_t>(decrypted_size));
                }

                const uint32_t computed_crc32 = Encryption::crc32N(
                    network_.recv_cursor_ + offsetof(CS_Header, seq),
                    packet_size - static_cast<int>(offsetof(CS_Header, seq)));
                uint32_t server_checksum = computed_crc32;
                const uint32_t wire_checksum = c2s.checksum;
                uint32_t client_checksum = wire_checksum;
                _NS_PI_2ND_MakeChecksumTo1Byte2(server_checksum,
                                                 client_checksum);
                if (client_checksum != server_checksum) {
                    LogManager::logFormat(
                        1, "user.cpp", "bool CUser::Parsing(int)", 0x73A,
                        "[CHECK SUM] check_sum error PACKET : %d",
                        static_cast<int>(c2s.type));
                    LogManager::logFormat(
                        1, "user_diag.cpp", "bool CUser::Parsing(int)", 0,
                        "[DIAG0828][CRC_FAIL] id=%u expected=%u received=%u",
                        static_cast<unsigned int>(c2s.type),
                        static_cast<unsigned int>(server_checksum),
                        static_cast<unsigned int>(client_checksum));
                    return 0;
                }
            }

            CStreamGuard stream_guard(true);
            const int stream_size = packet_size + 8;
            if (stream_size > 4095) {
                if (stream_size > 0x3FFF) {
                    stream_guard.Set(GlobalData::big_stream_pool()->Acquire(
                        "user.cpp", 1888));
                } else {
                    stream_guard.Set(GlobalData::stream_pool()->Acquire(
                        "user.cpp", 1884));
                }
            } else {
                stream_guard.Set(GlobalData::small_stream_pool()->Acquire(
                    "user.cpp", 1880));
            }
            stream_guard << 0;
            // IDA 0x8647E10 calls CStreamGuard::operator<<(int) for the
            // dispatcher user index.  DispatchThread subsequently extracts
            // this field as int before reading the client packet bytes.
            stream_guard << GetUID();
            stream_guard.put_binary(network_.recv_cursor_, packet_size);
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q,
                                        stream_guard);
            network_.recv_flag_ = 0;
        }

        parsing_length -= packet_size;
        network_.recv_cursor_ += packet_size;
        network_.remain_recv_len_ = 0;
        if (!parsing_length) {
            network_.recv_cursor_ = network_.recv_buffer_;
            break;
        }
    } while (parsing_length > header_size - 1);

    network_.recv_flag_ = 0;
    if (parsing_length > 0) {
        if (parsing_length > 4096) {
            LogManager::logFormat(
                1, "user.cpp", "bool CUser::Parsing(int)", 0x793,
                "[PARSING LENGTH EXCEPTION] parsinglength > MAX_RECV_BUF , memmove : parsinglength = %d",
                parsing_length);
            return 0;
        }
        std::memmove(network_.recv_buffer_, network_.recv_cursor_,
                     static_cast<size_t>(parsing_length));
        network_.remain_recv_len_ = parsing_length;
        network_.recv_cursor_ = network_.recv_buffer_ + parsing_length;
    }
    return 1;
}

// IDA_EA: 0x8647FFC
// IDA_NAME: _ZN5CUser4RecvEv
// DEMANGLED: CUser::Recv(void)
int CUser::Recv() {
    const int received_bytes = network_.recv_packet();
    if (received_bytes > 0) {
        if (Parsing(received_bytes) == 1) {
            return 1;
        }
        DisConnSig(static_cast<DISCONN_SIG>(4), false, 0);
        return 0;
    }
    if (received_bytes >= 0) {
        return 1;
    }
    LogManager::logFormat(
        1, "recv_diag.cpp", "int CUser::Recv(void)", 0,
        "[DIAG0828][RECV_CLOSE] fd=%d result=%d state=%d",
        network_.get_socket(), received_bytes, get_state());
    DisConnSig(static_cast<DISCONN_SIG>(5), false, 0);
    return 0;
}

// IDA_EA: 0x822AAF4
// IDA_NAME: _ZN13CHotkeyOption5clearEv
// DEMANGLED: CHotkeyOption::clear(void)
bool CHotkeyOption::clear() {
    std::memset(bytes_, 0, sizeof(bytes_));
    return true;
}

// IDA_EA: 0x822AAE0
// IDA_NAME: _ZN13CHotkeyOptionC2Ev
// DEMANGLED: CHotkeyOption::CHotkeyOption(void)
CHotkeyOption::CHotkeyOption() {
    clear();
}

bool CHotkeyOption::isEmpty() const {
    for (unsigned int index = 0; index != sizeof(bytes_); ++index) {
        if (bytes_[index] != '\0') {
            return false;
        }
    }
    return true;
}

unsigned int CHotkeyOption::getTextLength() const {
    unsigned int length = 0;
    while (length != sizeof(bytes_) && bytes_[length] != '\0') {
        ++length;
    }
    return length;
}

bool CHotkeyOption::isValidText(unsigned int text_length) const {
    if (text_length == 0 || text_length > sizeof(bytes_) ||
        getTextLength() != text_length || bytes_[text_length - 1] != '\n') {
        return false;
    }

    unsigned char seen_keys[94];
    std::memset(seen_keys, 0, sizeof(seen_keys));
    unsigned int record_count = 0;
    unsigned int cursor = 0;
    while (cursor < text_length) {
        const unsigned int label_begin = cursor;
        while (cursor < text_length && bytes_[cursor] != '\t') {
            const unsigned char value =
                static_cast<unsigned char>(bytes_[cursor]);
            if (value < 0x20u || value > 0x7eu || value == '\n') {
                return false;
            }
            ++cursor;
        }
        if (cursor == text_length || cursor - label_begin != 3u) {
            return false;
        }

        // sub_6FD710 in the 0828 client emits exactly I01..I90 followed by
        // C00, J00, J01 and C01.  Binding the parser to that finite key set
        // both preserves the official format and gives the 1280-byte object
        // a formal maximum text size of 1034 bytes.
        const char key_prefix = bytes_[label_begin];
        const char key_tens = bytes_[label_begin + 1u];
        const char key_ones = bytes_[label_begin + 2u];
        unsigned int key_index = 94u;
        if (key_prefix == 'I' &&
            key_tens >= '0' && key_tens <= '9' &&
            key_ones >= '0' && key_ones <= '9') {
            const unsigned int key_number =
                static_cast<unsigned int>(key_tens - '0') * 10u +
                static_cast<unsigned int>(key_ones - '0');
            if (key_number >= 1u && key_number <= 90u) {
                key_index = key_number - 1u;
            }
        } else if (key_tens == '0' &&
                   (key_ones == '0' || key_ones == '1')) {
            if (key_prefix == 'C') {
                key_index = key_ones == '0' ? 90u : 93u;
            } else if (key_prefix == 'J') {
                key_index = key_ones == '0' ? 91u : 92u;
            }
        }
        if (key_index >= sizeof(seen_keys) || seen_keys[key_index] != 0u) {
            return false;
        }
        seen_keys[key_index] = 1u;
        ++cursor;  // tab

        bool negative = false;
        if (cursor < text_length && bytes_[cursor] == '-') {
            negative = true;
            ++cursor;
        }
        const unsigned int digits_begin = cursor;
        unsigned int magnitude = 0;
        while (cursor < text_length && bytes_[cursor] != '\n') {
            const unsigned char value =
                static_cast<unsigned char>(bytes_[cursor]);
            if (value < '0' || value > '9') {
                return false;
            }
            const unsigned int limit = negative ? 32768u : 32767u;
            const unsigned int digit =
                static_cast<unsigned int>(value - '0');
            if (magnitude > (limit - digit) / 10u) {
                return false;
            }
            magnitude = magnitude * 10u + digit;
            ++cursor;
        }
        if (cursor == digits_begin || cursor == text_length) {
            return false;
        }
        ++cursor;  // newline
        if (++record_count > 94u) {
            return false;
        }
    }
    return record_count != 0;
}

// IDA_EA: 0x822AB34
// IDA_NAME: _ZN10CETCOption5clearEv
// DEMANGLED: CETCOption::clear(void)
bool CETCOption::clear() {
    for (unsigned int index = 0;
         index < oldsrv::config::kGameEtcOptionCount;
         ++index) {
        values_[index] = oldsrv::config::kGameEtcOptionDefaultValue;
    }
    return true;
}

// IDA_EA: 0x822AB20
// IDA_NAME: _ZN10CETCOptionC2Ev
// DEMANGLED: CETCOption::CETCOption(void)
CETCOption::CETCOption() {
    clear();
}

// IDA_EA: 0x822ABAC
// IDA_NAME: _ZN20CQuickChattingOption5clearEv
// DEMANGLED: CQuickChattingOption::clear(void)
void* CQuickChattingOption::clear() {
    std::memset(bytes_, 0, sizeof(bytes_));
    return this;
}

// IDA_EA: 0x822AB98
// IDA_NAME: _ZN20CQuickChattingOptionC2Ev
// DEMANGLED: CQuickChattingOption::CQuickChattingOption(void)
CQuickChattingOption::CQuickChattingOption() {
    clear();
}

// IDA_EA: 0x822F002
// IDA_NAME: _ZN11CGameOption25resetFlag_QuickChatChnageEv
// DEMANGLED: CGameOption::resetFlag_QuickChatChnage(void)
CGameOption* CGameOption::resetFlag_QuickChatChnage() {
    quick_chat_changed_ = 0;
    return this;
}

// IDA_EA: 0x84B7282
// IDA_NAME: _ZN11CGameOption22reset_load_hotkey_flagEv
// DEMANGLED: CGameOption::reset_load_hotkey_flag(void)
bool CGameOption::reset_load_hotkey_flag() {
    hotkey_load_flags_[0] = 0;
    hotkey_load_flags_[1] = 0;
    return true;
}

// IDA_EA: 0x84B6C58
// IDA_NAME: _ZN11CGameOptionC2Ev
// DEMANGLED: CGameOption::CGameOption(void)
// The declaration order provides the exact old child order: both hotkey
// objects, ETC, both quick-chat objects, then the emoticon configuration.
CGameOption::CGameOption()
    : hotkeys_(),
      etc_option_(),
      quick_chat_(),
      emoticon_config_() {
    resetFlag_QuickChatChnage();
    reset_load_hotkey_flag();
}

// IDA_EA: 0x80FB5C8
// IDA_NAME: _ZN8WongWork12stHackInfo_tD2Ev
// DEMANGLED: WongWork::stHackInfo_t::~stHackInfo_t()
WongWork::stHackInfo_t::~stHackInfo_t() {}

// IDA_EA: 0x80F81A6
// IDA_NAME: _ZN8WongWork13CHackAnalyzer13resetHackInfoEv
// DEMANGLED: WongWork::CHackAnalyzer::resetHackInfo(void)
WongWork::CHackAnalyzer* WongWork::CHackAnalyzer::resetHackInfo() {
    reports_.clear();
    report_flags_ = 0;
    user_ = 0;
    pending_hack_types_.clear();
    collect_begin_sec_ = 0;
    last_monster_dead_sec_ = 0;
    base_hack_type_ = static_cast<ENUM_HACKTYPE>(0);
    return this;
}

// IDA_EA: 0x80F8550
// IDA_NAME: _ZN8WongWork13CHackAnalyzer24AddCheckRateHackTypeListEv
// DEMANGLED: WongWork::CHackAnalyzer::AddCheckRateHackTypeList(void)
int WongWork::CHackAnalyzer::AddCheckRateHackTypeList() {
    static const int kPartyRateHackTypes[] = {
        132, 220, 312, 313, 134, 222, 420, 421, 423, 223, 224, 225,
        420, 421, 423, 223, 224, 225,
    };
    static const int kDifficultRateHackTypes[] = {
        132, 202, 205, 206, 208, 209, 215, 216, 217, 218, 219, 220,
        312, 313, 403, 414, 134, 222, 420, 421, 423, 223, 224, 225,
    };

    for (unsigned int index = 0;
         index != sizeof(kPartyRateHackTypes) / sizeof(kPartyRateHackTypes[0]);
         ++index) {
        party_rate_hack_types_.push_back(kPartyRateHackTypes[index]);
    }
    for (unsigned int index = 0;
         index != sizeof(kDifficultRateHackTypes) /
                      sizeof(kDifficultRateHackTypes[0]);
         ++index) {
        difficult_rate_hack_types_.push_back(kDifficultRateHackTypes[index]);
    }
    return 0;
}

// IDA_EA: 0x86950D2
// IDA_NAME: _ZN8WongWork13CHackAnalyzerC2Ev
// DEMANGLED: WongWork::CHackAnalyzer::CHackAnalyzer(void)
WongWork::CHackAnalyzer::CHackAnalyzer()
    : reports_(),
      pending_hack_types_(),
      exception_counts_(),
      party_rate_hack_types_(),
      difficult_rate_hack_types_() {
    resetHackInfo();
    std::memset(server_hack_count_, 0, sizeof(server_hack_count_));
    std::memset(server_hack_accumulated_, 0, sizeof(server_hack_accumulated_));
    AddCheckRateHackTypeList();
}

// IDA_EA: 0x86951EA
// IDA_NAME: _ZN8WongWork13CHackAnalyzerD2Ev
// DEMANGLED: WongWork::CHackAnalyzer::~CHackAnalyzer()
WongWork::CHackAnalyzer::~CHackAnalyzer() {}

// IDA_EA: 0x8287F6A
// IDA_NAME: _ZN14user_gold_infoC2Ev
// DEMANGLED: user_gold_info::user_gold_info(void)
user_gold_info::user_gold_info() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8694F68
// IDA_NAME: _ZN16Secu_GoldControlC2Ev
// DEMANGLED: Secu_GoldControl::Secu_GoldControl(void)
Secu_GoldControl::Secu_GoldControl()
    : range_info_(), range_daily_info_(), range_saved_info_() {}

// IDA_EA: 0x8694F9A
// IDA_NAME: _ZN16Secu_GoldControlD2Ev
// DEMANGLED: Secu_GoldControl::~Secu_GoldControl()
Secu_GoldControl::~Secu_GoldControl() {}

// IDA_EA: 0x8697BC6
// IDA_NAME: _ZN8XNuclear6CHadesC2Ev
// DEMANGLED: XNuclear::CHades::CHades(void)
XNuclear::CHades::CHades() : standard_category_counts_() {}

// IDA_EA: 0x8694F54
// IDA_NAME: _ZN8XNuclear6CHadesD2Ev
// DEMANGLED: XNuclear::CHades::~CHades()
XNuclear::CHades::~CHades() {}

// IDA_EA: 0x8695068
// IDA_NAME: _ZN8WongWork12CMCAPManagerC2Ev
// DEMANGLED: WongWork::CMCAPManager::CMCAPManager(void)
WongWork::CMCAPManager::CMCAPManager()
    : point_(1), increment_rate_(1000) {}

// IDA_EA: 0x8695080
// IDA_NAME: _ZN8WongWork12CMCAPManagerD2Ev
// DEMANGLED: WongWork::CMCAPManager::~CMCAPManager()
WongWork::CMCAPManager::~CMCAPManager() {}

// IDA_EA: 0x8694BD0
// IDA_NAME: _ZN8APSystem19CActionPointManagerC2Ev
// DEMANGLED: APSystem::CActionPointManager::CActionPointManager(void)
APSystem::CActionPointManager::CActionPointManager()
    : dirty_(0), previous_action_point_(0), action_() {
    std::memset(pad_001_003_, 0, sizeof(pad_001_003_));
}

// IDA_EA: 0x8694BF6
// IDA_NAME: _ZN8APSystem19CActionPointManagerD2Ev
// DEMANGLED: APSystem::CActionPointManager::~CActionPointManager()
APSystem::CActionPointManager::~CActionPointManager() {}

// Old C2 obtains this byte twice, once for the PI sender state and once for
// the CUser CS-hash state.  It intentionally remains an external old-server
// dependency; this pre-ABI provider neither activates nor substitutes it.
extern uint8_t _NS_PI_2ND_GetDefaultRandomHashKey();

// IDA_EA: 0x864E410
// IDA_NAME: _ZN5CUserC2Ev
// DEMANGLED: CUser::CUser(void)
//
// All direct non-trivial members are real fields in CUser.  C++ therefore
// emits their C2 calls in the declaration order recovered from old C2.  The
// four scalar initializers below are the only writes interleaved among those
// children: their locations are deliberately placed after skill_commands_,
// user_strings_, charac_money_logs_, and the two Mutex objects respectively.
CUser::CUser()
    : unique_id_(0),
      before_area_time_(0),
      pu_user_(0),
      state_(0),
      playing_dungeon_index_(-1),
      trade_space_index_(-1),
      party_index_(-1),
      pvp_index_(-1),
      war_room_index_(-1),
      blue_marble_index_(-1) {
    // Every remaining store below is after the final
    // CharacLevelUpGiftAccountOnce C2 in the old constructor.  Keep the old
    // statement order while naming each physical scalar directly.
    charac_antibot_serial_number_ = 0;
    recipe_force_probability_ = 0;
    mailbox_ = 0;
    guild_add_exp_flag_ = 0;
    connection_check_interval_ = 300;
    resume_checksum_time_ = 0;
    checksum_answer_count_ = 0;
    setAntibotKey(-1);
    setClientVersion(0);
    item_serial_check_.serial_count = 0;
    special_monster_kill_counter_ = 0;
    pc_room_no_ = 0;
    history_stream_ = 0;
    reset();
    history_log_.SetUser(this);
    client_spec_c2_state_ = 0;
    guild_cargo_capacity_ = 0;
    ResetUsedCoinInDungeon();
    pi_sender_random_key_ = _NS_PI_2ND_GetDefaultRandomHashKey();
    cs_hash_random_key_ = _NS_PI_2ND_GetDefaultRandomHashKey();
    account_cargo_.Clear();
    used_coin_count_ = 0;
    ClearItemSellCount();
    check_pick_up_random_option_item_ = 0;
    move_map_hack_count_ = 0;
    proper_level_dungeon_user_ = 0;
    resetInformNoticeFlag();
    disableSaveInformNoticeFlag();
    save_rental_info_to_exchange_ = 1;
    load_rental_info_from_exchange_ = 0;
    rental_info_store_.clear();
    move_space_ = 0;
    trade_punish_over_thirty_days_ = 0;
    advance_altar_index_ = -1;
    used_fatigue_ = 0;
    used_gift_fatigue_ = 0;
    gift_fatigue_changed_ = 0;
    combo_skill_set_ = 0;
    allow_location_geo_state_ = 0;
}

// IDA_EA: 0x864EF9C
// IDA_NAME: _ZN5CUserD2Ev
// DEMANGLED: CUser::~CUser()
// C++ performs the remaining exact reverse direct-member teardown after this
// body.  Expanded D2 first clears CUser+0x8d5a3 before any child destructor.
CUser::~CUser() {
    npc_gift::Erase(this);
    clone_title::ResetUser(this);
    power_up_ = 0;
}
