#ifndef OLDSRV_GLOBAL_TIME_GATE_H_
#define OLDSRV_GLOBAL_TIME_GATE_H_

#include <stdint.h>

#include "old_game/world/region/game_world.h"

// IDA unit: 99_misc_global/CTimeGate.
// IDA_EA: ctor/dtor @0x8284512/@0x8284536; direct getters/state setters
// @0x8234E1C/@0x8234E2E/@0x8234E3A/@0x8234E46 and
// @0x84ED480/@0x84ED48C/@0x84ED498/@0x84ED4A4/@0x86D2788.
// The old class is in the global namespace and is a concrete village object.
// Its IObject vptr occupies the first dword; the remaining fields keep their
// original offsets and the complete object remains 0x1c bytes on i386.
class CTimeGate : public village_object::IObject {
public:
    CTimeGate();
    ~CTimeGate();

    virtual bool include(const village_object::IObject& rhs) const;
    virtual bool put_object_info(PacketGuard& packet);

    bool isOpen() const;  // IDA_EA: 0x8234E1C.
    int getQuestIndex() const;  // IDA_EA: 0x8234E2E.
    int getOpenedQuestIndex() const;  // IDA_EA: 0x8234E3A.
    int setBlindTimeGateStateQuest(int quest_index);  // IDA_EA: 0x8234E46.
    int getVillageIndex() const;  // IDA_EA: 0x84ED480.
    int getAreaIndex() const;  // IDA_EA: 0x84ED48C.
    int getNpcIndex() const;  // IDA_EA: 0x84ED498.
    CTimeGate* SetLevel(int level);  // IDA_EA: 0x84ED4A4.
    CTimeGate* SetTimeGate(int village_index,
                           int area_index,
                           int npc_index,
                           int quest_index,
                           int opened_quest_index);  // IDA_EA: 0x86D2788.

private:
    uint8_t open_level_;
    uint8_t reserved_05_[3];
    int32_t village_index_;
    int32_t area_index_;
    int32_t npc_index_;
    int32_t quest_index_;
    int32_t opened_quest_index_;
};

static_assert(sizeof(CTimeGate) == 0x1Cu,
              "CTimeGate IDA ABI size must stay 0x1C");

#endif  // OLDSRV_GLOBAL_TIME_GATE_H_
