#include "old_data/db/core/db_suicide_when_queue_is_empty.h"


// IDA_EA: 0x844E11A
// IDA_NAME: _ZN26DB_SuicideWhenQueueIsEmptyC2Ev
// DEMANGLED: DB_SuicideWhenQueueIsEmpty::DB_SuicideWhenQueueIsEmpty(void)
DB_SuicideWhenQueueIsEmpty::DB_SuicideWhenQueueIsEmpty() {}

// IDA_EA: 0x845FA00
// IDA_NAME: _ZN26DB_SuicideWhenQueueIsEmptyD2Ev
// DEMANGLED: DB_SuicideWhenQueueIsEmpty::~DB_SuicideWhenQueueIsEmpty()
// IDA_EA: 0x845FA30
// IDA_NAME: _ZN26DB_SuicideWhenQueueIsEmptyD0Ev
// DEMANGLED: DB_SuicideWhenQueueIsEmpty::~DB_SuicideWhenQueueIsEmpty() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SuicideWhenQueueIsEmpty::~DB_SuicideWhenQueueIsEmpty() {}

// IDA_EA: 0x841B206
// IDA_NAME: _ZN26DB_SuicideWhenQueueIsEmpty8dispatchEiiP6Stream
// DEMANGLED: DB_SuicideWhenQueueIsEmpty::dispatch(int,int,Stream *)
bool DB_SuicideWhenQueueIsEmpty::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}
