#include "common/manager/statistics_runtime.h"

#include <cstring>
#include <string>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x861002E
// IDA_NAME: _ZN21Packet_Cube_StatisticC2Ev
// DEMANGLED: Packet_Cube_Statistic::Packet_Cube_Statistic(void)
Packet_Cube_Statistic::Packet_Cube_Statistic()
    : header(0x0C33u, 0x17u),
      channel_no(0),
      charac_level(0),
      item_id(0),
      count(0),
      field(0) {}

// IDA_EA: 0x86100CA
// IDA_NAME: _ZN22Packet_Value_StatisticC2Ev
// DEMANGLED: Packet_Value_Statistic::Packet_Value_Statistic(void)
Packet_Value_Statistic::Packet_Value_Statistic()
    : header(0x17CAu, 0x86u), charac_level(0) {}

// IDA_EA: 0x86100F8
// IDA_NAME: _ZN28Packet_Circulation_StatisticC2Ev
// DEMANGLED: Packet_Circulation_Statistic::Packet_Circulation_Statistic(void)
Packet_Circulation_Statistic::Packet_Circulation_Statistic()
    : header(0x17CBu, 0xD0u), charac_level(0), channel_no(0) {}

// IDA_EA: 0x8610320
// IDA_NAME: _ZN15CValueStatistic16stValueStatistic5resetEv
// DEMANGLED: CValueStatistic::stValueStatistic::reset(void)
CValueStatistic::stValueStatistic*
CValueStatistic::stValueStatistic::reset() {
    std::memset(this, 0, sizeof(*this));
    return this;
}

// IDA_EA: 0x8610358
// IDA_NAME: _ZN15CValueStatisticC2Ev
// DEMANGLED: CValueStatistic::CValueStatistic(void)
CValueStatistic::CValueStatistic() {}

// IDA_EA: 0x8610872
// IDA_NAME: _ZN15CValueStatisticD2Ev
// DEMANGLED: CValueStatistic::~CValueStatistic()
CValueStatistic::~CValueStatistic() {}

// IDA_EA: 0x860D682
// IDA_NAME: _ZN15CValueStatistic17AddValueStatisticE21VALUE_STATISTIC_FIELDP5CUserj
// DEMANGLED: CValueStatistic::AddValueStatistic(VALUE_STATISTIC_FIELD,CUser *,unsigned int)
bool CValueStatistic::AddValueStatistic(VALUE_STATISTIC_FIELD field,
                                        CUser* user,
                                        unsigned int value) {
    const int field_index = static_cast<int>(field);
    const int charac_level = user->get_charac_level();
    if (field_index < 0 || field_index > 29 ||
        charac_level < oldsrv::config::kCharacterLevelMinimum ||
        charac_level > oldsrv::config::kCharacterLevelMaximum) {
        cMyTrace trace(
            "bool CValueStatistic::AddValueStatistic(VALUE_STATISTIC_FIELD, CUser*, unsigned int)",
            774,
            0);
        trace("VALUE_STATISTIC wrong index (%d,%d)",
              field_index,
              charac_level);
        return false;
    }

    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
    std::map<ENUM_SERVER_GROUP, stValueStatistic>::iterator found =
        values_.find(server_group);
    if (found != values_.end()) {
        found->second.values[charac_level][field_index] += value;
    } else {
        stValueStatistic statistic;
        std::memset(&statistic, 0, sizeof(statistic));
        statistic.values[charac_level][field_index] = value;
        values_.insert(std::make_pair(server_group, statistic));
    }
    return true;
}

// IDA_EA: 0x860D8A4
// IDA_NAME: _ZN15CValueStatistic20InsertValueStatisticEv
// DEMANGLED: CValueStatistic::InsertValueStatistic(void)
bool CValueStatistic::InsertValueStatistic() {
    for (std::map<ENUM_SERVER_GROUP, stValueStatistic>::iterator group =
             values_.begin();
         group != values_.end();
         ++group) {
        for (int level = oldsrv::config::kCharacterLevelMinimum;
             level <= oldsrv::config::kCharacterLevelMaximum;
             ++level) {
            bool has_value = false;
            for (int field_index = 0; field_index <= 29; ++field_index) {
                if (group->second.values[level][field_index]) {
                    has_value = true;
                    break;
                }
            }
            if (has_value) {
                Packet_Value_Statistic packet;
                packet.charac_level = level;
                std::memcpy(packet.values,
                            group->second.values[level],
                            sizeof(packet.values));
                GlobalData::s_statistic_proxy_mgr
                    ->GetServerProxy(group->first)
                    ->SendPacket(reinterpret_cast<char*>(&packet),
                                 sizeof(packet));
            }
        }
        group->second.reset();
    }
    return true;
}

// IDA_EA: 0x860D5EF
// IDA_NAME: _Z25GetInstanceValueStatisticv
// DEMANGLED: GetInstanceValueStatistic(void)
CValueStatistic* GetInstanceValueStatistic() {
    static CValueStatistic instance;
    return &instance;
}

// IDA_EA: 0x861036C
// IDA_NAME: _ZN21CCirculationStatistic22stCirculationStatistic5resetEv
// DEMANGLED: CCirculationStatistic::stCirculationStatistic::reset(void)
CCirculationStatistic::stCirculationStatistic*
CCirculationStatistic::stCirculationStatistic::reset() {
    std::memset(this, 0, sizeof(*this));
    return this;
}

// IDA_EA: 0x86103A4
// IDA_NAME: _ZN21CCirculationStatisticC2Ev
// DEMANGLED: CCirculationStatistic::CCirculationStatistic(void)
CCirculationStatistic::CCirculationStatistic() {}

// IDA_EA: 0x8610886
// IDA_NAME: _ZN21CCirculationStatisticD2Ev
// DEMANGLED: CCirculationStatistic::~CCirculationStatistic()
CCirculationStatistic::~CCirculationStatistic() {}

// IDA_EA: 0x860DAAA
// IDA_NAME: _ZN21CCirculationStatistic23AddCirculationStatisticE27CIRCULATION_STATISTIC_FIELDP5CUserj
// DEMANGLED: CCirculationStatistic::AddCirculationStatistic(CIRCULATION_STATISTIC_FIELD,CUser *,unsigned int)
bool CCirculationStatistic::AddCirculationStatistic(
    CIRCULATION_STATISTIC_FIELD field,
    CUser* user,
    unsigned int value) {
    const int field_index = static_cast<int>(field);
    const int charac_level = user->get_charac_level();
    if (field_index < 0 || field_index > 47 ||
        charac_level < oldsrv::config::kCharacterLevelMinimum ||
        charac_level > oldsrv::config::kCharacterLevelMaximum) {
        cMyTrace trace(
            "bool CCirculationStatistic::AddCirculationStatistic(CIRCULATION_STATISTIC_FIELD, CUser*, unsigned int)",
            923,
            0);
        trace("CIRCULATION_STATISTIC wrong index (%d,%d)",
              field_index,
              charac_level);
        return false;
    }

    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
    std::map<ENUM_SERVER_GROUP, stCirculationStatistic>::iterator found =
        values_.find(server_group);
    if (found != values_.end()) {
        found->second.values[charac_level][field_index] += value;
    } else {
        stCirculationStatistic statistic;
        std::memset(&statistic, 0, sizeof(statistic));
        statistic.values[charac_level][field_index] = value;
        values_.insert(std::make_pair(server_group, statistic));
    }
    return true;
}

// IDA_EA: 0x860DCCC
// IDA_NAME: _ZN21CCirculationStatistic26InsertCirculationStatisticEv
// DEMANGLED: CCirculationStatistic::InsertCirculationStatistic(void)
bool CCirculationStatistic::InsertCirculationStatistic() {
    for (std::map<ENUM_SERVER_GROUP, stCirculationStatistic>::iterator group =
             values_.begin();
         group != values_.end();
         ++group) {
        for (int level = oldsrv::config::kCharacterLevelMinimum;
             level <= oldsrv::config::kCharacterLevelMaximum;
             ++level) {
            bool has_value = false;
            for (int field_index = 0; field_index <= 47; ++field_index) {
                if (group->second.values[level][field_index]) {
                    has_value = true;
                    break;
                }
            }
            if (has_value) {
                Packet_Circulation_Statistic packet;
                packet.channel_no =
                    static_cast<int16_t>(G_CEnvironment()->get_channel_no());
                packet.charac_level = level;
                std::memcpy(packet.values,
                            group->second.values[level],
                            sizeof(packet.values));
                GlobalData::s_statistic_proxy_mgr
                    ->GetServerProxy(group->first)
                    ->SendPacket(reinterpret_cast<char*>(&packet),
                                 sizeof(packet));
            }
        }
        group->second.reset();
    }
    return true;
}

// IDA_EA: 0x860DA17
// IDA_NAME: _Z31GetInstanceCirculationStatisticv
// DEMANGLED: GetInstanceCirculationStatistic(void)
CCirculationStatistic* GetInstanceCirculationStatistic() {
    static CCirculationStatistic instance;
    return &instance;
}

// IDA_EA: 0x86103B8
// IDA_NAME: _ZN14CCubeStatisticC2Ev
// DEMANGLED: CCubeStatistic::CCubeStatistic(void)
CCubeStatistic::CCubeStatistic() = default;

// IDA_EA: 0x860DE96
// IDA_NAME: _ZN14CCubeStatistic13findCubeStuffEi
// DEMANGLED: CCubeStatistic::findCubeStuff(int)
bool CCubeStatistic::findCubeStuff(int item_group) {
    const std::map<std::string, int> group_names =
        *getItemGroupNameTable();
    const std::map<std::string, int>::const_iterator found =
        group_names.find("cube stuff");
    return found != group_names.end() && found->second == item_group;
}

// IDA_EA: 0x860DFB8
// IDA_NAME: _ZN14CCubeStatistic21collectCubeStatisticsEiiP5CUser20CUBE_STATISTIC_FIELD
// DEMANGLED: CCubeStatistic::collectCubeStatistics(int,int,CUser *,CUBE_STATISTIC_FIELD)
int CCubeStatistic::collectCubeStatistics(int item_id,
                                           int count,
                                           CUser* user,
                                           CUBE_STATISTIC_FIELD field) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) {
        return 0;
    }
    if (findCubeStuff(item->getItemGroupName())) {
        sendCubeStatistic(item_id, count, user, field);
    }
    CCubeStatistic* const statistic = GetInstanceCubeStatistic();
    if (statistic->findCubeStuff(item->getItemGroupName())) {
        return statistic->sendCubeStatistic(item_id, count, user, field);
    }
    return 0;
}

// IDA_EA: 0x860E07A
// IDA_NAME: _ZN14CCubeStatistic21collectCubeStatisticsEP5CItemiP5CUser20CUBE_STATISTIC_FIELD
// DEMANGLED: CCubeStatistic::collectCubeStatistics(CItem *,int,CUser *,CUBE_STATISTIC_FIELD)
void CCubeStatistic::collectCubeStatistics(CItem* item,
                                            int count,
                                            CUser* user,
                                            CUBE_STATISTIC_FIELD field) {
    if (item && findCubeStuff(item->getItemGroupName())) {
        sendCubeStatistic(item->get_index(), count, user, field);
    }
}

// IDA_EA: 0x860E0D8
// IDA_NAME: _ZN14CCubeStatistic17sendCubeStatisticEiiP5CUser20CUBE_STATISTIC_FIELD
// DEMANGLED: CCubeStatistic::sendCubeStatistic(int,int,CUser *,CUBE_STATISTIC_FIELD)
bool CCubeStatistic::sendCubeStatistic(int item_id,
                                        int count,
                                        CUser* user,
                                        CUBE_STATISTIC_FIELD field) {
    Packet_Cube_Statistic packet;
    packet.channel_no =
        static_cast<int16_t>(G_CEnvironment()->get_channel_no());
    packet.charac_level =
        static_cast<int16_t>(user->get_charac_level());
    packet.item_id = item_id;
    packet.count = count;
    packet.field = static_cast<uint8_t>(field);
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    return true;
}

// IDA_EA: 0x860DE50
// IDA_NAME: _Z24GetInstanceCubeStatisticv
// DEMANGLED: GetInstanceCubeStatistic(void)
CCubeStatistic* GetInstanceCubeStatistic() {
    static CCubeStatistic instance;
    return &instance;
}
