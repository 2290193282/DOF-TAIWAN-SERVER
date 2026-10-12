#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_VENDING_MACHINE_STATISTIC_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_VENDING_MACHINE_STATISTIC_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

#include <map>
#include <vector>

// IDA unit: 99_misc_global/CVendingMachineStatistic.  This B-owned producer
// accumulates CMD218 output and sends signal 99 to the existing B DB receiver.
class CVendingMachineStatistic {
public:
    // IDA_EA: 0x861062C
    // IDA_NAME: _ZN24CVendingMachineStatisticC2Ev
    // DEMANGLED: CVendingMachineStatistic::CVendingMachineStatistic(void)
    CVendingMachineStatistic();

    // IDA_EA: 0x8610694
    // IDA_NAME: _ZN24CVendingMachineStatisticD2Ev
    // DEMANGLED: CVendingMachineStatistic::~CVendingMachineStatistic()
    ~CVendingMachineStatistic();

    // IDA_EA: 0x822D714
    // IDA_NAME: _ZN24CVendingMachineStatistic11incUseCountEv
    // DEMANGLED: CVendingMachineStatistic::incUseCount(void)
    CVendingMachineStatistic* incUseCount();

    // IDA_EA: 0x860F5A4
    // IDA_NAME: _ZN24CVendingMachineStatistic13addOutputItemEmj
    // DEMANGLED: CVendingMachineStatistic::addOutputItem(unsigned long,unsigned int)
    int addOutputItem(unsigned long item_index, unsigned int item_count);

    // IDA_EA: 0x860F6DE
    // IDA_NAME: _ZN24CVendingMachineStatistic16updateDataIntoDBEv
    // DEMANGLED: CVendingMachineStatistic::updateDataIntoDB(void)
    void updateDataIntoDB();

private:
    // IDA_EA: 0x86105AC
    // IDA_NAME: _ZN24CVendingMachineStatistic5_initEv
    // DEMANGLED: CVendingMachineStatistic::_init(void)
    int _init();

    std::vector<unsigned long> excluded_item_indices_;  // IDA +0x00
    uint32_t use_count_;                                // IDA +0x0c
    std::map<unsigned long, uint32_t> output_counts_;   // IDA +0x10
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
// IDA's singleton occupies 0x28 bytes at 0x9473380; the following BSS
// singleton begins at 0x94733a8.  The native i386 std::map object is 0x18.
static_assert(sizeof(CVendingMachineStatistic) == 0x28u,
              "CVendingMachineStatistic IDA ABI size must stay 0x28");
#endif

// IDA_EA: 0x860F8AD
// IDA_NAME: _Z34GetInstanceVendingMachineStatisticv
// DEMANGLED: GetInstanceVendingMachineStatistic()
CVendingMachineStatistic* GetInstanceVendingMachineStatistic();

// IDA unit: 13_database_dispatch/DB_UpdateVendingMachineStatistic.
class DB_UpdateVendingMachineStatistic : public DBDispatcher {
public:
    DB_UpdateVendingMachineStatistic();
    virtual ~DB_UpdateVendingMachineStatistic();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_VENDING_MACHINE_STATISTIC_H_
