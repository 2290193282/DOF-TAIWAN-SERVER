#ifndef OLD_GAME_EVENT_EVENT_RUNTIME_H_
#define OLD_GAME_EVENT_EVENT_RUNTIME_H_

#include <bitset>
#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_data/PVF/event/object_bring_up_rule.h"
#include "old_game/economy/mall/purchase_cash_item_bonus_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"

class CUser;
class CItem;
class PacketGuard;
struct Inven_Item;
struct stCeraShopIPGRequest_t;

// IDA unit: 11_event_runtime/InGameEventScriptLoader.
// This is the actual 0x18-byte script-map subobject embedded at +0x18 in
// EventClassify::CEventScriptMng.  It remains a real class rather than a
// forged CEventScriptMng member family.
class InGameEventScriptLoader {
public:
    // IDA_EA: 0x8110F4C
    // IDA_NAME: _ZN23InGameEventScriptLoaderC2Ev
    // DEMANGLED: InGameEventScriptLoader::InGameEventScriptLoader(void)
    InGameEventScriptLoader();

    // IDA_EA: 0x8110F60
    // IDA_NAME: _ZN23InGameEventScriptLoaderD2Ev
    // DEMANGLED: InGameEventScriptLoader::~InGameEventScriptLoader()
    ~InGameEventScriptLoader();

    // IDA_EA: 0x8982A4A
    // IDA_NAME: _ZN23InGameEventScriptLoader23importInGameEventScriptEiRKSs
    // DEMANGLED: InGameEventScriptLoader::importInGameEventScript(int,std::string const&)
    InGameEventScript* importInGameEventScript(int event_id,
                                               const std::string& filename);

    // IDA_EA: 0x8982F10
    // IDA_NAME: _ZN23InGameEventScriptLoader17insertEventScriptERKSsP17InGameEventScript
    // DEMANGLED: InGameEventScriptLoader::insertEventScript(std::string const&,InGameEventScript *)
    InGameEventScript* insertEventScript(const std::string& filename,
                                         InGameEventScript* script);

    // IDA_EA: 0x8982F38
    // IDA_NAME: _ZN23InGameEventScriptLoader14getEventScriptEiRKSs
    // DEMANGLED: InGameEventScriptLoader::getEventScript(int,std::string const&)
    InGameEventScript* getEventScript(int event_id,
                                      const std::string& filename);

    typedef std::map<std::string, InGameEventScript*> ScriptMap;
    ScriptMap scripts_;
};

typedef char InGameEventScriptLoader_Size_Must_Be_0x18[
    (sizeof(InGameEventScriptLoader) == 0x18) ? 1 : -1];

namespace EventClassify {

class CEventEntity {
public:
    CEventEntity();
    virtual ~CEventEntity();

    virtual int is_eventing();
    virtual CEventEntity* set_event_flag(bool enabled);
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual int get_event_record_type();
    virtual CEventEntity* set_event_record_type(int record_type);
    virtual int send_init_data(CUser* user,
                               PacketGuard& guard,
                               short event_id,
                               unsigned char today_day,
                               bool reset);
    virtual const std::string& get_db_table_name();

    CEventEntity* set_event_start_time(int start_time);
    CEventEntity* set_event_end_time(int end_time);
    int get_event_start_time();
    int get_event_end_time();

protected:
    unsigned char event_flag_;
    unsigned char reserved_05_[3];
    std::string db_table_name_;
    int event_start_time_;
    int event_end_time_;
    InGameEventScript* script_;
    int event_record_type_;
};

class CScriptBackedEventEntity : public CEventEntity {
public:
    CScriptBackedEventEntity();
    virtual ~CScriptBackedEventEntity();

    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);

protected:
    InGameEventScript* typed_script_;
};

class CPcRoomPlayTime : public CScriptBackedEventEntity {
public:
    CPcRoomPlayTime();
    virtual ~CPcRoomPlayTime();

    // IDA_EA: 0x8110814
    // IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime11is_eventingEv
    // DEMANGLED: EventClassify::CPcRoomPlayTime::is_eventing(void)
    virtual int is_eventing();

    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();

    // IDA_EA: 0x8110A38
    // IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime25verifyUserPcRoomPlayEventEP5CUser
    // DEMANGLED: EventClassify::CPcRoomPlayTime::verifyUserPcRoomPlayEvent(CUser *)
    int verifyUserPcRoomPlayEvent(CUser* user);

    // IDA_EA: 0x811089C
    // IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime26processEventPcRoomPlayTimeEP5CUser
    // DEMANGLED: EventClassify::CPcRoomPlayTime::processEventPcRoomPlayTime(CUser *)
    int processEventPcRoomPlayTime(CUser* user);
};

// IDA C2 @0x8110F88 calls CEventEntity::C2 directly and then installs this
// class's vtable.  Its only derived storage is the event-script pointer at
// +0x1c; it is not one of the CScriptBackedEventEntity families.
class CAccountFirstLogin : public CEventEntity {
public:
    CAccountFirstLogin();
    virtual ~CAccountFirstLogin();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();

    // IDA_EA: 0x810DECC
    // IDA_NAME: _ZN13EventClassify18CAccountFirstLogin13get_mail_dataEv
    // DEMANGLED: EventClassify::CAccountFirstLogin::get_mail_data(void)
    Action_SendMail* get_mail_data();

    // IDA_EA: 0x810DF04
    // IDA_NAME: _ZN13EventClassify18CAccountFirstLogin9do_actionEiP5CUserSt6bitsetILj32EE
    // DEMANGLED: EventClassify::CAccountFirstLogin::do_action(int,CUser *,std::bitset<32u>)
    virtual void do_action(int event_type,
                           CUser* user,
                           std::bitset<32> user_confirm_flags);

private:
    InGameEventScript* typed_script_;  // +0x1c; C2 leaves it uninitialized.
};

class CAccountFirstLoginDaily : public CAccountFirstLogin {
public:
    CAccountFirstLoginDaily();
    virtual ~CAccountFirstLoginDaily();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);

    // IDA_EA: 0x810DFE8
    // IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDaily9do_actionEiP5CUserSt6bitsetILj32EE
    // DEMANGLED: EventClassify::CAccountFirstLoginDaily::do_action(int,CUser *,std::bitset<32u>)
    virtual void do_action(int event_type,
                           CUser* user,
                           std::bitset<32> user_confirm_flags);
};

class CAttendance : public CEventEntity {
public:
    CAttendance();
    virtual ~CAttendance();
    virtual int is_eventing();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual int send_init_data(CUser* user,
                               PacketGuard& guard,
                               short event_id,
                               unsigned char today_day,
                               bool reset);
    virtual const std::string& get_db_table_name();
    Action_SendMail* get_mail_data();
    int get_fatigue_condition();
    int get_final_day_condition();
    int send_attend_reward_item(CUser* user, int pass_day);
    int send_attend_final_rward_item(CUser* user,
                                     int pass_day,
                                     int attend_count);

private:
    // Raw CAttendance storage at offset 0x1c; its constructor leaves it unset.
    InGameEventScript* typed_script_;
};

class CAddItemCountDB : public CEventEntity {
public:
    CAddItemCountDB();
    virtual ~CAddItemCountDB();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();

    // IDA_EA: 0x810E08E
    // IDA_NAME: _ZN13EventClassify15CAddItemCountDB13is_event_itemEi
    // DEMANGLED: EventClassify::CAddItemCountDB::is_event_item(int)
    int is_event_item(int item_id);

    // IDA_EA: 0x810E0F4
    // IDA_NAME: _ZN13EventClassify15CAddItemCountDB14is_event_questEi
    // DEMANGLED: EventClassify::CAddItemCountDB::is_event_quest(int)
    int is_event_quest(int quest_id);

private:
    // Raw CAddItemCountDB storage at offset 0x1c; its constructor leaves it unset.
    InGameEventScript* typed_script_;
};

class CLevelUpReward : public CEventEntity {
public:
    CLevelUpReward();
    virtual ~CLevelUpReward();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();
    int check_recv_level(int charac_level,
                         int charac_job,
                         int first_grow_type,
                         bool include_lower_levels);
    Action_SendMail* get_mail_data();
    int is_recv_check_item();
    int set_reward_item(int charac_level,
                        int charac_job,
                        int first_grow_type);
    int do_action(CUser* user);
    int send_reward_from_recv_level(CUser* user,
                                    int current_level,
                                    int received_level);

private:
    // Raw CLevelUpReward storage at offset 0x1c; its constructor leaves it unset.
    InGameEventScript* typed_script_;
};

class CExchangeRandomItemReward : public CScriptBackedEventEntity {
public:
    CExchangeRandomItemReward();
    virtual ~CExchangeRandomItemReward();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);

    int verifyMaterialItemByRule(
        CUser* user,
        const std::vector<int>& item_slots,
        std::vector<std::pair<int, int> >& used_material_items);
    int getRewardItem(std::vector<std::pair<int, int> >& reward_items);
    int deleteUsedMaterialItem(
        CUser* user,
        const std::vector<std::pair<int, int> >& used_material_slots,
        std::vector<std::pair<int, int> >& used_material_items);
    int sendRandomItemReward(CUser* user,
                             const std::vector<std::pair<int, int> >& rewards,
                             std::vector<int>& mail_result_items,
                             uint8_t& reward_count);
};

class CAvatarDisjointRandomReward : public CScriptBackedEventEntity {
public:
    CAvatarDisjointRandomReward();
    virtual ~CAvatarDisjointRandomReward();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);

    int checkAvatarDisjointError(CUser* user,
                                 uint16_t avatar_slot,
                                 const Inven_Item& inventory_item,
                                 const CItem* item_proto);
    int deleteAvatarItem(CUser* user,
                         uint16_t avatar_slot,
                         const Inven_Item& inventory_item,
                         const CItem* item_proto);
    int SendDisjointAvatarStatistic(const CItem* item_proto);
    int getRewardItem(const CItem* item_proto,
                      std::vector<std::pair<int, int> >& reward_items);
    int SendCreateEmblemStatistic(
        const std::vector<std::pair<int, int> >& reward_items);
    int sendRandomItemReward(CUser* user,
                             const std::vector<std::pair<int, int> >& rewards);
};

class CFatigueQuantity : public CEventEntity {
public:
    CFatigueQuantity();
    virtual ~CFatigueQuantity();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();
    int get_user_type() const;

private:
    // Raw CFatigueQuantity storage at offset 0x1c; its constructor leaves it unset.
    InGameEventScript* typed_script_;
};

class CPurchaseCashItemBonus : public CScriptBackedEventEntity {
public:
    CPurchaseCashItemBonus();
    virtual ~CPurchaseCashItemBonus();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();
    bool verifyGiveBonusReward(
        PurchaseCashItemBonusScript::PurchaseBonusRule& rule,
        int purchase_count);
    int updateTargetGoodsCount(CUser* user,
                               unsigned int charac_no,
                               stCeraShopIPGRequest_t* ipg_request);
    int sendPurchaseBonusReward(CUser* user, int purchase_count);
};

class CObjectBringUp : public CScriptBackedEventEntity {
public:
    CObjectBringUp();
    virtual ~CObjectBringUp();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();
    bool verifyMaterialItemByRule(CUser* user, unsigned int item_id);
    bool verifyGiveBringUpReward(
        ObjectBringUpScript::ObjectBringUpRule& rule,
        int material_total);
    int sendBringUpReward(CUser* user,
                          std::vector<int>& reward_values,
                          unsigned char& reward_count);
    int sendEventItemFromFatigue(CUser* user, unsigned int amount);
};

class CUsedFatigueGiveItem : public CScriptBackedEventEntity {
public:
    CUsedFatigueGiveItem();
    virtual ~CUsedFatigueGiveItem();
    virtual int is_eventing();
    virtual CEventEntity* set_event_data(int record_type,
                                         InGameEventScript* script);
    virtual const std::string& get_db_table_name();
    int getIsCharacEvent(bool& is_charac_event) const;
    int get_user_type() const;
    bool verifyApplyUser(CUser* user);
    bool sendEventItemFromFatigue(CUser* user, unsigned int amount);
};

class CEventActionMng {
public:
    // IDA_EA: 0x810DC5A
    // IDA_NAME: _ZN13EventClassify15CEventActionMng24getResultItemFromLotteryEjRSt6vectorISt4pairIiiESaIS3_EE
    // DEMANGLED: EventClassify::CEventActionMng::getResultItemFromLottery(unsigned int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
    int getResultItemFromLottery(
        unsigned int item_id,
        std::vector<std::pair<int, int> >& reward_items);

    // IDA_EA: 0x810CF32
    // IDA_NAME: _ZN13EventClassify15CEventActionMng24process_action_send_mailEP5CUserR15Action_SendMailb
    // DEMANGLED: EventClassify::CEventActionMng::process_action_send_mail(CUser *,Action_SendMail &,bool)
    int process_action_send_mail(CUser* user, Action_SendMail& mail, bool notify);

    // IDA_EA: 0x810D4AE
    // IDA_NAME: _ZN13EventClassify15CEventActionMng17process_coin_sendEP5CUserR15Action_SendMailR10Inven_Item
    // DEMANGLED: EventClassify::CEventActionMng::process_coin_send(CUser *,Action_SendMail &,Inven_Item &)
    int process_coin_send(CUser* user, Action_SendMail& mail, Inven_Item& item);

    // IDA_EA: 0x810D70C
    // IDA_NAME: _ZN13EventClassify15CEventActionMng19process_avatar_sendEP5CUserR10Inven_ItemPK5CItem
    // DEMANGLED: EventClassify::CEventActionMng::process_avatar_send(CUser *,Inven_Item &,CItem const*)
    int process_avatar_send(CUser* user,
                            Inven_Item& item,
                            const CItem* item_proto);

    // IDA_EA: 0x810D83C
    // IDA_NAME: _ZN13EventClassify15CEventActionMng22process_gold_mail_sendEP5CUserR15Action_SendMail
    // DEMANGLED: EventClassify::CEventActionMng::process_gold_mail_send(CUser *,Action_SendMail &)
    int process_gold_mail_send(CUser* user, Action_SendMail& mail);

    // IDA_EA: 0x810DA1C
    // IDA_NAME: _ZN13EventClassify15CEventActionMng17process_mail_sendEP5CUserR15Action_SendMailR10Inven_ItemPK5CItem
    // DEMANGLED: EventClassify::CEventActionMng::process_mail_send(CUser *,Action_SendMail &,Inven_Item &,CItem const*)
    int process_mail_send(CUser* user,
                          Action_SendMail& mail,
                          Inven_Item& item,
                          const CItem* item_proto);
};

class CEventScriptMng {
public:
    CEventScriptMng();
    ~CEventScriptMng();

    int get_convert_to_user_confirmflag(int event_type);
    const std::string* get_db_table_name(int event_type);
    CEventEntity* create_event_entity(int event_id, bool base_entity);
    CEventEntity* get_event_entity(int event_type);
    bool is_event_entity(int event_type) const;
    int is_eventing(int event_type);
    int initScriptlist(InGameEventInfoList& info_list);
    InGameEventInfoList& get_info_list() { return info_list_; }
    const InGameEventInfoList& get_info_list() const { return info_list_; }
    bool init_evnet_entity();
    // IDA_EA: 0x810BDE0
    // IDA_NAME: _ZN13EventClassify15CEventScriptMng27process_account_first_loginEiP5CUserSt6bitsetILj32EE
    // DEMANGLED: EventClassify::CEventScriptMng::process_account_first_login(int,CUser *,std::bitset<32u>)
    void process_account_first_login(int event_type,
                                     CUser* user,
                                     std::bitset<32> user_confirm_flags);
    void process_add_item_count_db(CUser* user,
                                   int item_id,
                                   int count);
    void process_add_quest_count_db(CUser* user, int quest_id);
    void process_level_up_reward(CUser* user,
                                 bool include_lower_levels);
    void process_level_up_reward_recv_level(CUser* user,
                                            int current_level,
                                            int received_level);
    void process_add_fatigue_count(CUser* user, int count);
    bool inc_user_fatigue(CUser* user, unsigned int amount);
    bool send_gift_fatigue_item(CUser* user, unsigned int amount);
    int send_bring_up_item_fatigue(CUser* user, int amount);
    bool send_pc_room_play_init_time(CUser* user);
    void send_event_init_data(CUser* user, bool reset);
    CEventScriptMng* update_time_values(int reset_hour);
    int get_today_time_day();
    int get_today_time();
    int get_ingame_event_record_type_list(std::vector<int>& record_types);
    bool is_current_ingame_event_record_type(int record_type);
    int get_record_type(int event_type);
    void process_update_ingame_history(CUser* user);
    int get_pass_day(int start_day, int current_day);
    void send_event_init_record_data(CUser* user);
    // IDA_EA: 0x810CC70
    // IDA_NAME: _ZN13EventClassify15CEventScriptMng23test_process_change_dayEi
    // DEMANGLED: EventClassify::CEventScriptMng::test_process_change_day(int)
    void test_process_change_day(int today);
    int update_purchase_target_item_count(CUser* user,
                                          unsigned int charac_no,
                                          stCeraShopIPGRequest_t* ipg_request);
    int send_purchase_bonus_mail(CUser* user, unsigned int purchase_count);
    int init_event_nums();

private:
    typedef std::map<int, CEventEntity*> EntityMap;

    InGameEventInfoList info_list_;
    InGameEventScriptLoader script_loader_;
    EntityMap event_entities_;
    int today_time_;
    unsigned char today_day_;
    unsigned char reserved_4d_[3];
    std::vector<int> event_nums_;
};

typedef char CEventScriptMng_Size_Must_Be_0x5c[
    (sizeof(CEventScriptMng) == 0x5c) ? 1 : -1];

}  // namespace EventClassify

#endif  // OLD_GAME_EVENT_EVENT_RUNTIME_H_
