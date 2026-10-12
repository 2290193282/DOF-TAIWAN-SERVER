#ifndef OLDSRV_LOGGING_HISTORY_TRACE_H_
#define OLDSRV_LOGGING_HISTORY_TRACE_H_

#include <stddef.h>


// The old caller uses four bytes of automatic storage and writes the source
// code ('D' or 'X') at byte +1 before invoking operator().
class cHistoryTrace {
public:
    unsigned char reserved_00;
    char source_code;
    unsigned char reserved_02[2];

    int operator()(const char* format, ...);
};

typedef char cHistoryTrace_Size_Must_Be_4[
    (sizeof(cHistoryTrace) == 4u) ? 1 : -1];
typedef char cHistoryTrace_Source_Offset_Must_Be_1[
    (offsetof(cHistoryTrace, source_code) == 1u) ? 1 : -1];


#endif  // OLDSRV_LOGGING_HISTORY_TRACE_H_
