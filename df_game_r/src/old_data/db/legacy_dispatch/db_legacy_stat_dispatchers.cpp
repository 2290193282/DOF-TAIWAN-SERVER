#include "old_data/db/legacy_dispatch/db_legacy_stat_dispatchers.h"

#include "infra/network/packet/stream.h"


// IDA_EA: 0x8417A12
// IDA_NAME: _ZN15DB_StatBreakJarC2Ev
// DEMANGLED: DB_StatBreakJar::DB_StatBreakJar(void)
DB_StatBreakJar::DB_StatBreakJar() {}

// IDA_EA: 0x8417A2E
// IDA_NAME: _ZN15DB_StatBreakJarD2Ev
// DEMANGLED: DB_StatBreakJar::~DB_StatBreakJar()
// IDA_EA: 0x8417A5E
// IDA_NAME: _ZN15DB_StatBreakJarD0Ev
// DEMANGLED: DB_StatBreakJar::~DB_StatBreakJar() deleting destructor
DB_StatBreakJar::~DB_StatBreakJar() {}

// IDA_EA: 0x8417A7C
// IDA_NAME: _ZN15DB_StatBreakJar8dispatchEiiP6Stream
// DEMANGLED: DB_StatBreakJar::dispatch(int,int,Stream *)
bool DB_StatBreakJar::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MSG_STAT_BREAKJAR* message = stream->GetOutBuffer<MSG_STAT_BREAKJAR>();
    return message != 0 && LogWrite(message) == 1;
}

// IDA_EA: 0x8417AFA
// IDA_NAME: _ZN15DB_StatBreakJar8LogWriteEP17MSG_STAT_BREAKJAR
// DEMANGLED: DB_StatBreakJar::LogWrite(MSG_STAT_BREAKJAR *)
int DB_StatBreakJar::LogWrite(MSG_STAT_BREAKJAR* message) {
    (void)message;
    return 1;
}

// IDA_EA: 0x8417B82
// IDA_NAME: _ZN18DB_StatCompositionC2Ev
// DEMANGLED: DB_StatComposition::DB_StatComposition(void)
DB_StatComposition::DB_StatComposition() {}

// IDA_EA: 0x8417B9E
// IDA_NAME: _ZN18DB_StatCompositionD2Ev
// DEMANGLED: DB_StatComposition::~DB_StatComposition()
// IDA_EA: 0x8417BCE
// IDA_NAME: _ZN18DB_StatCompositionD0Ev
// DEMANGLED: DB_StatComposition::~DB_StatComposition() deleting destructor
DB_StatComposition::~DB_StatComposition() {}

// IDA_EA: 0x8417BEC
// IDA_NAME: _ZN18DB_StatComposition8dispatchEiiP6Stream
// DEMANGLED: DB_StatComposition::dispatch(int,int,Stream *)
bool DB_StatComposition::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MSG_STAT_COMPOSITON* message =
        stream->GetOutBuffer<MSG_STAT_COMPOSITON>();
    return message != 0 && LogWrite(message) == 1;
}

// IDA_EA: 0x8417C6A
// IDA_NAME: _ZN18DB_StatComposition8LogWriteEP19MSG_STAT_COMPOSITON
// DEMANGLED: DB_StatComposition::LogWrite(MSG_STAT_COMPOSITON *)
int DB_StatComposition::LogWrite(MSG_STAT_COMPOSITON* message) {
    (void)message;
    return 1;
}

// IDA_EA: 0x8417B04
// IDA_NAME: _ZN14DB_StatCoinUseC2Ev
// DEMANGLED: DB_StatCoinUse::DB_StatCoinUse(void)
DB_StatCoinUse::DB_StatCoinUse() {}

// IDA_EA: 0x8417B20
// IDA_NAME: _ZN14DB_StatCoinUseD2Ev
// DEMANGLED: DB_StatCoinUse::~DB_StatCoinUse()
// IDA_EA: 0x8417B50
// IDA_NAME: _ZN14DB_StatCoinUseD0Ev
// DEMANGLED: DB_StatCoinUse::~DB_StatCoinUse() deleting destructor
DB_StatCoinUse::~DB_StatCoinUse() {}

// IDA_EA: 0x8417B6E
// IDA_NAME: _ZN14DB_StatCoinUse8dispatchEiiP6Stream
// DEMANGLED: DB_StatCoinUse::dispatch(int,int,Stream *)
bool DB_StatCoinUse::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}

// IDA_EA: 0x8417B78
// IDA_NAME: _ZN14DB_StatCoinUse8LogWriteER16MSG_STAT_USECOIN
// DEMANGLED: DB_StatCoinUse::LogWrite(MSG_STAT_USECOIN &)
int DB_StatCoinUse::LogWrite(MSG_STAT_USECOIN& message) {
    (void)message;
    return 1;
}

// IDA_EA: 0x8417D66
// IDA_NAME: _ZN15DB_StatGameRankC2Ev
// DEMANGLED: DB_StatGameRank::DB_StatGameRank(void)
DB_StatGameRank::DB_StatGameRank() {}

// IDA_EA: 0x8417D82
// IDA_NAME: _ZN15DB_StatGameRankD2Ev
// DEMANGLED: DB_StatGameRank::~DB_StatGameRank()
// IDA_EA: 0x8417DB2
// IDA_NAME: _ZN15DB_StatGameRankD0Ev
// DEMANGLED: DB_StatGameRank::~DB_StatGameRank() deleting destructor
DB_StatGameRank::~DB_StatGameRank() {}

// IDA_EA: 0x8417DD0
// IDA_NAME: _ZN15DB_StatGameRank8dispatchEiiP6Stream
// DEMANGLED: DB_StatGameRank::dispatch(int,int,Stream *)
bool DB_StatGameRank::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MSG_STAT_RANKRES* message = stream->GetOutBuffer<MSG_STAT_RANKRES>();
    return message != 0 && LogWrite(message) == 1;
}

// IDA_EA: 0x8417E4E
// IDA_NAME: _ZN15DB_StatGameRank8LogWriteEP16MSG_STAT_RANKRES
// DEMANGLED: DB_StatGameRank::LogWrite(MSG_STAT_RANKRES *)
int DB_StatGameRank::LogWrite(MSG_STAT_RANKRES* message) {
    (void)message;
    return 1;
}

// IDA_EA: 0x844E1A6
// IDA_NAME: _ZN23DB_StatUseShusiaServiceC2Ev
// DEMANGLED: DB_StatUseShusiaService::DB_StatUseShusiaService(void)
DB_StatUseShusiaService::DB_StatUseShusiaService() {}

// IDA_EA: 0x845010C
// IDA_NAME: _ZN23DB_StatUseShusiaServiceD2Ev
// DEMANGLED: DB_StatUseShusiaService::~DB_StatUseShusiaService()
// IDA_EA: 0x845013C
// IDA_NAME: _ZN23DB_StatUseShusiaServiceD0Ev
// DEMANGLED: DB_StatUseShusiaService::~DB_StatUseShusiaService() deleting destructor
DB_StatUseShusiaService::~DB_StatUseShusiaService() {}

// IDA_EA: 0x841BCE4
// IDA_NAME: _ZN23DB_StatUseShusiaService8dispatchEiiP6Stream
// DEMANGLED: DB_StatUseShusiaService::dispatch(int,int,Stream *)
bool DB_StatUseShusiaService::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}
