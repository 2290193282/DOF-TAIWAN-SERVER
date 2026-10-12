#ifndef OLDSRV_GLOBAL_APP_LOAD_CHECKER_H_
#define OLDSRV_GLOBAL_APP_LOAD_CHECKER_H_

#include <stdint.h>


// IDA class: CAppLoadChecker.  The original object is four queue baselines
// followed by four signed one-byte hysteresis states (20 bytes total).
class CAppLoadChecker {
public:
    // IDA_EA: 0x85FFC6A
    // IDA_NAME: _ZN15CAppLoadCheckerC2Ev
    // DEMANGLED: CAppLoadChecker::CAppLoadChecker(void)
    CAppLoadChecker();

    // IDA_EA: 0x85FFCB2
    // IDA_NAME: _ZN15CAppLoadChecker13checkLoad_dbqEi
    // DEMANGLED: CAppLoadChecker::checkLoad_dbq(int)
    int checkLoad_dbq(int dbq_count);

    // IDA_EA: 0x85FFEB0
    // IDA_NAME: _ZN15CAppLoadChecker16checkLoad_logdbqEi
    // DEMANGLED: CAppLoadChecker::checkLoad_logdbq(int)
    int checkLoad_logdbq(int logdbq_count);

    // IDA_EA: 0x86000B0
    // IDA_NAME: _ZN15CAppLoadChecker16checkLoad_timerqEi
    // DEMANGLED: CAppLoadChecker::checkLoad_timerq(int)
    int checkLoad_timerq(int timerq_count);

    // IDA_EA: 0x86002B0
    // IDA_NAME: _ZN15CAppLoadChecker17checkLoad_packetqEi
    // DEMANGLED: CAppLoadChecker::checkLoad_packetq(int)
    int checkLoad_packetq(int packetq_count);

    // IDA_EA: 0x86004B0
    // IDA_NAME: _ZN15CAppLoadChecker8setQueueEiiii
    // DEMANGLED: CAppLoadChecker::setQueue(int,int,int,int)
    CAppLoadChecker* setQueue(int dbq_count, int logdbq_count,
                              int timerq_count, int packetq_count);

    // IDA_EA: 0x86004D8
    // IDA_NAME: _ZN15CAppLoadChecker9CheckLoadEiiii
    // DEMANGLED: CAppLoadChecker::CheckLoad(int,int,int,int)
    int CheckLoad(int dbq_count, int logdbq_count, int netq_count,
                  int packetq_count);

    // IDA_EA: 0x8600580
    // IDA_NAME: _ZN15CAppLoadChecker9RequestDBEiiii
    // DEMANGLED: CAppLoadChecker::RequestDB(int,int,int,int)
    void RequestDB(int dbq_count, int logdbq_count, int netq_count,
                   int packetq_count);

private:
    int dbq_baseline_;
    int logdbq_baseline_;
    int timerq_baseline_;
    int packetq_baseline_;
    int8_t dbq_pressure_;
    int8_t logdbq_pressure_;
    int8_t timerq_pressure_;
    int8_t packetq_pressure_;
};

static_assert(sizeof(CAppLoadChecker) == 20u,
              "CAppLoadChecker ABI must remain 20 bytes");

// IDA_EA: 0x85FFC24
// IDA_NAME: _Z23CAppLoadCheckerInstancev
// DEMANGLED: CAppLoadCheckerInstance(void)
CAppLoadChecker* CAppLoadCheckerInstance();


#endif  // OLDSRV_GLOBAL_APP_LOAD_CHECKER_H_
