#include "old_game/pvp/match/fair_pvp_match_interface.h"

// IDA_EA: 0x8568998
// IDA_NAME: _ZN6IMatchC2Ev
// DEMANGLED: IMatch::IMatch(void)
IMatch::IMatch() {
}

// IDA_EA: 0x8568664
// IDA_NAME: _ZN6IMatchD2Ev
// DEMANGLED: IMatch::~IMatch()
// IDA_EA: 0x856868A
// IDA_NAME: _ZN6IMatchD0Ev
// DEMANGLED: IMatch::~IMatch()
IMatch::~IMatch() {
}

// IDA_EA: 0x85686A8
// IDA_NAME: _ZN6IMatch18GetIndentificationEv
// DEMANGLED: IMatch::GetIndentification(void)
int IMatch::GetIndentification() {
    return 0;
}

// IDA_EA: 0x85686B2
// IDA_NAME: _ZN6IMatch6GetGapEPS_
// DEMANGLED: IMatch::GetGap(IMatch*)
float IMatch::GetGap(IMatch*) {
    return 0.0f;
}

// IDA_EA: 0x85686C6
// IDA_NAME: _ZN6IMatch7GetUserEj
// DEMANGLED: IMatch::GetUser(unsigned int)
CUser* IMatch::GetUser(uint32_t) {
    return 0;
}

// IDA_EA: 0x85686D0
// IDA_NAME: _ZN6IMatch11GetUserListEPv
// DEMANGLED: IMatch::GetUserList(void *)
void IMatch::GetUserList(void*) {
}

// IDA_EA: 0x85686D6
// IDA_NAME: _ZN6IMatch10RequestMapERsR15PVP_BATTLE_MODE
// DEMANGLED: IMatch::RequestMap(short &,PVP_BATTLE_MODE &)
void IMatch::RequestMap(int16_t&, PVP_BATTLE_MODE&) {
}

// IDA_EA: 0x85686DC
// IDA_NAME: _ZN6IMatch16GetExtraWinPointEi
// DEMANGLED: IMatch::GetExtraWinPoint(int)
int IMatch::GetExtraWinPoint(int) {
    return 0;
}

// IDA_EA: 0x85686E6
// IDA_NAME: _ZN6IMatch12OnCreateRoomEP5CUser
// DEMANGLED: IMatch::OnCreateRoom(CUser *)
int IMatch::OnCreateRoom(CUser*) {
    return 0;
}

// IDA_EA: 0x85686F0
// IDA_NAME: _ZN6IMatch13OnDestroyRoomEP5CUser
// DEMANGLED: IMatch::OnDestroyRoom(CUser *)
void IMatch::OnDestroyRoom(CUser*) {
}

// IDA_EA: 0x85686F6
// IDA_NAME: _ZN6IMatch10OnJoinRoomEP5CUser
// DEMANGLED: IMatch::OnJoinRoom(CUser *)
void IMatch::OnJoinRoom(CUser*) {
}

// IDA_EA: 0x85686FC
// IDA_NAME: _ZN6IMatch11OnLeaveRoomEP5CUserP8PvP_Roomii
// DEMANGLED: IMatch::OnLeaveRoom(CUser *,PvP_Room *,int,int)
void IMatch::OnLeaveRoom(CUser*, PvP_Room*, int, int) {
}

// IDA_EA: 0x8568702
// IDA_NAME: _ZN6IMatch15OnRegisterMatchEP5CUserP8PvP_Room
// DEMANGLED: IMatch::OnRegisterMatch(CUser *,PvP_Room *)
int IMatch::OnRegisterMatch(CUser*, PvP_Room*) {
    return 1;
}

// IDA_EA: 0x856870C
// IDA_NAME: _ZN6IMatch17OnUnregisterMatchEP5CUserP8PvP_Room
// DEMANGLED: IMatch::OnUnregisterMatch(CUser *,PvP_Room *)
int IMatch::OnUnregisterMatch(CUser*, PvP_Room*) {
    return 1;
}

// IDA_EA: 0x8568716
// IDA_NAME: _ZN6IMatch11OnMatchRoomEP8PvP_Roomi
// DEMANGLED: IMatch::OnMatchRoom(PvP_Room *,int)
int IMatch::OnMatchRoom(PvP_Room*, int) {
    return 1;
}

// IDA_EA: 0x8568720
// IDA_NAME: _ZN6IMatch12OnStartFightEv
// DEMANGLED: IMatch::OnStartFight(void)
void IMatch::OnStartFight() {
}

// IDA_EA: 0x8568726
// IDA_NAME: _ZN6IMatch10OnEndFightEP8PvP_Room
// DEMANGLED: IMatch::OnEndFight(PvP_Room *)
void IMatch::OnEndFight(PvP_Room*) {
}

// IDA_EA: 0x856872C
// IDA_NAME: _ZN6IMatch9OnWalkOutEP8PvP_Roomi
// DEMANGLED: IMatch::OnWalkOut(PvP_Room *,int)
int IMatch::OnWalkOut(PvP_Room*, int) {
    return 0;
}

// IDA_EA: 0x8568736
// IDA_NAME: _ZN6IMatch7OnReadyEP5CUserb
// DEMANGLED: IMatch::OnReady(CUser *,bool)
bool IMatch::OnReady(CUser*, bool) {
    return true;
}

// IDA_EA: 0x856874A
// IDA_NAME: _ZN6IMatch8OnRewardEP8PvP_Roomib
// DEMANGLED: IMatch::OnReward(PvP_Room *,int,bool)
void IMatch::OnReward(PvP_Room*, int, bool) {
}

// IDA_EA: 0x8568758
// IDA_NAME: _ZN6IMatch8OnEndPvPEP8PvP_Room
// DEMANGLED: IMatch::OnEndPvP(PvP_Room *)
void IMatch::OnEndPvP(PvP_Room*) {
}

// IDA_EA: 0x856875E
// IDA_NAME: _ZN6IMatch14OnCompleteLoadEP5CUser
// DEMANGLED: IMatch::OnCompleteLoad(CUser *)
void IMatch::OnCompleteLoad(CUser*) {
}

// IDA_EA: 0x8568764
// IDA_NAME: _ZN6IMatch19OnConnectP2PTimeoutEP8PvP_Room
// DEMANGLED: IMatch::OnConnectP2PTimeout(PvP_Room *)
void IMatch::OnConnectP2PTimeout(PvP_Room*) {
}

// IDA_EA: 0x856876A
// IDA_NAME: _ZN6IMatch12OnConnectP2PEP5CUserP8PvP_RoomPci
// DEMANGLED: IMatch::OnConnectP2P(CUser *,PvP_Room *,char *,int)
void IMatch::OnConnectP2P(CUser*, PvP_Room*, char*, int) {
}

// IDA_EA: 0x8568770
// IDA_NAME: _ZN6IMatch8OnGiveupEP5CUser
// DEMANGLED: IMatch::OnGiveup(CUser *)
int IMatch::OnGiveup(CUser*) {
    return 1;
}

// IDA_EA: 0x856877A
// IDA_NAME: _ZN6IMatch16OnRematchTimeoutEP8PvP_Room
// DEMANGLED: IMatch::OnRematchTimeout(PvP_Room *)
void IMatch::OnRematchTimeout(PvP_Room*) {
}

// IDA_EA: 0x8568780
// IDA_NAME: _ZN6IMatch12OnAskRematchEP5CUseri
// DEMANGLED: IMatch::OnAskRematch(CUser *,int)
void IMatch::OnAskRematch(CUser*, int) {
}

// IDA_EA: 0x8568786
// IDA_NAME: _ZN6IMatch13OnWaitRematchEP5CUserP8PvP_Room
// DEMANGLED: IMatch::OnWaitRematch(CUser *,PvP_Room *)
void IMatch::OnWaitRematch(CUser*, PvP_Room*) {
}

// IDA_EA: 0x856878C
// IDA_NAME: _ZN6IMatch13CheckJoinableEP8PvP_RoomP5CUser
// DEMANGLED: IMatch::CheckJoinable(PvP_Room *,CUser *)
int IMatch::CheckJoinable(PvP_Room*, CUser*) {
    return 0;
}

// IDA_EA: 0x8568796
// IDA_NAME: _ZN6IMatch9CheckSeatEi
// DEMANGLED: IMatch::CheckSeat(int)
int IMatch::CheckSeat(int) {
    return 1;
}

// IDA_EA: 0x85687A0
// IDA_NAME: _ZN6IMatch14CheckMatchableEPS_
// DEMANGLED: IMatch::CheckMatchable(IMatch*)
bool IMatch::CheckMatchable(IMatch*) {
    return true;
}

// IDA_EA: 0x85687AA
// IDA_NAME: _ZN6IMatch22GetEndPvPResultTimeoutEv
// DEMANGLED: IMatch::GetEndPvPResultTimeout(void)
int IMatch::GetEndPvPResultTimeout() {
    return 10000;
}

// IDA_EA: 0x85687B4
// IDA_NAME: _ZN6IMatch14OnEndPvPResultEP8PvP_RoomP5CUser
// DEMANGLED: IMatch::OnEndPvPResult(PvP_Room *,CUser *)
void IMatch::OnEndPvPResult(PvP_Room*, CUser*) {
}

// IDA_EA: 0x85687BA
// IDA_NAME: _ZN6IMatch14GetPlayerCountEv
// DEMANGLED: IMatch::GetPlayerCount(void)
int IMatch::GetPlayerCount() {
    return 0;
}

// IDA_EA: 0x85687C4
// IDA_NAME: _ZN6IMatch12GetNextMatchEv
// DEMANGLED: IMatch::GetNextMatch(void)
IMatch* IMatch::GetNextMatch() {
    return 0;
}

// IDA_EA: 0x85687CE
// IDA_NAME: _ZN6IMatch12SetNextMatchEPS_
// DEMANGLED: IMatch::SetNextMatch(IMatch*)
void IMatch::SetNextMatch(IMatch*) {
}

// IDA_EA: 0x8568FA2
// IDA_NAME: _ZN9IRoomListC2Ev
// DEMANGLED: IRoomList::IRoomList(void)
IRoomList::IRoomList() {
}
