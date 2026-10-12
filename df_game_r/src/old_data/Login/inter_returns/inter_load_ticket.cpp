#include "old_data/Login/inter_returns/inter_load_ticket.h"

#include "old_data/db/inventory/load_ticket_payload.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84C515C
// IDA_NAME: _ZN16Inter_LoadTicket12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadTicket::dispatch_sig(CUser *,char *,int)
int Inter_LoadTicket::dispatch_sig(CUser* user,
                                   char* payload,
                                   int /*payload_size*/) {
    SIG_LOAD_TICKET& ticket = *reinterpret_cast<SIG_LOAD_TICKET*>(payload);
    if (!user) {
        return 6768;
    }
    if (!user->get_state()) {
        return 6770;
    }
    if (ticket.result == 1) {
        return 6777;
    }
    if (ticket.count > 99) {
        return 6782;
    }
    for (uint32_t index = 0; index < ticket.count; ++index) {
        user->AddTicket(ticket.villages[index], 0);
    }
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(4));
    return 0;
}

// IDA_EA: 0x84E9EC8
// IDA_NAME: _ZN16Inter_LoadTicketC2Ev
// DEMANGLED: Inter_LoadTicket::Inter_LoadTicket(void)
Inter_LoadTicket::Inter_LoadTicket() : ISigDispatcher() {}

// IDA_EA: 0x84E9EE4 / 0x84E9F14
// IDA_NAME: _ZN16Inter_LoadTicketD2Ev / _ZN16Inter_LoadTicketD0Ev
// DEMANGLED: Inter_LoadTicket::~Inter_LoadTicket()
Inter_LoadTicket::~Inter_LoadTicket() {}
