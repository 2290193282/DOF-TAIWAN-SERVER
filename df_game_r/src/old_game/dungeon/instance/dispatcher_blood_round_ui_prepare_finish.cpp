#include "old_game/dungeon/instance/dispatcher_blood_round_ui_prepare_finish.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8596DC8
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishC2Ev
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::Dispatcher_BloodRoundUiPrepareFinish(void)
Dispatcher_BloodRoundUiPrepareFinish::Dispatcher_BloodRoundUiPrepareFinish()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240EA0
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishD2Ev
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::~Dispatcher_BloodRoundUiPrepareFinish()
// IDA_EA: 0x8240ED0
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinishD0Ev
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::~Dispatcher_BloodRoundUiPrepareFinish()
Dispatcher_BloodRoundUiPrepareFinish::~Dispatcher_BloodRoundUiPrepareFinish() {}

// IDA_EA: 0x8222270
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;

    if (!user) {
        return LineFunc(
            56255,
            "virtual int Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(
            56256,
            "virtual int Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->get_state() != 5) {
        return LineFunc(
            56258,
            "virtual int Dispatcher_BloodRoundUiPrepareFinish::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CParty* const party = user->GetParty();
    if (party) {
        CBattle_Field& battle_field = party->battle_field();
        CDungeon* const dungeon = battle_field.GetDungeon();
        if (dungeon && dungeon->get_blood_dungeon_type() > 0 &&
            battle_field.GetMapInfo()->visited_goto_ != 1u) {
            battle_field.BloodSpawnTimerManager(false, false);
        }
    }
    return 0;
}

// IDA_EA: 0x8233F92
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BloodRoundUiPrepareFinish::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233F9C
// IDA_NAME: _ZN36Dispatcher_BloodRoundUiPrepareFinish7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BloodRoundUiPrepareFinish::process(CUser*, MSG_BASE &,ParamBase &)
int Dispatcher_BloodRoundUiPrepareFinish::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}
