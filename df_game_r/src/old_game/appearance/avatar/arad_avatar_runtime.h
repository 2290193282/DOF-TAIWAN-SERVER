#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_RUNTIME_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_RUNTIME_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "old_data/PVF/common/arad_script_data.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/equipment/upgrade_table.h"
#include "old_game/appearance/avatar/arad_avatar_packet_signals.h"

class CUser;
class CItem;
struct Inven_Item;

namespace arad {
struct SigAradAvatarEnduranceSave;
}

// These four singleton-backed services own the actual CMD 588..591 business
// transitions.  The old constructors place ARAD::ScriptInterface at +0 and
// their parsed PVF script object at +4; ARAD::Singleton<T> is an empty base.
// Keep the concrete fields here so every C-owned ingress/return path shares
// the old i386 layout instead of duplicating a partial facade.
class AvatarFixedHiddenOptionServer
    : public ARAD::Singleton<AvatarFixedHiddenOptionServer>,
      public ARAD::ScriptInterface {
public:
    // IDA_EA: 0x817E02C / 0x817E0E6
    AvatarFixedHiddenOptionServer();
    ~AvatarFixedHiddenOptionServer();

    // IDA_EA: 0x817E154 / 0x817E1E2
    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x817E1EC, 0x817E23E, 0x817E334, 0x817E392, 0x817E3E8
    int GetSkillIndex(ENUM_RARITY rarity);
    int GetSkillIndex(ENUM_RARITY rarity, int input);
    int GetRandomInput(ENUM_RARITY rarity);
    bool IsVaildSkillIndex(ENUM_RARITY rarity, int skill_index);
    int Init();

    // IDA_EA: 0x817ECD8.  This is a cdecl static diagnostic entry: the
    // pointer supplied by its only ABI argument is the service to test.
    static int UnitTest(AvatarFixedHiddenOptionServer* server);

    // IDA_EA: 0x817E4C6 / 0x817E8D6
    bool CheckCondition(CUser* user,
                        arad::SigAvatarHiddenOptionChange* signal);
    bool ChangeHiddenOption(CUser* user,
                            arad::SigAvatarHiddenOptionChange* signal);

private:
    ARAD::SCRIPT::AvatarFixedHiddenOption script_;  // +0x04
    std::vector<int> skill_indices_[6];             // +0x50
};

class AvatarRechargeServer : public ARAD::Singleton<AvatarRechargeServer>,
                             public ARAD::ScriptInterface {
public:
    // IDA_EA: 0x8190200 / 0x8190246
    AvatarRechargeServer();
    ~AvatarRechargeServer();

    // IDA_EA: 0x8190266
    // IDA_NAME: _ZN20AvatarRechargeServer4TestEv
    // DEMANGLED: AvatarRechargeServer::Test(void)
    void Test();

    // IDA_EA: 0x819026C / 0x81902DE
    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x81906E2 / 0x8190A70
    bool CheckConditionUseRecharge(CUser* user,
                                   arad::SigAradUseAvatarRecharge* signal);
    bool Recharge(CUser* user, arad::SigAradUseAvatarRecharge* signal);
    bool CalcDurability(CUser* user, int fatigue);
    int CalcDurabilityDetail(CUser* user,
                             int slot,
                             std::vector<int>& changed_slots);
    // IDA_EA: 0x8190416
    // IDA_NAME: _ZN20AvatarRechargeServer14SendDurabilityEP5CUsercsci
    // DEMANGLED: AvatarRechargeServer::SendDurability(CUser *,char,short,char,int)
    char SendDurability(CUser* user,
                        char source_space,
                        short source_slot,
                        char destination_space,
                        int destination_slot);
    // IDA_EA: 0x81905B4
    // IDA_NAME: _ZN20AvatarRechargeServer25CalcDurabilityChangeEquipEP5CUseri
    // DEMANGLED: AvatarRechargeServer::CalcDurabilityChangeEquip(CUser *,int)
    void CalcDurabilityChangeEquip(CUser* user, int equipment_slot);
    // IDA callers pass only CUser::eSendTarget, CUser*, and slot: this is a
    // static utility, not an instance member.  Keeping an integer surface
    // avoids forcing the complete CUser definition into this shared service
    // header; the asm label locks the exact old nested-enum ABI.
    static void UpdateAvatarEndurance(int target, CUser* user, short slot);

    // IDA_EA: 0x8190DEA
    // IDA_NAME: _ZN20AvatarRechargeServer23SaveAvatarEnduranceUnitERN4arad26SigAradAvatarEnduranceSaveE
    // DEMANGLED: AvatarRechargeServer::SaveAvatarEnduranceUnit(arad::SigAradAvatarEnduranceSave &)
    static void SaveAvatarEnduranceUnit(
        arad::SigAradAvatarEnduranceSave& save);

    // IDA_EA: 0x8190EAE.  The original overload is static: its two integer
    // inputs become the queued persistence record's avatar key/endurance.
    static void SaveAvatarEnduranceUnit(int avatar_key, int endurance);

    // IDA_EA: 0x8190ED8
    // IDA_NAME: _ZN20AvatarRechargeServer9UseEmblemEP5CUserRSt6vectorIiSaIiEEP10Inven_Items
    // DEMANGLED: AvatarRechargeServer::UseEmblem(CUser *,std::vector<int,std::allocator<int>> &,Inven_Item *,short)
    bool UseEmblem(CUser* user,
                   std::vector<int>& emblem_grades,
                   Inven_Item* emblem_item,
                   short avatar_slot);

    // IDA_EA: 0x81911B6.  The original caller passes CUser directly; this
    // utility has no AvatarRechargeServer instance parameter.
    static int GmReduceEndurance(CUser* user,
                                 short avatar_slot,
                                 short endurance_delta);

    // IDA_EA: 0x8191318.  This is likewise a static GM gate that forwards to
    // the singleton-backed instance setter below.
    static int GmChangeEnduranceReduceRate(CUser* user,
                                           bool enabled,
                                           int percentage);

    // IDA_EA: 0x8191364
    // IDA_NAME: _ZN20AvatarRechargeServer24GmSetEnduranceReduceRateEP5CUserbi
    // DEMANGLED: AvatarRechargeServer::GmSetEnduranceReduceRate(CUser *,bool,int)
    int GmSetEnduranceReduceRate(CUser* user,
                                 bool enabled,
                                 int percentage);

    // IDA_EA: 0x819114C
    // IDA_NAME: _ZN20AvatarRechargeServer22GetRateFromEmblemGradeEi
    // DEMANGLED: AvatarRechargeServer::GetRateFromEmblemGrade(int)
    int GetRateFromEmblemGrade(int grade);

private:
    // AvatarConvertServer::Convert @0x81926DE calls the proven
    // AvatarRecharge script subobject at +0x04 directly to classify its
    // selected result.  Friendship retains that original call without
    // creating a new forwarding ABI or exposing a guessed offset.
    friend class AvatarConvertServer;
    ARAD::SCRIPT::AvatarRecharge script_;  // +0x04
    int value_54_;                          // +0x54, zeroed by C2
    bool value_58_;                         // +0x58, zeroed by C2
    uint8_t padding_59_[3];
};

class EmblemCompoundServer : public ARAD::Singleton<EmblemCompoundServer>,
                             public ARAD::ScriptInterface {
public:
    // IDA_EA: 0x819155E / 0x8191594
    EmblemCompoundServer();
    ~EmblemCompoundServer();

    // IDA_EA: 0x81915BA / 0x819162C
    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x8191636 / 0x8191AFA
    bool CheckCondition(CUser* user, arad::SigAradEmblemCompound* signal);
    // IDA_EA: 0x8191818
    // IDA_NAME: _ZN20EmblemCompoundServer13GetEmblemInfoEP5CUserPN4arad21SigAradEmblemCompoundEi
    bool GetEmblemInfo(CUser* user,
                       arad::SigAradEmblemCompound* signal,
                       int material_index);
    // IDA_EA: 0x81919E8
    // IDA_NAME: _ZN20EmblemCompoundServer20GetEmblemRandomIndexEit
    // DEMANGLED: EmblemCompoundServer::GetEmblemRandomIndex(int,unsigned short)
    int GetEmblemRandomIndex(int grade, uint16_t select_no);
    // IDA_EA: 0x8191AD6
    // IDA_NAME: _ZN20EmblemCompoundServer18IsResultSelectableEi
    // DEMANGLED: EmblemCompoundServer::IsResultSelectable(int)
    int IsResultSelectable(int grade);
    bool Process(CUser* user, arad::SigAradEmblemCompound* signal);

private:
    ARAD::SCRIPT::EmblemCompound script_;  // +0x04
};

class AvatarConvertServer : public ARAD::Singleton<AvatarConvertServer>,
                            public ARAD::ScriptInterface {
public:
    // IDA_EA: 0x81921B6 / 0x81921EC
    AvatarConvertServer();
    ~AvatarConvertServer();

    // IDA_EA: 0x819220C
    // IDA_NAME: _ZN19AvatarConvertServer4TestEv
    // DEMANGLED: AvatarConvertServer::Test(void)
    void Test();

    // IDA_EA: 0x81930BA / 0x819312C
    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x8192212 / 0x81926DE
    bool CheckCondition(CUser* user, arad::SigAradAvatarConvert* signal);
    bool Convert(CUser* user, arad::SigAradAvatarConvert* signal);
    // IDA_EA: 0x8193136
    // IDA_NAME: _ZN19AvatarConvertServer24StatisticsAvatarDisjointEPK5CItemi
    // DEMANGLED: AvatarConvertServer::StatisticsAvatarDisjoint(CItem const*,int)
    static int StatisticsAvatarDisjoint(
        const CItem* source_item, int is_emblem_recharge_stone);
    // IDA_EA: 0x81931DC
    // IDA_NAME: _ZN19AvatarConvertServer22StatisticsEmblemCreateERKSt6vectorISt4pairIiiESaIS2_EE
    static char StatisticsEmblemCreate(
        const std::vector<std::pair<int, int> >& items);

private:
    ARAD::SCRIPT::AvatarConvert script_;  // +0x04
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(AvatarFixedHiddenOptionServer) == 0x98u,
              "AvatarFixedHiddenOptionServer IDA ABI must stay 0x98");
static_assert(sizeof(AvatarRechargeServer) == 0x5cu,
              "AvatarRechargeServer IDA ABI must stay 0x5c");
static_assert(sizeof(EmblemCompoundServer) == 0x88u,
              "EmblemCompoundServer IDA ABI must stay 0x88");
static_assert(sizeof(AvatarConvertServer) == 0x1cu,
              "AvatarConvertServer IDA ABI must stay 0x1c");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_RUNTIME_H_
