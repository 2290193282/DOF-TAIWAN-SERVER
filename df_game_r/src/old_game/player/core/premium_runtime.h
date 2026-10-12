#ifndef OLDSRV_OLD_GAME_USERINFO_PREMIUM_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_PREMIUM_RUNTIME_H_

#include <stdint.h>
#include <time.h>

#include <map>
#include <utility>
#include <vector>

#include <common/manager/premium_public_abi.h>

#include "common/config.h"
#include "common/manager/play_info_public_abi.h"
#include "old_data/PVF/equipment/upgrade_table.h"

class CUser;
enum ENUM_PREMIUM_TYPE : int;

namespace WongWork {

// The original ABI names the state as a WongWork enum.  Its values are not
// needed by the leaf setter, so retain the recovered fixed-underlying type
// without fabricating an enum table.
enum ENUM_PREMIUM_STATE : int;

// IDA's premium table is a fixed 20-byte record.  Entries 1..105 are runtime
// premium types; entry zero remains the sentinel used by the original loops.
struct SUserPremiumInfo {
    int premium_type;
    int start_time;
    int end_time;
    int param;
    int state;
};

static_assert(sizeof(SUserPremiumInfo) == 0x14u,
              "SUserPremiumInfo IDA ABI size must stay 0x14");

// Complete IDA-observed CUserPremium layout through the return-item-rate map.
// The two _Additioal_info records use A's canonical 0x52-byte shared ABI type;
// no local byte substitute is introduced here.
class CUserPremium {
public:
    // IDA_EA: 0x8694B0E
    // IDA_NAME: _ZN8WongWork12CUserPremiumC2Ev
    // DEMANGLED: WongWork::CUserPremium::CUserPremium(void)
    CUserPremium();

    // IDA_EA: 0x8694B7A
    // IDA_NAME: _ZN8WongWork12CUserPremiumD2Ev
    // DEMANGLED: WongWork::CUserPremium::~CUserPremium()
    ~CUserPremium();

    // IDA_EA: 0x86AD9E8
    // IDA_NAME: _ZN8WongWork12CUserPremium11InitPremiumEv
    // DEMANGLED: WongWork::CUserPremium::InitPremium(void)
    int InitPremium();

    // B-owned PVF/DataManager loaders operate on this C-owned canonical class.
    static int LoadPremiumInfo(const char* filename);

    // IDA_EA: 0x86AF70A
    // IDA_NAME: _ZN8WongWork12CUserPremium18SetPremiumItemDataEmRK17STPremiumItemData
    // DEMANGLED: WongWork::CUserPremium::SetPremiumItemData(unsigned long,STPremiumItemData const&)
    static int SetPremiumItemData(unsigned long item_id,
                                  const STPremiumItemData& item_data);

    static int IsRestrictedPremium(int premium_type);

    // IDA_EA: 0x86ADAD2
    // IDA_NAME: _ZN8WongWork12CUserPremium10AddPremiumEiiii
    // DEMANGLED: WongWork::CUserPremium::AddPremium(int,int,int,int)
    int AddPremium(int premium_type,
                   int start_time,
                   int end_time,
                   int param);

    // IDA_EA: 0x86ADBA6
    // IDA_NAME: _ZN8WongWork12CUserPremium13RemovePremiumEi
    // DEMANGLED: WongWork::CUserPremium::RemovePremium(int)
    CUserPremium* RemovePremium(int premium_type);

    // IDA_EA: 0x86ADC6A
    // IDA_NAME: _ZN8WongWork12CUserPremium12startPremiumEib
    // DEMANGLED: WongWork::CUserPremium::startPremium(int,bool)
    int startPremium(int premium_type, bool force_start);

    // IDA_EA: 0x86AE10E
    // IDA_NAME: _ZN8WongWork12CUserPremium15ReCalcAdvantageEv
    // DEMANGLED: WongWork::CUserPremium::ReCalcAdvantage(void)
    bool ReCalcAdvantage();

    // IDA_EA: 0x86AE7A8
    // IDA_NAME: _ZNK8WongWork12CUserPremium17GetReturnItemRateEi
    // DEMANGLED: WongWork::CUserPremium::GetReturnItemRate(int)const
    int GetReturnItemRate(int item_key) const;

    // IDA_EA: 0x86AE834
    // IDA_NAME: _ZN8WongWork12CUserPremium11_CheckApplyEi2tm
    // DEMANGLED: WongWork::CUserPremium::_CheckApply(int,tm)
    int _CheckApply(int premium_type, tm local_time);

    // IDA_EA: 0x86AE8C6
    // IDA_NAME: _ZN8WongWork12CUserPremium20RecalcAdditionalInfoEPK5CUser
    // DEMANGLED: WongWork::CUserPremium::RecalcAdditionalInfo(CUser const*)
    CUserPremium* RecalcAdditionalInfo(const CUser* user);

    // IDA_EA: 0x863BD5C
    // IDA_NAME: _ZNK8WongWork12CUserPremium20GetTotalPremiumCountEv
    // DEMANGLED: WongWork::CUserPremium::GetTotalPremiumCount(void)const
    int GetTotalPremiumCount() const;

    // IDA_EA: 0x80E5FE6
    // IDA_NAME: _ZNK8WongWork12CUserPremium12CheckPremiumEi
    // DEMANGLED: WongWork::CUserPremium::CheckPremium(int)const
    int CheckPremium(int premium_type) const;

    // IDA_EA: 0x812CDA0
    // IDA_NAME: _ZNK8WongWork12CUserPremium18GetAdvPremiumCountEv
    // DEMANGLED: WongWork::CUserPremium::GetAdvPremiumCount(void)const
    int GetAdvPremiumCount() const;

    // IDA_EA: 0x822E4BE
    // IDA_NAME: _ZNK8WongWork12CUserPremium16GetAdvantageCoinEv
    // DEMANGLED: WongWork::CUserPremium::GetAdvantageCoin(void)const
    int GetAdvantageCoin() const;

    // IDA_EA: 0x814AA08
    // IDA_NAME: _ZNK8WongWork12CUserPremium19GetAdvantageExpRateEv
    // DEMANGLED: WongWork::CUserPremium::GetAdvantageExpRate(void)const
    int GetAdvantageExpRate() const;

    // IDA_EA: 0x85BF940
    // IDA_NAME: _ZNK8WongWork12CUserPremium14GetAddExpBonusEi
    // DEMANGLED: WongWork::CUserPremium::GetAddExpBonus(int)const
    int GetAddExpBonus(int premium_type) const;

    // IDA_EA: 0x830ED48
    // IDA_NAME: _ZN8WongWork12CUserPremium25GetAddIndependentDropRateEij
    // DEMANGLED: WongWork::CUserPremium::GetAddIndependentDropRate(
    //     int,unsigned int)
    int GetAddIndependentDropRate(int premium_type,
                                  unsigned int drop_slot);

    // IDA_EA: 0x84E99FC
    // IDA_NAME: _ZNK8WongWork12CUserPremium21GetOverEquipableLevelE18ENUM_EQUIPMENTTYPE
    // DEMANGLED: WongWork::CUserPremium::GetOverEquipableLevel(ENUM_EQUIPMENTTYPE)const
    int GetOverEquipableLevel(ENUM_EQUIPMENTTYPE equipment_type) const;

    // IDA_EA: 0x8694A28
    // IDA_NAME: _ZN8WongWork12CUserPremium23GetAddQuestItemDropRateEi
    // DEMANGLED: WongWork::CUserPremium::GetAddQuestItemDropRate(int)
    int GetAddQuestItemDropRate(int premium_type);

    // IDA_EA: 0x812CDAE
    // IDA_NAME: _ZNK8WongWork12CUserPremium23GetAdvantageFatigueRateEv
    // DEMANGLED: WongWork::CUserPremium::GetAdvantageFatigueRate(void)const
    int GetAdvantageFatigueRate() const;

    // IDA_EA: 0x8694A54
    // IDA_NAME: _ZNK8WongWork12CUserPremium24isAffectedUnlimitFatigueEv
    // DEMANGLED: WongWork::CUserPremium::isAffectedUnlimitFatigue(void)const
    int isAffectedUnlimitFatigue() const;

    // IDA_EA: 0x8694A64
    // IDA_NAME: _ZN8WongWork12CUserPremium12GetGoldBonusEi
    // DEMANGLED: WongWork::CUserPremium::GetGoldBonus(int)
    int GetGoldBonus(int charac_level);

    // IDA_EA: 0x8328100
    // IDA_NAME: _ZNK8WongWork12CUserPremium23getCeraShopDiscountRateEv
    // DEMANGLED: WongWork::CUserPremium::getCeraShopDiscountRate(void)const
    int getCeraShopDiscountRate() const;

    // IDA_EA: 0x86ADD9C
    // IDA_NAME: _ZNK8WongWork12CUserPremium19CheckPremiumTimeoutEv
    // DEMANGLED: WongWork::CUserPremium::CheckPremiumTimeout(void)const
    SUserPremiumInfo CheckPremiumTimeout() const;

    // IDA_EA: 0x86ADE9C
    // IDA_NAME: _ZNK8WongWork12CUserPremium18GetPremiumInfoListEi
    // DEMANGLED: WongWork::CUserPremium::GetPremiumInfoList(int)const
    std::vector<SUserPremiumInfo> GetPremiumInfoList(int premium_type) const;

    // IDA_EA: 0x86ADF52
    // IDA_NAME: _ZNK8WongWork12CUserPremium14GetPremiumInfoEi
    // DEMANGLED: WongWork::CUserPremium::GetPremiumInfo(int)const
    const SUserPremiumInfo* GetPremiumInfo(int premium_type) const;

    // IDA_EA: 0x86ADF7C
    // IDA_NAME: _ZN8WongWork12CUserPremium16GetAdvantageItemEi
    // DEMANGLED: WongWork::CUserPremium::GetAdvantageItem(int)
    std::vector<std::pair<int, int> > GetAdvantageItem(int premium_type);

    // IDA_EA: 0x8694A88
    // IDA_NAME: _ZNK8WongWork12CUserPremium17GetAdditionalInfoEv
    // DEMANGLED: WongWork::CUserPremium::GetAdditionalInfo(void)const
    _Additioal_info GetAdditionalInfo() const;

    // IDA_EA: 0x8609398
    // IDA_NAME: _ZNK8WongWork12CUserPremium17getOverSkillLevelEv
    // DEMANGLED: WongWork::CUserPremium::getOverSkillLevel(void)const
    int getOverSkillLevel() const;

    // IDA_EA: 0x86B086C
    // IDA_NAME: _ZN8WongWork12CUserPremium19IsNeedCheckEverydayEv
    // DEMANGLED: WongWork::CUserPremium::IsNeedCheckEveryday(void)
    int IsNeedCheckEveryday();

    // IDA_EA: 0x86B084C
    // IDA_NAME: _ZN8WongWork12CUserPremium15setPremiumStateE17ENUM_PREMIUM_TYPENS_18ENUM_PREMIUM_STATEE
    // DEMANGLED: WongWork::CUserPremium::setPremiumState(ENUM_PREMIUM_TYPE,WongWork::ENUM_PREMIUM_STATE)
    int setPremiumState(ENUM_PREMIUM_TYPE premium_type,
                        ENUM_PREMIUM_STATE state);

private:
    SUserPremiumInfo premium_info_[106];       // IDA +0x000..+0x847
    int advantage_premium_count_;              // IDA +0x848
    int normal_premium_count_;                 // IDA +0x84c
    uint16_t advantage_fatigue_rate_;          // IDA +0x850
    uint16_t padding_852_;                     // i386 alignment
    int advantage_coin_;                       // IDA +0x854
    int advantage_exp_rate_;                   // IDA +0x858
    bool unlimited_fatigue_;                   // IDA +0x85c
    uint8_t padding_85d_;                      // i386 alignment
    uint16_t over_skill_level_;                // IDA +0x85e
    uint16_t cera_shop_discount_rate_;         // IDA +0x860
    bool need_check_everyday_;                 // IDA +0x862
    uint8_t padding_863_;                      // i386 alignment
    int over_equipable_level_[
        oldsrv::config::kCharacterEquipmentTypeCount]; // target +0x864..+0x8bf
    bool gold_bonus_;                          // target +0x8c0
    _Additioal_info advantage_additional_info_;  // target +0x8c1
    _Additioal_info calculated_additional_info_; // target +0x913
    uint8_t padding_965_[3];                   // map alignment
    std::map<int, int> return_item_rates_;     // target +0x968
};

class CHandlePremium {
public:
    // IDA_EA: 0x86AF7E8
    // IDA_NAME: _ZN8WongWork14CHandlePremium20handleSetUserPremiumEP5CUser17ENUM_PREMIUM_TYPEllib
    // DEMANGLED: WongWork::CHandlePremium::handleSetUserPremium(CUser *,ENUM_PREMIUM_TYPE,long,long,int,bool)
    static CUserPremium* handleSetUserPremium(CUser* user,
                                               ENUM_PREMIUM_TYPE premium_type,
                                               long start_time,
                                               long end_time,
                                               int param,
                                               bool recalculate);

    // IDA_EA: 0x86AF8B4
    // IDA_NAME: _ZN8WongWork14CHandlePremium23handleNotifyPremiumInfoEP5CUserii
    // DEMANGLED: WongWork::CHandlePremium::handleNotifyPremiumInfo(CUser *,int,int)
    static void handleNotifyPremiumInfo(CUser* user,
                                        int premium_type,
                                        int value);

    // IDA_EA: 0x86AFA7C
    // IDA_NAME: _ZN8WongWork14CHandlePremium20handlePremiumTimeoutEP5CUser
    // DEMANGLED: WongWork::CHandlePremium::handlePremiumTimeout(CUser *)
    static void handlePremiumTimeout(CUser* user);

    // IDA_EA: 0x86AFC32
    // IDA_NAME: _ZN8WongWork14CHandlePremium19handlePremiumEffectEP5CUser17ENUM_PREMIUM_TYPE
    // DEMANGLED: WongWork::CHandlePremium::handlePremiumEffect(CUser *,ENUM_PREMIUM_TYPE)
    static char handlePremiumEffect(CUser* user,
                                    ENUM_PREMIUM_TYPE premium_type);

    // IDA_EA: 0x86B02FE
    // IDA_NAME: _ZN8WongWork14CHandlePremium19handleSetUserPCRoomEP5CUserll
    // DEMANGLED: WongWork::CHandlePremium::handleSetUserPCRoom(CUser *,long,long)
    static void handleSetUserPCRoom(CUser* user,
                                    long start_time,
                                    long end_time);

    // IDA_EA: 0x86AF96E
    // IDA_NAME: _ZN8WongWork14CHandlePremium23handleReturn2CharacViewEP5CUser
    // DEMANGLED: WongWork::CHandlePremium::handleReturn2CharacView(CUser *)
    static void handleReturn2CharacView(CUser* user);

    // IDA_EA: 0x86B0594
    // IDA_NAME: _ZN8WongWork14CHandlePremium30handleRemoveOneDayLethePremiumEP5CUser
    // DEMANGLED: WongWork::CHandlePremium::handleRemoveOneDayLethePremium(CUser *)
    static int handleRemoveOneDayLethePremium(CUser* user);
};

static_assert(sizeof(CUserPremium) == 0x980u,
              "CUserPremium target ABI size must stay 0x980");

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_USERINFO_PREMIUM_RUNTIME_H_
