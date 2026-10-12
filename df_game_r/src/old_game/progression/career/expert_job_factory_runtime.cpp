#include "old_game/progression/quest/quest_runtime.h"

#include "old_game/progression/career/expert_job_disjointer.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/collision_detection_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <new>
#include <stdio.h>
#include <string>

enum ENUM_ERROR : int;

namespace {

const uint32_t kExpertJobPoolDummy = 36641u;

int TraceReferenceCount(uint8_t reference_count) {
    return static_cast<int>(static_cast<int8_t>(reference_count));
}

}  // namespace

namespace expert_job {

class CExpertJobFactory {
public:
    CExpertJobFactory();
    ~CExpertJobFactory();

    CExpertJob* AcquireExpertJob(ENUM_EXPERT_JOB_TYPE expert_job_type);
    CExpertJob* FreeExpertJob(CExpertJob* expert_job);

private:
    StaticPool<CEnchanter, 300> enchanter_pool_;    // +0x00
    StaticPool<CDisjointer, 600> disjointer_pool_;  // +0x30
};


// IDA_EA: 0x849F2B4
// IDA_NAME: _ZN10expert_job10CExpertJobC2Ev
// DEMANGLED: expert_job::CExpertJob::CExpertJob(void)
CExpertJob::CExpertJob() {}

// IDA_EA: 0x849F1FA / 0x849F220
// IDA_NAME: _ZN10expert_job10CExpertJobD2Ev
// DEMANGLED: expert_job::CExpertJob::~CExpertJob()
CExpertJob::~CExpertJob() {}

// IDA_EA: 0x849F1EC
// IDA_NAME: _ZN10expert_job10CExpertJob7SetTypeE20ENUM_EXPERT_JOB_TYPE
// DEMANGLED: expert_job::CExpertJob::SetType(ENUM_EXPERT_JOB_TYPE)
CExpertJob* CExpertJob::SetType(ENUM_EXPERT_JOB_TYPE expert_job_type) {
    type_ = static_cast<int>(expert_job_type);
    return this;
}

// IDA_EA: 0x85D3D7E
// IDA_NAME: _ZN10expert_job16CDisjointMachineC2Ev
// DEMANGLED: expert_job::CDisjointMachine::CDisjointMachine(void)
CDisjointMachine::CDisjointMachine()
    : village_object::IObject(),
      bounds_(),
      user_(0),
      title_(),
      cost_(0),
      grade_(0),
      pos_x_(0),
      pos_y_(0) {}

// IDA_EA: 0x85D1AE8
// IDA_NAME: _ZN10expert_job16CDisjointMachineC2EP5CUserPKcicii
// DEMANGLED: expert_job::CDisjointMachine::CDisjointMachine(CUser *,char const *,int,char,int,int)
CDisjointMachine::CDisjointMachine(CUser* user,
                                   const char* title,
                                   int cost,
                                   char grade,
                                   int pos_x,
                                   int pos_y)
    : village_object::IObject(),
      bounds_(),
      user_(user),
      title_(title),
      cost_(cost),
      grade_(static_cast<uint8_t>(grade)),
      pos_x_(pos_x),
      pos_y_(pos_y) {
    // IDA 0x85D1B75/0x85D1B7B/0x85D1B8E/0x85D1B94 initializes the
    // collision rectangle from the named village coordinates.
    bounds_.x = pos_x_ - 80;
    bounds_.width = 160;
    bounds_.y = pos_y_ - 50;
    bounds_.height = 100;
}

// IDA_EA: 0x85D3ED6
// IDA_NAME: _ZN10expert_job16CDisjointMachineaSERKS0_
// DEMANGLED: expert_job::CDisjointMachine::operator=(expert_job::CDisjointMachine const&)
CDisjointMachine& CDisjointMachine::operator=(
    const CDisjointMachine& rhs) {
    village_object::IObject::operator=(rhs);
    bounds_ = rhs.bounds_;
    user_ = rhs.user_;
    title_ = rhs.title_;
    cost_ = rhs.cost_;
    grade_ = rhs.grade_;
    pos_x_ = rhs.pos_x_;
    pos_y_ = rhs.pos_y_;
    return *this;
}

// IDA_EA: 0x85D3EB6
// IDA_NAME: _ZN10expert_job16CDisjointMachineD2Ev
// DEMANGLED: expert_job::CDisjointMachine::~CDisjointMachine()
CDisjointMachine::~CDisjointMachine() {}

// IDA_EA: 0x85D3DE6
// IDA_NAME: _ZNK10expert_job16CDisjointMachine7includeERKN14village_object7IObjectE
// DEMANGLED: expert_job::CDisjointMachine::include(
//     village_object::IObject const&)const
bool CDisjointMachine::include(
    const village_object::IObject& rhs) const {
    // All original village IObject implementations place their collision
    // rectangle immediately after the shared vptr.  IDA 0x85D3DE6 performs
    // this exact +0x04 base adjustment before Rectangle::include.
    const collision_detection::Rectangle* const rhs_bounds =
        reinterpret_cast<const collision_detection::Rectangle*>(
            reinterpret_cast<const uint8_t*>(&rhs) + sizeof(void*));
    return bounds_.include(*rhs_bounds);
}

// IDA_EA: 0x85D1CC2
// IDA_NAME: _ZN10expert_job16CDisjointMachine15put_object_infoER11PacketGuard
// DEMANGLED: expert_job::CDisjointMachine::put_object_info(PacketGuard &)
bool CDisjointMachine::put_object_info(PacketGuard& packet) {
    if (!user_) {
        cMyTrace trace(
            "virtual bool expert_job::CDisjointMachine::put_object_info(PacketGuard&)",
            51,
            0);
        trace("NOTEXIST_ERROR : disjoint_machine_user(null)");
        return false;
    }

    packet.put_byte(0);
    packet.put_short(user_->get_unique_id());
    const int title_length = static_cast<int>(title_.length());
    packet.put_int(title_length);
    packet.put_str(title_.c_str(), title_length);
    packet.put_byte(user_->getCurCharacVill());
    packet.put_byte(user_->get_area(false));
    packet.put_short(pos_x_);
    packet.put_short(pos_y_);
    packet.put_int(cost_);
    return true;
}

// IDA_EA: 0x85D1BC0
// IDA_NAME: _ZN10expert_job16CDisjointMachine25put_disjoint_machine_infoER11PacketGuard
// DEMANGLED: expert_job::CDisjointMachine::put_disjoint_machine_info(PacketGuard &)
int CDisjointMachine::put_disjoint_machine_info(PacketGuard& packet) {
    packet.put_short(user_->get_unique_id());
    const int title_length = static_cast<int>(title_.length());
    packet.put_int(title_length);
    packet.put_str(title_.c_str(), title_length);
    packet.put_byte(user_->getCurCharacVill());
    packet.put_byte(user_->get_area(false));
    packet.put_short(pos_x_);
    packet.put_short(pos_y_);
    return packet.put_int(cost_);
}

// IDA_EA: 0x82347E0
// IDA_NAME: _ZNK10expert_job16CDisjointMachine8get_userEv
CUser* CDisjointMachine::get_user() const {
    return user_;
}

// IDA_EA: 0x85D3E06
// IDA_NAME: _ZNK10expert_job16CDisjointMachine8get_costEv
int CDisjointMachine::get_cost() const {
    return cost_;
}

// IDA_EA: 0x85D3E12
// IDA_NAME: _ZNK10expert_job16CDisjointMachine9get_gradeEv
int CDisjointMachine::get_grade() const {
    return grade_;
}

// IDA_EA: 0x85D3E1E
// IDA_NAME: _ZN10expert_job16CDisjointMachine5resetEv
void CDisjointMachine::reset() {
    user_ = 0;
    title_.clear();
    cost_ = 0;
    grade_ = 0;
}

// IDA_EA: 0x85D1D3C
// IDA_NAME: _ZN10expert_job11CDisjointerC2Ev
CDisjointer::CDisjointer()
    : CExpertJob(),
      machine_() {}

// IDA_EA: 0x85D1D8A / 0x85D1DEC
// IDA_NAME: _ZN10expert_job11CDisjointerD2Ev
CDisjointer::~CDisjointer() {}

// IDA_EA: 0x849F596
// IDA_NAME: _ZN10expert_job11CDisjointer14IsAvailableJobEv
int CDisjointer::IsAvailableJob() {
    return 1;
}

// IDA_EA: 0x85D2CF2
// IDA_NAME: _ZN10expert_job11CDisjointer5ResetEv
void CDisjointer::Reset() {
    machine_.reset();
}

// IDA_EA: 0x85D1E62
// IDA_NAME: _ZN10expert_job11CDisjointer10send_errorEP5CUseri
void CDisjointer::send_error(CUser* user, int error) {
    PacketGuard packet;
    packet.put_header(0, 203);
    packet.put_byte(0);
    packet.put_byte(error);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85D1F24
// IDA_NAME: _ZN10expert_job11CDisjointer24check_disjoint_enduranceEv
bool CDisjointer::check_disjoint_endurance() {
    CUser* const owner = machine_.get_user();
    if (!owner) {
        return false;
    }
    CCharacExpertJob* const state = static_cast<CCharacExpertJob*>(
        owner->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
    return state && state->get_disjoint_machine_endurance() > 0;
}

// IDA_EA: 0x85D2A16
// IDA_NAME: _ZN10expert_job11CDisjointer14OnStartMissionEP5CUserS2_
int CDisjointer::OnStartMission(CUser* user, CUser* peer_user) {
    if (user != peer_user &&
        peer_user->getCurCharacMoney() < machine_.get_cost()) {
        throw static_cast<ENUM_ERROR>(21);
    }

    if (!user->CheckMoney(machine_.get_cost())) {
        send_error(user, 10);
        OnCloseMission(machine_.get_user(),
                       static_cast<ENUM_CLOSE_MISSION_TYPE>(0));
        throw static_cast<ENUM_ERROR>(10);
    }

    if (!check_disjoint_endurance()) {
        send_error(user, 189);
        throw static_cast<ENUM_ERROR>(189);
    }
    return 0;
}

// IDA_EA: 0x85D2B74
// IDA_NAME: _ZN10expert_job11CDisjointer14OnCloseMissionEP5CUserNS_23ENUM_CLOSE_MISSION_TYPEE
void CDisjointer::OnCloseMission(
    CUser* user,
    ENUM_CLOSE_MISSION_TYPE close_type) {
    (void)close_type;
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 217);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);
    G_GameWorld()->send_to_area(
        user->getCurCharacVill(), user->get_area(false), packet);

    village_object::Zone zone(
        static_cast<uint8_t>(user->getCurCharacVill()),
        static_cast<uint8_t>(user->get_area(false)));
    if (GlobalData::s_villageObjectMgr &&
        GlobalData::s_villageObjectMgr->unregister_object(
            zone, &machine_) != 1) {
        LogManager::logFormat(
            1,
            "ProfessionDisjointer.cpp",
            "virtual void expert_job::CDisjointer::OnCloseMission(CUser*, expert_job::ENUM_CLOSE_MISSION_TYPE)",
            0x15b,
            "unregister_object error");
    }

    user->SetCurCharacExpertJob(0);
    if (GlobalData::s_ExpertJobMgr) {
        GlobalData::s_ExpertJobMgr->FreeExpertJob(this);
    }
}

}  // namespace expert_job

template <typename Type, int Count>
StaticPool<Type, Count>::CNode::CNode()
    : Type(),
      dummy(kExpertJobPoolDummy),
      reference_count(0) {}

template <typename Type, int Count>
StaticPool<Type, Count>::CNode::~CNode() {}

template <typename Type, int Count>
StaticPool<Type, Count>::StaticPool()
    : nodes(0),
      free_nodes(),
      error_state(0) {
    nodes = new CNode[Count];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }
    for (int index = 0; index < Count; ++index) {
        free_nodes.push(nodes + index);
    }
}

template <typename Type, int Count>
StaticPool<Type, Count>::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

template <typename Type, int Count>
int StaticPool<Type, Count>::Size() {
    return Count;
}

template <typename Type, int Count>
int StaticPool<Type, Count>::Remain() {
    return static_cast<int>(free_nodes.size());
}

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kExpertJobPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<Type*>(node);
}

template <typename Type, int Count>
int StaticPool<Type, Count>::GetIndex(Type* value) {
    const uintptr_t begin = reinterpret_cast<uintptr_t>(nodes);
    const uintptr_t address = reinterpret_cast<uintptr_t>(value);
    if (address < begin) {
        return -1;
    }
    const uintptr_t distance = address - begin;
    const int index = static_cast<int>(distance / sizeof(CNode));
    if (index < 0 || index >= Count) {
        return -1;
    }

    CNode* const node = static_cast<CNode*>(value);
    if (node->dummy != kExpertJobPoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 1;
    }
    if (node->reference_count != 1) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 2;
    }
    return index;
}

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Get(int index) {
    if (index < 0 || index >= Count) {
        return 0;
    }
    CNode* const node = nodes + index;
    if (node->dummy != kExpertJobPoolDummy) {
        cMyTrace trace("Get", 195, 0);
        trace("STATIC MEMORY_POOL Get error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 7;
    }
    if (node->reference_count >= 2u) {
        cMyTrace trace("Get", 202, 0);
        trace("STATIC MEMORY_POOL Get error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 8;
    }
    return static_cast<Type*>(node);
}

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Free(Type* value) {
    if (!value) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
        return 0;
    }

    CNode* const node = static_cast<CNode*>(value);
    free_nodes.push(node);
    if (node->dummy != kExpertJobPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 5;
    }
    if (node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return value;
}

template class StaticPool<expert_job::CEnchanter, 300>;
template class StaticPool<expert_job::CDisjointer, 600>;

namespace expert_job {

// IDA_EA: 0x849F68C / 0x849F6D2
CExpertJobFactory::CExpertJobFactory()
    : enchanter_pool_(),
      disjointer_pool_() {}

CExpertJobFactory::~CExpertJobFactory() {}

// IDA_EA: 0x849F5D2
// IDA_NAME: _ZN10expert_job17CExpertJobFactory16AcquireExpertJobE20ENUM_EXPERT_JOB_TYPE
CExpertJob* CExpertJobFactory::AcquireExpertJob(
    ENUM_EXPERT_JOB_TYPE expert_job_type) {
    CExpertJob* result = 0;
    if (static_cast<int>(expert_job_type) == 1) {
        result = enchanter_pool_.Acquire();
    } else if (static_cast<int>(expert_job_type) == 3) {
        result = disjointer_pool_.Acquire();
    }

    if (result) {
        result->Reset();
        result->SetType(expert_job_type);
    }
    return result;
}

// IDA_EA: 0x849F63E
// IDA_NAME: _ZN10expert_job17CExpertJobFactory13FreeExpertJobEPNS_10CExpertJobE
CExpertJob* CExpertJobFactory::FreeExpertJob(CExpertJob* expert_job) {
    if (!expert_job) {
        return 0;
    }
    if (expert_job->GetType() == 1) {
        return enchanter_pool_.Free(
            static_cast<CEnchanter*>(expert_job));
    }
    if (expert_job->GetType() == 3) {
        return disjointer_pool_.Free(
            static_cast<CDisjointer*>(expert_job));
    }
    return expert_job;
}

// IDA_EA: 0x849E102
// IDA_NAME: _ZN10expert_job13CExpertJobMgrC2Ev
CExpertJobMgr::CExpertJobMgr()
    : factory_(new (std::nothrow) CExpertJobFactory()) {
    if (!factory_) {
        printf("memory error func(%s) line(%d)",
               "expert_job::CExpertJobMgr::CExpertJobMgr()",
               1407);
    }
}

// IDA_EA: 0x849E1AC
// IDA_NAME: _ZN10expert_job13CExpertJobMgrD2Ev
CExpertJobMgr::~CExpertJobMgr() {
    delete factory_;
}

// IDA_EA: 0x849E1DC
// IDA_NAME: _ZN10expert_job13CExpertJobMgr16AcquireExpertJobE20ENUM_EXPERT_JOB_TYPE
CExpertJob* CExpertJobMgr::AcquireExpertJob(
    ENUM_EXPERT_JOB_TYPE expert_job_type) {
    return factory_ ? factory_->AcquireExpertJob(expert_job_type) : 0;
}

// IDA_EA: 0x849E1F8
// IDA_NAME: _ZN10expert_job13CExpertJobMgr13FreeExpertJobEPNS_10CExpertJobE
CExpertJob* CExpertJobMgr::FreeExpertJob(CExpertJob* expert_job) {
    return factory_ ? factory_->FreeExpertJob(expert_job) : 0;
}

#if defined(__i386__)
static_assert(sizeof(CDisjointMachine) == 0x2cu,
              "CDisjointMachine old ABI must stay 0x2c");
static_assert(sizeof(CDisjointer) == 0x34u,
              "CDisjointer old ABI must stay 0x34");
static_assert(
    sizeof(StaticPool<CEnchanter, 300>::CNode) == 0x50u,
    "StaticPool<CEnchanter,300>::CNode old ABI must stay 0x50");
static_assert(
    sizeof(StaticPool<CDisjointer, 600>::CNode) == 0x3cu,
    "StaticPool<CDisjointer,600>::CNode old ABI must stay 0x3c");
static_assert(sizeof(CExpertJobFactory) == 0x60u,
              "CExpertJobFactory old ABI must stay 0x60");
#endif

}  // namespace expert_job
