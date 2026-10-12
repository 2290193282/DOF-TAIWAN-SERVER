#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_SET_MULTIBOX_FAIL_COUNT_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_SET_MULTIBOX_FAIL_COUNT_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA_EA: C2 0x82A891E, parse 0x84B34A4, execute 0x84B34F8.
// CGameMasterMgr registers this object under the string command "bongfail".
class CSetMultiboxFailCnt : public CCommand {
public:
    CSetMultiboxFailCnt();

    virtual void execute();
    virtual void parse();

private:
    int failure_count_;
};

#if defined(__i386__)
static_assert(sizeof(CSetMultiboxFailCnt) == 0x0c,
              "CSetMultiboxFailCnt must remain the old 12-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_SET_MULTIBOX_FAIL_COUNT_H_
