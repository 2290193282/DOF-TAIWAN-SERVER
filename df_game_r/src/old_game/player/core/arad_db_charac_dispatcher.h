#ifndef OLDSRV_CHARACTER_SELECT_ARAD_DB_CHARAC_DISPATCHER_H_
#define OLDSRV_CHARACTER_SELECT_ARAD_DB_CHARAC_DISPATCHER_H_

class Stream;
struct SIG_CREATE_CHARAC;

namespace ARAD {

// IDA type-info at 0x8B8D874 is __class_type_info, not
// __si_class_type_info: this is a standalone vptr-only interface rather than
// a DBDispatcher derivative.  Its only virtual slot is dispatch at 0x8184114.
class Arad_DB_Charac_Dispatcher {
public:
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    int create_charac(SIG_CREATE_CHARAC* payload);
    int create_charac_stat(SIG_CREATE_CHARAC* payload);
    int create_inventory(SIG_CREATE_CHARAC* payload);
    int create_equip(SIG_CREATE_CHARAC* payload);
    int create_skill(SIG_CREATE_CHARAC* payload);
    int create_pvp(SIG_CREATE_CHARAC* payload);
    int create_charac_dungeon(SIG_CREATE_CHARAC* payload);
    int create_charac_quest(SIG_CREATE_CHARAC* payload);
    int create_charac_npc_relationship(SIG_CREATE_CHARAC* payload);
};

#if defined(__i386__)
static_assert(sizeof(Arad_DB_Charac_Dispatcher) == 0x04u,
              "ARAD character DB dispatcher must remain one i386 vptr");
#endif

}  // namespace ARAD

#endif  // OLDSRV_CHARACTER_SELECT_ARAD_DB_CHARAC_DISPATCHER_H_
