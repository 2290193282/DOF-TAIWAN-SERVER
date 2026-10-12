#include "old_game/economy/mall/quest_shop_runtime.h"

#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/itemshop/datamanager/quest_shop_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_data/PVF/itemshop/quest_shop_status.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <new>

namespace {

static const unsigned int kMaxQuestShopStatusType = 76u;
static const int kQuestShopLegacyResetTick = 1342645199;

typedef std::map<int, stQuestShopStatus> QuestShopStatusMap;
typedef std::map<uint8_t, short> StatusMap;

QuestShopStatusMap& QuestShopStatuses() {
    return G_CDataManager()->get_quest_shop_script()->statuses;
}

const stQuestShoptInitCost& QuestShopInitCost() {
    return G_CDataManager()->get_quest_shop_script()->init_cost;
}

const unsigned char* QuestShopAppendageStatusMap() {
    return G_CDataManager()->get_quest_shop_script()->appendage_status_map;
}

void FillCharacQuestShopData(const StatusMap& statuses,
                             int quest_point,
                             int init_count,
                             int quest_piece,
    CHARAC_QUEST_SHOP_DATA* data) {
    data->clear();
    data->quest_point = static_cast<uint32_t>(quest_point);
    data->init_count = static_cast<uint32_t>(init_count);
    data->quest_piece = static_cast<uint32_t>(quest_piece);
    for (StatusMap::const_iterator it = statuses.begin();
         it != statuses.end();
        ++it) {
        if (it->first <= kMaxQuestShopStatusType) {
            data->buying_count[it->first] =
                static_cast<uint16_t>(it->second);
        }
    }
}

void LoadStatusMap(const CHARAC_QUEST_SHOP_DATA* data,
                   StatusMap* statuses) {
    for (int type = 0; type <= static_cast<int>(kMaxQuestShopStatusType);
         ++type) {
        const short count = static_cast<short>(data->buying_count[type]);
        if (count) {
            statuses->insert(std::make_pair(static_cast<uint8_t>(type),
                                            count));
        }
    }
}

}  // namespace

typedef char MSG_QUEST_SHOP_Size_Must_Be_0x0f[
    (sizeof(MSG_QUEST_SHOP) == 0x0f) ? 1 : -1];
typedef char SIG_QUEST_SHOP_DATA_Size_Must_Be_0x155[
    (sizeof(SIG_QUEST_SHOP_DATA) == 0x155) ? 1 : -1];
typedef char CQuestShop_Size_Must_Be_0x48[
    (sizeof(CQuestShop) == 0x48) ? 1 : -1];

// IDA_EA: 0x822DE1C
// IDA_NAME: _ZN14MSG_QUEST_SHOPC2Ev
// DEMANGLED: MSG_QUEST_SHOP::MSG_QUEST_SHOP(void)
MSG_QUEST_SHOP::MSG_QUEST_SHOP()
    : MSG_BASE(),
      status_id(0),
      count(0) {}

// IDA_EA: 0x848F92E
// IDA_NAME: _ZN19SIG_QUEST_SHOP_DATAC2Ev
// DEMANGLED: SIG_QUEST_SHOP_DATA::SIG_QUEST_SHOP_DATA(void)
SIG_QUEST_SHOP_DATA::SIG_QUEST_SHOP_DATA()
    : current(),
      tagged(),
      has_tagged(0) {
    current.clear();
    tagged.clear();
}

// IDA_EA: 0x85EEBC8
// IDA_NAME: _ZN10CQuestShopC2Ev
// DEMANGLED: CQuestShop::CQuestShop(void)
CQuestShop::CQuestShop()
    : charac_expand::CData(),
      buying_status_list_(),
      tagged_buying_status_list_(),
      quest_point_(0),
      init_count_(0),
      init_quest_point_(0),
      quest_piece_(0) {
    _reset();
}

// IDA_EA: 0x85EEC60
// IDA_NAME: _ZN10CQuestShopD2Ev
// DEMANGLED: CQuestShop::~CQuestShop()
// IDA_EA: 0x85EECEA
// IDA_NAME: _ZN10CQuestShopD0Ev
// DEMANGLED: CQuestShop::~CQuestShop() deleting destructor
CQuestShop::~CQuestShop() {}

// IDA_EA: 0x85EED08
// IDA_NAME: _ZN10CQuestShop21clearBuyingStatuslistEv
// DEMANGLED: CQuestShop::clearBuyingStatuslist(void)
int CQuestShop::clearBuyingStatuslist() {
    alter();
    buying_status_list_.clear();
    return 0;
}

// IDA_EA: 0x85EED2A
// IDA_NAME: _ZN10CQuestShop5subQpEhh
// DEMANGLED: CQuestShop::subQp(unsigned char,unsigned char)
int CQuestShop::subQp(uint8_t status_id, uint8_t count) {
    const int status_qp = findStatusQp(status_id);
    if (status_qp <= 0) {
        return 1;
    }
    const int total_qp = status_qp * count;
    if (getQp() - total_qp < 0) {
        return 22;
    }
    subQp(total_qp);
    return 0;
}

// IDA_EA: 0x85EEDA8
// IDA_NAME: _ZN10CQuestShop12findStatusQpEh
// DEMANGLED: CQuestShop::findStatusQp(unsigned char)
int CQuestShop::findStatusQp(uint8_t status_id) {
    QuestShopStatusMap::iterator it =
        QuestShopStatuses().find(status_id);
    if (it != QuestShopStatuses().end()) {
        return it->second.quest_point;
    }
    return -1;
}

// IDA_EA: 0x85EEE6A
// IDA_NAME: _ZN10CQuestShop14findStatusTypeEh
// DEMANGLED: CQuestShop::findStatusType(unsigned char)
int CQuestShop::findStatusType(uint8_t status_id) {
    QuestShopStatusMap::iterator it =
        QuestShopStatuses().find(status_id);
    if (it != QuestShopStatuses().end()) {
        return it->second.appendage_type;
    }
    return 77;
}

// IDA_EA: 0x85EEF28
// IDA_NAME: _ZN10CQuestShop20incStatusBuyingCountEhh
// DEMANGLED: CQuestShop::incStatusBuyingCount(unsigned char,unsigned char)
int CQuestShop::incStatusBuyingCount(uint8_t status_id, uint8_t count) {
    const int status_type = findStatusType(status_id);
    if (status_type == 77) {
        return status_type;
    }

    alter();
    const uint8_t key = static_cast<uint8_t>(status_type);
    StatusMap::iterator it = buying_status_list_.find(key);
    if (it != buying_status_list_.end()) {
        it->second = static_cast<short>(it->second + count);
        return status_type;
    }
    buying_status_list_.insert(std::make_pair(key, static_cast<short>(count)));
    return status_type;
}

// IDA_EA: 0x85EF02E
// IDA_NAME: _ZN10CQuestShop20getStatusBuyingCountEh
// DEMANGLED: CQuestShop::getStatusBuyingCount(unsigned char)
int CQuestShop::getStatusBuyingCount(uint8_t status_id) {
    const int status_type = findStatusType(status_id);
    if (status_type == 77 || status_type == -1) {
        return -1;
    }
    StatusMap::iterator it =
        buying_status_list_.find(static_cast<uint8_t>(status_type));
    if (it != buying_status_list_.end()) {
        return static_cast<unsigned short>(it->second);
    }
    return 0;
}

// IDA_EA: 0x85EF0CE
// IDA_NAME: _ZN10CQuestShop28checkBuyingStatus_LimitLevelEhsi
// DEMANGLED: CQuestShop::checkBuyingStatus_LimitLevel(unsigned char,short,int)
bool CQuestShop::checkBuyingStatus_LimitLevel(uint8_t status_id,
                                              short buying_count,
                                              int charac_level) {
    QuestShopStatusMap::iterator it =
        QuestShopStatuses().find(status_id);
    if (it == QuestShopStatuses().end()) {
        return false;
    }
    return buying_count <= it->second.getMaxCount(charac_level);
}

// IDA_EA: 0x85EF1BA
// IDA_NAME: _ZN10CQuestShop11getInitCostEi
// DEMANGLED: CQuestShop::getInitCost(int)
int CQuestShop::getInitCost(int init_count) {
    const stQuestShoptInitCost& cost = QuestShopInitCost();
    if (!init_count) {
        return cost.unknown_00;
    }
    const int value = cost.unknown_00 + cost.unknown_04 * init_count;
    if (value < cost.unknown_08) {
        return value;
    }
    return cost.unknown_08;
}

// IDA_EA: 0x85EF214
// IDA_NAME: _ZN10CQuestShop9calUsedQpEv
// DEMANGLED: CQuestShop::calUsedQp(void)
int CQuestShop::calUsedQp() {
    int used_qp = 0;
    const unsigned char* appendage_status_map = QuestShopAppendageStatusMap();
    for (StatusMap::iterator it = buying_status_list_.begin();
         it != buying_status_list_.end();
         ++it) {
        const int status_type = it->first;
        const int buying_count = it->second;
        if (status_type > static_cast<int>(kMaxQuestShopStatusType)) {
            return -1;
        }
        const uint8_t status_id = appendage_status_map[status_type];
        const int status_qp = findStatusQp(status_id);
        if (!status_qp) {
            return -1;
        }
        used_qp += buying_count * status_qp;
    }
    return used_qp;
}

// IDA_EA: 0x85EF306
// IDA_NAME: _ZN10CQuestShop9buyStatusEP5CUserhhRs
// DEMANGLED: CQuestShop::buyStatus(CUser *,unsigned char,unsigned char,short &)
int CQuestShop::buyStatus(CUser* user,
                          uint8_t status_id,
                          uint8_t count,
                          short& buying_count) {
    const int current_count =
        static_cast<short>(getStatusBuyingCount(status_id));
    if (current_count == -1) {
        return 1;
    }

    buying_count = static_cast<short>(count + current_count);
    const int charac_level = user->get_charac_level();
    if (!checkBuyingStatus_LimitLevel(status_id, buying_count, charac_level)) {
        return 14;
    }

    const int before_qp = getQp();
    const int result = subQp(status_id, count);
    if (result) {
        return result;
    }

    incStatusBuyingCount(status_id, count);
    const int after_qp = getQp();
    CUser_history_log(user)->QPSub(before_qp, after_qp, status_id);
    return 0;
}

// IDA_EA: 0x85EF426
// IDA_NAME: _ZN10CQuestShop11clearUsedQpEP5CUser
// DEMANGLED: CQuestShop::clearUsedQp(CUser *)
int CQuestShop::clearUsedQp(CUser* user) {
    const int init_cost = getInitCost(getInitCount());
    const CInventory* inventory_r = reinterpret_cast<const CInventory*>(
        user->getCurCharacInvenR());
    if (inventory_r->get_money() < init_cost) {
        return 10;
    }

    CInventory* inventory_w = reinterpret_cast<CInventory*>(
        user->getCurCharacInvenW());
    if (inventory_w->use_money(
            init_cost, static_cast<eMoneySubReason>(43), true) != 1) {
        return 1;
    }

    const int before_qp = getQp();
    incInitCount();
    user->setInitQuestPoint();
    setQp(getInitQp());
    clearBuyingStatuslist();
    const int after_qp = getQp();
    CUser_history_log(user)->ClearUsedQP(before_qp,
                                         after_qp,
                                         getInitCount(),
                                         init_cost);
    return 0;
}

// IDA_EA: 0x85EF54C
// IDA_NAME: _ZN10CQuestShop7clearQPEP5CUser
// DEMANGLED: CQuestShop::clearQP(CUser *)
int CQuestShop::clearQP(CUser* user) {
    const int before_qp = getQp();
    user->setInitQuestPoint();
    setQp(getInitQp());
    clearBuyingStatuslist();
    const int after_qp = getQp();
    return CUser_history_log(user)->ClearUsedQP(before_qp,
                                                 after_qp,
                                                 getInitCount(),
                                                 0);
}

// IDA_EA: 0x85EF5DC
// IDA_NAME: _ZN10CQuestShop27makePacket_BuyingStatusListER11PacketGuardb
// DEMANGLED: CQuestShop::makePacket_BuyingStatusList(PacketGuard &,bool)
bool CQuestShop::makePacket_BuyingStatusList(PacketGuard& packet,
                                             bool tagged_status_list) {
    StatusMap& statuses = tagged_status_list ? tagged_buying_status_list_
                                             : buying_status_list_;
    packet.put_byte(static_cast<int>(statuses.size()));
    const unsigned char* appendage_status_map = QuestShopAppendageStatusMap();
    for (StatusMap::iterator it = statuses.begin(); it != statuses.end();
         ++it) {
        int status_id = -1;
        int buying_count = it->second;
        if (it->first <= kMaxQuestShopStatusType) {
            status_id = appendage_status_map[it->first];
        } else {
            buying_count = 0;
        }
        packet.put_byte(status_id);
        packet.put_short(buying_count);
    }
    return false;
}

// IDA_EA: 0x85EF6FC
// IDA_NAME: _ZN10CQuestShop12sendCharacQpEP5CUserc
// DEMANGLED: CQuestShop::sendCharacQp(CUser *,char)
void CQuestShop::sendCharacQp(CUser* user, char reason) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHARAC_QP_POINT);
    packet.put_int(getQp());
    packet.put_short(getInitCount());
    packet.put_byte(reason);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85EF7CC
// IDA_NAME: _ZN10CQuestShop20sendCharacQuestPieceEP5CUser
// DEMANGLED: CQuestShop::sendCharacQuestPiece(CUser *)
void CQuestShop::sendCharacQuestPiece(CUser* user) {
    // CLIENT_IDA_EVIDENCE: packet 446 is registered at 0x5A6F4F
    // to reader 0x5923C0, which consumes one byte for the character's
    // job/awakening appearance visibility flag.  The old quest-piece u32
    // writer cannot be sent on this single-version wire.  Keep the quest
    // piece state owned by CQuestShop until a distinct 0828 notification
    // id and reader are proven; do not corrupt NOTI446 with a guessed field.
    (void)user;
}

// IDA_EA: 0x85EF868
// IDA_NAME: _ZNK10CQuestShop7getDataEPc
// DEMANGLED: CQuestShop::getData(char *)const
void CQuestShop::getData(char* data) const {
    std::memset(data, 0, sizeof(SIG_QUEST_SHOP_DATA));
    SIG_QUEST_SHOP_DATA* payload =
        reinterpret_cast<SIG_QUEST_SHOP_DATA*>(data);
    FillCharacQuestShopData(buying_status_list_,
                            quest_point_,
                            init_count_,
                            quest_piece_,
                            &payload->current);
    FillCharacQuestShopData(tagged_buying_status_list_,
                            0,
                            0,
                            0,
                            &payload->tagged);
    payload->has_tagged = tagged_buying_status_list_.empty() ? 0 : 1;
}

// IDA_EA: 0x85EFA2C
// IDA_NAME: _ZN10CQuestShop8loadDataEP5CUserPc
// DEMANGLED: CQuestShop::loadData(CUser *,char *)
bool CQuestShop::loadData(CUser* user, char* data) {
    _reset();
    SIG_QUEST_SHOP_DATA* const payload =
        reinterpret_cast<SIG_QUEST_SHOP_DATA*>(data);
    quest_point_ = static_cast<int>(payload->current.quest_point);
    setInitCount(static_cast<int>(payload->current.init_count));
    quest_piece_ = static_cast<int>(payload->current.quest_piece);
    LoadStatusMap(&payload->current, &buying_status_list_);

    if (payload->has_tagged) {
        SetTagCharacQuestShopData(&payload->tagged);
    }

    if (user->getCurCharacLastPlayTick() <=
        kQuestShopLegacyResetTick) {
        clearQP(user);
    }
    return 1;
}

// IDA_EA: 0x85EFB66
// IDA_NAME: _ZN10CQuestShop9_saveDataEP5CUser
// DEMANGLED: CQuestShop::_saveData(CUser *)
int CQuestShop::_saveData(CUser* user) {
    Stream* stream = GlobalData::stream_pool()->Acquire("QuestShop.cpp", 539);
    CStreamGuard guard(stream, true);
    guard << 633;
    guard << user->GetUID();
    SIG_QUEST_SHOP_DATA* payload =
        guard.GetInBuffer<SIG_QUEST_SHOP_DATA>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    FillCharacQuestShopData(buying_status_list_,
                            quest_point_,
                            init_count_,
                            quest_piece_,
                            &payload->current);
    payload->current.charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x85EFD74
// IDA_NAME: _ZN10CQuestShop6_resetEv
// DEMANGLED: CQuestShop::_reset(void)
int CQuestShop::_reset() {
    buying_status_list_.clear();
    quest_point_ = 0;
    init_count_ = 0;
    init_quest_point_ = 0;
    tagged_buying_status_list_.clear();
    quest_piece_ = 0;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x85EFDC0
// IDA_NAME: _ZN10CQuestShop25SetTagCharacQuestShopDataEP22CHARAC_QUEST_SHOP_DATA
// DEMANGLED: CQuestShop::SetTagCharacQuestShopData(CHARAC_QUEST_SHOP_DATA *)
int CQuestShop::SetTagCharacQuestShopData(CHARAC_QUEST_SHOP_DATA* data) {
    LoadStatusMap(data, &tagged_buying_status_list_);
    return 1;
}

// IDA_EA: 0x85EFEB0
// IDA_NAME: _ZNK10CQuestShop5getQpEv
// DEMANGLED: CQuestShop::getQp(void)const
int CQuestShop::getQp() const {
    return quest_point_;
}

// IDA_EA: 0x85EFEBC
// IDA_NAME: _ZN10CQuestShop5subQpEi
// DEMANGLED: CQuestShop::subQp(int)
CQuestShop* CQuestShop::subQp(int quest_point) {
    alter();
    quest_point_ -= quest_point;
    return this;
}

// IDA_EA: 0x85EFEE0
// IDA_NAME: _ZN10CQuestShop5setQpEi
// DEMANGLED: CQuestShop::setQp(int)
CQuestShop* CQuestShop::setQp(int quest_point) {
    alter();
    quest_point_ = quest_point;
    return this;
}

// IDA_EA: 0x85EFEFC
// IDA_NAME: _ZN10CQuestShop12setInitCountEi
// DEMANGLED: CQuestShop::setInitCount(int)
CQuestShop* CQuestShop::setInitCount(int init_count) {
    init_count_ = init_count;
    return this;
}

// IDA_EA: 0x85EFF0A
// IDA_NAME: _ZNK10CQuestShop12getInitCountEv
// DEMANGLED: CQuestShop::getInitCount(void)const
int CQuestShop::getInitCount() const {
    return init_count_;
}

// IDA_EA: 0x85EFF16
// IDA_NAME: _ZN10CQuestShop12incInitCountEv
// DEMANGLED: CQuestShop::incInitCount(void)
CQuestShop* CQuestShop::incInitCount() {
    alter();
    ++init_count_;
    return this;
}

// IDA_EA: 0x85EFF38
// IDA_NAME: _ZNK10CQuestShop9getInitQpEv
// DEMANGLED: CQuestShop::getInitQp(void)const
int CQuestShop::getInitQp() const {
    return init_quest_point_;
}

// IDA_EA: 0x85EFF44
// IDA_NAME: _ZNK10CQuestShop8getPieceEv
// DEMANGLED: CQuestShop::getPiece(void)const
int CQuestShop::getPiece() const {
    return quest_piece_;
}

// IDA_EA: 0x8695A1C
// IDA_NAME: _ZN10CQuestShop5addQpEi
// DEMANGLED: CQuestShop::addQp(int)
CQuestShop* CQuestShop::addQp(int quest_point) {
    alter();
    quest_point_ += quest_point;
    return this;
}

// IDA_EA: 0x8695A40
// IDA_NAME: _ZN10CQuestShop9incInitQpEi
// DEMANGLED: CQuestShop::incInitQp(int)
CQuestShop* CQuestShop::incInitQp(int quest_point) {
    init_quest_point_ += quest_point;
    return this;
}

// IDA_EA: 0x8695A56
// IDA_NAME: _ZN10CQuestShop9setInitQpEi
// DEMANGLED: CQuestShop::setInitQp(int)
CQuestShop* CQuestShop::setInitQp(int quest_point) {
    init_quest_point_ = quest_point;
    return this;
}

// IDA_EA: 0x8695A64
// IDA_NAME: _ZN10CQuestShop8addPieceEi
// DEMANGLED: CQuestShop::addPiece(int)
CQuestShop* CQuestShop::addPiece(int piece) {
    alter();
    quest_piece_ += piece;
    return this;
}

// IDA_EA: 0x8695A88
// IDA_NAME: _ZN10CQuestShop8setPieceEi
// DEMANGLED: CQuestShop::setPiece(int)
CQuestShop* CQuestShop::setPiece(int piece) {
    alter();
    quest_piece_ = piece;
    return this;
}
