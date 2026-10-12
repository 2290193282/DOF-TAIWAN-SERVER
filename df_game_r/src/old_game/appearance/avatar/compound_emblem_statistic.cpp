#include "old_game/appearance/avatar/compound_emblem_statistic.h"

// IDA_EA: 0x861078C
// IDA_NAME: _ZN24CCompoundEmblemStatisticC2Ev
CCompoundEmblemStatistic::CCompoundEmblemStatistic() {}

// IDA_EA: 0x8610792
// IDA_NAME: _ZN24CCompoundEmblemStatisticD2Ev
CCompoundEmblemStatistic::~CCompoundEmblemStatistic() {}

// IDA_EA: 0x81933B4
// IDA_NAME: _ZN24CCompoundEmblemStatistic13increaseCountEi
unsigned int CCompoundEmblemStatistic::increaseCount(unsigned int grade) {
    if (grade <= 6u) {
        ++grade_counts_[grade];
    }
    return grade;
}

// IDA_EA: 0x863BCEA
// IDA_NAME: _ZN24CCompoundEmblemStatistic13getGradeCountEi
int CCompoundEmblemStatistic::getGradeCount(int grade) const {
    return grade_counts_[grade];
}

// IDA_EA: 0x863BCF8
// IDA_NAME: _ZN24CCompoundEmblemStatistic13getTotalCountEv
int CCompoundEmblemStatistic::getTotalCount() const {
    int total = 0;
    for (int grade = 0; grade <= 6; ++grade) {
        total += grade_counts_[grade];
    }
    return total;
}

// IDA_EA: 0x863BD2E
// IDA_NAME: _ZN24CCompoundEmblemStatistic5clearEv
bool CCompoundEmblemStatistic::clear() {
    for (int grade = 0; grade <= 6; ++grade) {
        grade_counts_[grade] = 0;
    }
    return false;
}

// IDA_EA: 0x860FD6F
// IDA_NAME: _Z26getCompoundEmblemStatisticv
CCompoundEmblemStatistic* getCompoundEmblemStatistic() {
    static CCompoundEmblemStatistic instance;
    return &instance;
}
