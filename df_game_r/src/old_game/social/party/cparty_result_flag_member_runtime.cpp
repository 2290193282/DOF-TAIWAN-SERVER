#include "old_game/social/party/party.h"

#include <cstring>

// IDA_EA: 0x82A4CB2
// IDA_NAME: _ZN20CPartyResultRecvFlagC2Ev
// DEMANGLED: CPartyResultRecvFlag::CPartyResultRecvFlag(void)
CPartyResultRecvFlag::CPartyResultRecvFlag() {
    Clear();
}

// IDA_EA: 0x82A4CC6
// IDA_NAME: _ZN20CPartyResultRecvFlag5ClearEv
// DEMANGLED: CPartyResultRecvFlag::Clear(void)
void* CPartyResultRecvFlag::Clear() {
    return std::memset(this, 0, sizeof(recv_flags));
}

// IDA_EA: 0x85BF3D8
// IDA_NAME: _ZN20CPartyResultRecvFlag11SetRecvFlagEi
// DEMANGLED: CPartyResultRecvFlag::SetRecvFlag(int)
int CPartyResultRecvFlag::SetRecvFlag(int index) {
    int result = IsValidIdx(index) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        result = index;
        recv_flags[index] = 1;
    }
    return result;
}

// IDA_EA: 0x85BF406
// IDA_NAME: _ZNK20CPartyResultRecvFlag6IsRecvEi
// DEMANGLED: CPartyResultRecvFlag::IsRecv(int)const
int CPartyResultRecvFlag::IsRecv(int index) const {
    if (IsValidIdx(index) == 1) {
        return recv_flags[index];
    }
    return 0;
}

// IDA_EA: 0x85BF438
// IDA_NAME: _ZNK20CPartyResultRecvFlag10IsValidIdxEi
// DEMANGLED: CPartyResultRecvFlag::IsValidIdx(int)const
bool CPartyResultRecvFlag::IsValidIdx(int index) const {
    return static_cast<unsigned int>(index) < 4u;
}

// IDA_EA: 0x8599A14
// IDA_NAME: _ZN6CParty7cMemberC2Ev
// DEMANGLED: CParty::cMember::cMember(void)
CParty::cMember::cMember() {
    Init();
}

// IDA_EA: 0x8599A28
// IDA_NAME: _ZN6CParty7cMember4InitEv
// DEMANGLED: CParty::cMember::Init(void)
CParty::cMember* CParty::cMember::Init() {
    user = 0;
    state = 0;
    user_uid = 0;
    p2p_received = 0xffu;
    last_channel = 0xffu;
    std::memset(p2p_connectivity, 0, sizeof(p2p_connectivity));
    teleport_state = 0xffu;
    used_coin_count = 0;
    return this;
}
