#ifndef OLDSRV_DATAMANAGER_AVATAR_ROULETTE_SERVER_H_
#define OLDSRV_DATAMANAGER_AVATAR_ROULETTE_SERVER_H_

#include "old_game/appearance/avatar/avatar_roulette.h"
#include "old_game/progression/quest/quest_skill_value_types.h"

#include <map>
#include <stdint.h>
#include <vector>

// IDA unit: 12_pvf_datamanager_scripts/AvatarRouletteServer.
class AvatarRouletteServer {
public:
    struct PickedAvatarInfo {
        // IDA_EA: 0x819A336
        // IDA_NAME: _ZN20AvatarRouletteServer16PickedAvatarInfoC2Ev
        // DEMANGLED: AvatarRouletteServer::PickedAvatarInfo::PickedAvatarInfo(void)
        PickedAvatarInfo();

        uint32_t item_no;
        uint32_t value_04;
        uint8_t avatar_option;
        uint8_t value_09;
        uint8_t value_0a;
        uint8_t picked_group;
    };

    // IDA_EA: 0x817F354
    // IDA_NAME: _ZN20AvatarRouletteServerC2Ev
    // DEMANGLED: AvatarRouletteServer::AvatarRouletteServer(void)
    AvatarRouletteServer();

    // IDA_EA: 0x817F372
    // IDA_NAME: _ZN20AvatarRouletteServerD2Ev
    // DEMANGLED: AvatarRouletteServer::~AvatarRouletteServer()
    ~AvatarRouletteServer();

    // IDA_EA: 0x817F386
    // IDA_NAME: _ZN20AvatarRouletteServer18InitAddCharacSkillEv
    // DEMANGLED: AvatarRouletteServer::InitAddCharacSkill(void)
    int InitAddCharacSkill();

    // IDA_EA: 0x817F706
    // IDA_NAME: _ZN20AvatarRouletteServer4InitEP14AvatarRoulette
    // DEMANGLED: AvatarRouletteServer::Init(AvatarRoulette *)
    int Init(AvatarRoulette* roulette);

    // IDA_EA: 0x817FB0A
    // IDA_NAME: _ZN20AvatarRouletteServer15isAvatarPotteryEm
    // DEMANGLED: AvatarRouletteServer::isAvatarPottery(unsigned long)
    bool isAvatarPottery(unsigned long item_no);

    // IDA_EA: 0x817FB28
    // IDA_NAME: _ZN20AvatarRouletteServer6PickupERNS_16PickedAvatarInfoEii
    // DEMANGLED: AvatarRouletteServer::Pickup(AvatarRouletteServer::PickedAvatarInfo &,int,int)
    int Pickup(PickedAvatarInfo& picked, int character_job, int item_part);

private:
    typedef std::map<int, std::vector<LevelUpSkill> > CharacterSkillMap;
    typedef std::map<unsigned long, CharacterSkillMap> AvatarCharacterSkills;

    AvatarCharacterSkills character_skills_;
    AvatarRoulette* avatar_roulette_;
    int total_avatar_rate_;
};

#if UINTPTR_MAX == 0xffffffffu
static_assert(sizeof(AvatarRouletteServer::PickedAvatarInfo) == 0x0cu,
              "AvatarRouletteServer::PickedAvatarInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(AvatarRouletteServer) == 0x20u,
              "AvatarRouletteServer IDA ABI size must stay 0x20");
#endif

#endif  // OLDSRV_DATAMANAGER_AVATAR_ROULETTE_SERVER_H_
