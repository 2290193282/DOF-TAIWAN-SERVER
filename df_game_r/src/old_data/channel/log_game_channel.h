#ifndef OLDSRV_OLD_DATA_CHANNEL_LOG_GAME_CHANNEL_H_
#define OLDSRV_OLD_DATA_CHANNEL_LOG_GAME_CHANNEL_H_


class MySQL;

#pragma pack(push, 1)
struct SIG_SAVE_LOGGAMECHANNEL {
    char gc_game;        // +0
    int gc_no;           // +1
    char gc_channel[30]; // +5
    short gc_ch_group;   // +35
    char gc_ip[16];      // +37
    short gc_new;        // +53
    short gc_now;        // +55
    short gc_out;        // +57
};
#pragma pack(pop)

namespace WongWork {

// The original ABI type is global (23SIG_SAVE_LOGGAMECHANNEL).  Retain this
// namespace alias so existing B/A callers written as WongWork:: continue to
// name the same global type without creating a second layout.
typedef ::SIG_SAVE_LOGGAMECHANNEL SIG_SAVE_LOGGAMECHANNEL;

// IDA unit: 03_channel_select/WongWork/WongWork_CLogGameChannel.
class CLogGameChannel {
public:
    CLogGameChannel();

    MySQL* GetLogTargetDB();
    int StartLogTimer();
    int SetNextLogTimer();
    void ReqSaveLog();
    int DoSaveLog(SIG_SAVE_LOGGAMECHANNEL* payload);
    CLogGameChannel* ResetInOutUser();
    CLogGameChannel* IncInUser();
    CLogGameChannel* IncOutUser();

private:
    int in_user_;
    int out_user_;
    int now_user_;
    int period_minutes_;
};

}  // namespace WongWork
#endif  // OLDSRV_OLD_DATA_CHANNEL_LOG_GAME_CHANNEL_H_
