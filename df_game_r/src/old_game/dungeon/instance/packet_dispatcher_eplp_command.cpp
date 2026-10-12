#include "old_game/dungeon/instance/packet_dispatcher_eplp_command.h"

#include "old_game/activity/reward/advance_altar.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/etc/global_script_tutorial.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kEplpSignature[] =
    "virtual int Dispatcher_EPLPCommand::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8595BA0
// IDA_NAME: _ZN22Dispatcher_EPLPCommandC2Ev
// DEMANGLED: Dispatcher_EPLPCommand::Dispatcher_EPLPCommand(void)
Dispatcher_EPLPCommand::Dispatcher_EPLPCommand() {}

// IDA_EA: 0x8243552
// IDA_NAME: _ZN22Dispatcher_EPLPCommandD2Ev
// DEMANGLED: Dispatcher_EPLPCommand::~Dispatcher_EPLPCommand()
// IDA_EA: 0x8243582
// IDA_NAME: _ZN22Dispatcher_EPLPCommandD0Ev
// DEMANGLED: Dispatcher_EPLPCommand::~Dispatcher_EPLPCommand()
Dispatcher_EPLPCommand::~Dispatcher_EPLPCommand() {}

// IDA_EA: 0x81FED4C
// IDA_NAME: _ZN22Dispatcher_EPLPCommand14_SendEPLPErrorEP6CPartyhh
// DEMANGLED: Dispatcher_EPLPCommand::_SendEPLPError(CParty *,unsigned char,unsigned char)
void Dispatcher_EPLPCommand::_SendEPLPError(CParty* party,
                                            uint8_t command,
                                            uint8_t error) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_EPLP_COMMAND);
    packet.put_byte(0);
    packet.put_byte(error);
    packet.put_byte(command);
    packet.finalize(true);
    party->send_to_party(packet);
}

// IDA_EA: 0x81FEE14
// IDA_NAME: _ZN22Dispatcher_EPLPCommand16_BroadCastPacketEP6CPartyR16MSG_EPLP_COMMAND
// DEMANGLED: Dispatcher_EPLPCommand::_BroadCastPacket(CParty *,MSG_EPLP_COMMAND &)
void Dispatcher_EPLPCommand::_BroadCastPacket(CParty* party,
                                               MSG_EPLP_COMMAND& request) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_EPLP_COMMAND);
    packet.put_byte(1);
    packet.put_byte(request.request);
    packet.put_byte(request.command);
    packet.finalize(true);
    party->send_to_party(packet);
}

// IDA_EA: 0x825F760
// IDA_NAME: _ZN22Dispatcher_EPLPCommand16excludeEplpStateEP5CUser
// DEMANGLED: Dispatcher_EPLPCommand::excludeEplpState(CUser *)
bool Dispatcher_EPLPCommand::excludeEplpState(CUser* user) {
    CParty* const party = user->GetParty();
    if (!party) {
        return false;
    }

    const int dungeon_index = party->battle_field().get_dungeon_index();
    if (GlobalData::s_pTowerOfDespairMgr->IsTowerOfDespairDungeon_ByIndex(
            dungeon_index)) {
        return false;
    }
    if (user->getMoveSpace() == 1) {
        return false;
    }
    return !GlobalScriptTutorial::IsGlobalTutorialDungeon(
        static_cast<unsigned int>(dungeon_index));
}

// IDA_EA: 0x825F7F4
// IDA_NAME: _ZN22Dispatcher_EPLPCommand17isReTurnToVillageEP5CUser
// DEMANGLED: Dispatcher_EPLPCommand::isReTurnToVillage(CUser *)
bool Dispatcher_EPLPCommand::isReTurnToVillage(CUser* user) {
    CParty* const party = user->GetParty();
    if (!party) {
        return false;
    }
    if (user->getMoveSpace() == 1) {
        return true;
    }
    return GlobalScriptTutorial::IsGlobalTutorialDungeon(
        static_cast<unsigned int>(party->battle_field().get_dungeon_index()));
}

// IDA_EA: 0x81FEEDC
// IDA_NAME: _ZN22Dispatcher_EPLPCommand12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_EPLPCommand::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_EPLPCommand::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!Global::CGlobalFunc::Get_State_Check(user->get_state())) {
        return 0;
    }

    MSG_EPLP_COMMAND request = {};
    if (packet.get_byte(request.request) != 1) {
        return LineFunc(37432, kEplpSignature, 0, 0);
    }
    if (packet.get_byte(request.command) != 1) {
        return LineFunc(37433, kEplpSignature, 0, 0);
    }

    if (user->get_state() == 13) {
        PacketGuard response;
        response.put_header(1, ENUM_CMDPACKET_EPLP_COMMAND);
        response.put_byte(advancealtar::isOpenAdvanceAltar());
        response.put_byte(request.request);
        response.put_byte(request.command);
        response.finalize(true);
        user->Send(response);
        if (request.request == 1) {
            advancealtar::Manager::eplp(
                user,
                static_cast<advancealtar::EplpType::T>(request.command));
        }
        return 0;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }
    if (party->getManager() != user) {
        _SendEPLPError(party, request.command, 8);
        return 0;
    }
    if (party->GetEPLPState() != 2 && excludeEplpState(user)) {
        return 0;
    }

    _BroadCastPacket(party, request);
    if (request.request != 1) {
        return 0;
    }

    secretshop::SECRET_SHOP_DATA* const secret_shop = user->GetSecretShopData();
    if (secret_shop && secret_shop->IsOpen() &&
        secret_shop->IsCompleteBuy() != 1) {
        _SendEPLPError(party, request.command, 0x85);
        return 0;
    }

    int delay_sec = 3;
    if (request.command == 2) {
        delay_sec = 1;
    } else if (request.command > 2) {
        if (request.command == 3) {
            party->SetSelectedEPLPCmd(3);
            return 0;
        }
        _SendEPLPError(party, request.command, 1);
    }

    if (user->get_state() == 12) {
        WongWork::CBossTower* const tower = user->getBossTower();
        if (tower) {
            tower->handleEPLPCommand(request.command);
        }
        return 0;
    }
    if (user->get_state() == 10) {
        WongWork::CDeathTower* const tower = user->getDeathTower();
        if (!tower) {
            return LineFunc(37561, kEplpSignature, 0, 0);
        }
        tower->handleSelectEPLPCommand(user, delay_sec);
        return 0;
    }

    if (isReTurnToVillage(user)) {
        party->ReturnToVillage();
    } else {
        party->SelectEPLPCommand(delay_sec);
    }
    return 0;
}

// IDA_EA: 0x8233508
// IDA_NAME: _ZN22Dispatcher_EPLPCommand4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_EPLPCommand::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EPLPCommand::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233512
// IDA_NAME: _ZN22Dispatcher_EPLPCommand7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EPLPCommand::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EPLPCommand::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
