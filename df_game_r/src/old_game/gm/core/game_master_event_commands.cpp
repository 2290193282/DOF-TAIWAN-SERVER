#include "old_game/gm/core/game_master_event_commands.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_data/db/event/db_event_growth_weapon_reset.h"
#include "old_data/db/event/db_event_update_growth_weapon_timepiece.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/activity/schedule/bingo_runtime.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/cuser_lifecycle_support_types.h"
#include "infra/network/packet/packet_guard.h"

namespace game_master {

namespace {

// CStartEvent @0x84AB64E and BlueMarbleSet @0x84B235C split only on
// literal spaces.  Empty fields from consecutive spaces are retained.
std::vector<std::string> SplitLiteralSpaceFields(const std::string& command) {
    std::vector<std::string> fields;
    std::string remaining(command);
    while (true) {
        const std::string::size_type separator = remaining.find(" ", 0);
        if (separator == std::string::npos) {
            fields.push_back(remaining);
            return fields;
        }
        fields.push_back(remaining.substr(0, separator));
        remaining = remaining.substr(separator + 1, std::string::npos);
    }
}

const char* FieldOrEmpty(const std::vector<std::string>& fields,
                         size_t index) {
    return index < fields.size() ? fields[index].c_str() : "";
}

struct MonitorEventStartPayload {
    int event_id;
    int16_t first_parameter;
    int16_t second_parameter;
};

struct MonitorEventEndPayload {
    int event_id;
};

typedef char MonitorEventStartPayloadSizeMustBeEight[
    (sizeof(MonitorEventStartPayload) == 0x08) ? 1 : -1];
typedef char MonitorEventEndPayloadSizeMustBeFour[
    (sizeof(MonitorEventEndPayload) == 0x04) ? 1 : -1];

void GenerateClearRewardItems(WongWork::CItemGenerator& generator,
                              const WongWork::stGenerateRefData_t& reference,
                              WongWork::stGenerateResult_t& result) {
    // 0x84B2A08 performs 100,000 inclusive-index trials in both report
    // passes.  Keep the same count and preserve the result's LuckPoint state
    // across the normal and gold reports.
    for (int trial = 0; trial <= 99999; ++trial) {
        generator.generateRandomItem(reference, result);
    }
}

std::string FormatClearRewardItems(
    const std::map<int, int>& item_counts) {
    std::string report;
    bool first_entry = true;
    for (std::map<int, int>::const_iterator it = item_counts.begin();
         it != item_counts.end(); ++it) {
        if (first_entry) {
            report +=
                "    Item Index\t\t                     Item Name\t\tRarity\t\tCount\n";
            first_entry = false;
        }

        CItem* const item = G_CDataManager()->find_item(it->first);
        if (!item) {
            continue;
        }

        char line[1024] = {};
        const char* const name = item->GetItemName();
        std::sprintf(line,
                     "%14d\t\t%30s\t\t%6d\t\t%5d\n",
                     item->get_index(),
                     name ? name : "",
                     item->get_rarity(),
                     it->second);
        report += line;
    }
    return report;
}

std::map<int, int> CountClearRewardItems(
    const WongWork::stGenerateResult_t& result) {
    std::map<int, int> counts;
    for (std::vector<Inven_Item>::const_iterator item =
             result.items.begin();
         item != result.items.end(); ++item) {
        ++counts[static_cast<int>(item->item_id_)];
    }
    return counts;
}

}  // namespace

// IDA_EA: 0x82A8442
// IDA_NAME: _ZN11game_master11CStartEventC2Ev
// DEMANGLED: game_master::CStartEvent::CStartEvent(void)
CStartEvent::CStartEvent() : CCommand() {}

// IDA_EA: 0x84AB5B6
// IDA_NAME: _ZN11game_master11CStartEvent5parseEv
// DEMANGLED: game_master::CStartEvent::parse(void)
void CStartEvent::parse() {
    token_count_ = 0;
    event_id_ = 0;
    event_param_0_ = 0;
    event_param_1_ = 0;
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    getParam(command);
}

// IDA_EA: 0x84AB64E
// IDA_NAME: _ZN11game_master11CStartEvent8getParamESs
// DEMANGLED: game_master::CStartEvent::getParam(std::string)
void CStartEvent::getParam(std::string command) {
    const std::vector<std::string> fields = SplitLiteralSpaceFields(command);
    token_count_ = static_cast<int>(fields.size()) - 1;
    event_id_ = std::atoi(FieldOrEmpty(fields, 0));
    event_param_0_ = static_cast<int16_t>(std::atoi(FieldOrEmpty(fields, 1)));
    event_param_1_ = static_cast<int16_t>(std::atoi(FieldOrEmpty(fields, 2)));
}

// IDA_EA: 0x84AB886
// IDA_NAME: _ZN11game_master11CStartEvent7executeEv
// DEMANGLED: game_master::CStartEvent::execute(void)
void CStartEvent::execute() {
    CUser* const user = GetUser();
    if (!user || !user->isGMUser()) {
        return;
    }

    Packet_Monitor_Event_Start packet;
    const MonitorEventStartPayload payload = {
        event_id_, event_param_0_, event_param_1_};
    std::memcpy(packet.raw_0a, &payload, sizeof(payload));

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    if (proxy) {
        proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }
}

// IDA_EA: 0x82A845E
// IDA_NAME: _ZN11game_master9CEndEventC2Ev
// DEMANGLED: game_master::CEndEvent::CEndEvent(void)
CEndEvent::CEndEvent() : CCommand() {}

// IDA_EA: 0x84AB914
// IDA_NAME: _ZN11game_master9CEndEvent5parseEv
// DEMANGLED: game_master::CEndEvent::parse(void)
void CEndEvent::parse() {
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    event_id_ = std::atoi(command.c_str());
}

// IDA_EA: 0x84AB94C
// IDA_NAME: _ZN11game_master9CEndEvent7executeEv
// DEMANGLED: game_master::CEndEvent::execute(void)
void CEndEvent::execute() {
    CUser* const user = GetUser();
    if (!user || !user->isGMUser()) {
        return;
    }

    Packet_Monitor_Event_End packet;
    const MonitorEventEndPayload payload = {event_id_};
    std::memcpy(packet.raw_0a, &payload, sizeof(payload));

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    if (proxy) {
        proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }
}

// IDA_EA: 0x82A87A6
// IDA_NAME: _ZN11game_master18CInGameEventSetDayC2Ev
// DEMANGLED: game_master::CInGameEventSetDay::CInGameEventSetDay(void)
CInGameEventSetDay::CInGameEventSetDay() : CCommand() {}

// IDA_EA: 0x84B09D0
// IDA_NAME: _ZN11game_master18CInGameEventSetDay5parseEv
// DEMANGLED: game_master::CInGameEventSetDay::parse(void)
void CInGameEventSetDay::parse() {
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    day_ = std::atoi(command.c_str());
}

// IDA_EA: 0x84B0A08
// IDA_NAME: _ZN11game_master18CInGameEventSetDay7executeEv
// DEMANGLED: game_master::CInGameEventSetDay::execute(void)
void CInGameEventSetDay::execute() {
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        return;
    }

    EventClassify::CEventScriptMng* const event_scripts =
        data_manager->get_event_script_mng();
    if (event_scripts) {
        event_scripts->test_process_change_day(day_);
    }
}

// IDA_EA: 0x82A88AE
// IDA_NAME: _ZN11game_master13BlueMarbleSetC2Ev
// DEMANGLED: game_master::BlueMarbleSet::BlueMarbleSet(void)
BlueMarbleSet::BlueMarbleSet() : CCommand() {}

// IDA_EA: 0x84B22CA
// IDA_NAME: _ZN11game_master13BlueMarbleSet5parseEv
// DEMANGLED: game_master::BlueMarbleSet::parse(void)
void BlueMarbleSet::parse() {
    move_requested_ = 0;
    enter_reset_requested_ = 0;
    position_ = 0;
    token_count_ = 0;
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    getParam(command);
}

// IDA_EA: 0x84B235C
// IDA_NAME: _ZN11game_master13BlueMarbleSet8getParamESs
// DEMANGLED: game_master::BlueMarbleSet::getParam(std::string)
void BlueMarbleSet::getParam(std::string command) {
    const std::vector<std::string> fields = SplitLiteralSpaceFields(command);
    token_count_ = static_cast<int>(fields.size()) - 1;
    if (fields[0] == "move") {
        move_requested_ = 1;
    } else if (fields[0] == "enter") {
        enter_reset_requested_ = 1;
    }
    position_ = static_cast<uint8_t>(std::atoi(FieldOrEmpty(fields, 1)));
}

// IDA_EA: 0x84B25A4
// IDA_NAME: _ZN11game_master13BlueMarbleSet7executeEv
// DEMANGLED: game_master::BlueMarbleSet::execute(void)
void BlueMarbleSet::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    if (enter_reset_requested_) {
        user->resetBlueMarbleEnterCount();
        user->sendBlueMarbleEnterCount();
        return;
    }

    BlueMarble* const blue_marble = user->getBlueMarble();
    if (blue_marble && move_requested_ && position_) {
        blue_marble->movePosition(user, position_);
    }
}

// IDA_EA: 0x82A88E6
// IDA_NAME: _ZN11game_master17CGrowhWeaponEventC2Ev
// DEMANGLED: game_master::CGrowhWeaponEvent::CGrowhWeaponEvent(void)
CGrowhWeaponEvent::CGrowhWeaponEvent() : CCommand() {}

// IDA_EA: 0x84B28D6
// IDA_NAME: _ZN11game_master17CGrowhWeaponEvent5parseEv
// DEMANGLED: game_master::CGrowhWeaponEvent::parse(void)
void CGrowhWeaponEvent::parse() {
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    if (command == "gw") {
        reset_requested_ = 1;
    } else {
        reset_requested_ = 0;
        timepiece_ = std::atoi(command.c_str());
    }
}

// IDA_EA: 0x84B293C
// IDA_NAME: _ZN11game_master17CGrowhWeaponEvent7executeEv
// DEMANGLED: game_master::CGrowhWeaponEvent::execute(void)
void CGrowhWeaponEvent::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    GrowthWeaponEvent& event_data = *reinterpret_cast<GrowthWeaponEvent*>(
        user->getGrowthWeaponEventdata());
    if (reset_requested_) {
        event_data.event_value_ = 0;
        event_data.charac_no_ = 0;
        event_data.enabled_ = 0;
        event_data.timepiece_ = 0;
        event_data.infinity_weapon_id_ = 0;
        DB_EventGrowthWeaponReset::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()));
        return;
    }

    event_data.timepiece_ = static_cast<uint16_t>(timepiece_);
    event_data.infinity_weapon_id_ = 0;
    DB_EventUpdateGrowthWeaponTimepiece::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        event_data.timepiece_,
        0);
}

// IDA_EA: 0x82A8902
// IDA_NAME: _ZN11game_master16CClearRewardTestC2Ev
// DEMANGLED: game_master::CClearRewardTest::CClearRewardTest(void)
CClearRewardTest::CClearRewardTest() : CCommand() {}

// IDA_EA: 0x84B2A02
// IDA_NAME: _ZN11game_master16CClearRewardTest5parseEv
// DEMANGLED: game_master::CClearRewardTest::parse(void)
void CClearRewardTest::parse() {
    // The original body is a single return instruction.
}

// IDA_EA: 0x84B2A08
// IDA_NAME: _ZN11game_master16CClearRewardTest7executeEv
// DEMANGLED: game_master::CClearRewardTest::execute(void)
void CClearRewardTest::execute() {
    CUser* const user = GetUser();
    if (!user || !user->getCurCharacR() || user->get_state() != 5) {
        return;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return;
    }

    CDungeon* const dungeon = party->battle_field().GetDungeon();
    if (!dungeon) {
        return;
    }

    const int standard_level = dungeon->get_dimension_possible() <= 0
        ? dungeon->get_standard_level()
        : party->getStandardDimensionLevel();
    const int dungeon_difficulty = party->battle_field().get_dungeon_diff();
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager || !data_manager->getItemGenerator()) {
        return;
    }

    WongWork::CItemGenerator* const generator =
        data_manager->getItemGenerator()->getGenerator(
            WongWork::CItemGeneratorMgr::kClearReward);
    if (!generator) {
        return;
    }

    WongWork::stGenerateRefData_t reference;
    reference.clear_reward_scale = 1.0f;
    reference.generator_data.clear_reward.character_level_or_world_area =
        static_cast<uint8_t>(standard_level);
    reference.generator_data.clear_reward.dungeon_difficulty_or_world_index =
        static_cast<uint8_t>(dungeon_difficulty);
    reference.generator_data.clear_reward.item_probability_scale = 0.0f;
    reference.generator_data.clear_reward.clear_reward_drop_probability = 0;
    reference.generator_data.clear_reward.dungeon = dungeon;

    WongWork::stGenerateResult_t result;
    GenerateClearRewardItems(*generator, reference, result);
    printClient(user,
                "cltr_normal.txt",
                FormatClearRewardItems(CountClearRewardItems(result)).c_str());

    result.items.clear();
    reference.generator_data.clear_reward.clear_reward_drop_probability = 1;
    GenerateClearRewardItems(*generator, reference, result);
    printClient(user,
                "cltr_gold.txt",
                FormatClearRewardItems(CountClearRewardItems(result)).c_str());
}

// IDA_EA: 0x84B3398
// IDA_NAME: _ZN11game_master16CClearRewardTest11printClientEP5CUserPKcS4_
// DEMANGLED: game_master::CClearRewardTest::printClient(CUser *,char const*,char const*)
void CClearRewardTest::printClient(CUser* user,
                                   const char* filename,
                                   const char* contents) {
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 318);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(std::strlen(filename)));
    packet.put_str(filename, static_cast<int>(std::strlen(filename)));
    packet.put_int(static_cast<int>(std::strlen(contents)));
    packet.put_str(contents, static_cast<int>(std::strlen(contents)));
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82A893A
// IDA_NAME: _ZN11game_master12CBingoGmTestC2Ev
// DEMANGLED: game_master::CBingoGmTest::CBingoGmTest(void)
CBingoGmTest::CBingoGmTest() : CCommand() {}

// IDA_EA: 0x84B35C4
// IDA_NAME: _ZN11game_master12CBingoGmTest5parseEv
// DEMANGLED: game_master::CBingoGmTest::parse(void)
void CBingoGmTest::parse() {
    if (!parser_) {
        return;
    }

    std::string& command = parser_->command_tail();
    trim(command);
    requested_value_ = std::atoi(command.c_str());
}

// IDA_EA: 0x84B35FC
// IDA_NAME: _ZN11game_master12CBingoGmTest7executeEv
// DEMANGLED: game_master::CBingoGmTest::execute(void)
void CBingoGmTest::execute() {
    if (!GetUser()) {
        return;
    }
    if (requested_value_) {
        reqValue();
    } else {
        resetData();
    }
}

// IDA_EA: 0x84B363E
// IDA_NAME: _ZN11game_master12CBingoGmTest9resetDataEv
// DEMANGLED: game_master::CBingoGmTest::resetData(void)
void CBingoGmTest::resetData() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    BingoData* const data = user->getBingoData();
    data->clear();
    data->reward_state = 1;
    user->sendBingoData();
}

// IDA_EA: 0x84B368A
// IDA_NAME: _ZN11game_master12CBingoGmTest8reqValueEv
// DEMANGLED: game_master::CBingoGmTest::reqValue(void)
void CBingoGmTest::reqValue() {
    CUser* const user = GetUser();
    if (!user || !GlobalData::s_event_manager) {
        return;
    }

    CEventBase* const base =
        GlobalData::s_event_manager->GetRepeatEvent(106);
    if (!base) {
        return;
    }

    BingoEvent* const event = static_cast<BingoEvent*>(base);
    boost::shared_ptr<CBingo>& bingo = event->getBingoSystem();
    BingoData* const data = user->getBingoData();
    const int existing_line_count =
        static_cast<int>(static_cast<int8_t>(data->line_count));
    if (!bingo.get() || existing_line_count > 2) {
        return;
    }

    const unsigned int old_board = data->bingo_count;
    const int line_count = bingo->calNewValue(data->bingo_count,
                                              requested_value_);
    if (data->bingo_count != old_board) {
        data->reward_state = 1;
        int added_lines = line_count - existing_line_count;
        if (added_lines > 0) {
            if (added_lines > 3) {
                added_lines = 3;
            }
            data->line_count = static_cast<uint8_t>(line_count);
            for (int index = 0; index < added_lines; ++index) {
                event->sendOneMatchLineReward(*user);
            }
        }
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 451);
    packet.put_byte(requested_value_);
    packet.put_int(static_cast<int>(data->bingo_count));
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace game_master
