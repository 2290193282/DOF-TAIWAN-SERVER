#ifndef OLDSRV_OLD_GAME_USERINFO_CBOOSTER_GAGE_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_CBOOSTER_GAGE_RUNTIME_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/charac_expand_cdata.h"


class CUser;

typedef std::vector<std::pair<int, int> > BoosterInsertedSlotList;
typedef std::vector<std::pair<Inven_Item, bool> > BoosterRewardList;

// IDA unit: 99_misc_global/CBoosterGage.
// The old charac-expand slot is 17 and the complete object is 0x0c bytes.
class CBoosterGage : public charac_expand::CData {
public:
    // IDA_EA: 0x832AF06
    // IDA_NAME: _ZN12CBoosterGageC2Ev
    // DEMANGLED: CBoosterGage::CBoosterGage(void)
    CBoosterGage();

    // IDA_EA: 0x80DE1DC / 0x80DE20C
    // IDA_NAME: _ZN12CBoosterGageD2Ev / _ZN12CBoosterGageD0Ev
    // DEMANGLED: CBoosterGage::~CBoosterGage()
    virtual ~CBoosterGage();

    // IDA_EA: 0x80DC640
    // IDA_NAME: _ZN12CBoosterGage14inc_total_gageEi
    // DEMANGLED: CBoosterGage::inc_total_gage(int)
    void inc_total_gage(int point);

    // IDA_EA: 0x80DC664
    // IDA_NAME: _ZN12CBoosterGage14sub_total_gageEi
    // DEMANGLED: CBoosterGage::sub_total_gage(int)
    void sub_total_gage(int point);

    // IDA_EA: 0x80DC69C
    // IDA_NAME: _ZN12CBoosterGage17get_booster_pointEm
    // DEMANGLED: CBoosterGage::get_booster_point(unsigned long)
    int get_booster_point(unsigned long item_index);

    // IDA_EA: 0x80DC6D6
    // IDA_NAME: _ZN12CBoosterGage11make_postarEP5CUserRK10Inven_ItemPKci
    // DEMANGLED: CBoosterGage::make_postar(CUser *,Inven_Item const&,char const*,int)
    void make_postar(CUser* user,
                     const Inven_Item& item,
                     const char* agency_no,
                     int duration_days);

    // IDA_EA: 0x80DC9CA
    // IDA_NAME: _ZN12CBoosterGage29log_booster_gage_bonus_rewardEP5CUserRKSt6vectorISt4pairI10Inven_ItembESaIS5_EE
    // DEMANGLED: CBoosterGage::log_booster_gage_bonus_reward(CUser *,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> const&)
    void log_booster_gage_bonus_reward(CUser* user,
                                       const BoosterRewardList& rewards);

    // IDA_EA: 0x80DCBD6
    // IDA_NAME: _ZN12CBoosterGage13AddAvatarItemERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_ItemibccPKc20eAvatarItemAddReasonRi
    // DEMANGLED: CBoosterGage::AddAvatarItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,int,bool,char,char,char const*,eAvatarItemAddReason,int &)
    int AddAvatarItem(BoosterInsertedSlotList& inserted_slots,
                      bool booster_bonus,
                      CUser* user,
                      const Inven_Item& item,
                      int usable_period,
                      bool period_is_seconds,
                      char first_flag,
                      char avatar_select,
                      const char* agency_no,
                      eAvatarItemAddReason reason,
                      int& add_info);

    // IDA_EA: 0x80DCD8C
    // IDA_NAME: _ZN12CBoosterGage18InsertCreatureItemERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_Item14eItemAddReasonbbiiPKc
    // DEMANGLED: CBoosterGage::InsertCreatureItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,eItemAddReason,bool,bool,int,int,char const*)
    int InsertCreatureItem(BoosterInsertedSlotList& inserted_slots,
                           bool booster_bonus,
                           CUser* user,
                           const Inven_Item& item,
                           eItemAddReason reason,
                           bool log_history,
                           bool creature_flag,
                           int usable_period,
                           int expiration_date,
                           const char* agency_no);

    // IDA_EA: 0x80DCF56
    // IDA_NAME: _ZN12CBoosterGage23insertItemIntoInventoryERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_Item14eItemAddReasonb
    // DEMANGLED: CBoosterGage::insertItemIntoInventory(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,eItemAddReason,bool)
    int insertItemIntoInventory(BoosterInsertedSlotList& inserted_slots,
                                bool booster_bonus,
                                CUser* user,
                                const Inven_Item& item,
                                eItemAddReason reason,
                                bool log_history);

    // IDA_EA: 0x80DD0DA
    // IDA_NAME: _ZN12CBoosterGage22check_max_booster_gageEm
    // DEMANGLED: CBoosterGage::check_max_booster_gage(unsigned long)
    bool check_max_booster_gage(unsigned long item_index);

    // IDA_EA: 0x80DD134
    // IDA_NAME: _ZN12CBoosterGage16cal_booster_gageEmP5CUser
    // DEMANGLED: CBoosterGage::cal_booster_gage(unsigned long,CUser *)
    int cal_booster_gage(unsigned long item_index, CUser* user);

    // IDA_EA: 0x80DD142
    // IDA_NAME: _ZN12CBoosterGage23send_error_state_packetEP5CUserm
    // DEMANGLED: CBoosterGage::send_error_state_packet(CUser *,unsigned long)
    void send_error_state_packet(CUser* user, unsigned long item_index);

    // IDA_EA: 0x80DD246
    // IDA_NAME: _ZN12CBoosterGage9send_dataEP5CUseri
    // DEMANGLED: CBoosterGage::send_data(CUser *,int)
    void send_data(CUser* user, int state);

    // IDA_EA: 0x80DD24E
    // IDA_NAME: _ZNK12CBoosterGage7getDataEPc
    // DEMANGLED: CBoosterGage::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x80DD284
    // IDA_NAME: _ZN12CBoosterGage8loadDataEP5CUserPc
    // DEMANGLED: CBoosterGage::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x80DD322
    // IDA_NAME: _ZN12CBoosterGage10ResetDailyEv
    // DEMANGLED: CBoosterGage::ResetDaily(void)
    virtual void ResetDaily();

protected:
    // IDA_EA: 0x80DD340
    // IDA_NAME: _ZN12CBoosterGage9_saveDataEP5CUser
    // DEMANGLED: CBoosterGage::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x80DD468
    // IDA_NAME: _ZN12CBoosterGage6_resetEv
    // DEMANGLED: CBoosterGage::_reset(void)
    virtual int _reset();

private:
    int32_t total_gage_;  // IDA +0x08
};

static_assert(sizeof(CBoosterGage) == 0x0cu,
              "CBoosterGage IDA ABI size must stay 0x0c");


#endif  // OLDSRV_OLD_GAME_USERINFO_CBOOSTER_GAGE_RUNTIME_H_
