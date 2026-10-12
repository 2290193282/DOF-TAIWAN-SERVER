#ifndef OLDSRV_OLD_GAME_PVP_BATTLEFIELD_INTER_INTEGRATED_MATCH_PVP_SCORE_H_
#define OLDSRV_OLD_GAME_PVP_BATTLEFIELD_INTER_INTEGRATED_MATCH_PVP_SCORE_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

class Inter_IntegratedMatchPvpScore : public ISigDispatcher {
public:
    Inter_IntegratedMatchPvpScore();
    virtual ~Inter_IntegratedMatchPvpScore();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_IntegratedMatchPvpScore) == 0x04u,
              "integrated-match Inter owner must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_PVP_BATTLEFIELD_INTER_INTEGRATED_MATCH_PVP_SCORE_0828_H_
