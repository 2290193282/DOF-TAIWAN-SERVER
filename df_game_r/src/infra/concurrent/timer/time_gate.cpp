#include "infra/concurrent/timer/time_gate.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"

CTimeGate* GlobalData::s_timeGate_ = 0;

namespace {

// The official live service raised this byte through the monitor-server sand
// collection event.  A standalone old-server deployment has no population
// aggregator, so the value would otherwise remain zero forever.  Keep the
// gate permanently open at the same authoritative server-side value used by
// the original runtime (CTimeGate + 0x04 == 100).
const uint8_t kForcedTimeGateOpenLevel = 100u;

}  // namespace

// IDA_EA: 0x8284512
// IDA_NAME: _ZN9CTimeGateC2Ev
// DEMANGLED: CTimeGate::CTimeGate(void)
CTimeGate::CTimeGate()
    : village_object::IObject(),
      open_level_(kForcedTimeGateOpenLevel),
      reserved_05_(),
      village_index_(0),
      area_index_(0),
      npc_index_(0),
      quest_index_(0),
      opened_quest_index_(0) {}

// IDA_EA: 0x8284536
// IDA_NAME: _ZN9CTimeGateD2Ev
// DEMANGLED: CTimeGate::~CTimeGate(void)
CTimeGate::~CTimeGate() {}

// IDA_EA: 0x82845A0
// IDA_NAME: _ZNK9CTimeGate7includeERKN14village_object7IObjectE
// DEMANGLED: CTimeGate::include(village_object::IObject const&)const
bool CTimeGate::include(const village_object::IObject&) const {
    return false;
}

// IDA_EA: 0x82844D8
// IDA_NAME: _ZN9CTimeGate15put_object_infoER11PacketGuard
// DEMANGLED: CTimeGate::put_object_info(PacketGuard &)
bool CTimeGate::put_object_info(PacketGuard& packet) {
    packet.put_byte(2);
    packet.put_byte(open_level_);
    return true;
}

// IDA_EA: 0x8234E1C
// IDA_NAME: _ZNK9CTimeGate6isOpenEv
// DEMANGLED: CTimeGate::isOpen(void)const
bool CTimeGate::isOpen() const {
    return open_level_ == 100;
}

// IDA_EA: 0x8234E2E
// IDA_NAME: _ZNK9CTimeGate13getQuestIndexEv
// DEMANGLED: CTimeGate::getQuestIndex(void)const
int CTimeGate::getQuestIndex() const {
    return quest_index_;
}

// IDA_EA: 0x8234E3A
// IDA_NAME: _ZNK9CTimeGate19getOpenedQuestIndexEv
// DEMANGLED: CTimeGate::getOpenedQuestIndex(void)const
int CTimeGate::getOpenedQuestIndex() const {
    return opened_quest_index_;
}

// IDA_EA: 0x8234E46
// IDA_NAME: _ZN9CTimeGate26setBlindTimeGateStateQuestEi
// DEMANGLED: CTimeGate::setBlindTimeGateStateQuest(int)
int CTimeGate::setBlindTimeGateStateQuest(int quest_index) {
    if (getQuestIndex() == quest_index && isOpen()) {
        return 1;
    }
    return getOpenedQuestIndex() == quest_index && !isOpen();
}

// IDA_EA: 0x84ED480
// IDA_NAME: _ZNK9CTimeGate15getVillageIndexEv
// DEMANGLED: CTimeGate::getVillageIndex(void)const
int CTimeGate::getVillageIndex() const {
    return village_index_;
}

// IDA_EA: 0x84ED48C
// IDA_NAME: _ZNK9CTimeGate12getAreaIndexEv
// DEMANGLED: CTimeGate::getAreaIndex(void)const
int CTimeGate::getAreaIndex() const {
    return area_index_;
}

// IDA_EA: 0x84ED498
// IDA_NAME: _ZNK9CTimeGate11getNpcIndexEv
// DEMANGLED: CTimeGate::getNpcIndex(void)const
int CTimeGate::getNpcIndex() const {
    return npc_index_;
}

// IDA_EA: 0x84ED4A4
// IDA_NAME: _ZN9CTimeGate8SetLevelEi
// DEMANGLED: CTimeGate::SetLevel(int)
CTimeGate* CTimeGate::SetLevel(int level) {
    (void)level;
    open_level_ = kForcedTimeGateOpenLevel;
    return this;
}

// IDA_EA: 0x86D2788
// IDA_NAME: _ZN9CTimeGate11SetTimeGateEiiiii
// DEMANGLED: CTimeGate::SetTimeGate(int,int,int,int,int)
CTimeGate* CTimeGate::SetTimeGate(int village_index,
                                  int area_index,
                                  int npc_index,
                                  int quest_index,
                                  int opened_quest_index) {
    village_index_ = village_index;
    area_index_ = area_index;
    npc_index_ = npc_index;
    quest_index_ = quest_index;
    opened_quest_index_ = opened_quest_index;
    return this;
}
