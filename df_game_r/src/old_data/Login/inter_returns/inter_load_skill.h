#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_SKILL_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_SKILL_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 08_quest_skill_growth/Inter_LoadSkill.
// The original receiver has no side effects: its 0x84C5152 body returns zero.
class Inter_LoadSkill : public ISigDispatcher {
public:
    // IDA_EA: 0x84E9E5E / 0x84E9E7A / 0x84E9EAA
    // IDA_NAME: _ZN15Inter_LoadSkillC2Ev / D2Ev / D0Ev
    Inter_LoadSkill();
    virtual ~Inter_LoadSkill();

    // IDA_EA: 0x84C5152
    // IDA_NAME: _ZN15Inter_LoadSkill12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadSkill) == 0x04u,
              "Inter_LoadSkill must remain the base-only i386 dispatcher");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_SKILL_H_
