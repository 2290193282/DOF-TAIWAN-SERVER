#include "old_game/pvp/battlefield/timer_blood_round_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

// IDA_EA: 0x863C47A
// IDA_NAME: _ZN28TimerBloodSmallRoundRegisterC2Ev
TimerBloodSmallRoundRegister::TimerBloodSmallRoundRegister()
    : TimerSig() {}

// IDA_EA: 0x864061A (D2), 0x864064A (D0)
TimerBloodSmallRoundRegister::~TimerBloodSmallRoundRegister() {}

// IDA_EA: 0x8634FD2
// IDA_NAME: _ZN28TimerBloodSmallRoundRegister12dispatch_sigEiij
int TimerBloodSmallRoundRegister::dispatch_sig(int party_index,
                                                int timer_key,
                                                unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_SMALL_ROUND_REGISTER, timer_key)) {
        return 0;
    }

    party->battle_field().BloodSpawnTimerManager(false, true);
    return 1;
}

TimerBloodPhaseRegister0::TimerBloodPhaseRegister0() : TimerSig() {}
TimerBloodPhaseRegister0::~TimerBloodPhaseRegister0() {}

// IDA_EA: 0x863504C
// IDA_NAME: _ZN24TimerBloodPhaseRegister012dispatch_sigEiij
int TimerBloodPhaseRegister0::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_0, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[0],
                          field.blood_spawn_runtime_.phase_finished[0],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_0, 0);
    return 1;
}

TimerBloodPhaseRegister1::TimerBloodPhaseRegister1() : TimerSig() {}
TimerBloodPhaseRegister1::~TimerBloodPhaseRegister1() {}

// IDA_EA: 0x86350EE
// IDA_NAME: _ZN24TimerBloodPhaseRegister112dispatch_sigEiij
int TimerBloodPhaseRegister1::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_1, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[1],
                          field.blood_spawn_runtime_.phase_finished[1],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_1, 1);
    return 1;
}

TimerBloodPhaseRegister2::TimerBloodPhaseRegister2() : TimerSig() {}
TimerBloodPhaseRegister2::~TimerBloodPhaseRegister2() {}

// IDA_EA: 0x8635190
// IDA_NAME: _ZN24TimerBloodPhaseRegister212dispatch_sigEiij
int TimerBloodPhaseRegister2::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_2, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[2],
                          field.blood_spawn_runtime_.phase_finished[2],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_2, 2);
    return 1;
}

TimerBloodPhaseRegister3::TimerBloodPhaseRegister3() : TimerSig() {}
TimerBloodPhaseRegister3::~TimerBloodPhaseRegister3() {}

// IDA_EA: 0x8635232
// IDA_NAME: _ZN24TimerBloodPhaseRegister312dispatch_sigEiij
int TimerBloodPhaseRegister3::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_3, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[3],
                          field.blood_spawn_runtime_.phase_finished[3],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_3, 3);
    return 1;
}

TimerBloodPhaseRegister4::TimerBloodPhaseRegister4() : TimerSig() {}
TimerBloodPhaseRegister4::~TimerBloodPhaseRegister4() {}

// IDA_EA: 0x86352D4
// IDA_NAME: _ZN24TimerBloodPhaseRegister412dispatch_sigEiij
int TimerBloodPhaseRegister4::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_4, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[4],
                          field.blood_spawn_runtime_.phase_finished[4],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_4, 4);
    return 1;
}

TimerBloodPhaseRegister5::TimerBloodPhaseRegister5() : TimerSig() {}
TimerBloodPhaseRegister5::~TimerBloodPhaseRegister5() {}

// IDA_EA: 0x8635376
// IDA_NAME: _ZN24TimerBloodPhaseRegister512dispatch_sigEiij
int TimerBloodPhaseRegister5::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_5, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[5],
                          field.blood_spawn_runtime_.phase_finished[5],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_5, 5);
    return 1;
}

TimerBloodPhaseRegister6::TimerBloodPhaseRegister6() : TimerSig() {}
TimerBloodPhaseRegister6::~TimerBloodPhaseRegister6() {}

// IDA_EA: 0x8635418
// IDA_NAME: _ZN24TimerBloodPhaseRegister612dispatch_sigEiij
int TimerBloodPhaseRegister6::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_6, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[6],
                          field.blood_spawn_runtime_.phase_finished[6],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_6, 6);
    return 1;
}

TimerBloodPhaseRegister7::TimerBloodPhaseRegister7() : TimerSig() {}
TimerBloodPhaseRegister7::~TimerBloodPhaseRegister7() {}

// IDA_EA: 0x86354BA
// IDA_NAME: _ZN24TimerBloodPhaseRegister712dispatch_sigEiij
int TimerBloodPhaseRegister7::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_7, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[7],
                          field.blood_spawn_runtime_.phase_finished[7],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_7, 7);
    return 1;
}

TimerBloodPhaseRegister8::TimerBloodPhaseRegister8() : TimerSig() {}
TimerBloodPhaseRegister8::~TimerBloodPhaseRegister8() {}

// IDA_EA: 0x863555C
// IDA_NAME: _ZN24TimerBloodPhaseRegister812dispatch_sigEiij
int TimerBloodPhaseRegister8::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_8, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[8],
                          field.blood_spawn_runtime_.phase_finished[8],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_8, 8);
    return 1;
}

TimerBloodPhaseRegister9::TimerBloodPhaseRegister9() : TimerSig() {}
TimerBloodPhaseRegister9::~TimerBloodPhaseRegister9() {}

// IDA_EA: 0x86355FE
// IDA_NAME: _ZN24TimerBloodPhaseRegister912dispatch_sigEiij
int TimerBloodPhaseRegister9::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_PHASE_REGISTER_9, timer_key)) {
        return 0;
    }
    CBattle_Field& field = party->battle_field();
    field.BloodPhaseSpawn(field.blood_spawn_runtime_.registered_small_rounds[9],
                          field.blood_spawn_runtime_.phase_finished[9],
                          TIMER_MESSAGE_BLOOD_PHASE_REGISTER_9, 9);
    return 1;
}

// IDA_EA: 0x863C5AE
// IDA_NAME: _ZN22TimerBloodCrazyMonsterC2Ev
TimerBloodCrazyMonster::TimerBloodCrazyMonster() : TimerSig() {}

// IDA_EA: 0x86402C0 (D2), 0x86402F0 (D0)
TimerBloodCrazyMonster::~TimerBloodCrazyMonster() {}

// IDA_EA: 0x86356A0
// IDA_NAME: _ZN22TimerBloodCrazyMonster12dispatch_sigEiij
int TimerBloodCrazyMonster::dispatch_sig(int party_index,
                                          int timer_key,
                                          unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_BLOOD_CRAZY_MONSTER, timer_key)) {
        return 0;
    }

    party->battle_field().BloodCrazyMonsterSpawnPacket();
    return 1;
}
