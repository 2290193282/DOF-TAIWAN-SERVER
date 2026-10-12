#ifndef OLDSRV_ENVIRONMENT_ENVIRONMENT_H_
#define OLDSRV_ENVIRONMENT_ENVIRONMENT_H_

#include "old_data/db/core/db_conn_info.h"
#include "common/manager/global_instance.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/security/crypto/ctea_abi.h"

#include <cstddef>
#include <cstdio>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>


typedef std::map<std::string, std::string> ConfigTagMap;
typedef std::map<ENUM_SERVER_GROUP, ConfigTagMap> TournamentConfigMap;
typedef std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo> DBConnInfoMap;
typedef std::map<ENUM_SERVER_GROUP, DBConnInfoMap> ServerGroupDBConnInfoMap;

// IDA: SK_IP vector at CEnvironment+864. CUser::InitOnAccept serializes the
// fixed 16-byte text buffer and then two values from the companion int vector.
struct SK_IP {
    char text[16];

    SK_IP();
    explicit SK_IP(const char* value);
};

// IDA_EA: 0x80CBCA8 / 0x80CBF36.
//
// This is the physical Server_Envir subobject constructed at
// CEnvironment+0x198.  The original type ends on the byte boundary at
// +0x20A: both channel_name and the following CEnvironment::file_name are
// 30-byte buffers.  The packed declaration is therefore part of the old
// i386 ABI; all non-trivial STL members still start at 4-byte-aligned offsets.
#pragma pack(push, 1)
struct Server_Envir {
    Server_Envir();
    ~Server_Envir();

    int gc_no;                              // +0x000 / CEnvironment+0x198
    int small_stream_pool_count;            // +0x004
    int stream_pool_count;                  // +0x008
    int big_stream_pool_count;              // +0x00C
    int server_type;                        // +0x010
    int process_sequence;                   // +0x014
    int channel_no;                         // +0x018
    char server_ip[16];                     // +0x01C
    uint32_t server_ip_address;              // +0x02C, inet_addr(server_ip)
    int tcp_port;                           // +0x030
    int udp_port;                           // +0x034
    char udp_ip_monitor[16];                // +0x038
    int udp_port_monitor;                   // +0x048
    char udp_ip_guild[16];                  // +0x04C
    int udp_port_guild;                     // +0x05C
    int tcp_port_monitor;                   // +0x060
    int tcp_port_guild;                     // +0x064
    char udp_ip_hades[16];                  // +0x068
    int udp_port_hades;                     // +0x078
    int tcp_port_hades;                     // +0x07C
    char udp_ip_statistic[16];              // +0x080
    int udp_port_statistic;                 // +0x090
    char udp_ip_doublecheck[16];            // +0x094
    int udp_port_doublecheck;               // +0x0A4
    char udp_ip_channel[16];                // +0x0A8
    int channel_bridge_port;                // +0x0B8
    uint8_t reserved_before_exchange_ip[4]; // +0x0BC, identity not recovered
    char exchange_server_ip[16];            // +0x0C0
    int exchange_server_port;               // +0x0D0
    char school_server_ip[16];              // +0x0D4
    int school_server_port;                 // +0x0E4
    char pcroom_server_ip[16];              // +0x0E8
    int pcroom_server_port;                 // +0x0F8
    char community_server_ip[16];           // +0x0FC
    int community_server_port;              // +0x10C
    char system_monitoring_ip[16];           // +0x110
    char ipg_ip[16];                        // +0x120
    char auction_server_ip[16];             // +0x130
    int auction_server_port;                // +0x140
    char cera_auction_server_ip[16];        // +0x144
    int cera_auction_server_port;           // +0x154
    int accept_lock_dbqueue_count;          // +0x158
    int accept_unlock_dbqueue_count;        // +0x15C
    std::string script_dir;                 // +0x160
    std::vector<std::string> priority_script_dirs; // +0x164
    bool scriptpacks;                       // +0x170
    uint8_t reserved_before_channel_type[3];// +0x171
    int channel_type;                       // +0x174
    bool validate_script;                   // +0x178
    uint8_t reserved_before_publickey[3];   // +0x179
    std::string garena_publickey;           // +0x17C
    char lls_key[0x2D];                     // +0x180
    char relay_ip[16];                      // +0x1AD
    uint8_t reserved_after_relay_ip[3];     // +0x1BD
    int relay_tcp_port;                     // +0x1C0
    int relay_udp_port;                     // +0x1C4
    std::vector<SK_IP> sk_ip_list;           // +0x1C8
    std::vector<int> sk_port_values;        // +0x1D4
    int server_group;                       // +0x1E0
    int fatigue_time;                       // +0x1E4
    int avatar_time;                        // +0x1E8
    char channel_name[30];                  // +0x1EC
};
#pragma pack(pop)

// IDA: eStartArgumentType_t consumed by CEnvironment::set_operation_type
// @0x82A6CFC from check_argv @0x829EAB4.
enum eStartArgumentType_t {
    START_ARGUMENT_START = 0,
    START_ARGUMENT_START_ALL = 1,
    START_ARGUMENT_NOFORK = 2,
    START_ARGUMENT_TEST = 3,
};

// IDA: CEnvironment C2 @0x80CC00C / D2 @0x82A6BEE.
// parse is the first and only virtual slot in the original table.  The
// destructor is deliberately non-virtual: the old ELF has D1/D2 but no D0.
class CEnvironment {
public:
    CEnvironment();
    ~CEnvironment();

    // IDA: CEnvironment::load @0x8290450.
    bool load();

    // IDA: CEnvironment::parse @0x82906BE, vtable slot +0x0.
    virtual bool parse(char* line);

    // IDA: CEnvironment::fgetline @0x8290540.
    bool fgetline(FILE* stream, char* line);

    // IDA: CEnvironment::check_comment @0x8290610.
    bool check_comment(const char* line);

    // IDA: CEnvironment::set_file_name, called by check_argv @0x829EA7C.
    void set_file_name(const char* name);

    // IDA: CEnvironment::get_file_name @0x80DA39A.
    const char* get_file_name();

    // IDA: CEnvironment::DecryptValue @0x8292FD4.
    bool DecryptValue(const char* encrypted_hex, char* dest);

    const DBConnInfoMap& db_conn_info_map() const;
    DBConnInfoMap& db_conn_info_map();

    const TournamentConfigMap& tournament_config_map() const;
    TournamentConfigMap& tournament_config_map();

    const ServerGroupDBConnInfoMap& server_group_db_conn_info_map() const;
    ServerGroupDBConnInfoMap& server_group_db_conn_info_map();

    const char* get_db_tbl_file() const;
    int get_server_group();
    int get_fatigue_time() const;
    int get_avatar_time() const;
    int get_channel_no();
    void set_channel_no(int channel_no);
    int get_channel_type() const;
    void set_channel_type(int channel_type);
    int get_gc_no();
    int get_gc_no_hardcode(int proxy_index);
    const char* get_channel_name();
    void set_channel_name(const char* channel_name);
    int get_udp_port();
    int get_tcp_port();
    const char* get_server_ip();
    // Typed semantic accessor for Server_Envir::server_type at old +0x1A8.
    // No standalone old-server getter export was recovered for this field.
    int get_server_type() const;
    int get_max_user_num() const;
    // IDA: get_tcp_port_channel @0x8106CEE and get_udp_port_channel
    // @0x82A6E4A both read old offset +592. get_udp_ip_channel @0x82A6E58
    // returns the fixed channel IP buffer at old offset +576.
    int get_tcp_port_channel();
    int get_udp_port_channel();
    const char* get_udp_ip_channel();
    int get_udp_port_monitor();
    const char* get_udp_ip_monitor();
    int get_tcp_port_monitor();
    int get_udp_port_guild();
    const char* get_udp_ip_guild();
    int get_tcp_port_guild();
    int get_udp_port_statistic();
    const char* get_udp_ip_statistic();
    int get_udp_port_doublecheck();
    const char* get_udp_ip_doublecheck();
    const char* get_udp_ip_hades();
    int get_udp_port_hades();
    int get_tcp_port_hades();
    const char* get_ipg_ip() const { return server_envir_.ipg_ip; }
    // AcceptThread @0x857D003 reads the DB-dispatch back-pressure watermarks
    // directly from old CEnvironment+0x2F0/+0x2F4.  They are parsed config
    // values, not a new acceptance policy.
    int get_accept_lock_dbqueue_count() const;
    int get_accept_unlock_dbqueue_count() const;
    const char* get_school_server_ip();
    int get_school_server_port();
    int get_pcroom_server_port();
    const char* get_pcroom_server_ip();
    int get_community_server_port();
    const char* get_community_server_ip();
    int get_auction_server_port();
    const char* get_auction_server_ip();
    int get_cera_auction_server_port();
    const char* get_cera_auction_server_ip();
    const char* get_system_monitoring_ip();
    int getProcessSequence() const;

    // IDA: CEnvironment::adjust_base_script_directory @0x8293084.
    void adjust_base_script_directory();

    // IDA: CEnvironment::parse_se_event_db @0x82930B8.
    bool parse_se_event_db(char* key, char* value);

    // IDA: CEnvironment::lock/unlock @0x82A6CCC/@0x82A6CE4.
    int lock();
    int unlock();

    // IDA: CDataManager::init @0x8358332 reads these fields directly from
    // CEnvironment old offsets +0x2F8/+0x2FC/+0x308.
    const std::string& script_dir() const;
    const std::vector<std::string>& priority_script_dirs() const;
    bool scriptpacks() const;
    bool validate_script() const;

    // IDA: CEnvironment::get_exchange_server_ip @0x82A6E74 and
    // get_exchange_server_port @0x82A6E66.
    const char* get_exchange_server_ip();
    int get_exchange_server_port();

    // IDA: check_argv reads the lls key directly at CEnvironment+0x318.
    const char* get_lls_key() const;

    // IDA: CEnvironment::parse @0x8292E77 handles tag id 0x803
    // "garena_publickey" and writes a std::string at old offset +0x314.
    const char* get_garena_publickey() const;

    // IDA: CEnvironment::set_operation_type @0x82A6CFC.
    void set_operation_type(eStartArgumentType_t operation_type);
    eStartArgumentType_t get_operation_type();
    eStartArgumentType_t operation_type() const;

    // IDA: CEnvironment::set_suspend_state @0x82A6D0A writes old offset +988.
    void set_suspend_state(bool suspend);
    bool get_suspend_state();
    bool suspend_state() const;

    // IDA: SmallStreamPool @0x828FE62 reads *(G_CEnvironment()+412).
    int get_small_stream_pool_count() const;

    // IDA: StreamPool @0x828F974 reads *(G_CEnvironment()+416).
    int get_stream_pool_count() const;

    // IDA: BigStreamPool @0x8290150 reads *(G_CEnvironment()+420).
    int get_big_stream_pool_count() const;

    // IDA: CUser::InitOnAccept @0x8649658 reads the 32-bit Server_Envir
    // values server_group (+888), channel_no (+432), and server_type (+424),
    // then serializes their low bytes in that order.
    uint8_t get_accept_handshake_flag_888() const;
    uint8_t get_accept_handshake_flag_432() const;
    uint8_t get_accept_handshake_flag_424() const;

    // IDA: CUser::InitOnAccept @0x86496AC serializes vectors at +864/+876.
    const std::vector<SK_IP>& get_sk_ip_list() const;
    const std::vector<int>& get_sk_port_values() const;

private:
    friend class CUser;

    // Inter_MonitorLoginOK @0x84C7E0C serializes the original relay fields
    // directly at +0x345/+0x358/+0x35c.  CheckRestoredOffsets() keeps those
    // i386 offsets asserted; friendship avoids inventing a new accessor ABI.
    friend class Inter_MonitorLoginOK;
    // DB_GatheringParyStatus @0x8420E1A / @0x842156A persists the same
    // original relay triple in party-result history.  It uses the proven
    // fields directly, so grant that one DB dispatcher access without adding
    // a synthetic CEnvironment getter or changing this layout.
    friend class DB_GatheringParyStatus;

    static void CheckRestoredOffsets();

    bool decrypt_db_password_block(const uint8_t* src, char* dest);
    bool decrypt_or_copy_db_password(const char* value, char* dest);
    bool IsEnableValue(const char* value);
    void push_priority_script_dir(const char* value);

    CTEA password_decryptor_;                    // +0x004, sizeof 0x48
    eStartArgumentType_t operation_type_;        // +0x04C
    TournamentConfigMap tournament_config_values_; // +0x050
    DBConnInfoMap db_conn_info_;                 // +0x068
    ServerGroupDBConnInfoMap server_group_db_conn_info_; // +0x080
    char db_tbl_file_[256];                      // +0x098
    Server_Envir server_envir_;                  // +0x198, sizeof 0x20A
    char file_name_[30];                         // +0x3A2
    int max_user_num_;                           // +0x3C0
    Mutex mutex_;                                // +0x3C4, sizeof 0x18
    bool suspend_state_;                         // +0x3DC
    uint8_t reserved_after_suspend_state_[3];    // +0x3DD
};

// IDA: CDBEnviroment inherits CEnvironment and appends the current
// game_db_type/server_group parser context at this+0x3E0.
class CDBEnviroment : public CEnvironment {
public:
    CDBEnviroment();
    ~CDBEnviroment();

    // IDA: CDBEnviroment::parse @0x829D8E8.
    virtual bool parse(char* line);

    int current_server_group() const;

private:
    static void CheckRestoredOffsets();
    int current_server_group_;
};

// IDA: GetConfigFileString @0x829D880.
const char* GetConfigFileString(const ConfigTagMap& values, const char* tag);

// IDA: G_CEnvironment @0x80CC181 -> GlobalInstance<CEnvironment>::inst_ptr().
CEnvironment* G_CEnvironment();

typedef GlobalInstance<CEnvironment> GlobalEnvironmentInstance;


#endif  // OLDSRV_ENVIRONMENT_ENVIRONMENT_H_
