#include "common/manager/in_game_advertisement_abi.h"

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <new>
#include <utility>

#include "common/manager/global_data.h"
#include "common/manager/data_manager.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/mt_rand.h"


CAdvertisementExposeInfo*
MemPool<CAdvertisementExposeInfo>::headOfFreeList_ = 0;

// IDA_EA: 0x80FE010
// IDA_NAME: _ZN7MemPoolI24CAdvertisementExposeInfoEC2Ej
// DEMANGLED: MemPool<CAdvertisementExposeInfo>::MemPool(unsigned int)
MemPool<CAdvertisementExposeInfo>::MemPool(unsigned int count)
    : size_(0x850u), count_(count), allocated_blocks_() {
}

// IDA_EA: 0x80FF966
// IDA_NAME: _ZN7MemPoolI24CAdvertisementExposeInfoED2Ev
// DEMANGLED: MemPool<CAdvertisementExposeInfo>::~MemPool()
MemPool<CAdvertisementExposeInfo>::~MemPool() {
    for (std::vector<void*>::iterator block = allocated_blocks_.begin();
         block != allocated_blocks_.end(); ++block) {
        ::operator delete(*block);
    }
    allocated_blocks_.clear();
}

// IDA_EA: 0x80FE038
// IDA_NAME: _ZN7MemPoolI24CAdvertisementExposeInfoE5allocEv
// DEMANGLED: MemPool<CAdvertisementExposeInfo>::alloc(void)
CAdvertisementExposeInfo* MemPool<CAdvertisementExposeInfo>::alloc() {
    if (size_ != 0x850u) {
        return static_cast<CAdvertisementExposeInfo*>(::operator new(0x850u));
    }

    CAdvertisementExposeInfo* result = headOfFreeList_;
    if (result != 0) {
        headOfFreeList_ = result->free_list_next_;
        return result;
    }

    char* block = static_cast<char*>(::operator new(size_ * count_));
    for (unsigned int index = 0; index + 1 < count_; ++index) {
        CAdvertisementExposeInfo* current =
            reinterpret_cast<CAdvertisementExposeInfo*>(block + size_ * index);
        current->free_list_next_ = reinterpret_cast<CAdvertisementExposeInfo*>(
            block + size_ * (index + 1));
    }
    CAdvertisementExposeInfo* last =
        reinterpret_cast<CAdvertisementExposeInfo*>(block + size_ * (count_ - 1));
    last->free_list_next_ = 0;

    result = reinterpret_cast<CAdvertisementExposeInfo*>(block);
    headOfFreeList_ = result->free_list_next_;
    allocated_blocks_.push_back(block);

    CMyFileLog log("MemPool<CAdvertisementExposeInfo>::alloc", 125);
    log("./log/Mempool", "class size(%d) cnt(%d)",
        static_cast<int>(size_), static_cast<int>(count_));
    return result;
}

// IDA_EA: 0x80FE188
// IDA_NAME: _ZN7MemPoolI24CAdvertisementExposeInfoE4freeEPvj
// DEMANGLED: MemPool<CAdvertisementExposeInfo>::free(void *,unsigned int)
void MemPool<CAdvertisementExposeInfo>::free(void* entry, unsigned int bytes) {
    if (entry == 0) {
        return;
    }
    if (bytes != size_) {
        ::operator delete(entry);
        return;
    }
    free(entry);
}

// IDA_EA: 0x80FE1CE
// IDA_NAME: _ZN7MemPoolI24CAdvertisementExposeInfoE4freeEPv
// DEMANGLED: MemPool<CAdvertisementExposeInfo>::free(void *)
void MemPool<CAdvertisementExposeInfo>::free(void* entry) {
    if (entry == 0) {
        return;
    }
    CAdvertisementExposeInfo* result =
        static_cast<CAdvertisementExposeInfo*>(entry);
    result->free_list_next_ = headOfFreeList_;
    headOfFreeList_ = result;
}


MemPool<CAdvertisementExposeInfo>
    CAdvertisementExposeInfo::advertisementExposeInfoPool_(50);

// IDA_EA: 0x80FC998
// IDA_NAME: _ZN24CAdvertisementExposeInfonwEj
// DEMANGLED: CAdvertisementExposeInfo::operator new(unsigned int)
void* CAdvertisementExposeInfo::operator new(std::size_t bytes) {
    (void)bytes;
    return advertisementExposeInfoPool_.alloc();
}

// IDA_EA: 0x80FC9AC
// IDA_NAME: _ZN24CAdvertisementExposeInfodlEPvj
// DEMANGLED: CAdvertisementExposeInfo::operator delete(void *,unsigned int)
void CAdvertisementExposeInfo::operator delete(void* entry,
                                                std::size_t bytes) {
    advertisementExposeInfoPool_.free(entry,
                                      static_cast<unsigned int>(bytes));
}

// IDA_EA: 0x80FC9CE
// IDA_NAME: _ZN24CAdvertisementExposeInfodlEPv
// DEMANGLED: CAdvertisementExposeInfo::operator delete(void *)
void CAdvertisementExposeInfo::operator delete(void* entry) {
    advertisementExposeInfoPool_.free(entry);
}

// IDA_EA: 0x80FDD0A
// IDA_NAME: _ZN24CAdvertisementExposeInfo26SetAdvertisementExposeInfoER25stInGameADExposeCondition
// DEMANGLED: CAdvertisementExposeInfo::SetAdvertisementExposeInfo(stInGameADExposeCondition &)
void* CAdvertisementExposeInfo::SetAdvertisementExposeInfo(
    stInGameADExposeCondition& condition) {
    return std::memcpy(&condition_, &condition, sizeof(condition_));
}

// IDA_EA: 0x80FDD2C
// IDA_NAME: _ZN24CAdvertisementExposeInfo18getExposeConditionEv
// DEMANGLED: CAdvertisementExposeInfo::getExposeCondition(void)
stInGameADExposeCondition* CAdvertisementExposeInfo::getExposeCondition() {
    return &condition_;
}


// IDA_EA: 0x80FDD6A
// IDA_NAME: _ZN27CInGameAdvertisementManagerC2Ev
// DEMANGLED: CInGameAdvertisementManager::CInGameAdvertisementManager(void)
CInGameAdvertisementManager::CInGameAdvertisementManager()
    : expose_infos_(),
      random_(new CMTRand(static_cast<unsigned long>(std::time(0)))) {
}

// IDA_EA: 0x80FCA7C
// IDA_NAME: _ZN27CInGameAdvertisementManagerD2Ev
// DEMANGLED: CInGameAdvertisementManager::~CInGameAdvertisementManager()
CInGameAdvertisementManager::~CInGameAdvertisementManager() {
    delete random_;
}

// IDA_EA: 0x80FCA9E
// IDA_NAME: _ZN27CInGameAdvertisementManager10checkLevelEP5CUserPc
// DEMANGLED: CInGameAdvertisementManager::checkLevel(CUser *,char *)
int CInGameAdvertisementManager::checkLevel(CUser* user,
                                            char* level_filter) {
    if (!user || !level_filter) {
        return 0;
    }

    const char* cursor = level_filter;
    for (int index = 0; index < 120; ++index) {
        char* end = 0;
        const long level = std::strtol(cursor, &end, 10);
        if (level == -1 || user->get_charac_level() == level) {
            return 1;
        }
        if (!end || *end != ',') {
            break;
        }
        cursor = end + 1;
    }
    return 0;
}

// IDA_EA: 0x80FCB3E
// IDA_NAME: _ZN27CInGameAdvertisementManager24CheckFailMultiBoxLotteryEP5CUserRSt6vectorISt4pairI10Inven_ItembESaIS5_EE
// DEMANGLED: CInGameAdvertisementManager::CheckFailMultiBoxLottery(CUser *,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> &)
void CInGameAdvertisementManager::CheckFailMultiBoxLottery(
    CUser* user,
    std::vector<std::pair<Inven_Item, bool> >& results) {
    if (!user || results.empty()) {
        return;
    }

    bool all_multibox_items = true;
    for (std::vector<std::pair<Inven_Item, bool> >::const_iterator item =
             results.begin();
         item != results.end();
         ++item) {
        const int item_id = item->first.item_id_;
        if (item_id != 7740 && item_id != 7741 && item_id != 7742) {
            all_multibox_items = false;
            break;
        }
    }

    if (!all_multibox_items) {
        user->set_multiboxLotteryItemFailCnt(0);
        return;
    }

    if (user->inc_multiboxLotteryItemFailCnt() > 2) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                          ENUM_CMDPACKET_REQ_URGENT_QUEST);
        packet.finalize(true);
        user->Send(packet);
    }
}

// IDA_EA: 0x80FCD2E
// IDA_NAME: _ZN27CInGameAdvertisementManager14checkConditionEP5CUserP24CAdvertisementExposeInfo
// DEMANGLED: CInGameAdvertisementManager::checkCondition(CUser *,CAdvertisementExposeInfo *)
int CInGameAdvertisementManager::checkCondition(
    CUser* user,
    CAdvertisementExposeInfo* info) {
    if (!user || !info) {
        return 0;
    }

    stInGameADExposeCondition* const condition =
        info->getExposeCondition();
    if (!condition) {
        return 0;
    }

    if (condition->condition_value != static_cast<char>(0xff)) {
        const bool age_allowed = G_CDataManager()->is_in_game_ad_age_allowed(
            static_cast<unsigned int>(user->GetAge()));
        if (condition->condition_value != 0) {
            if (!age_allowed) {
                return 0;
            }
        } else if (age_allowed) {
            return 0;
        }
    }

    if (condition->condition_type != static_cast<char>(0xff)) {
        if (condition->condition_type == 1) {
            if (!user->getSex()) {
                return 0;
            }
        } else if (user->getSex() == 1) {
            return 0;
        }
    }

    if (checkLevel(user, condition->level_filter) != 1 ||
        static_cast<unsigned int>(user->GetCera()) < condition->min_cera ||
        static_cast<unsigned int>(user->GetCera()) >= condition->max_cera) {
        return 0;
    }

    if (condition->condition_flag != static_cast<char>(0xff) &&
        user->get_charac_job() != condition->condition_flag) {
        return 0;
    }

    condition->notify.condition_match =
        condition->probability_divisor == 0 ||
        user->get_acc_id() % condition->probability_divisor != 0;
    return 1;
}

// IDA_EA: 0x80FCFA8
// IDA_NAME: _ZN27CInGameAdvertisementManager33CheckIngameAdvertisementConditionEP5CUserR20stNotifyIngameADInfoi
// DEMANGLED: CInGameAdvertisementManager::CheckIngameAdvertisementCondition(CUser *,stNotifyIngameADInfo &,int)
int CInGameAdvertisementManager::CheckIngameAdvertisementCondition(
    CUser* user,
    stNotifyIngameADInfo& output,
    int request_id) {
    std::vector<stNotifyIngameADInfo> candidates;
    const std::pair<std::multimap<short, CAdvertisementExposeInfo*>::iterator,
                    std::multimap<short, CAdvertisementExposeInfo*>::iterator>
        range = expose_infos_.equal_range(static_cast<short>(request_id));
    for (std::multimap<short, CAdvertisementExposeInfo*>::iterator current =
             range.first;
         current != range.second;
         ++current) {
        stInGameADExposeCondition* const condition =
            current->second->getExposeCondition();
        if (condition && checkCondition(user, current->second) == 1) {
            candidates.push_back(condition->notify);
        }
    }

    if (candidates.empty()) {
        return 0;
    }
    output = candidates[RandNum(static_cast<int>(candidates.size()))];
    return 1;
}

// IDA_EA: 0x80FD122
// IDA_NAME: _ZN27CInGameAdvertisementManager16InitADExposeInfoEv
// DEMANGLED: CInGameAdvertisementManager::InitADExposeInfo(void)
int CInGameAdvertisementManager::InitADExposeInfo() {
    for (std::multimap<short, CAdvertisementExposeInfo*>::iterator info =
             expose_infos_.begin();
         info != expose_infos_.end(); ++info) {
        CAdvertisementExposeInfo::operator delete(info->second);
    }
    expose_infos_.clear();
    return 0;
}

// IDA_EA: 0x80FD19E
// IDA_NAME: _ZN27CInGameAdvertisementManager17queryADExposeInfoEP25stInGameADExposeCondition
// DEMANGLED: CInGameAdvertisementManager::queryADExposeInfo(stInGameADExposeCondition *)
int CInGameAdvertisementManager::queryADExposeInfo(
    stInGameADExposeCondition* conditions) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOGIN, SERVER_GROUP_DEFAULT);
    db_handle->set_query("seLect * from in_game_ad where visible = 1");
    if (db_handle->exec(true) != true) {
        return 0;
    }

    const int row_count = db_handle->get_n_rows();
    int index = 0;
    for (; index < row_count; ++index) {
        if (db_handle->fetch() != true) {
            break;
        }
        if (index > 199) {
            cMyTrace trace(
                "bool CInGameAdvertisementManager::queryADExposeInfo(stInGameADExposeCondition*)",
                235, 5);
            trace("[IN_GAME_ADVERTISEMENT_ERROR] count(%d)", row_count);
            break;
        }

        stInGameADExposeCondition& item = conditions[index];
        if (db_handle->get_uint(0, item.notify.banner_id) != true ||
            db_handle->get_str(1, item.notify.title,
                               sizeof(item.notify.title)) != true ||
            db_handle->get_byte(2, item.condition_type) != true ||
            db_handle->get_byte(3, item.condition_value) != true ||
            db_handle->get_byte(4, item.condition_flag) != true ||
            db_handle->get_str(5, item.level_filter,
                               sizeof(item.level_filter)) != true ||
            db_handle->get_str(6, item.notify.raw_col6,
                               sizeof(item.notify.raw_col6)) != true ||
            db_handle->get_uint(7, item.notify.raw_col7) != true ||
            db_handle->get_str(8, item.notify.raw_col8,
                               sizeof(item.notify.raw_col8)) != true ||
            db_handle->get_str(9, item.notify.raw_col9,
                               sizeof(item.notify.raw_col9)) != true ||
            db_handle->get_str(10, item.notify.raw_col10,
                               sizeof(item.notify.raw_col10)) != true ||
            db_handle->get_uint(11, item.notify.raw_col11) != true ||
            db_handle->get_str(12, item.notify.raw_col12,
                               sizeof(item.notify.raw_col12)) != true ||
            db_handle->get_uint(13, item.min_cera) != true ||
            db_handle->get_uint(14, item.max_cera) != true ||
            db_handle->get_byte(15, reinterpret_cast<char&>(item.notify.raw_col15)) != true ||
            db_handle->get_ubyte(17, item.probability_divisor) != true ||
            db_handle->get_short(18, item.advertisement_kind) != true ||
            db_handle->get_str(20, item.notify.raw_col20,
                               sizeof(item.notify.raw_col20)) != true ||
            db_handle->get_byte(21, reinterpret_cast<char&>(item.notify.raw_col21)) != true) {
            return 0;
        }
    }

    cMyTrace trace(
        "bool CInGameAdvertisementManager::queryADExposeInfo(stInGameADExposeCondition*)",
        300, 9, true, true);
    trace("[!] Load In Game Advertisement Expose Info From DB(%d) Complete",
          row_count);
    // The raw function writes its sentinel at the terminating index.  The
    // original DB contract therefore requires no more than 200 visible rows.
    conditions[index].notify.banner_id = static_cast<uint32_t>(-1);
    return 1;
}

// IDA_EA: 0x80FD83C
// IDA_NAME: _ZN27CInGameAdvertisementManager21LoadADExposeInfoAtAppEv
// DEMANGLED: CInGameAdvertisementManager::LoadADExposeInfoAtApp(void)
int CInGameAdvertisementManager::LoadADExposeInfoAtApp() {
    stInGameADExposeCondition conditions[200];
    std::memset(conditions, 0xff, sizeof(conditions));
    if (queryADExposeInfo(conditions) != 1) {
        return 0;
    }
    for (int index = 0; index <= 199 &&
                        conditions[index].notify.banner_id !=
                            static_cast<uint32_t>(-1);
         ++index) {
        InsertADExposeInfo(conditions[index]);
    }
    return 1;
}

// IDA_EA: 0x80FD8E4
// IDA_NAME: _ZN27CInGameAdvertisementManager18InsertADExposeInfoER25stInGameADExposeCondition
// DEMANGLED: CInGameAdvertisementManager::InsertADExposeInfo(stInGameADExposeCondition &)
int CInGameAdvertisementManager::InsertADExposeInfo(
    stInGameADExposeCondition& condition) {
    CAdvertisementExposeInfo* info = new CAdvertisementExposeInfo;
    info->SetAdvertisementExposeInfo(condition);
    expose_infos_.insert(
        std::make_pair(static_cast<short>(condition.advertisement_kind), info));
    return 0;
}

// IDA_EA: 0x80FD95E
// IDA_NAME: _ZN27CInGameAdvertisementManager18ReloadADExposeInfoEv
// DEMANGLED: CInGameAdvertisementManager::ReloadADExposeInfo(void)
int CInGameAdvertisementManager::ReloadADExposeInfo() {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "InGameAdvertisementManager.cpp", 336);
    CStreamGuard guard(stream, true);
    // Raw instructions write the full integer Inter signal 761.  Hex-Rays
    // rendered only its low byte (249), which routes to a different receiver.
    *guard << 761;
    *guard << -1;
    SIG_RELOAD_IN_GAME_ADVERTISEMENT* payload =
        guard->GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>();
    if (queryADExposeInfo(payload->entries) != 1) {
        return 0;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 1;
}

// IDA_EA: 0x80FDA56
// IDA_NAME: _ZN27CInGameAdvertisementManager8WriteLogEP5CUsercscj
// DEMANGLED: CInGameAdvertisementManager::WriteLog(CUser *,char,short,char,unsigned int)
void CInGameAdvertisementManager::WriteLog(CUser* user,
                                           char state,
                                           short view_code,
                                           char type_code,
                                           unsigned int banner_id) {
    if (!user) {
        return;
    }

    int dungeon_index = -1;
    int village_index = -1;
    int area_index = -1;
    if (user->get_state() == 5) {
        CParty* const party = user->GetParty();
        if (party) {
            CDungeon* const dungeon = party->battle_field().GetDungeon();
            if (dungeon) {
                dungeon_index = dungeon->get_index();
            }
        }
    } else if (user->get_state() == 3) {
        // The original calls these two accessors but never copies their
        // results to the payload.  Preserve that observable behavior.
        const int ignored_village = user->getCurCharacVill();
        const int ignored_area = user->get_area(false);
        (void)ignored_village;
        (void)ignored_area;
    }

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("InGameAdvertisementManager.cpp",
                                           378),
        true);
    guard << 812;
    guard << user->GetUID();
    SIG_IN_GAME_ADVERTISEMENT_LOG* const payload =
        guard.GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>();
    payload->m_id = user->get_acc_id();
    payload->charac_no = user->get_charac_no(-1);
    payload->area_index = area_index;
    payload->view_code = view_code;
    payload->state = state;
    payload->type_code = type_code;
    payload->banner_id = banner_id;
    payload->dungeon_index = dungeon_index;
    payload->village_index = village_index;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x80FDD34
// IDA_NAME: _ZN27CInGameAdvertisementManager7RandNumEi
// DEMANGLED: CInGameAdvertisementManager::RandNum(int)
unsigned int CInGameAdvertisementManager::RandNum(int maximum) {
    return random_->randInt() % static_cast<unsigned int>(maximum);
}

// IDA_EA: 0x80FC9E9
// IDA_NAME: _Z23InGameADManagerInstancev
// DEMANGLED: InGameADManagerInstance(void)
CInGameAdvertisementManager* InGameADManagerInstance() {
    static CInGameAdvertisementManager instance;
    return &instance;
}
