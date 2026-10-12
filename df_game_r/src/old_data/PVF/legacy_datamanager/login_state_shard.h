#ifndef OLDSRV_OLD_DATA_PVF_LEGACY_DATAMANAGER_LOGIN_STATE_SHARD_H_
#define OLDSRV_OLD_DATA_PVF_LEGACY_DATAMANAGER_LOGIN_STATE_SHARD_H_

// gc_no follows the original physical-shard encoding:
//     gc_no = physical_server_group * 1000 + channel_identity
//
// 0828 virtual channel groups are independent of that physical login shard.
// Login-state tables therefore
// use the gc_no-derived shard when the encoded value is available, while
// legacy/unencoded gc values retain the configured server group.
inline int ResolveLoginTableSuffix(int configured_server_group, int gc_no) {
    const int physical_server_group = gc_no / 1000;
    return physical_server_group > 0 ? physical_server_group
                                     : configured_server_group;
}

#endif  // OLDSRV_OLD_DATA_PVF_LEGACY_DATAMANAGER_LOGIN_STATE_SHARD_H_
