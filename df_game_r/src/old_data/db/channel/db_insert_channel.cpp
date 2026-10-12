#include "old_data/db/channel/db_insert_channel.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <cstdio>
#include <cstring>
#include <string>



// IDA_EA: 0x840364C
// IDA_NAME: _ZN16DB_InsertChannelC2Ev
// DEMANGLED: DB_InsertChannel::DB_InsertChannel(void)
DB_InsertChannel::DB_InsertChannel() {}

// IDA_EA: 0x8403668
// IDA_NAME: _ZN16DB_InsertChannelD2Ev
// DEMANGLED: DB_InsertChannel::~DB_InsertChannel()
// IDA_EA: 0x8403698
// IDA_NAME: _ZN16DB_InsertChannelD0Ev
// DEMANGLED: DB_InsertChannel::~DB_InsertChannel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertChannel::~DB_InsertChannel() {}

// IDA_EA: 0x84036B6
// IDA_NAME: _ZN16DB_InsertChannel8dispatchEiiP6Stream
// DEMANGLED: DB_InsertChannel::dispatch(int,int,Stream *)
bool DB_InsertChannel::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_INSERT_CHANNEL* channel = stream->GetOutBuffer<SIG_INSERT_CHANNEL>();
    return channel && InsertChannel(channel) == true;
}

// IDA_EA: 0x8403734
// IDA_NAME: _ZN16DB_InsertChannel13InsertChannelEP18SIG_INSERT_CHANNEL
// DEMANGLED: DB_InsertChannel::InsertChannel(SIG_INSERT_CHANNEL *)
bool DB_InsertChannel::InsertChannel(SIG_INSERT_CHANNEL* channel) {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( channel->server_group));
    db_handle->set_query(
        "inSert into game_channel (gc_no,gc_now,gc_ip,gc_port,gc_max,gc_game,gc_channel,gc_ch_group,gc_channeltype,gc_up_time"
        ", gc_type) values(%d,0,'%s',%d,%d,%d,'%s',%d,'%s',now(),%d)",
        channel->gc_no,
        channel->gc_ip,
        channel->gc_port,
        channel->gc_max,
        channel->gc_game,
        channel->gc_channel,
        channel->gc_ch_group,
        channel->gc_channeltype,
        channel->gc_type);

    char ok = db_handle->exec(true);
    if (ok != 1) {
        db_handle->set_query(
            "upDate game_channel set gc_now=0,gc_ip='%s',gc_port=%d,gc_max=%d,gc_game=%d,gc_channel='%s',gc_ch_group=%d,gc_chan"
            "neltype='%s',gc_up_time=now(),gc_type=%d where gc_no=%d",
            channel->gc_ip,
            channel->gc_port,
            channel->gc_max,
            channel->gc_game,
            channel->gc_channel,
            channel->gc_ch_group,
            channel->gc_channeltype,
            channel->gc_type,
            channel->gc_no);
        ok = db_handle->exec(true);
        if (ok != 1) {
            return false;
        }
    }

    std::string values;
    values.clear();
    char entry[100];
    for (unsigned int age = 0; age <= 0x63; ++age) {
        std::memset(entry, 0, sizeof(entry));
        std::sprintf(entry, "(%d, %d, 0)", channel->gc_no, age + 1);
        values.append(entry);
        if (age <= 0x62) {
            values.append(", ");
        }
    }

    db_handle->set_query(
        "inSert into channel_occ_info (gc_no, age, occ_num) values %s",
        values.c_str());
    ok = db_handle->exec(true);
    if (ok != 1) {
        db_handle->set_query(
            "upDate channel_occ_info set occ_num = 0 where gc_no = %d",
            channel->gc_no);
        ok = db_handle->exec(true);
        if (ok != 1) {
            return false;
        }
    }
    return true;
}
