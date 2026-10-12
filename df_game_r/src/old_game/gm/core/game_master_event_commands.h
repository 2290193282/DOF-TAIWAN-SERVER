#ifndef OLDSRV_OLD_GAME_EVENT_GAME_MASTER_EVENT_COMMANDS_H_
#define OLDSRV_OLD_GAME_EVENT_GAME_MASTER_EVENT_COMMANDS_H_

#include <stdint.h>

#include <string>

#include "old_game/gm/core/game_master_runtime_abi.h"

class CUser;

namespace game_master {

// IDA unit: 11_event_runtime/game_master/game_master_CStartEvent.
class CStartEvent : public CCommand {
public:
    CStartEvent();

    virtual void execute();
    virtual void parse();

    void getParam(std::string command);

private:
    int token_count_;
    int event_id_;
    int16_t event_param_0_;
    int16_t event_param_1_;
};

// IDA unit: 11_event_runtime/game_master/game_master_CEndEvent.
class CEndEvent : public CCommand {
public:
    CEndEvent();

    virtual void execute();
    virtual void parse();

private:
    int event_id_;
};

// IDA unit: 11_event_runtime/game_master/game_master_CInGameEventSetDay.
class CInGameEventSetDay : public CCommand {
public:
    CInGameEventSetDay();

    virtual void execute();
    virtual void parse();

private:
    int day_;
};

// IDA unit: 11_event_runtime/game_master/game_master_BlueMarbleSet.
class BlueMarbleSet : public CCommand {
public:
    BlueMarbleSet();

    virtual void execute();
    virtual void parse();

    void getParam(std::string command);

private:
    uint8_t position_;
    uint8_t move_requested_;
    uint8_t enter_reset_requested_;
    uint8_t padding_0b_;
    int token_count_;
};

// IDA unit: 11_event_runtime/game_master/game_master_CGrowhWeaponEvent.
// The original typo "Growh" is part of the exported ABI.
class CGrowhWeaponEvent : public CCommand {
public:
    CGrowhWeaponEvent();

    virtual void execute();
    virtual void parse();

private:
    uint8_t reset_requested_;
    uint8_t padding_09_0b_[3];
    int timepiece_;
};

// IDA unit: 11_event_runtime/game_master/game_master_CClearRewardTest.
class CClearRewardTest : public CCommand {
public:
    CClearRewardTest();

    virtual void execute();
    virtual void parse();

    void printClient(CUser* user,
                     const char* filename,
                     const char* contents);
};

// IDA unit: 11_event_runtime/game_master/game_master_CBingoGmTest.
class CBingoGmTest : public CCommand {
public:
    CBingoGmTest();

    virtual void execute();
    virtual void parse();

    void resetData();
    void reqValue();

private:
    int requested_value_;
};

#if defined(__i386__)
static_assert(sizeof(CStartEvent) == 0x14u,
              "CStartEvent must retain the old 0x14-byte ABI");
static_assert(sizeof(CEndEvent) == 0x0cu,
              "CEndEvent must retain the old 0x0c-byte ABI");
static_assert(sizeof(CInGameEventSetDay) == 0x0cu,
              "CInGameEventSetDay must retain the old 0x0c-byte ABI");
static_assert(sizeof(BlueMarbleSet) == 0x10u,
              "BlueMarbleSet must retain the old 0x10-byte ABI");
static_assert(sizeof(CGrowhWeaponEvent) == 0x10u,
              "CGrowhWeaponEvent must retain the old 0x10-byte ABI");
static_assert(sizeof(CClearRewardTest) == 0x08u,
              "CClearRewardTest must retain the old 0x08-byte ABI");
static_assert(sizeof(CBingoGmTest) == 0x0cu,
              "CBingoGmTest must retain the old 0x0c-byte ABI");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_EVENT_GAME_MASTER_EVENT_COMMANDS_H_
