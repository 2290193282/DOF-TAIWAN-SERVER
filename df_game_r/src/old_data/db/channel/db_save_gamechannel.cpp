#include "old_data/db/channel/db_save_gamechannel.h"

#include "old_data/channel/log_game_channel.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844E232
// IDA_NAME: _ZN18DB_SaveGamechannelC2Ev
// DEMANGLED: DB_SaveGamechannel::DB_SaveGamechannel(void)
DB_SaveGamechannel::DB_SaveGamechannel() {}

// IDA_EA: 0x845F742
// IDA_NAME: _ZN18DB_SaveGamechannelD2Ev
// DEMANGLED: DB_SaveGamechannel::~DB_SaveGamechannel()
// IDA_EA: 0x845F772
// IDA_NAME: _ZN18DB_SaveGamechannelD0Ev
// DEMANGLED: DB_SaveGamechannel::~DB_SaveGamechannel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveGamechannel::~DB_SaveGamechannel() {}

// IDA_EA: 0x841C428
// IDA_NAME: _ZN18DB_SaveGamechannel8dispatchEiiP6Stream
// DEMANGLED: DB_SaveGamechannel::dispatch(int,int,Stream *)
bool DB_SaveGamechannel::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    WongWork::SIG_SAVE_LOGGAMECHANNEL* payload =
        stream->GetOutBuffer<WongWork::SIG_SAVE_LOGGAMECHANNEL>();
    GlobalData::s_pLogGameChannel->DoSaveLog(payload);
    return true;
}
