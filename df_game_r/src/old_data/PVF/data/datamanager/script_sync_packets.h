// IDA_EA: 0x8106C48, 0x8106C94.
#ifndef OLDSRV_DATAMANAGER_SCRIPT_SYNC_PACKETS_H_
#define OLDSRV_DATAMANAGER_SCRIPT_SYNC_PACKETS_H_

#include "infra/network/packet/PacketHeader.h"

#pragma pack(push, 1)

struct tagCS_GET_SCRIPT : public tagPacketHeader {
    tagCS_GET_SCRIPT();
};

struct tagCS_CHECK_SCRIPT_VERSION : public tagPacketHeader {
    tagCS_CHECK_SCRIPT_VERSION();
};

// IDA unit: 03_channel_select/tagCS_UPDATE_CHANNEL_INFO.
// The old channel-sync packets reserve bytes +0x06..+0x0A after the common
// six-byte tagPacketHeader.  CmmChannelProxy::SendChannelInfo @0x810619E
// then copies the original CEnvironment fields at +408, +888, +432, +960,
// +436 and +456, in this exact wire order.  In particular, this is the
// game-server identity sent to bridge, not the PVF channel name/type tuple.
struct tagCS_UPDATE_CHANNEL_INFO : public tagPacketHeader {
    tagCS_UPDATE_CHANNEL_INFO();

    uint8_t reserved_06[5];
    int32_t gc_no;
    int32_t server_group;
    int32_t channel_no;
    int32_t max_user_num;
    int32_t current_user_count;
    char server_ip[16];
    int32_t tcp_port;
};

#pragma pack(pop)

static_assert(sizeof(tagCS_GET_SCRIPT) == 6u,
              "tagCS_GET_SCRIPT IDA ABI size must stay 6");
static_assert(sizeof(tagCS_CHECK_SCRIPT_VERSION) == 6u,
              "tagCS_CHECK_SCRIPT_VERSION IDA ABI size must stay 6");
static_assert(sizeof(tagCS_UPDATE_CHANNEL_INFO) == 51u,
              "tagCS_UPDATE_CHANNEL_INFO IDA ABI size must stay 51");

#endif  // OLDSRV_DATAMANAGER_SCRIPT_SYNC_PACKETS_H_
