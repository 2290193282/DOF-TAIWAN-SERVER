#include "infra/security/policy/hack_analyzer_public_abi.h"

#include "common/manager/global_data.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/anti_cheat/db_req_save_hack_user_punish.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>

#include <algorithm>
#include <map>

namespace {

const short kHackCheckRateByPartyCount[4] = {100, 50, 33, 25};
const short kHackCheckRateByDifficult[5] = {20, 15, 10, 0, 0};

}  // namespace

extern unsigned int get_rand_int(int limit) __asm__("_Z12get_rand_inti");

// IDA_EA: 0x80F8F62
// IDA_NAME: _ZN8WongWork13CHackAnalyzer17ReqDBSaveHackInfoEjjtii
// DEMANGLED: WongWork::CHackAnalyzer::ReqDBSaveHackInfo(unsigned int,unsigned int,unsigned short,int,int)
void WongWork::CHackAnalyzer::ReqDBSaveHackInfo(unsigned int charac_no,
                                                 unsigned short hack_type,
                                                 int etc,
                                                 int hack_count) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("HackAnalyzer.cpp", 585);
    CStreamGuard guard(stream, true);
    guard << 154;
    guard << -1;

    SIG_SAVE_CLIENT_HACK_LOG* payload =
        guard.GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>();
    payload->account_id =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this));
    payload->charac_no = charac_no;
    payload->hack_type = hack_type;
    payload->etc = etc;
    payload->hack_count = hack_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x80F79E8
// IDA_NAME: _ZN8WongWork13CHackAnalyzer20beginCollectHackInfoEP5CUser
// DEMANGLED: WongWork::CHackAnalyzer::beginCollectHackInfo(CUser *)
int WongWork::CHackAnalyzer::beginCollectHackInfo(CUser* user) {
    user_ = user;
    collect_begin_sec_ = GlobalData::system_time()->getCurSec();
    last_monster_dead_sec_ = 0;
    pending_hack_types_.clear();
    return 0;
}

// IDA_EA: 0x80F9876
// IDA_NAME: _ZNK8WongWork13CHackAnalyzer18checkCollectedHackE23ENUM_REPORT_4_HACK_FLAG
// DEMANGLED: WongWork::CHackAnalyzer::checkCollectedHack(ENUM_REPORT_4_HACK_FLAG) const
bool WongWork::CHackAnalyzer::checkCollectedHack(
    ENUM_REPORT_4_HACK_FLAG flag) const {
    return (static_cast<unsigned char>(report_flags_) &
            static_cast<unsigned char>(flag)) ==
           static_cast<unsigned char>(flag);
}

// IDA_EA: 0x80F7FD2
// IDA_NAME: _ZN8WongWork13CHackAnalyzer12_addHackInfoE23ENUM_REPORT_4_HACK_FLAGNS_13ENUM_HACKTYPEEi
// DEMANGLED: WongWork::CHackAnalyzer::_addHackInfo(ENUM_REPORT_4_HACK_FLAG,WongWork::ENUM_HACKTYPE,int)
int WongWork::CHackAnalyzer::_addHackInfo(ENUM_REPORT_4_HACK_FLAG flag,
                                          ENUM_HACKTYPE hack_type,
                                          int payload) {
    stHackInfo_t info;
    info.account_id_ = static_cast<uint32_t>(user_->get_acc_id());
    info.charac_no_ = static_cast<uint32_t>(user_->getCurCharacNo());
    info.hack_type_ = static_cast<uint16_t>(hack_type);
    info.pad_0a_0b_ = 0;
    info.payload_ = payload;
    reports_.push_back(info);
    report_flags_ |= static_cast<uint8_t>(flag);
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x80F8032
// IDA_NAME: _ZN8WongWork13CHackAnalyzer22_reportIncurrectReportEv
// DEMANGLED: WongWork::CHackAnalyzer::_reportIncurrectReport(void)
int WongWork::CHackAnalyzer::_reportIncurrectReport() {
    if (!checkCollectedHack(REPORT_4_HACK_INCORRECT)) {
        return _addHackInfo(REPORT_4_HACK_INCORRECT,
                            static_cast<ENUM_HACKTYPE>(964),
                            user_->get_state());
    }
    return 1;
}

// IDA_EA: 0x80F7CCA
// IDA_NAME: _ZN8WongWork13CHackAnalyzer17_analyzeSpeedHackEj
// DEMANGLED: WongWork::CHackAnalyzer::_analyzeSpeedHack(unsigned int)
int WongWork::CHackAnalyzer::_analyzeSpeedHack(unsigned int elapsed_client_msec) {
    if (checkCollectedHack(REPORT_4_HACK_SPEED)) {
        return 1;
    }
    if (last_monster_dead_sec_ == 0) {
        return _reportIncurrectReport();
    }

    const int speed = static_cast<int>(
        (static_cast<long double>(elapsed_client_msec) /
         static_cast<long double>(last_monster_dead_sec_ - collect_begin_sec_)) *
        100.0);
    if (speed <= 69 || speed > 130) {
        return _addHackInfo(REPORT_4_HACK_SPEED,
                            static_cast<ENUM_HACKTYPE>(961), speed);
    }
    return 0;
}

// IDA_EA: 0x80F7D82
// IDA_NAME: _ZN8WongWork13CHackAnalyzer22_analyzeDifficultyHackEc
// DEMANGLED: WongWork::CHackAnalyzer::_analyzeDifficultyHack(char)
int WongWork::CHackAnalyzer::_analyzeDifficultyHack(char value) {
    if (!checkCollectedHack(REPORT_4_HACK_DIFFICULTY)) {
        return _addHackInfo(REPORT_4_HACK_DIFFICULTY,
                            static_cast<ENUM_HACKTYPE>(959), value);
    }
    return 1;
}

// IDA_EA: 0x80F7DCE
// IDA_NAME: _ZN8WongWork13CHackAnalyzer18_analyzeMemoryHackEj
// DEMANGLED: WongWork::CHackAnalyzer::_analyzeMemoryHack(unsigned int)
int WongWork::CHackAnalyzer::_analyzeMemoryHack(unsigned int value) {
    if (!checkCollectedHack(REPORT_4_HACK_MEMORY) && value != 0) {
        return _addHackInfo(REPORT_4_HACK_MEMORY,
                            static_cast<ENUM_HACKTYPE>(956),
                            static_cast<int>(value));
    }
    return 0;
}

// IDA_EA: 0x80F906A
// IDA_NAME: _ZN8WongWork13CHackAnalyzer23ReqDBSaveHackUserPunishEjj
// DEMANGLED: WongWork::CHackAnalyzer::ReqDBSaveHackUserPunish(unsigned int,unsigned int)
void WongWork::CHackAnalyzer::ReqDBSaveHackUserPunish(
    unsigned int account_id,
    unsigned int punish_value) {
    DB_ReqSaveHackUserPunish::makeRequest(
        account_id,
        punish_value,
        0);
}

// IDA_EA: 0x80F7A22
// IDA_NAME: _ZN8WongWork13CHackAnalyzer17analyzeHackReportEP5CUserP9PacketBuf
// DEMANGLED: WongWork::CHackAnalyzer::analyzeHackReport(CUser *,PacketBuf *)
int WongWork::CHackAnalyzer::analyzeHackReport(CUser* user, PacketBuf* packet) {
    unsigned short flags = 0;
    if (packet->get_short(flags) != 1) {
        return 0;
    }
    if (!user_) {
        return 1;
    }

    if ((flags & 0x01u) != 0u) {
        unsigned int value = 0;
        if (packet->get_int(value) != 1) {
            return 0;
        }
        _analyzeSpeedHack(value);
    }
    if ((flags & 0x02u) != 0u) {
        _analyzeDifficultyHack(0);
        user_->DisConnSig(static_cast<DISCONN_SIG>(24), false, 0);
    }
    if ((flags & 0x08u) != 0u) {
        unsigned int value = 0;
        if (packet->get_int(value) != 1) {
            return 0;
        }
        _analyzeMemoryHack(value);
    }
    if ((flags & 0x04u) != 0u) {
        _addHackInfo(REPORT_4_HACK_EQUIPMENT,
                     static_cast<ENUM_HACKTYPE>(965), 0);
        ReqDBSaveHackUserPunish(
            static_cast<unsigned int>(user_->get_acc_id()), 0xA8u);
        user_->DisConnSig(static_cast<DISCONN_SIG>(24), false, 0);
    }
    if ((flags & 0x80u) != 0u) {
        _addHackInfo(REPORT_4_HACK_MONSTER_DIE,
                     static_cast<ENUM_HACKTYPE>(957), 0);
    }
    if ((flags & 0x10u) != 0u) {
        char value = 0;
        if (packet->get_byte(value) != 1) {
            return 0;
        }
        _addHackInfo(REPORT_4_HACK_CLIENT_FILE,
                     static_cast<ENUM_HACKTYPE>(958), value);
    }
    if ((flags & 0x20u) != 0u) {
        _addHackInfo(REPORT_4_HACK_UNKNOWN_20,
                     static_cast<ENUM_HACKTYPE>(966), 0);
    }
    if ((flags & 0x100u) != 0u) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<ENUM_HACKTYPE>(812), 1, 0, 0);
    }
    return 1;
}

// IDA_EA: 0x80F8080
// IDA_NAME: _ZN8WongWork13CHackAnalyzer14reportHackInfoEv
// DEMANGLED: WongWork::CHackAnalyzer::reportHackInfo(void)
int WongWork::CHackAnalyzer::reportHackInfo() {
    const int was_empty = reports_.empty() ? 1 : 0;
    if (was_empty) {
        return was_empty;
    }

    for (std::vector<stHackInfo_t>::const_iterator it = reports_.begin();
         it != reports_.end(); ++it) {
        const stHackInfo_t& info = *it;
        reinterpret_cast<CHackAnalyzer*>(
            static_cast<uintptr_t>(info.account_id_))->ReqDBSaveHackInfo(
            info.charac_no_,
            info.hack_type_,
            info.payload_,
            1);
    }
    return 0;
}

// IDA_EA: 0x8278A48
// IDA_NAME: _ZN19Secu_AccountHacking27checkAccountHackingHackTypeEN8WongWork13ENUM_HACKTYPEE
// DEMANGLED: Secu_AccountHacking::checkAccountHackingHackType(WongWork::ENUM_HACKTYPE)
int Secu_AccountHacking::checkAccountHackingHackType(
    WongWork::ENUM_HACKTYPE hack_type) {
    const int type = static_cast<int>(hack_type);
    return type == 710 || type == 711 || type == 712 || type == 713 ||
           type == 714 || type == 715 || type == 717;
}

// IDA_EA: 0x8695086
// IDA_NAME: _ZN8WongWork13CHackAnalyzer16getServerHackCntENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CHackAnalyzer::getServerHackCnt(WongWork::ENUM_HACKTYPE)
int WongWork::CHackAnalyzer::getServerHackCnt(ENUM_HACKTYPE hack_type) {
    // 0x80F8EC2 computes (hack_type - 0x65 + 8) * 4 + 8, which is
    // server_hack_count_[hack_type - 101] at the named +0x28 array.
    return static_cast<int>(server_hack_count_[static_cast<int>(hack_type) - 101]);
}

// IDA_EA: 0x869509C
// IDA_NAME: _ZN8WongWork13CHackAnalyzer18resetServerHackCntENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CHackAnalyzer::resetServerHackCnt(WongWork::ENUM_HACKTYPE)
WongWork::CHackAnalyzer*
WongWork::CHackAnalyzer::resetServerHackCnt(ENUM_HACKTYPE hack_type) {
    server_hack_count_[static_cast<int>(hack_type) - 101] = 0;
    return this;
}

// IDA_EA: 0x86950B6
// IDA_NAME: _ZN8WongWork13CHackAnalyzer29resetServerHackAccumulatedCntENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CHackAnalyzer::resetServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE)
WongWork::CHackAnalyzer*
WongWork::CHackAnalyzer::resetServerHackAccumulatedCnt(ENUM_HACKTYPE hack_type) {
    // IDA @0x86950B6 stores at this + 0x3d18 + 4 * (hack_type - 101).
    // The public enum interval is 101..4000, while the physical array has
    // exactly 3900 entries.  Indexing it by the raw enum overwrites the CUser
    // member immediately after CHackAnalyzer when the reset loop reaches 4000.
    server_hack_accumulated_[static_cast<int>(hack_type) - 101] = 0;
    return this;
}

// IDA_EA: 0x84EB57C
// IDA_NAME: _ZN8WongWork13CHackAnalyzer27setServerHackAccumulatedCntENS_13ENUM_HACKTYPEEj
// DEMANGLED: WongWork::CHackAnalyzer::setServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE,unsigned int)
WongWork::CHackAnalyzer*
WongWork::CHackAnalyzer::setServerHackAccumulatedCnt(ENUM_HACKTYPE hack_type,
                                                      unsigned int count) {
    // IDA @0x84EB57C: *(this + 15236 + 4 * hack_type) = count.
    // This is the same physical array expressed from its +0x3d18 origin.
    server_hack_accumulated_[static_cast<int>(hack_type) - 101] = count;
    return this;
}

// IDA_EA: 0x822EEF6
// IDA_NAME: _ZNK8WongWork13CHackAnalyzer27getServerHackAccumulatedCntENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CHackAnalyzer::getServerHackAccumulatedCnt(WongWork::ENUM_HACKTYPE)const
int WongWork::CHackAnalyzer::getServerHackAccumulatedCnt(
    ENUM_HACKTYPE hack_type) const {
    // IDA @0x822EEF6 reads from this + 15236 + 4 * hack_type.
    return static_cast<int>(
        server_hack_accumulated_[static_cast<int>(hack_type) - 101]);
}

// IDA_EA: 0x85DF98C
// IDA_NAME: _ZN8WongWork13CHackAnalyzer22setLastMonsterDeadTimeEl
// DEMANGLED: WongWork::CHackAnalyzer::setLastMonsterDeadTime(long)
WongWork::CHackAnalyzer*
WongWork::CHackAnalyzer::setLastMonsterDeadTime(long current_sec) {
    last_monster_dead_sec_ = static_cast<int32_t>(current_sec);
    return this;
}

// IDA_EA: 0x80F9856
// IDA_NAME: _ZN8WongWork13CHackAnalyzer15setBaseHackTypeENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CHackAnalyzer::setBaseHackType(WongWork::ENUM_HACKTYPE)
int WongWork::CHackAnalyzer::setBaseHackType(ENUM_HACKTYPE hack_type) {
    base_hack_type_ = hack_type;
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x80F9868
// IDA_NAME: _ZN8WongWork13CHackAnalyzer15getBaseHackTypeEv
// DEMANGLED: WongWork::CHackAnalyzer::getBaseHackType(void)
int WongWork::CHackAnalyzer::getBaseHackType() {
    return static_cast<int>(base_hack_type_);
}

// IDA_EA: 0x80F81FC
// IDA_NAME: _ZN8WongWork13CHackAnalyzer17CheckExceptionCntENS_13ENUM_HACKTYPEEi
// DEMANGLED: WongWork::CHackAnalyzer::CheckExceptionCnt(WongWork::ENUM_HACKTYPE,int)
int WongWork::CHackAnalyzer::CheckExceptionCnt(ENUM_HACKTYPE hack_type,
                                                int count) {
    const WongWork::stHacktypeExceptionInfo_t info =
        GlobalData::g_autoPunishRuleHackTypeMgr->GetExceptionCnt(hack_type);
    if (!info.exception_count) {
        return info.accumulated_count &&
               info.accumulated_count >= static_cast<short>(count);
    }

    std::pair<std::map<int, int>::iterator, bool> inserted =
        exception_counts_.insert(std::make_pair(static_cast<int>(hack_type), 1));
    if (inserted.second) {
        return 1;
    }
    inserted.first->second += count;
    if (inserted.first->second >= info.exception_count) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x80F8320
// IDA_NAME: _ZN8WongWork13CHackAnalyzer25CheckHackRateByPartyCountENS_13ENUM_HACKTYPEEi
// DEMANGLED: WongWork::CHackAnalyzer::CheckHackRateByPartyCount(WongWork::ENUM_HACKTYPE,int)
int WongWork::CHackAnalyzer::CheckHackRateByPartyCount(ENUM_HACKTYPE hack_type,
                                                        int member_count) {
    if (std::find(party_rate_hack_types_.begin(), party_rate_hack_types_.end(),
                  static_cast<int>(hack_type)) == party_rate_hack_types_.end()) {
        return 1;
    }
    const int index = member_count - 1;
    if (index < 0 || index > 3 || kHackCheckRateByPartyCount[index] == 100) {
        return 1;
    }
    return static_cast<int>(get_rand_int(100)) <=
           kHackCheckRateByPartyCount[index];
}

// IDA_EA: 0x80F842E
// IDA_NAME: _ZN8WongWork13CHackAnalyzer24CheckHackRateByDifficultENS_13ENUM_HACKTYPEEii
// DEMANGLED: WongWork::CHackAnalyzer::CheckHackRateByDifficult(WongWork::ENUM_HACKTYPE,int,int)
int WongWork::CHackAnalyzer::CheckHackRateByDifficult(ENUM_HACKTYPE hack_type,
                                                       int level_delta,
                                                       int dungeon_diff) {
    if (std::find(difficult_rate_hack_types_.begin(),
                  difficult_rate_hack_types_.end(),
                  static_cast<int>(hack_type)) == difficult_rate_hack_types_.end()) {
        return 1;
    }
    if (static_cast<int>(hack_type) == 134) {
        level_delta *= 2;
    }
    int rate = 100 - level_delta;
    if (static_cast<unsigned int>(dungeon_diff) > 4u) {
        return 1;
    }
    rate -= kHackCheckRateByDifficult[dungeon_diff];
    if (rate <= 24 && static_cast<int>(hack_type) != 134) {
        rate = 25;
    }
    return static_cast<int>(get_rand_int(100)) <= rate;
}

// IDA_EA: 0x80F8C4E
// IDA_NAME: _ZN8WongWork13CHackAnalyzer17onUpdateHackCountEP5CUserNS_13ENUM_HACKTYPEEijj
// DEMANGLED: WongWork::CHackAnalyzer::onUpdateHackCount(CUser *,WongWork::ENUM_HACKTYPE,int,unsigned int,unsigned int)
int WongWork::CHackAnalyzer::onUpdateHackCount(CUser* user,
                                                ENUM_HACKTYPE hack_type,
                                                int count,
                                                unsigned int value1,
                                                unsigned int value2) {
    return user->onUpdateHackCount(hack_type, count, value1, value2);
}

// IDA_EA: 0x80F8C7E
// IDA_NAME: _ZN8WongWork13CHackAnalyzer16addServerHackCntEP5CUserNS_13ENUM_HACKTYPEEjjj
// DEMANGLED: WongWork::CHackAnalyzer::addServerHackCnt(CUser *,WongWork::ENUM_HACKTYPE,unsigned int,unsigned int,unsigned int)
char WongWork::CHackAnalyzer::addServerHackCnt(CUser* user,
                                                ENUM_HACKTYPE hack_type,
                                                unsigned int count,
                                                unsigned int value1,
                                                unsigned int value2) {
    const int type = static_cast<int>(hack_type);
    if (type < 0 || type > 0xFA1) {
        const int etc_value = user->SetETC(846, getBaseHackType());
        addServerHackCnt(user, static_cast<ENUM_HACKTYPE>(846), count,
                          static_cast<unsigned int>(etc_value), 0);
        return static_cast<char>(setBaseHackType(static_cast<ENUM_HACKTYPE>(0)));
    }

    if (CheckExceptionCnt(hack_type, static_cast<int>(count))) {
        return 1;
    }
    // IDA @0x80F8D44 reads dword[this + 15236 + 4 * type], which is
    // server_hack_accumulated_[type - 101] from the named array origin.
    const int accumulated_index = type - 101;
    if (Secu_AccountHacking::checkAccountHackingHackType(hack_type) &&
        server_hack_accumulated_[accumulated_index] != 0) {
        return 1;
    }

    CParty* party = user->GetParty();
    if (party && user->get_state() == 5) {
        const int member_count = party->get_member_count();
        if (CheckHackRateByPartyCount(hack_type, member_count)) {
            return 1;
        }

        CDungeon* dungeon = party->battle_field().GetDungeon();
        if (dungeon) {
            const int charac_level = user->get_charac_level();
            const int standard_level = dungeon->get_standard_level();
            const int level_delta =
                charac_level - standard_level < 0 ? 0 : charac_level - standard_level;
            const int dungeon_diff = party->battle_field().get_dungeon_diff();
            if (CheckHackRateByDifficult(hack_type, level_delta, dungeon_diff)) {
                return 1;
            }
        }
    }

    if (type != 603 && type != 604 && type != 605 && type != 606 &&
        type != 607 && type != 608 && type != 805 && type != 111) {
        server_hack_count_[type - 101] += count;
        // IDA @0x80F8F05 uses the same +15236 physical base as the read
        // above; do not index the 3900-entry storage by the raw enum.
        server_hack_accumulated_[accumulated_index] += count;
        return static_cast<char>(
            onUpdateHackCount(user, hack_type, static_cast<int>(count), value1, value2));
    }
    return 0;
}

namespace {

// IDA data at 0x936F9C4.  The binary loop visits FixMobInfoSize - 1 entries;
// the trailing size word is 11, so these ten packed records are the complete
// coordinate table.  The final zero-type record is intentionally retained.
struct FixMobDiePosition {
    uint16_t min_x;
    uint16_t max_x;
    uint16_t min_y;
    uint16_t max_y;
    uint16_t monster_type;
    uint16_t reserved;
};

const FixMobDiePosition kFixMobDiePositions[10] = {
    {0x0468u, 0x0468u, 0x00edu, 0x00edu, 0xd6edu, 0u},
    {0x0080u, 0x0337u, 0x009du, 0x0139u, 0x02f0u, 0u},
    {0x000bu, 0x000bu, 0x0082u, 0x015eu, 0x02efu, 0u},
    {0x0378u, 0x0378u, 0x0082u, 0x015eu, 0xdb29u, 0u},
    {0x04f9u, 0x04f9u, 0x00ffu, 0x00ffu, 0xf010u, 0u},
    {0xffeeu, 0xffeeu, 0x0120u, 0x0120u, 0xf108u, 0u},
    {0x0057u, 0x0057u, 0x0175u, 0x0175u, 0xef9cu, 0u},
    {0x0384u, 0x0384u, 0x00f0u, 0x00f0u, 0xf203u, 0u},
    {0x0377u, 0x0377u, 0x00cau, 0x00cau, 0xdcbbu, 0u},
    {0x0552u, 0x0552u, 0x00fbu, 0x00fbu, 0x0000u, 0u},
};

void AddFixMobDiePositionHack(CUser* user) {
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(0xd2u),
        1u,
        0u,
        0u);
}

}  // namespace

// IDA_EA: 0x80F908C
// IDA_NAME: _ZN8WongWork13CHackAnalyzer20ValidateFixMobDiePosEP5CUserttt
// DEMANGLED: WongWork::CHackAnalyzer::ValidateFixMobDiePos(CUser *,unsigned short,unsigned short,unsigned short)
char WongWork::CHackAnalyzer::ValidateFixMobDiePos(CUser* user,
                                                    uint16_t monster_type,
                                                    uint16_t reported_x,
                                                    uint16_t reported_y) {
    if (!user) return 0;

    for (unsigned int index = 0; index < 10u; ++index) {
        const FixMobDiePosition& fixed = kFixMobDiePositions[index];
        if (fixed.monster_type != monster_type) continue;

        if (reported_x < fixed.min_x || reported_x > fixed.max_x ||
            reported_y < fixed.min_y || reported_y > fixed.max_y) {
            AddFixMobDiePositionHack(user);
            return 1;
        }

        // The first fixed monster (0xD6ED) permits only the two declared X
        // endpoints after the rectangle test.  Its current bounds coincide,
        // but retaining this branch preserves the original special case.
        if (monster_type == 0xd6edu && reported_x != fixed.min_x &&
            reported_x != fixed.max_x) {
            AddFixMobDiePositionHack(user);
            return 1;
        }
        return 0;
    }
    return 0;
}
