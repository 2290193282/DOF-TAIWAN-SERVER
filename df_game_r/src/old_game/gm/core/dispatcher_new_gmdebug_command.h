#ifndef OLDSRV_OLD_GAME_GM_CORE_DISPATCHER_NEW_GMDEBUG_COMMAND_H_
#define OLDSRV_OLD_GAME_GM_CORE_DISPATCHER_NEW_GMDEBUG_COMMAND_H_

#include <string>
#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_New_Gmdebug_Command.
// The old dispatcher is base-only.  Its concrete command tree lives here,
// rather than in the generic derived-dispatcher fallback source.
class Dispatcher_New_Gmdebug_Command
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859663C / 0x8241E2A / 0x8241E5A.
    Dispatcher_New_Gmdebug_Command();
    virtual ~Dispatcher_New_Gmdebug_Command();

    // IDA_EA: 0x820BBDE / 0x8233B82 / 0x8233B8C.
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x820BF2C.
    int oldGmRoutine(CUser* user,
                     PacketBuf& packet,
                     std::vector<std::string>& tokens);
};

// IDA_EA: 0x820BA90.
void DoUserDefineCommand(CUser* user, int command_id, const char* command);

// IDA_EA: 0x820BB68.
char SendVillageAttackedGMCommand(CUser* user, int value);

#if defined(__i386__)
static_assert(sizeof(Dispatcher_New_Gmdebug_Command) == 0x04u,
              "GMDEBUG dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GM_CORE_DISPATCHER_NEW_GMDEBUG_COMMAND_H_
