#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_TREE_PARAM_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_TREE_PARAM_RUNTIME_H_

#include <stdint.h>

#include <vector>


class CUser;
class Inter_LoadEtc;
class PacketGuard;
struct SIG_LOAD_ETC;


// IDA unit: 99_misc_global/stSPReturnInfo_t.
// The four fields are serialized in this order by
// CSkillTreeParam::SendSkillPointReturnInfo @0x84EEFD4.
struct stSPReturnInfo_t {
    // IDA_EA: 0x86093A8
    // IDA_NAME: _ZN16stSPReturnInfo_tC2Ettjj
    // DEMANGLED: stSPReturnInfo_t::stSPReturnInfo_t(
    //     unsigned short,unsigned short,unsigned int,unsigned int)
    stSPReturnInfo_t(uint16_t value_00,
                     uint16_t value_02,
                     uint32_t value_04,
                     uint32_t value_08);

    // IDA_EA: 0x8609A0E
    // IDA_NAME: _ZN16stSPReturnInfo_tD2Ev
    // DEMANGLED: stSPReturnInfo_t::~stSPReturnInfo_t()
    ~stSPReturnInfo_t();

    uint16_t value_00;  // +0x00
    uint16_t value_02;  // +0x02
    uint32_t value_04;  // +0x04
    uint32_t value_08;  // +0x08
};

static_assert(sizeof(stSPReturnInfo_t) == 0x0cu,
              "stSPReturnInfo_t must preserve its 12-byte wire ABI");


// IDA unit: 08_quest_skill_growth/CSkillTreeParam.
// This is a stack-scoped Inter_LoadEtc helper.  It snapshots the four skill
// point pools, lets the real skill tree reconciliation update them, then emits
// CMD122 only for trees that returned points and have matching itemized rows.
class CSkillTreeParam {
public:
    // IDA_EA: 0x84EEF48
    // IDA_NAME: _ZN15CSkillTreeParamC2EP5CUserP12SIG_LOAD_ETCR11PacketGuard
    // DEMANGLED: CSkillTreeParam::CSkillTreeParam(
    //     CUser *,SIG_LOAD_ETC *,PacketGuard &)
    CSkillTreeParam(CUser* user, SIG_LOAD_ETC* payload, PacketGuard& packet);

    // IDA_EA: 0x84EEFD4
    // IDA_NAME: _ZN15CSkillTreeParam24SendSkillPointReturnInfoEv
    // DEMANGLED: CSkillTreeParam::SendSkillPointReturnInfo(void)
    int SendSkillPointReturnInfo();

    // IDA_EA: 0x84EF4A6
    // IDA_NAME: _ZN15CSkillTreeParam12SetSkillTreeEP13Inter_LoadEtc
    // DEMANGLED: CSkillTreeParam::SetSkillTree(Inter_LoadEtc *)
    //
    int SetSkillTree(Inter_LoadEtc* dispatcher);

    // IDA_EA: 0x84EFC92
    // IDA_NAME: _ZN15CSkillTreeParamD2Ev
    // DEMANGLED: CSkillTreeParam::~CSkillTreeParam()
    ~CSkillTreeParam();

private:
    CUser* user_;                    // +0x00
    SIG_LOAD_ETC* payload_;          // +0x04
    PacketGuard* packet_;            // +0x08
    int first_sp_snapshot_;          // +0x0c
    int second_sp_snapshot_;         // +0x10
    int first_sfp_snapshot_;         // +0x14
    int second_sfp_snapshot_;        // +0x18
    std::vector<stSPReturnInfo_t> first_return_info_;   // +0x1c
    std::vector<stSPReturnInfo_t> second_return_info_;  // +0x28
};

#if defined(__i386__)
static_assert(sizeof(CSkillTreeParam) == 0x34u,
              "CSkillTreeParam must preserve its old i386 stack ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_TREE_PARAM_RUNTIME_H_
