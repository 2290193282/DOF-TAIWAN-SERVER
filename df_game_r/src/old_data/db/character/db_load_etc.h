#ifndef OLDSRV_DB_DB_LOAD_ETC_H_
#define OLDSRV_DB_DB_LOAD_ETC_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/character/tag_charac_info_payload.h"


class MySQL;

// IDA_EA: 0x8403B84/0x8403BA0/0x8403BD0/0x8403BEE/0x84040B4/0x8404290/0x8404A68/0x8404B72/0x8405C60/0x8405C6E/0x8405E0A/0x8406092/0x8406422/0x8406600/0x84066AE/0x8406810/0x8406A6A/0x8406CCE/0x8406EEC/0x8407296/0x84073AA/0x8407482/0x84075C4/0x84075D2/0x8407DB4/0x8407DC2/0x840838E/0x84084E0/0x8408766/0x84356A2/0x843F038/0x8445660/0x8446D9E/0x8448944/0x844A20A/0x844A3B2/0x844A47A/0x844A704.
// IDA unit: 13_database_dispatch/DB_LoadEtc.
class DB_LoadEtc : public DBDispatcher {
public:
    DB_LoadEtc();
    virtual ~DB_LoadEtc();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    bool LoadItemMakingSkill(SIG_LOAD_ETC* payload);
    bool LoadFairPvPScore(SIG_LOAD_ETC* payload);
    bool LoadQuestShop(SIG_LOAD_ETC* payload);
    static bool _LoadQuestShop(unsigned int charac_no,
                               CHARAC_QUEST_SHOP_DATA* quest_shop_data);
    bool LoadLitmitItem(SIG_LOAD_ETC* payload);
    bool LoadAccountFirstLogin(SIG_LOAD_ETC* payload, int event_type);
    bool LoadAccountFirstLoginDaily(SIG_LOAD_ETC* payload, int event_type);
    bool LoadTitleBook(SIG_LOAD_ETC* payload);
    bool LoadAchievement(SIG_LOAD_ETC* payload);
    bool LoadOnceGiveAvengerTitle(SIG_LOAD_ETC* payload);
    bool LoadPcRoomPlayTimeInfo(SIG_LOAD_ETC* payload);
    bool LoadUsedGiftFatigueInfo(SIG_LOAD_ETC* payload);
    bool LoadRedeemItem(SIG_LOAD_ETC* payload);
    bool LoadPvPMissionList(SIG_LOAD_ETC* payload);
    bool LoadTODUserState(SIG_LOAD_ETC* payload);
    bool LoadConditionEventInfo(SIG_LOAD_ETC* payload);
    bool LoadProperDungeonClearCount(SIG_LOAD_ETC* payload);
    int _loadNPCRelasionship(SIG_LOAD_ETC* payload);
    bool GetUserRecord(SIG_LOAD_ETC* payload);
    bool GetUserSkill(SIG_LOAD_ETC* payload);
    int GetUserPvpBase(SIG_LOAD_ETC* payload);
    int GetUserPvp(SIG_LOAD_ETC* payload);
    bool LoadAuraAvatarOption(SIG_LOAD_ETC* payload);
    bool LoadCharacOption(int signal, int user_idx, SIG_LOAD_ETC* payload);
    bool SendResult(int signal, int user_idx, SIG_LOAD_ETC* payload);
    bool LoadEventServerMessageInfo(SIG_LOAD_ETC* payload);
    bool LoadVillageAttackInfo(SIG_LOAD_ETC* payload);
    bool LoadCharacItemStat(SIG_LOAD_ETC* payload);
    bool LoadBoosterGage(SIG_LOAD_ETC* payload);
    bool LoadAttendanceInfo(SIG_ATTENDANCE_EVENT* attendance_payload,
                            unsigned int account_id);
    bool LoadSeriaRoomAniDeco(SIG_LOAD_ETC* payload);
    void LoadComboSkillInfo(SIG_LOAD_ETC* payload);
    bool LoadComboSkill(SIG_LOAD_ETC* payload,
                        ENUM_SKILL_TREE_KIND skill_tree_kind);
    bool getComboSkill(MySQL* db_handle,
                       unsigned int charac_no,
                       int row_count,
                       comoboSkillData* combo_skill_data);
};

#endif  // OLDSRV_DB_DB_LOAD_ETC_H_
