#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_DISJOINTER_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_DISJOINTER_H_

#include <stdint.h>

#include <string>

#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/collision_detection_runtime.h"
#include "old_game/world/region/game_world.h"

class CUser;
class PacketGuard;
class CItem;
struct Inven_Item;
struct DisJointResult;
struct STExpertJobEtcScript;
struct STExpertJobScript;

namespace expert_job {

// IDA unit: 99_misc_global/expert_job/CDisjointMachine.
// CDisjointer embeds this 0x2c-byte village object at +0x08.  The packet
// dispatcher helpers below are static, but the factory owns actual instances;
// keeping the full object here prevents a second, layout-incompatible class
// declaration in a dispatcher translation unit.
class CDisjointMachine : public village_object::IObject {
public:
    // IDA_EA: 0x85D1AE8
    // IDA_NAME: _ZN10expert_job16CDisjointMachineC2EP5CUserPKcicii
    // DEMANGLED: expert_job::CDisjointMachine::CDisjointMachine(
    //     CUser *,char const *,int,char,int,int)
    CDisjointMachine(CUser* user,
                     const char* title,
                     int cost,
                     char grade,
                     int pos_x,
                     int pos_y);

    // IDA_EA: 0x85D3D7E
    // IDA_NAME: _ZN10expert_job16CDisjointMachineC2Ev
    // DEMANGLED: expert_job::CDisjointMachine::CDisjointMachine(void)
    CDisjointMachine();

    // IDA_EA: 0x85D3EB6
    // IDA_NAME: _ZN10expert_job16CDisjointMachineD2Ev
    // DEMANGLED: expert_job::CDisjointMachine::~CDisjointMachine()
    ~CDisjointMachine();

    // IDA_EA: 0x85D3ED6
    // IDA_NAME: _ZN10expert_job16CDisjointMachineaSERKS0_
    // DEMANGLED: expert_job::CDisjointMachine::operator=(
    //     expert_job::CDisjointMachine const&)
    CDisjointMachine& operator=(const CDisjointMachine& rhs);

    // IDA_EA: 0x85D3DE6
    // IDA_NAME: _ZNK10expert_job16CDisjointMachine7includeERKN14village_object7IObjectE
    virtual bool include(const village_object::IObject& rhs) const;

    // IDA_EA: 0x85D1CC2
    // IDA_NAME: _ZN10expert_job16CDisjointMachine15put_object_infoER11PacketGuard
    virtual bool put_object_info(PacketGuard& packet);

    // IDA_EA: 0x85D1BC0
    // IDA_NAME: _ZN10expert_job16CDisjointMachine25put_disjoint_machine_infoER11PacketGuard
    int put_disjoint_machine_info(PacketGuard& packet);

    // IDA_EA: 0x82347E0 / 0x85D3E06 / 0x85D3E12 / 0x85D3E1E
    CUser* get_user() const;
    int get_cost() const;
    int get_grade() const;
    void reset();

private:
    collision_detection::Rectangle bounds_;  // +0x04
    CUser* user_;                            // +0x14
    std::string title_;                      // +0x18
    int cost_;                               // +0x1c
    uint8_t grade_;                          // +0x20
    uint8_t reserved_21_[3];                 // +0x21
    int pos_x_;                              // +0x24
    int pos_y_;                              // +0x28
};

// IDA unit: 99_misc_global/expert_job/CDisjointer.
// C2/D2 and the four virtual slots are owned by the expert-job factory.  The
// CMD240/CMD244 helpers are static member functions: their original call
// sites place a single CUser* in the first ABI argument register/stack slot,
// and the mangled names contain exactly that explicit argument.
class CDisjointer : public CExpertJob {
public:
    CDisjointer();
    virtual ~CDisjointer();

    virtual int OnStartMission(CUser* user, CUser* peer_user);
    virtual void OnCloseMission(CUser* user,
                                ENUM_CLOSE_MISSION_TYPE close_type);
    virtual int IsAvailableJob();
    virtual void Reset();

    // IDA_EA: 0x85D1E0A / 0x85D1E3A
    static STExpertJobScript* GetScript();
    static STExpertJobEtcScript* GetEtcScript();

    // IDA_EA: 0x82347EC / 0x8234854 / 0x85D234A.  CMD239 delegates the
    // portable-machine result roll to these members after the owner/type
    // checks in DisPatcher_DisJointItem.
    int get_disjoint_result(CItem* item,
                            Inven_Item* inventory_item,
                            DisJointResult& result,
                            CUser* requester);
    bool isMine(CUser* user) const;
    static int get_disjoint_result_(CItem* item,
                                    Inven_Item* inventory_item,
                                    int machine_grade,
                                    DisJointResult& result,
                                    bool owner_request,
                                    CUser* owner);

    // IDA_EA: 0x85D31A0.  Appends money/endurance to the CMD28/CMD239
    // success packet and completes the machine-owner accounting.
    int complete_disjoint(CUser* user, PacketGuard& packet);

    // IDA_EA: 0x85D35CE / 0x85D37E8
    // IDA_NAME: _ZN10expert_job11CDisjointer23OnRepairDisjointMachineEP5CUser
    //           _ZN10expert_job11CDisjointer24OnUpgradeDisjointMachineEP5CUser
    static int OnRepairDisjointMachine(CUser* user);
    static int OnUpgradeDisjointMachine(CUser* user);

    // IDA_EA: 0x85D2D08 / 0x85D309A / 0x85D1F0A / 0x85D3E50
    // IDA_NAME: _ZN10expert_job11CDisjointer21OnCreateDisjointStoreEP5CUserPKciii
    //           _ZN10expert_job11CDisjointer20OnEnterDisjointStoreEP5CUser
    int OnCreateDisjointStore(CUser* user,
                              const char* title,
                              int cost,
                              int pos_x,
                              int pos_y);
    int OnEnterDisjointStore(CUser* user);
    int get_disjoint_machine_grade();
    int get_disjoint_cost() const;

private:
    void send_error(CUser* user, int error);
    bool check_disjoint_endurance();

    CDisjointMachine machine_;  // +0x08
};

#if defined(__i386__)
static_assert(sizeof(CDisjointMachine) == 0x2cu,
              "CDisjointMachine must retain the original i386 ABI");
static_assert(sizeof(CDisjointer) == 0x34u,
              "CDisjointer must retain the original i386 ABI");
#endif

}  // namespace expert_job

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_DISJOINTER_H_
