#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_STATE_RUNTIME_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_STATE_RUNTIME_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8491E92, 0x808BA78, 0x86AB95C, 0x84C5054,
// DisPatcher_QuestAction 0x81EE010/0x8232D1A/0x8232D24, and
// Dispatcher_SaveQuestNotify 0x825ED06/0x825EDFC/0x825EEC4 in the companion
// source.

#include <stdint.h>

#include <list>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_data/Login/quest_authen_data.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;
class Quest;
struct stSelectQuestParam;
enum QUEST_CONDITION : int;

namespace WongWork {

// IDA unit: 08_quest_skill_growth/WongWork_CQuestClear.
struct CQuestClear {
    CQuestClear();
    ~CQuestClear();

    int setClearedQuest(unsigned int quest_index);
    int resetClearedQuests(unsigned int quest_index);
    int isClearedQuest(unsigned int quest_index) const;
    CQuestClear* clear();
    int syncScript();
    int checkSaveFlag() const;
    CQuestClear* resetSaveFlags();
    void* setClearedQuest(const STClearedQuest* quest_clear);
    const STClearedQuest* getClearedQuest() const;
    // IDA_EA: 0x808BB88
    // IDA_NAME: _ZNK8WongWork11CQuestClear14enumQuestClearEPFbjPcEz
    // DEMANGLED: WongWork::CQuestClear::enumQuestClear(bool (*)(unsigned int,char *),...)const
    bool enumQuestClear(bool (*callback)(unsigned int, char*), ...) const;
    int getClearedQuestNum() const;

    STClearedQuest clear_quest[0x7530]; // +0x0000
    uint8_t save_flag;                  // +0x7530
    uint8_t pad_7531[3];                // +0x7531
};

}  // namespace WongWork

// IDA unit: 08_quest_skill_growth/SIG_LOAD_QUEST.
struct SIG_LOAD_QUEST {
    SIG_LOAD_QUEST();

    uint32_t charac_no;              // +0x0000
    uint32_t error_code;             // +0x0004
    uint32_t raw_08;                 // +0x0008
    uint32_t play_quest[20];         // +0x000C
    uint32_t play_quest_trigger[20]; // +0x005C
    uint32_t urgent_quest_index;     // +0x00AC
    WongWork::STClearedQuest clear_quest[0x7530]; // +0x00B0
    uint8_t quest_notify[0x28];      // +0x75E0
};

// IDA unit: 08_quest_skill_growth/UserQuest.
struct UserQuest {
    UserQuest* set_parent(CUser* user);
    UserQuest();
    ~UserQuest();
    void* reset();
    int isClearQuest(int quest_index) const;
    int resetClearQuest(int quest_index);
    int set_quest(const SIG_LOAD_QUEST* payload);
    int get_quest(SIG_LOAD_QUEST* payload) const;
    int giveup_quest(int quest_index);
    int IsDoingQuest(int quest_index) const;

    // IDA_EA: 0x86AD054
    // IDA_NAME: _ZNK9UserQuest27CheckDungeonEventStoryPauseEiii
    // DEMANGLED: UserQuest::CheckDungeonEventStoryPause(int,int,int)const
    int CheckDungeonEventStoryPause(int playing_dungeon_index,
                                    int current_map_index,
                                    int maze_event_value) const;

    // IDA_EA: 0x86ABF38
    // IDA_NAME: _ZN9UserQuest12accept_questEiPcRi
    // DEMANGLED: UserQuest::accept_quest(int,char*,int&)
    int accept_quest(int quest_index,
                     char* packet_storage,
                     int& accepted_slot);

    // IDA_EA: 0x86AC6AC
    // IDA_NAME: _ZN9UserQuest11set_triggerEicb
    // DEMANGLED: UserQuest::set_trigger(int,char,bool)
    int set_trigger(int quest_index, char trigger_state, bool trigger_value);

    // IDA_EA: 0x86ABDB0
    // IDA_NAME: _ZN9UserQuest15set_authen_dataE15QUEST_CONDITIONii
    // DEMANGLED: UserQuest::set_authen_data(QUEST_CONDITION,int,int)
    int set_authen_data(QUEST_CONDITION condition, int value, int auxiliary);

    // IDA_EA: 0x86ACB52
    // IDA_NAME: _ZN9UserQuest29check_quest_subtype_conditionEP5Quest
    // DEMANGLED: UserQuest::check_quest_subtype_condition(Quest *)
    int check_quest_subtype_condition(Quest* quest);
    // IDA_EA: 0x86ACE46
    // IDA_NAME: _ZN9UserQuest26check_quest_type_conditionEP5Quest
    // DEMANGLED: UserQuest::check_quest_type_condition(Quest *)
    int check_quest_type_condition(Quest* quest);
    // IDA_EA: 0x86ACF8A
    // IDA_NAME: _ZN9UserQuest21check_quest_conditionEi
    // DEMANGLED: UserQuest::check_quest_condition(int)
    int check_quest_condition(int quest_index);

    // IDA_EA: 0x86ABD7A
    // IDA_NAME:
    // _ZNK9UserQuest19get_mail_quest_infoERSt4listIiSaIiEERK18stSelectQuestParam
    // DEMANGLED: UserQuest::get_mail_quest_info(
    //             std::list<int>&,stSelectQuestParam const&)const
    bool get_mail_quest_info(
        std::list<int>& selected_quests,
        const stSelectQuestParam& parameter) const;

    // IDA_EA: 0x86AD178
    // IDA_NAME: _ZN9UserQuest27ResetUrgentQuestWaitingListEv
    // DEMANGLED: UserQuest::ResetUrgentQuestWaitingList(void)
    void ResetUrgentQuestWaitingList();

    // IDA_EA: 0x86AD266
    // IDA_NAME: _ZN9UserQuest12hasEpicQuestEv
    // DEMANGLED: UserQuest::hasEpicQuest(void)
    bool hasEpicQuest();

    // IDA_EA: 0x86AC854
    // IDA_NAME: _ZN9UserQuest12finish_questEi
    // DEMANGLED: UserQuest::finish_quest(int)
    int finish_quest(int quest_index);

    // IDA_EA: 0x86AC954
    // IDA_NAME: _ZNK9UserQuest17item_reward_questEi
    // DEMANGLED: UserQuest::item_reward_quest(int)const
    int item_reward_quest(int dungeon_index) const;

    // IDA_EA: 0x86ACA16
    // IDA_NAME: _ZNK9UserQuest19get_rescue_questmapEi
    // DEMANGLED: UserQuest::get_rescue_questmap(int)const
    int get_rescue_questmap(int quest_index) const;

    // IDA_EA: 0x86ACAD0
    // IDA_NAME: _ZNK9UserQuest16check_cond_clearEi
    // DEMANGLED: UserQuest::check_cond_clear(int)const
    int check_cond_clear(int quest_index) const;

    // IDA_EA: 0x86AD48A
    // IDA_NAME: _ZNK9UserQuest21isDoingJobChangeQuestEv
    // DEMANGLED: UserQuest::isDoingJobChangeQuest(void)const
    int isDoingJobChangeQuest() const;
    // IDA_EA: 0x86ABBA8
    // IDA_NAME: _ZNK9UserQuest14get_quest_infoEPc
    // DEMANGLED: UserQuest::get_quest_info(char *)const
    void get_quest_info(char* packet) const;

    CUser* parent;                   // +0x0000
    WongWork::CQuestClear clear_quest; // +0x0004
    uint32_t play_quest[20];         // +0x7538
    uint32_t play_quest_trigger[20]; // +0x7588
    _Quest_Authen_Data authen_data;  // +0x75D8
    uint32_t urgent_quest_index;     // +0x75F0
    uint8_t quest_notify[0x28];      // +0x75F4
};

// IDA unit: 08_quest_skill_growth/Inter_LoadQuest.
class Inter_LoadQuest : public ISigDispatcher {
public:
    Inter_LoadQuest();
    virtual ~Inter_LoadQuest();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 08_quest_skill_growth/DisPatcher_QuestAction.
class DisPatcher_QuestAction
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_QuestAction();
    virtual ~DisPatcher_QuestAction();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 08_quest_skill_growth/Dispatcher_SaveQuestNotify.
class Dispatcher_SaveQuestNotify
    : public IPacketDispatcher<MSG_SAVE_QUEST_NOTIFY,
                               saveQuestNotifyParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_SaveQuestNotify();
    virtual ~Dispatcher_SaveQuestNotify();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_QuestAction) == 0x04u,
              "CMD33-36 must retain the old vptr-only dispatcher ABI");
static_assert(sizeof(Dispatcher_SaveQuestNotify) == 0x04u,
              "CMD507 must retain the old vptr-only dispatcher ABI");
#endif


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_STATE_RUNTIME_H_
