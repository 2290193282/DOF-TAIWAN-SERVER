#ifndef OLDSRV_OLD_GAME_EVENT_HERO_MISSION_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_HERO_MISSION_RUNTIME_H_

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "old_game/activity/reward/event_base.h"
#include "old_data/PVF/arad/arad_singleton.h"

class CUser;

// IDA unit: localtaiwan/Event/EventHeroMission.
// HeroMissionCondition is the 0x2c-byte script record built by
// importHeroMissionConditionsData at 0x896171b.
struct HeroMissionCondition {
    struct MissionNo {
        enum T {
            CLEAR_DUNGEON = 1,
            USE_FATIGUE = 2,
            USE_CUBE = 3,
            LEVEL_UP = 4,
            ATTENDANCE = 5,
            TOTAL = 6
        };
    };

    struct RewardData {
        unsigned int item_index;
        uint16_t count;
        uint16_t reserved_06;
    };

    HeroMissionCondition();
    HeroMissionCondition(const HeroMissionCondition& other);
    ~HeroMissionCondition();
    HeroMissionCondition& operator=(const HeroMissionCondition& other);
    void clear();

    MissionNo::T mission_no;
    int mission_type;
    bool repeat;
    bool reset_state;
    uint8_t reserved_0a[2];
    std::string mail_title;
    std::string mail_content;
    std::vector<RewardData> rewards;
    std::vector<uint16_t> conditions;
};

struct HeroMissionValue {
    HeroMissionValue* clear();
    HeroMissionValue* reset();
    // IDA_EA: 0x8164BCA
    // IDA_NAME: _ZN16HeroMissionValue14getStringTokenERSs
    // The caller discards EAX; the original body is a serializer with no
    // caller-visible result, so retain the source-level void contract.
    void getStringToken(std::string& token);
    // IDA_EA: 0x8164D68
    // IDA_NAME: _ZN16HeroMissionValue14setStringTokenESs
    // DEMANGLED: HeroMissionValue::setStringToken(std::string)
    void setStringToken(std::string token);
    ~HeroMissionValue();

    HeroMissionCondition::MissionNo::T mission_no;
    uint16_t progress;
    uint16_t state;
    uint16_t value;
    uint16_t reward_count;
};

class BaseHeroMissionCondition {
public:
    explicit BaseHeroMissionCondition(
        HeroMissionCondition::MissionNo::T mission_no);
    virtual ~BaseHeroMissionCondition();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param) = 0;
    virtual bool rewardMission(CUser* user, HeroMissionValue& value) = 0;

    void setConditionData(HeroMissionCondition& condition);
    HeroMissionCondition::MissionNo::T getMissionCode();
    int getMissionType();
    bool isRepeatMission();
    bool isResetState();
    std::string getMailTitle();
    std::string getMailContent();
    bool rewardProcess(CUser* user);

protected:
    HeroMissionCondition::MissionNo::T mission_no_;
    HeroMissionCondition condition_;
};

// IDA vtables 0x8B7F0C0 .. 0x8B7F048 show that all six concrete conditions
// are empty derived objects: the complete 0x34-byte object is the
// BaseHeroMissionCondition subobject.  Keep the two gameplay slots in their
// original order after the ordinary/deleting destructor pair.
class ClearDungeonMission : public BaseHeroMissionCondition {
public:
    ClearDungeonMission();
    virtual ~ClearDungeonMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);
};

class UseFatigueMission : public BaseHeroMissionCondition {
public:
    UseFatigueMission();
    virtual ~UseFatigueMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);
};

class UseCubeMission : public BaseHeroMissionCondition {
public:
    UseCubeMission();
    virtual ~UseCubeMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);
};

class LevelUpMission : public BaseHeroMissionCondition {
public:
    LevelUpMission();
    virtual ~LevelUpMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);
};

class AttendanceMission : public BaseHeroMissionCondition {
public:
    AttendanceMission();
    virtual ~AttendanceMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);
};

class TotalMission : public BaseHeroMissionCondition {
public:
    TotalMission();
    virtual ~TotalMission();

    virtual void processMission(CUser* user,
                                HeroMissionValue& value,
                                unsigned int param);
    virtual bool rewardMission(CUser* user, HeroMissionValue& value);

    // IDA_EA: 0x81663FE.  HeroMissionEvent invokes this only for mission 6,
    // after locating the caller's complete value vector.
    bool rewardMission(CUser* user,
                       HeroMissionValue& value,
                       std::vector<HeroMissionValue>& values);
};

// IDA_EA: 0x896171B
// IDA_NAME:
// _Z31importHeroMissionConditionsDataPSt6vectorI20HeroMissionConditionSaIS0_EEPKc
// DEMANGLED: importHeroMissionConditionsData(
//     std::vector<HeroMissionCondition> *,char const*)
int importHeroMissionConditionsData(
    std::vector<HeroMissionCondition>* conditions,
    const char* filename);

// IDA RTTI at 0x8B7F100 records both direct bases.  The singleton base is
// empty, so GCC's empty-base optimization leaves CEventBase at +0x00.
class HeroMissionEvent : public ARAD::Singleton<HeroMissionEvent>,
                         public CEventBase {
public:
    HeroMissionEvent();
    virtual ~HeroMissionEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    // IDA_EA: 0x8166D38
    // IDA_NAME: _ZN16HeroMissionEvent13saveEventDataEP5CUserb
    // DEMANGLED: HeroMissionEvent::saveEventData(CUser *,bool)
    void saveEventData(CUser* user, bool force);

    // IDA_EA: 0x816709E
    // IDA_NAME: _ZN16HeroMissionEvent13loadEventDataEP5CUser
    // DEMANGLED: HeroMissionEvent::loadEventData(CUser *)
    void loadEventData(CUser* user);

    // IDA_EA: 0x8167216
    // IDA_NAME:
    // _ZN16HeroMissionEvent15setMissionValueEP5CUserN20HeroMissionCondition9MissionNo1TEj
    // DEMANGLED: HeroMissionEvent::setMissionValue(
    //     CUser *,HeroMissionCondition::MissionNo::T,unsigned int)
    void setMissionValue(CUser* user,
                         HeroMissionCondition::MissionNo::T mission_no,
                         unsigned int value);

    // IDA_EA: 0x81672BE
    // IDA_NAME:
    // _ZN16HeroMissionEvent14processMissionEP5CUserN20HeroMissionCondition9MissionNo1TEj
    void processMission(CUser* user,
                        HeroMissionCondition::MissionNo::T mission_no,
                        unsigned int param);

    // IDA_EA: 0x8166752 / 0x8166D0E.  The GM `heromission reset`
    // command uses the char-pointer form to repopulate the default mission
    // vector before it sends the live state packet.
    bool setEventData(CUser* user, const char* token_data);
    bool setEventData(CUser* user, const std::string& token_data);

    // IDA_EA: 0x816748E
    // IDA_NAME:
    // _ZN16HeroMissionEvent13rewardMissionEP5CUserN20HeroMissionCondition9MissionNo1TE
    // DEMANGLED: HeroMissionEvent::rewardMission(
    //     CUser *,HeroMissionCondition::MissionNo::T)
    bool rewardMission(CUser* user,
                       HeroMissionCondition::MissionNo::T mission_no);

    // IDA_EA: 0x816770E
    // IDA_NAME: _ZN16HeroMissionEvent15sendMissionDataEP5CUser
    // DEMANGLED: HeroMissionEvent::sendMissionData(CUser *)
    void sendMissionData(CUser* user);

    // IDA_EA: 0x8168010
    bool findMissionValue(CUser* user,
                          HeroMissionCondition::MissionNo::T mission_no,
                          HeroMissionValue*& value);

    // IDA_EA: 0x81679AC
    // IDA_NAME:
    // _ZN16HeroMissionEvent15setMissionForGmEP5CUserN20HeroMissionCondition9MissionNo1TEj
    // DEMANGLED: HeroMissionEvent::setMissionForGm(
    //     CUser *,HeroMissionCondition::MissionNo::T,unsigned int)
    int setMissionForGm(CUser* user,
                        HeroMissionCondition::MissionNo::T mission_no,
                        unsigned int value);

    // IDA_EA: 0x8167A3A
    // IDA_NAME:
    // _ZN16HeroMissionEvent17resetMissionForGmEP5CUserN20HeroMissionCondition9MissionNo1TE
    // DEMANGLED: HeroMissionEvent::resetMissionForGm(
    //     CUser *,HeroMissionCondition::MissionNo::T)
    HeroMissionValue* resetMissionForGm(
        CUser* user, HeroMissionCondition::MissionNo::T mission_no);

private:
    bool loadScript();

    // IDA +0x0c
    std::map<HeroMissionCondition::MissionNo::T,
             BaseHeroMissionCondition*> conditions_;
    // IDA +0x24
    std::map<unsigned int, std::vector<HeroMissionValue> > values_by_charac_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(HeroMissionCondition::RewardData) == 0x08u,
              "HeroMissionCondition::RewardData must retain the old ABI");
static_assert(sizeof(HeroMissionCondition) == 0x2cu,
              "HeroMissionCondition must retain the old ABI");
static_assert(sizeof(HeroMissionValue) == 0x0cu,
              "HeroMissionValue must retain the old ABI");
static_assert(sizeof(BaseHeroMissionCondition) == 0x34u,
              "BaseHeroMissionCondition must retain the old ABI");
static_assert(sizeof(ClearDungeonMission) == 0x34u,
              "ClearDungeonMission must retain the old ABI");
static_assert(sizeof(UseFatigueMission) == 0x34u,
              "UseFatigueMission must retain the old ABI");
static_assert(sizeof(UseCubeMission) == 0x34u,
              "UseCubeMission must retain the old ABI");
static_assert(sizeof(LevelUpMission) == 0x34u,
              "LevelUpMission must retain the old ABI");
static_assert(sizeof(AttendanceMission) == 0x34u,
              "AttendanceMission must retain the old ABI");
static_assert(sizeof(TotalMission) == 0x34u,
              "TotalMission must retain the old ABI");
static_assert(sizeof(HeroMissionEvent) == 0x3cu,
              "HeroMissionEvent must retain the old ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_HERO_MISSION_RUNTIME_H_
