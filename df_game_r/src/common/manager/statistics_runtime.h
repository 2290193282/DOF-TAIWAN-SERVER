#ifndef OLDSRV_GLOBAL_STATISTICS_RUNTIME_H_
#define OLDSRV_GLOBAL_STATISTICS_RUNTIME_H_

#include <map>
#include <stdint.h>

#include "common/config.h"
#include "old_data/db/core/db_types.h"
#include "infra/network/packet/PacketHeader.h"

class CItem;
class CUser;
enum VALUE_STATISTIC_FIELD : int;
enum CIRCULATION_STATISTIC_FIELD : int;
enum CUBE_STATISTIC_FIELD : int;

#pragma pack(push, 1)
struct Packet_Cube_Statistic {
    PacketHeader header;
    int16_t channel_no;
    int16_t charac_level;
    int32_t item_id;
    int32_t count;
    uint8_t field;

    // IDA_EA: 0x861002E
    // IDA_NAME: _ZN21Packet_Cube_StatisticC2Ev
    // DEMANGLED: Packet_Cube_Statistic::Packet_Cube_Statistic(void)
    Packet_Cube_Statistic();
};

struct Packet_Value_Statistic {
    PacketHeader header;
    int32_t charac_level;
    uint32_t values[30];

    // IDA_EA: 0x86100CA
    // IDA_NAME: _ZN22Packet_Value_StatisticC2Ev
    // DEMANGLED: Packet_Value_Statistic::Packet_Value_Statistic(void)
    Packet_Value_Statistic();
};

struct Packet_Circulation_Statistic {
    PacketHeader header;
    int32_t charac_level;
    uint32_t values[48];
    int16_t channel_no;

    // IDA_EA: 0x86100F8
    // IDA_NAME: _ZN28Packet_Circulation_StatisticC2Ev
    // DEMANGLED: Packet_Circulation_Statistic::Packet_Circulation_Statistic(void)
    Packet_Circulation_Statistic();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Cube_Statistic) == 0x17u,
              "Packet_Cube_Statistic IDA ABI size must stay 0x17");
static_assert(sizeof(Packet_Value_Statistic) == 0x86u,
              "Packet_Value_Statistic IDA ABI size must stay 0x86");
static_assert(sizeof(Packet_Circulation_Statistic) == 0xD0u,
              "Packet_Circulation_Statistic IDA ABI size must stay 0xD0");

class CValueStatistic {
public:
    struct stValueStatistic {
        uint32_t values[
            oldsrv::config::kCharacterLevelTableCount][30];

        // IDA_EA: 0x8610320
        // IDA_NAME: _ZN15CValueStatistic16stValueStatistic5resetEv
        // DEMANGLED: CValueStatistic::stValueStatistic::reset(void)
        stValueStatistic* reset();
    };

    // IDA_EA: 0x8610358
    // IDA_NAME: _ZN15CValueStatisticC2Ev
    // DEMANGLED: CValueStatistic::CValueStatistic(void)
    CValueStatistic();

    // IDA_EA: 0x8610872
    // IDA_NAME: _ZN15CValueStatisticD2Ev
    // DEMANGLED: CValueStatistic::~CValueStatistic()
    ~CValueStatistic();

    // IDA_EA: 0x860D682
    // IDA_NAME: _ZN15CValueStatistic17AddValueStatisticE21VALUE_STATISTIC_FIELDP5CUserj
    // DEMANGLED: CValueStatistic::AddValueStatistic(VALUE_STATISTIC_FIELD,CUser *,unsigned int)
    bool AddValueStatistic(VALUE_STATISTIC_FIELD field,
                           CUser* user,
                           unsigned int value);

    // IDA_EA: 0x860D8A4
    // IDA_NAME: _ZN15CValueStatistic20InsertValueStatisticEv
    // DEMANGLED: CValueStatistic::InsertValueStatistic(void)
    bool InsertValueStatistic();

private:
    std::map<ENUM_SERVER_GROUP, stValueStatistic> values_;
};

static_assert(
    sizeof(CValueStatistic::stValueStatistic) ==
        sizeof(uint32_t) * oldsrv::config::kCharacterLevelTableCount * 30,
    "stValueStatistic configured storage size drift");

class CCirculationStatistic {
public:
    struct stCirculationStatistic {
        uint32_t values[
            oldsrv::config::kCharacterLevelTableCount][48];

        // IDA_EA: 0x861036C
        // IDA_NAME: _ZN21CCirculationStatistic22stCirculationStatistic5resetEv
        // DEMANGLED: CCirculationStatistic::stCirculationStatistic::reset(void)
        stCirculationStatistic* reset();
    };

    // IDA_EA: 0x86103A4
    // IDA_NAME: _ZN21CCirculationStatisticC2Ev
    // DEMANGLED: CCirculationStatistic::CCirculationStatistic(void)
    CCirculationStatistic();

    // IDA_EA: 0x8610886
    // IDA_NAME: _ZN21CCirculationStatisticD2Ev
    // DEMANGLED: CCirculationStatistic::~CCirculationStatistic()
    ~CCirculationStatistic();

    // IDA_EA: 0x860DAAA
    // IDA_NAME: _ZN21CCirculationStatistic23AddCirculationStatisticE27CIRCULATION_STATISTIC_FIELDP5CUserj
    // DEMANGLED: CCirculationStatistic::AddCirculationStatistic(CIRCULATION_STATISTIC_FIELD,CUser *,unsigned int)
    bool AddCirculationStatistic(CIRCULATION_STATISTIC_FIELD field,
                                 CUser* user,
                                 unsigned int value);

    // IDA_EA: 0x860DCCC
    // IDA_NAME: _ZN21CCirculationStatistic26InsertCirculationStatisticEv
    // DEMANGLED: CCirculationStatistic::InsertCirculationStatistic(void)
    bool InsertCirculationStatistic();

private:
    std::map<ENUM_SERVER_GROUP, stCirculationStatistic> values_;
};

static_assert(
    sizeof(CCirculationStatistic::stCirculationStatistic) ==
        sizeof(uint32_t) * oldsrv::config::kCharacterLevelTableCount * 48,
    "circulation statistic configured storage size drift");

class CCubeStatistic {
public:
    // IDA_EA: 0x86103B8
    // IDA_NAME: _ZN14CCubeStatisticC2Ev
    // DEMANGLED: CCubeStatistic::CCubeStatistic(void)
    CCubeStatistic();

    // IDA_EA: 0x860DE96
    // IDA_NAME: _ZN14CCubeStatistic13findCubeStuffEi
    // DEMANGLED: CCubeStatistic::findCubeStuff(int)
    bool findCubeStuff(int item_group);

    // IDA_EA: 0x860DFB8
    // IDA_NAME: _ZN14CCubeStatistic21collectCubeStatisticsEiiP5CUser20CUBE_STATISTIC_FIELD
    // DEMANGLED: CCubeStatistic::collectCubeStatistics(int,int,CUser *,CUBE_STATISTIC_FIELD)
    int collectCubeStatistics(int item_id,
                              int count,
                              CUser* user,
                              CUBE_STATISTIC_FIELD field);

    // IDA_EA: 0x860E07A
    // IDA_NAME: _ZN14CCubeStatistic21collectCubeStatisticsEP5CItemiP5CUser20CUBE_STATISTIC_FIELD
    // DEMANGLED: CCubeStatistic::collectCubeStatistics(CItem *,int,CUser *,CUBE_STATISTIC_FIELD)
    void collectCubeStatistics(CItem* item,
                               int count,
                               CUser* user,
                               CUBE_STATISTIC_FIELD field);

    // IDA_EA: 0x860E0D8
    // IDA_NAME: _ZN14CCubeStatistic17sendCubeStatisticEiiP5CUser20CUBE_STATISTIC_FIELD
    // DEMANGLED: CCubeStatistic::sendCubeStatistic(int,int,CUser *,CUBE_STATISTIC_FIELD)
    bool sendCubeStatistic(int item_id,
                           int count,
                           CUser* user,
                           CUBE_STATISTIC_FIELD field);
};

// IDA_EA: 0x860D5EF
// IDA_NAME: _Z25GetInstanceValueStatisticv
// DEMANGLED: GetInstanceValueStatistic(void)
CValueStatistic* GetInstanceValueStatistic();

// IDA_EA: 0x860DA17
// IDA_NAME: _Z31GetInstanceCirculationStatisticv
// DEMANGLED: GetInstanceCirculationStatistic(void)
CCirculationStatistic* GetInstanceCirculationStatistic();

// IDA_EA: 0x860DE50
// IDA_NAME: _Z24GetInstanceCubeStatisticv
// DEMANGLED: GetInstanceCubeStatistic(void)
CCubeStatistic* GetInstanceCubeStatistic();

#endif  // OLDSRV_GLOBAL_STATISTICS_RUNTIME_H_
