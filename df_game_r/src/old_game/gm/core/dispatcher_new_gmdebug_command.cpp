#include "old_game/gm/core/dispatcher_new_gmdebug_command.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "infra/log/format/trace.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/gm/core/game_master_mgr_runtime.h"
#include "old_game/gm/core/game_master_runtime_abi.h"
#include "old_game/gm/core/packet_dispatcher_debug_command.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/world_packet_structs.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_New_Gmdebug_Command::dispatch_sig(CUser*, PacketBuf&)";

void SendLegacyCommandResult(CUser* user, PacketGuard& packet, int command) {
    packet.put_header(1, ENUM_CMDPACKET_GMDEBUG_COMMAND);
    packet.put_byte(1);
    packet.put_byte(command);
    packet.finalize(true);
    user->Send(packet);
}

const char* Token(const std::vector<std::string>& tokens, size_t index) {
    return tokens[index].c_str();
}

}  // namespace

// IDA_EA: 0x859663C
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_CommandC2Ev
// DEMANGLED: Dispatcher_New_Gmdebug_Command::Dispatcher_New_Gmdebug_Command(void)
Dispatcher_New_Gmdebug_Command::Dispatcher_New_Gmdebug_Command()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241E2A
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_CommandD2Ev
// DEMANGLED: Dispatcher_New_Gmdebug_Command::~Dispatcher_New_Gmdebug_Command()
// IDA_EA: 0x8241E5A
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_CommandD0Ev
// DEMANGLED: Dispatcher_New_Gmdebug_Command::~Dispatcher_New_Gmdebug_Command()
Dispatcher_New_Gmdebug_Command::~Dispatcher_New_Gmdebug_Command() {}

// IDA_EA: 0x820BA90
// IDA_NAME: _Z19DoUserDefineCommandP5CUseriPKc
// DEMANGLED: DoUserDefineCommand(CUser *,int,char const *)
void DoUserDefineCommand(CUser* user, int command_id, const char* command) {
    game_master::CParser parser;
    parser.set_user(user);
    std::sprintf(parser.raw_command_buffer(), "%d %s", command_id, command);
    parser.parse();

    game_master::CCommand* const game_master_command =
        GlobalData::s_GameMasterMgr->GetCommand(parser.command_id());
    if (game_master_command) {
        game_master_command->SetParser(&parser);
        GlobalData::s_GameMasterMgr->HandleRequest(game_master_command);
    }
}

// IDA_EA: 0x820BB68
// IDA_NAME: _Z28SendVillageAttackedGMCommandP5CUseri
// DEMANGLED: SendVillageAttackedGMCommand(CUser *,int)
char SendVillageAttackedGMCommand(CUser* user, int value) {
    Packet_VillageAttackedGMCommand packet;
    packet.uid = user->GetUID();
    packet.charac_no = user->get_charac_no(-1);
    packet.value = value;

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    return proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x820BBDE
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_Command12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_New_Gmdebug_Command::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_New_Gmdebug_Command::dispatch_sig(CUser* user, PacketBuf& packet) {
    char* const raw_packet = packet.get_packet(0);
    if (!user->getCurCharacR()) {
        return LineFunc(43452, kDispatchSignature, 0, 0);
    }
    if (!GlobalData::s_pGMAccounts_->isGM(
            static_cast<unsigned int>(user->get_acc_id()))) {
        return 0;
    }

    cMyTrace trace(kDispatchSignature, 43460, 0);
    trace("GM Pass Complete!");

    std::vector<std::string> tokens;
    const int command_length = *reinterpret_cast<int*>(raw_packet + 13);
    raw_packet[command_length + 17] = 0;
    user->getSeparatedTextUnits(tokens, raw_packet + 13, " ", 0);
    while (tokens.size() <= 9u) {
        tokens.push_back("");
    }

    PacketGuard unused_packet;
    DisPatcher_DebugCommand legacy_dispatcher;
    if (user->isGMUser() == 1) {
        game_master::CParser parser;
        parser.set_user(user);
        parser.token_vector() = tokens;

        game_master::CCommand* const command =
            GlobalData::s_GameMasterMgr->GetCommand(tokens[0]);
        if (command) {
            command->SetParser(&parser);
            GlobalData::s_GameMasterMgr->HandleRequest(command);
        } else {
            oldGmRoutine(user, packet, tokens);
        }
    }
    return 0;
}

// IDA_EA: 0x820BF2C
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_Command12oldGmRoutineEP5CUserR9PacketBufRSt6vectorISsSaISsEE
// DEMANGLED: Dispatcher_New_Gmdebug_Command::oldGmRoutine(CUser *,PacketBuf &,std::vector<std::string,std::allocator<std::string>> &)
int Dispatcher_New_Gmdebug_Command::oldGmRoutine(
    CUser* user,
    PacketBuf& packet,
    std::vector<std::string>& tokens) {
    (void)packet;
    DisPatcher_DebugCommand legacy_dispatcher;
    PacketGuard response_packet;
    const std::string& command = tokens[0];

    if (command == "lawless") {
        const int value = std::atoi(Token(tokens, 1));
        if (value > 0 && value <= 9999) {
            user->SetCurCharacChaosPoint(value);
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
            SendLegacyCommandResult(user, response_packet, 100);
        }
        return 0;
    }
    if (command == "bal") {
        const int value = std::atoi(Token(tokens, 1));
        if (value > 0 && value <= 9999) {
            user->setCurCharacHelpAbuseComputedRatio(
                static_cast<unsigned int>(100 * value));
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
            SendLegacyCommandResult(user, response_packet, 101);
        }
        return 0;
    }
    if (command == "questclear2" || command == "q") {
        if (tokens[1] != "on" && tokens[1] != "off") {
            const int quest_index = std::atoi(Token(tokens, 1));
            if (quest_index > 0 && quest_index <= 99999) {
                user->setGmQuestFlag(true);
                user->quest_action(
                    ENUM_CMDPACKET_SET_QUEST_TRIGGER, quest_index, 0, 0);
                SendLegacyCommandResult(user, response_packet, 102);
            }
        }
        return 0;
    }
    if (command == "troublemaker") {
        if (tokens[1] == "on") {
            user->setCurrCharacBlackCount(100u);
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
            SendLegacyCommandResult(user, response_packet, 104);
        } else if (tokens[1] == "off") {
            user->setCurrCharacBlackCount(0u);
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
            SendLegacyCommandResult(user, response_packet, 104);
        }
        return 0;
    }
    if (command == "fatigue") {
        const int value = std::atoi(Token(tokens, tokens[1] == "set" ? 2 : 1));
        if (tokens[1] == "set") {
            if (value < 10000) {
                user->setCurCharacFatigue(
                    user->getCurCharacMaxFatigue() - value);
                user->SendFatigue();
                SendLegacyCommandResult(user, response_packet, 105);
            }
        } else if (value < 10000 && value > 0) {
            user->FatigueUp(value);
            user->SendFatigue();
        }
        return 0;
    }
    if (command == "show" && tokens[1] == "lp") {
        char text[255] = {};
        std::sprintf(text,
                     "remained luck point : (%d) ",
                     user->GetCurCharacLuckPoint());
        response_packet.put_header(0, 12);
        response_packet.put_byte(0);
        response_packet.put_short(0);
        response_packet.put_byte(0);
        response_packet.put_int(static_cast<int>(std::strlen(text)));
        response_packet.put_str(text, static_cast<int>(std::strlen(text)));
        response_packet.finalize(true);
        user->Send(response_packet);
        return 0;
    }
    if (command == "max" && tokens[1] == "lp") {
        user->IncCurCharacLuckPoint(100000);
        user->enableSaveCharacStat();
        char text[] = "100000 luck point!!";
        response_packet.put_header(0, 12);
        response_packet.put_byte(0);
        response_packet.put_short(0);
        response_packet.put_byte(0);
        response_packet.put_int(static_cast<int>(sizeof(text) - 1));
        response_packet.put_str(text, static_cast<int>(sizeof(text) - 1));
        response_packet.finalize(true);
        user->Send(response_packet);
        return 0;
    }
    if (command == "item") {
        legacy_dispatcher._debugCommandAddItem(user,
                                                static_cast<unsigned long>(
                                                    std::atoi(Token(tokens, 1))),
                                                100);
        return 0;
    }
    if (command == "coin") {
        const int deduction = std::atoi(Token(tokens, 1));
        const int coin = static_cast<int>(user->getCurCharacInvenR()->GetCoin());
        const int remaining = coin - deduction < 0 ? 0 : coin - deduction;
        user->getCurCharacInvenW()->SetCoin(static_cast<unsigned int>(remaining));
        if (!deduction) {
            user->getCurCharacInvenW()->SetCoin(0u);
            user->getCurCharacInvenW()->SetPayCoin(0u);
            user->getCurCharacInvenW()->SetEventCoin(0u);
        }
        user->SendCoin();
        return 0;
    }
    if (command == "use" && tokens[1] == "item") {
        int item_space = std::atoi(Token(tokens, 2));
        const int count = std::atoi(Token(tokens, 3));
        if (item_space <= 0 || item_space > 6) {
            return 0;
        }
        item_space += 2;
        for (int index = 0; index < count; ++index) {
            user->increase_status(static_cast<short>(item_space));
        }
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), item_space);
        return 0;
    }
    if (command == "npcfav") {
        std::string arguments = tokens[1] + " " + tokens[2];
        DoUserDefineCommand(user, 36, arguments.c_str());
        return 0;
    }
    if (command == "favterm") {
        std::string arguments = tokens[1] + " " + tokens[2];
        DoUserDefineCommand(user, 35, arguments.c_str());
        return 0;
    }
    if (command == "mdt" || command == "monsterdroptest" ||
        command == "startmdt" || command == "endmdt" ||
        command == "goldmdt") {
        char arguments[255] = {};
        int mode = 0;
        if (command == "startmdt") {
            mode = 1;
        } else if (command == "endmdt") {
            mode = 2;
        } else if (command == "goldmdt") {
            mode = 3;
        }
        std::sprintf(arguments, "%d %s", mode, Token(tokens, 1));
        DoUserDefineCommand(user, 75, arguments);
        return 0;
    }
    if (command == "cli" || command == "checklottoitem") {
        char arguments[255] = {};
        if (tokens[1] == "ven") {
            std::sprintf(arguments, "1 %s", Token(tokens, 2));
            DoUserDefineCommand(user, 77, arguments);
        } else {
            std::sprintf(arguments, "%s %s", Token(tokens, 1), Token(tokens, 2));
            DoUserDefineCommand(user, 76, arguments);
        }
        return 0;
    }
    if (command == "sep") {
        if (tokens[1] != "upgrade" && tokens[1] != "up") {
            return 0;
        }

        char text[255] = {};
        if (tokens[2] == "success" || tokens[2] == "s") {
            if (user->getUpgradeSeparateTrueFlag()) {
                user->setUpgradeSeparateTrueFlag(0);
                std::sprintf(text, "sep upgrade success - %s", "end");
            } else {
                user->setUpgradeSeparateTrueFlag(1);
                user->setUpgradeSeparateFalseFlag(0);
                std::sprintf(text, "sep upgrade success - %s", "ok");
            }
        } else if (tokens[2] == "fail" || tokens[2] == "f") {
            if (user->getUpgradeSeparateFlaseFlag()) {
                user->setUpgradeSeparateFalseFlag(0);
                std::sprintf(text, "sep upgrade fail - %s", "end");
            } else {
                user->setUpgradeSeparateTrueFlag(0);
                user->setUpgradeSeparateFalseFlag(1);
                std::sprintf(text, "sep upgrade fail - %s", "ok");
            }
        }

        response_packet.put_header(0, 12);
        response_packet.put_byte(0);
        response_packet.put_short(0);
        response_packet.put_byte(0);
        response_packet.put_int(static_cast<int>(std::strlen(text)));
        response_packet.put_str(text, static_cast<int>(std::strlen(text)));
        response_packet.finalize(true);
        user->Send(response_packet);
        return 0;
    }
    if (command == "townatk") {
        if (tokens[1] == "success") {
            SendVillageAttackedGMCommand(user, 1);
        } else if (tokens[1] == "fail") {
            SendVillageAttackedGMCommand(user, 2);
        } else if (tokens[1] == "end") {
            SendVillageAttackedGMCommand(user, 3);
        } else {
            DoUserDefineCommand(user, 30, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "rec") {
        if (tokens[1] == "start") {
            DoUserDefineCommand(user, 18, Token(tokens, 1));
        } else if (tokens[1] == "stop") {
            DoUserDefineCommand(user, 19, Token(tokens, 1));
        } else if (tokens[1] == "run") {
            DoUserDefineCommand(user, 20, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "spd") {
        DoUserDefineCommand(user, 9, Token(tokens, 1));
        return 0;
    }
    if (command == "durable") {
        char arguments[255] = {};
        std::sprintf(arguments, "%s %s", Token(tokens, 1), Token(tokens, 2));
        DoUserDefineCommand(user, 41, arguments);
        return 0;
    }
    if (command == "initstat") {
        DoUserDefineCommand(user, 63, Token(tokens, 1));
        return 0;
    }
    if (command == "guilddebug") {
        DoUserDefineCommand(user, 66, Token(tokens, 1));
        return 0;
    }
    if (command == "delguild") {
        DoUserDefineCommand(user, 44, Token(tokens, 1));
        return 0;
    }
    if (command == "changename") {
        DoUserDefineCommand(user, 45, Token(tokens, 1));
        return 0;
    }
    if (command == "startevent") {
        char arguments[255] = {};
        std::sprintf(arguments,
                     "%s %s %s",
                     Token(tokens, 1),
                     Token(tokens, 2),
                     Token(tokens, 3));
        DoUserDefineCommand(user, 46, arguments);
        return 0;
    }
    if (command == "endevent") {
        DoUserDefineCommand(user, 47, Token(tokens, 1));
        return 0;
    }
    if (command == "shopcheck") {
        DoUserDefineCommand(user, 48, Token(tokens, 1));
        return 0;
    }
    if (command == "shopnpc") {
        DoUserDefineCommand(user, 49, Token(tokens, 1));
        return 0;
    }
    if (command == "cargo") {
        if (tokens[1] == "delete") {
            DoUserDefineCommand(user, 54, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "guildfund") {
        DoUserDefineCommand(user, 65, Token(tokens, 1));
        return 0;
    }
    if (command == "dropitem") {
        DoUserDefineCommand(user, 52, Token(tokens, 1));
        return 0;
    }
    if (command == "premiumdelete" || command == "pd") {
        DoUserDefineCommand(user, 60, Token(tokens, 1));
        return 0;
    }
    if (command == "mailitem") {
        DoUserDefineCommand(user, 61, Token(tokens, 1));
        return 0;
    }
    if (command == "ci") {
        if (tokens[1] == "true") {
            DoUserDefineCommand(user, 109, Token(tokens, 2));
        } else if (tokens[1] == "false") {
            DoUserDefineCommand(user, 109, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "ritemclear") {
        DoUserDefineCommand(user, 118, Token(tokens, 1));
        return 0;
    }
    if (command == "difficulty") {
        DoUserDefineCommand(user, 120, Token(tokens, 1));
        return 0;
    }
    if (command == "bluemarble") {
        char arguments[255] = {};
        std::sprintf(arguments, "%s %s", Token(tokens, 1), Token(tokens, 2));
        DoUserDefineCommand(user, 125, arguments);
        return 0;
    }
    if (command == "dirty") {
        DoUserDefineCommand(user, 31, Token(tokens, 1));
        return 0;
    }
    if (command == "sumhell") {
        DoUserDefineCommand(user, 21, Token(tokens, 1));
        return 0;
    }
    if (command == "sdale") {
        DoUserDefineCommand(user, 28, Token(tokens, 1));
        return 0;
    }
    if (command == "cbind" || command == "checkbind") {
        DoUserDefineCommand(user, 78, Token(tokens, 1));
        return 0;
    }
    if (command == "oneadayitem") {
        DoUserDefineCommand(user, 82, Token(tokens, 1));
        return 0;
    }
    if (command == "craneDay") {
        DoUserDefineCommand(user, 121, Token(tokens, 1));
        return 0;
    }
    if (command == "cinven") {
        DoUserDefineCommand(user, 70, Token(tokens, 1));
        return 0;
    }
    if (command == "pickup") {
        char arguments[255] = {};
        std::sprintf(arguments, "%s %s", Token(tokens, 1), Token(tokens, 2));
        DoUserDefineCommand(user, 71, arguments);
        return 0;
    }
    if (command == "refund") {
        char arguments[255] = {};
        std::sprintf(arguments, "%s %s", Token(tokens, 1), Token(tokens, 2));
        DoUserDefineCommand(user, 72, arguments);
        return 0;
    }
    if (command == "tb") {
        if (tokens[1] == "reset") {
            DoUserDefineCommand(user, 93, Token(tokens, 1));
        } else if (tokens[1] == "adjust") {
            DoUserDefineCommand(user, 94, Token(tokens, 1));
        } else if (tokens[1] == "no") {
            DoUserDefineCommand(user, 95, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "achieve" || command == "ac") {
        if (tokens[1] == "reset") {
            DoUserDefineCommand(user, 96, Token(tokens, 1));
        } else if (tokens[1] == "report") {
            DoUserDefineCommand(user, 97, Token(tokens, 1));
        } else if (tokens[1] == "clear") {
            DoUserDefineCommand(user, 98, Token(tokens, 2));
        } else if (tokens[1] == "all") {
            DoUserDefineCommand(user, 99, Token(tokens, 1));
        } else if (tokens[1] == "no") {
            DoUserDefineCommand(user, 100, Token(tokens, 1));
        }
        return 0;
    }
    if (command == "power" && tokens[1] == "win") {
        DoUserDefineCommand(user, 43, Token(tokens, 2));
        return 0;
    }
    if (command == "ingameday" || command == "day") {
        DoUserDefineCommand(user, 102, Token(tokens, 1));
        return 0;
    }
    if (command == "itemdrop") {
        DoUserDefineCommand(user, 111, Token(tokens, 1));
        return 0;
    }
    if (command == "upcard") {
        DoUserDefineCommand(user, 112, Token(tokens, 1));
        return 0;
    }
    if (command == "dontsend") {
        if (tokens[1] == "on") {
            user->SetDebugCommand(1);
        } else if (tokens[1] == "off") {
            user->SetDebugCommand(0);
        }
        return 0;
    }
    if (command == "mission") {
        user->SetMissionParameter(std::atoi(Token(tokens, 1)),
                                  std::atoi(Token(tokens, 2)));
        user->send_MissionList();
        return 0;
    }
    if (command == "newdailymission") {
        user->ResetDailyMission();
        user->send_MissionList();
        return 0;
    }
    if (command == "clearmission") {
        user->ClearMission(static_cast<short>(std::atoi(Token(tokens, 1))));
        user->send_MissionList();
        user->send_pvp_record();
        return 0;
    }
    if (command == "clearmissionkind") {
        user->ClearMissionKind(
            static_cast<short>(std::atoi(Token(tokens, 1))));
        user->send_MissionList();
        user->send_pvp_record();
        return 0;
    }
    if (command == "resetmissionlist") {
        user->ResetMissionList();
        user->send_MissionList();
        return 0;
    }
    if (command == "resetdaily") {
        G_GameWorld()->ResetAllFatigue();
        G_GameWorld()->ReselectDailyQuest();
        G_GameWorld()->AddDailyItem();
        return 0;
    }
    if (command == "combomission") {
        const int mission_index = user->getCurCharacGrowType() ? 32 : 31;
        user->Update_ComboMission(
            mission_index, static_cast<short>(std::atoi(Token(tokens, 1))));
        return 0;
    }
    if (command == "qp") {
        user->setQuestPoint(std::atoi(Token(tokens, 1)));
        return 0;
    }
    if (command == "inde" && tokens[1] == "item" &&
        tokens[2] == "drop") {
        DoUserDefineCommand(user,
                             119,
                             user->getForceDropFlag() == 1 ? "off" : "on");
        return 0;
    }
    if (command == "open") {
        if (tokens[1] == "sd") {
            GlobalData::s_DailyScheduleManager->GM_OpenSouthernDale();
        } else if (tokens[1] == "boss") {
            GlobalData::s_DailyScheduleManager->GM_OpenBossTower();
        } else if (tokens[1] == "aa") {
            GlobalData::s_DailyScheduleManager->GM_OpenAdvanceAltar();
        }
        return 0;
    }
    if (command == "close") {
        if (tokens[1] == "sd") {
            GlobalData::s_DailyScheduleManager->GM_CloseSouthernDale();
        } else if (tokens[1] == "boss") {
            GlobalData::s_DailyScheduleManager->GM_CloseBossTower();
        } else if (tokens[1] == "aa") {
            GlobalData::s_DailyScheduleManager->GM_CloseAdvanceAltar();
        }
        return 0;
    }
    if (command == "quickon" || command == "qon") {
        CParty* const party = user->GetParty();
        if (party && party->get_member_count() > 1) {
            party->send_quick_party_matching_result(true, 0);
            party->set_quick_party(true);
        }
        return 0;
    }
    if (command == "quickoff" || command == "qoff") {
        CParty* const party = user->GetParty();
        if (party && party->get_member_count() > 1) {
            party->send_quick_party_matching_result(false, 0);
            party->set_quick_party(false);
        }
        return 0;
    }
    if (command == "quickreward" || command == "qr") {
        const int buff_type = std::atoi(Token(tokens, 1));
        if (buff_type > 10) {
            return 0;
        }

        CParty* const party = user->GetParty();
        if (party && party->get_member_count() > 1) {
            party->set_gm_random_buff_type(
                static_cast<QuickParty::RandomBuffType>(buff_type));

            char text[255] = {};
            std::sprintf(text,
                         "GM QUICK PARTY RANDOM BUFF (%d) TYPE!!",
                         buff_type);

            PacketGuard quick_reward_packet;
            quick_reward_packet.put_header(0, 12);
            quick_reward_packet.put_byte(0);
            quick_reward_packet.put_short(0);
            quick_reward_packet.put_byte(0);
            quick_reward_packet.put_int(
                static_cast<int>(std::strlen(text)));
            quick_reward_packet.put_str(text,
                                        static_cast<int>(std::strlen(text)));
            quick_reward_packet.finalize(true);
            party->send_to_party(quick_reward_packet);
        }
        return 0;
    }

    return 0;
}

// IDA_EA: 0x8233B82
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_Command4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_New_Gmdebug_Command::read(PacketBuf &,MSG_BASE &)
int Dispatcher_New_Gmdebug_Command::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B8C
// IDA_NAME: _ZN30Dispatcher_New_Gmdebug_Command7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_New_Gmdebug_Command::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_New_Gmdebug_Command::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
