#ifndef OLDSRV_OLD_GAME_USERINFO_CUSER_GLOBAL_INFO_HANDLE_H_
#define OLDSRV_OLD_GAME_USERINFO_CUSER_GLOBAL_INFO_HANDLE_H_

#include <stddef.h>
#include <stdint.h>

// IDA unit: 06_userinfo_stat_inventory/CUserGlobalInfoHandle.  This is the
// process-wide allocator for the 16-bit user identity embedded in every
// accepted CUser.  It is deliberately distinct from the CGameManager slot
// and increment IDs: protocol packets carry this value after login.
class CUserGlobalInfoHandle {
public:
    // IDA_EA: 0x86AD820 / 0x86AD85A.
    CUserGlobalInfoHandle();
    ~CUserGlobalInfoHandle();

    // IDA_EA: 0x86AD8C2 / 0x86AD910 / 0x86AD95E.
    int find_uniqueid();
    int get_uniqueid();
    int reset_uniqueid_flag(uint16_t unique_id);

public:  // Exact i386 storage is visible for ABI verification only.
    uint8_t used_[0x10000];  // +0x00000; old server reserves 0xffff.
    uint32_t next_unique_id_;  // +0x10000.
};

#if defined(__i386__)
static_assert(offsetof(CUserGlobalInfoHandle, next_unique_id_) == 0x10000u,
              "CUserGlobalInfoHandle cursor must stay at old +0x10000");
static_assert(sizeof(CUserGlobalInfoHandle) == 0x10004u,
              "CUserGlobalInfoHandle must retain the old i386 layout");
#endif

// IDA_EA: 0x86AD85F
// IDA_NAME: _Z29CUserGlobalInfoHandleInstancev
CUserGlobalInfoHandle* CUserGlobalInfoHandleInstance();

#endif  // OLDSRV_OLD_GAME_USERINFO_CUSER_GLOBAL_INFO_HANDLE_H_
