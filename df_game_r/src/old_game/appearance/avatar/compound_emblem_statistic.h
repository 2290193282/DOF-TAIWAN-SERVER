#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_EMBLEM_STATISTIC_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_EMBLEM_STATISTIC_H_

#include <stdint.h>

// IDA_EA: C2 @0x861078C, D2 @0x8610792.  The process-wide statistic keeps
// exactly one counter per emblem grade (0..6); static storage is zeroed
// before the intentionally empty C2 executes.
class CCompoundEmblemStatistic {
public:
    CCompoundEmblemStatistic();
    ~CCompoundEmblemStatistic();

    // IDA_EA: 0x81933B4 / 0x863BCEA / 0x863BCF8 / 0x863BD2E.
    unsigned int increaseCount(unsigned int grade);
    int getGradeCount(int grade) const;
    int getTotalCount() const;
    bool clear();

private:
    int32_t grade_counts_[7];
};

static_assert(sizeof(CCompoundEmblemStatistic) == 0x1cu,
              "compound-emblem statistics must retain seven i386 counters");

// IDA_EA: 0x860FD6F
// IDA_NAME: _Z26getCompoundEmblemStatisticv
CCompoundEmblemStatistic* getCompoundEmblemStatistic();

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_EMBLEM_STATISTIC_H_
