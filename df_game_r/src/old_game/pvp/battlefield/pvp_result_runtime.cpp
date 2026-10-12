#include "old_game/pvp/battlefield/battle_field.h"

#include <cstring>


typedef char PvpResultType_Size_Must_Be_0x84[
    (sizeof(PvpResultType) == 0x84) ? 1 : -1];
typedef char PvpResultSet_Size_Must_Be_0x84[
    (sizeof(PvpResultSet) == 0x84) ? 1 : -1];

// IDA_EA: 0x85D58DA
// IDA_NAME: _ZN13PvpResultTypeC2Ev
// DEMANGLED: PvpResultType::PvpResultType(void)
PvpResultType::PvpResultType() {}

// IDA_EA: 0x85D58E0
// IDA_NAME: _ZN13PvpResultTypeD2Ev
// DEMANGLED: PvpResultType::~PvpResultType()
PvpResultType::~PvpResultType() {}

// IDA_EA: 0x85D58E6
// IDA_NAME: _ZN13PvpResultType3SetERKS_
// DEMANGLED: PvpResultType::Set(PvpResultType const&)
PvpResultType* PvpResultType::Set(const PvpResultType& rhs) {
    std::memcpy(this, &rhs, sizeof(PvpResultType));
    if (values[2] < 0) {
        values[2] = 0;
        values[5] = 0;
    }
    return this;
}

// IDA_EA: 0x85D5924
// IDA_NAME: _ZNK13PvpResultType3GetERS_
// DEMANGLED: PvpResultType::Get(PvpResultType&)const
int PvpResultType::Get(PvpResultType& dst) const {
    std::memcpy(&dst, this, sizeof(PvpResultType));
    return 33;
}

// IDA_EA: 0x85D5944
// IDA_NAME: _ZN13PvpResultType12AddNewResultERKS_
// DEMANGLED: PvpResultType::AddNewResult(PvpResultType const&)
PvpResultType* PvpResultType::AddNewResult(const PvpResultType& rhs) {
    if (values[21]) {
        for (int index = 6; index <= 20; ++index) {
            const long long old_value = values[index];
            const long long new_value = rhs.values[index];
            const long long old_count = values[21];
            values[index] = static_cast<int>(
                (new_value + old_count * old_value) / (old_count + 1));
        }
    } else {
        const int previous_value = values[22];
        Set(rhs);
        values[22] = previous_value;
    }
    ++values[21];
    return this;
}

// IDA_EA: 0x85D5DC0
// IDA_NAME: _ZN13PvpResultType5ClearEv
// DEMANGLED: PvpResultType::Clear(void)
PvpResultType* PvpResultType::Clear() {
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
    values[5] = 0;
    values[4] = -1;
    values[6] = 0;
    values[7] = 0;
    values[8] = 0;
    values[9] = 0;
    values[10] = 0;
    values[11] = 0;
    values[12] = 0;
    values[13] = 0;
    values[14] = 0;
    values[15] = 0;
    values[16] = 0;
    values[17] = 0;
    values[18] = 0;
    values[19] = 0;
    values[20] = 0;
    values[21] = 0;
    values[22] = 0;
    values[23] = 0;
    values[27] = 0;
    values[28] = 0;
    values[29] = 0;
    values[30] = 0;
    values[24] = 0;
    values[25] = 0;
    values[31] = 0;
    values[32] = 0;
    return this;
}

// IDA_EA: 0x85D5EFE
// IDA_NAME: _ZNK13PvpResultType13GetTotalScoreEv
// DEMANGLED: PvpResultType::GetTotalScore(void)const
int PvpResultType::GetTotalScore() const {
    return values[29] + values[28] + values[27] + values[20] + values[19] +
           values[18] + values[17] + values[30];
}

// IDA_EA: 0x85D5F42
// IDA_NAME: _ZN12PvpResultSetC2Ev
// DEMANGLED: PvpResultSet::PvpResultSet(void)
PvpResultSet::PvpResultSet() : PvpResultType() {}

// IDA_EA: 0x85D5F56
// IDA_NAME: _ZN12PvpResultSetD2Ev
// DEMANGLED: PvpResultSet::~PvpResultSet()
PvpResultSet::~PvpResultSet() {}

// IDA_EA: 0x85D5F6A
// IDA_NAME: _ZN12PvpResultSet5ClearEv
// DEMANGLED: PvpResultSet::Clear(void)
PvpResultType* PvpResultSet::Clear() {
    return PvpResultType::Clear();
}

// IDA_EA: 0x8235378
// IDA_NAME: _ZN16MSG_RES_PVP_RANKC2Ev
// DEMANGLED: MSG_RES_PVP_RANK::MSG_RES_PVP_RANK(void)
MSG_RES_PVP_RANK::MSG_RES_PVP_RANK() : pvp_result() {}

// IDA_EA: 0x823538E
// IDA_NAME: _ZN16MSG_RES_PVP_RANKD2Ev
// DEMANGLED: MSG_RES_PVP_RANK::~MSG_RES_PVP_RANK()
MSG_RES_PVP_RANK::~MSG_RES_PVP_RANK() {}

// IDA_EA: 0x85D5F7E
// IDA_NAME: _ZN17PvpResultRecvFlagC2Ev
// DEMANGLED: PvpResultRecvFlag::PvpResultRecvFlag(void)
PvpResultRecvFlag::PvpResultRecvFlag() {
    Clear();
}

// IDA_EA: 0x85D5F92
// IDA_NAME: _ZN17PvpResultRecvFlagD2Ev
// DEMANGLED: PvpResultRecvFlag::~PvpResultRecvFlag()
PvpResultRecvFlag::~PvpResultRecvFlag() {}

// IDA_EA: 0x85D5F98
// IDA_NAME: _ZN17PvpResultRecvFlag11SetRecvFlagEi
// DEMANGLED: PvpResultRecvFlag::SetRecvFlag(int)
int PvpResultRecvFlag::SetRecvFlag(int index) {
    int result = IsValidIdx(index) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        result = index;
        recv_flags[index] = 1;
    }
    return result;
}

// IDA_EA: 0x85D5FC6
// IDA_NAME: _ZN17PvpResultRecvFlag5ClearEv
// DEMANGLED: PvpResultRecvFlag::Clear(void)
bool PvpResultRecvFlag::Clear() {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        recv_flags[index] = 0;
    }
    return result;
}

// IDA_EA: 0x85D5FF0
// IDA_NAME: _ZNK17PvpResultRecvFlag6IsRecvEi
// DEMANGLED: PvpResultRecvFlag::IsRecv(int)const
int PvpResultRecvFlag::IsRecv(int index) const {
    if (static_cast<unsigned int>(index) < 8) {
        return recv_flags[index];
    }
    return 0;
}

// IDA_EA: 0x85DF7B2
// IDA_NAME: _ZNK17PvpResultRecvFlag10IsValidIdxEi
// DEMANGLED: PvpResultRecvFlag::IsValidIdx(int)const
bool PvpResultRecvFlag::IsValidIdx(int index) const {
    return static_cast<unsigned int>(index) < 8u;
}
