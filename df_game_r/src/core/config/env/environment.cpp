#include "core/config/env/environment.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <type_traits>

#include <arpa/inet.h>


namespace {

// IDA: CEnvironment::CEnvironment @0x80CC00C initializes CTEA(this+4) with
// this fixed key and CTEA::sm_chain0 before DecryptValue is ever called.
const char kDbPasswordTeaKey[] = "qortmddkqortmdcksqordudwlswjdguswn";

void copy_cstr(char* dest, std::size_t dest_size, const char* src, std::size_t ida_n) {
    if (dest == 0 || dest_size == 0) {
        return;
    }
    const std::size_t n = ida_n < dest_size ? ida_n : dest_size - 1;
    std::memset(dest, 0, dest_size);
    if (src != 0) {
        std::strncpy(dest, src, n);
    }
    dest[dest_size - 1] = '\0';
}

char* trim(char* text) {
    if (text == 0) {
        return text;
    }
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        ++text;
    }
    char* end = text + std::strlen(text);
    while (end > text) {
        const char ch = *(end - 1);
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            break;
        }
        --end;
    }
    *end = '\0';
    return text;
}

int game_db_type_to_server_group(const char* value) {
    if (value == 0) {
        return 0;
    }
    if (std::strcmp(value, "cain") == 0) return 1;
    if (std::strcmp(value, "diregie") == 0) return 2;
    if (std::strcmp(value, "siroco") == 0) return 3;
    if (std::strcmp(value, "prey") == 0) return 4;
    if (std::strcmp(value, "casillas") == 0) return 5;
    if (std::strcmp(value, "hilder") == 0) return 6;
    if (std::strcmp(value, "ruke") == 0) return 7;
    if (std::strcmp(value, "seria") == 0) return 8;
    if (std::strcmp(value, "anton") == 0) return 9;
    if (std::strcmp(value, "roxy") == 0) return 10;
    if (std::strcmp(value, "bakal") == 0) return 11;
    if (std::strcmp(value, "ozma") == 0) return 12;
    if (std::strcmp(value, "khazan") == 0) return 13;
    if (std::strcmp(value, "starter_server") == 0) return 17;
    if (std::strcmp(value, "first1") == 0) return 99;
    if (std::strcmp(value, "first2") == 0) return 100;
    return 0;
}

bool hex_nibble(char ch, uint8_t& value) {
    if (ch >= '0' && ch <= '9') {
        value = static_cast<uint8_t>(ch - '0');
        return true;
    }
    if (ch >= 'a' && ch <= 'f') {
        value = static_cast<uint8_t>(ch - 'a' + 10);
        return true;
    }
    return false;
}

bool hex_to_binary_24(const char* src, uint8_t* out) {
    if (src == 0 || out == 0) {
        return false;
    }
    for (int i = 0; i < 24; ++i) {
        uint8_t high = 0;
        uint8_t low = 0;
        if (!hex_nibble(src[i * 2], high) || !hex_nibble(src[i * 2 + 1], low)) {
            return false;
        }
        out[i] = static_cast<uint8_t>((high << 4) | low);
    }
    return true;
}

}  // namespace

SK_IP::SK_IP() {
    std::memset(text, 0, sizeof(text));
}

SK_IP::SK_IP(const char* value) {
    std::memset(text, 0, sizeof(text));
    if (value != 0) {
        // CEnvironment::parse @0x82906BE uses strncpy(dest, value, 0x10)
        // after zeroing the exact 16-byte SK_IP storage.
        std::strncpy(text, value, sizeof(text));
    }
}

// IDA_EA: 0x80CBCA8
// IDA_NAME: _ZN12Server_EnvirC2Ev
// DEMANGLED: Server_Envir::Server_Envir(void)
Server_Envir::Server_Envir() {
    std::memset(ipg_ip, 0, sizeof(ipg_ip));
    accept_lock_dbqueue_count = 0;
    accept_unlock_dbqueue_count = 0;

    small_stream_pool_count = 3000;
    stream_pool_count = 3000;
    big_stream_pool_count = 500;

    std::memset(pcroom_server_ip, 0, sizeof(pcroom_server_ip));
    pcroom_server_port = 0;
    std::memset(community_server_ip, 0, sizeof(community_server_ip));
    community_server_port = 0;
    std::memset(auction_server_ip, 0, sizeof(auction_server_ip));
    auction_server_port = 0;
    std::memset(cera_auction_server_ip, 0, sizeof(cera_auction_server_ip));
    cera_auction_server_port = 0;

    std::memset(system_monitoring_ip, 0, sizeof(system_monitoring_ip));
    std::strncpy(system_monitoring_ip, "127.0.0.1",
                 sizeof(system_monitoring_ip) - 1);
    scriptpacks = false;
    // 0828 extends the valid channel-type range with Zombie (19) and
    // Tournament (20).  Type 21 is the 0828 "not resolved" sentinel.
    channel_type = 21;
    validate_script = true;
    garena_publickey = "publickey.pem";
}

// IDA_EA: 0x80CBF36
// IDA_NAME: _ZN12Server_EnvirD2Ev
// DEMANGLED: Server_Envir::~Server_Envir()
Server_Envir::~Server_Envir() {}

// IDA_EA: 0x80CC00C
// IDA_NAME: _ZN12CEnvironmentC2Ev
// DEMANGLED: CEnvironment::CEnvironment(void)
CEnvironment::CEnvironment()
    : password_decryptor_(),
      tournament_config_values_(),
      db_conn_info_(),
      server_group_db_conn_info_(),
      server_envir_(),
      mutex_() {
    CheckRestoredOffsets();
    suspend_state_ = false;
    password_decryptor_.Initialize(kDbPasswordTeaKey, 33,
                                   CTEA::sm_chain0, 0, 0);
    std::memset(server_envir_.exchange_server_ip, 0,
                sizeof(server_envir_.exchange_server_ip));
    server_envir_.exchange_server_port = 0;
}

// IDA_EA: 0x82A6BEE
// IDA_NAME: _ZN12CEnvironmentD2Ev
// DEMANGLED: CEnvironment::~CEnvironment()
CEnvironment::~CEnvironment() {}

// IDA_EA: 0x8290450
// IDA_NAME: _ZN12CEnvironment4loadEv
// DEMANGLED: CEnvironment::load(void)
bool CEnvironment::load() {
    char path[34];
    std::memset(path, 0, sizeof(path));
    std::snprintf(path, sizeof(path), "./cfg/%s.cfg", get_file_name());

    FILE* stream = std::fopen(path, "rb");
    if (stream == 0) {
        return false;
    }

    char line[1024];
    while (fgetline(stream, line)) {
        if (!check_comment(line) && !parse(line)) {
            std::fclose(stream);
            return false;
        }
    }
    std::fclose(stream);
    return true;
}

// IDA_EA: 0x82906BE
// IDA_NAME: _ZN12CEnvironment5parseEPc
// DEMANGLED: CEnvironment::parse(char *)
bool CEnvironment::parse(char* line) {
    char* key = std::strtok(line, "=");
    char* value = std::strtok(0, "=");
    key = trim(key);
    value = trim(value);
    if (key == 0 || value == 0 || key[0] == '\0') {
        return true;
    }

    STDBConnInfo& master = db_conn_info_[DB_HANDLE_0];
    if (std::strcmp(key, "master_db_ip") == 0) {
        copy_cstr(master.db_ip, sizeof(master.db_ip), value, 0x10u);
    } else if (std::strcmp(key, "master_db_port") == 0) {
        master.db_port = static_cast<unsigned int>(std::strtoul(value, 0, 10));
    } else if (std::strcmp(key, "master_db_acc") == 0) {
        copy_cstr(master.db_userid, sizeof(master.db_userid), value, 0x14u);
    } else if (std::strcmp(key, "master_db_pwd") == 0) {
        if (!decrypt_or_copy_db_password(value, master.db_passwd)) {
            return false;
        }
    } else if (std::strcmp(key, "master_db_name") == 0) {
        copy_cstr(master.db_name, sizeof(master.db_name), value, 0x1Eu);
    } else if (std::strcmp(key, "db_tbl_file") == 0) {
        copy_cstr(db_tbl_file_, sizeof(db_tbl_file_), value, 0x100u);
    } else if (std::strcmp(key, "gc_no") == 0) {
        server_envir_.gc_no = std::atoi(value);
    } else if (std::strcmp(key, "server_type") == 0) {
        server_envir_.server_type = std::atoi(value);
    } else if (std::strcmp(key, "small_pool_size") == 0) {
        // IDA: CEnvironment::parse @0x82906BE writes atoi(value) to +0x19C.
        server_envir_.small_stream_pool_count = std::atoi(value);
    } else if (std::strcmp(key, "default_pool_size") == 0) {
        // IDA: CEnvironment::parse @0x82906BE writes atoi(value) to +0x1A0.
        server_envir_.stream_pool_count = std::atoi(value);
    } else if (std::strcmp(key, "big_pool_size") == 0) {
        // IDA: CEnvironment::parse @0x82906BE writes atoi(value) to +0x1A4.
        server_envir_.big_stream_pool_count = std::atoi(value);
    } else if (std::strcmp(key, "process_sequence") == 0) {
        // IDA: CEnvironment::parse @0x82906BE writes atoi(value) to +0x1AC.
        server_envir_.process_sequence = std::atoi(value);
    } else if (std::strcmp(key, "ip") == 0) {
        copy_cstr(server_envir_.server_ip, sizeof(server_envir_.server_ip), value, sizeof(server_envir_.server_ip) - 1);
        server_envir_.server_ip_address =
            static_cast<uint32_t>(::inet_addr(server_envir_.server_ip));
    } else if (std::strcmp(key, "tcp_port") == 0) {
        server_envir_.tcp_port = std::atoi(value);
    } else if (std::strcmp(key, "server_group") == 0) {
        server_envir_.server_group = std::atoi(value);
    } else if (std::strcmp(key, "fatigue_time") == 0) {
        server_envir_.fatigue_time = std::atoi(value);
    } else if (std::strcmp(key, "avatar_time") == 0) {
        server_envir_.avatar_time = std::atoi(value);
    } else if (std::strcmp(key, "channel_no") == 0) {
        server_envir_.channel_no = std::atoi(value);
    } else if (std::strcmp(key, "udp_port") == 0) {
        server_envir_.udp_port = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_channel") == 0) {
        // IDA CEnvironment::parse @0x8291835: strncpy(this + 0x240, value, 0x10).
        copy_cstr(server_envir_.udp_ip_channel, sizeof(server_envir_.udp_ip_channel), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_channel") == 0) {
        // IDA CEnvironment::parse @0x8291908: *(int*)(this + 0x250) = atoi(value).
        server_envir_.channel_bridge_port = std::atoi(value);
    } else if (std::strcmp(key, "tcp_port_of_channel") == 0) {
        // The old TCP/UDP channel configuration tags intentionally share +0x250.
        server_envir_.channel_bridge_port = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_monitor") == 0) {
        copy_cstr(server_envir_.udp_ip_monitor, sizeof(server_envir_.udp_ip_monitor), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_monitor") == 0) {
        server_envir_.udp_port_monitor = std::atoi(value);
    } else if (std::strcmp(key, "tcp_port_of_monitor") == 0) {
        server_envir_.tcp_port_monitor = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_guild") == 0) {
        copy_cstr(server_envir_.udp_ip_guild, sizeof(server_envir_.udp_ip_guild), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_guild") == 0) {
        server_envir_.udp_port_guild = std::atoi(value);
    } else if (std::strcmp(key, "tcp_port_of_guild") == 0) {
        server_envir_.tcp_port_guild = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_statistic") == 0) {
        copy_cstr(server_envir_.udp_ip_statistic, sizeof(server_envir_.udp_ip_statistic), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_statistic") == 0) {
        server_envir_.udp_port_statistic = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_doublecheck") == 0) {
        copy_cstr(server_envir_.udp_ip_doublecheck, sizeof(server_envir_.udp_ip_doublecheck), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_doublecheck") == 0) {
        server_envir_.udp_port_doublecheck = std::atoi(value);
    } else if (std::strcmp(key, "udp_ip_of_hades") == 0) {
        copy_cstr(server_envir_.udp_ip_hades, sizeof(server_envir_.udp_ip_hades), value, 0x10u);
    } else if (std::strcmp(key, "udp_port_of_hades") == 0) {
        server_envir_.udp_port_hades = std::atoi(value);
    } else if (std::strcmp(key, "tcp_port_of_hades") == 0) {
        server_envir_.tcp_port_hades = std::atoi(value);
    } else if (std::strcmp(key, "school_server_ip") == 0) {
        copy_cstr(server_envir_.school_server_ip, sizeof(server_envir_.school_server_ip), value, 0x10u);
    } else if (std::strcmp(key, "school_server_port") == 0) {
        server_envir_.school_server_port = std::atoi(value);
    } else if (std::strcmp(key, "pcroom_server_ip") == 0) {
        copy_cstr(server_envir_.pcroom_server_ip, sizeof(server_envir_.pcroom_server_ip), value, 0x10u);
    } else if (std::strcmp(key, "pcroom_server_port") == 0) {
        server_envir_.pcroom_server_port = std::atoi(value);
    } else if (std::strcmp(key, "ipg_ip") == 0) {
        std::strncpy(server_envir_.ipg_ip, value, 0x10u);
    } else if (std::strcmp(key, "accept_lock_dbqueue_count") == 0) {
        server_envir_.accept_lock_dbqueue_count = std::atoi(value);
    } else if (std::strcmp(key, "accept_unlock_dbqueue_count") == 0) {
        server_envir_.accept_unlock_dbqueue_count = std::atoi(value);
    } else if (std::strcmp(key, "community_server_ip") == 0) {
        copy_cstr(server_envir_.community_server_ip, sizeof(server_envir_.community_server_ip), value, 0x10u);
    } else if (std::strcmp(key, "community_server_port") == 0) {
        server_envir_.community_server_port = std::atoi(value);
    } else if (std::strcmp(key, "auction_server_ip") == 0) {
        copy_cstr(server_envir_.auction_server_ip, sizeof(server_envir_.auction_server_ip), value, 0x10u);
    } else if (std::strcmp(key, "auction_server_port") == 0) {
        server_envir_.auction_server_port = std::atoi(value);
    } else if (std::strcmp(key, "cera_auction_server_ip") == 0) {
        copy_cstr(server_envir_.cera_auction_server_ip, sizeof(server_envir_.cera_auction_server_ip), value, 0x10u);
    } else if (std::strcmp(key, "cera_auction_server_port") == 0) {
        server_envir_.cera_auction_server_port = std::atoi(value);
    } else if (std::strcmp(key, "system_monitoring_ip") == 0) {
        copy_cstr(server_envir_.system_monitoring_ip, sizeof(server_envir_.system_monitoring_ip), value, 0x10u);
    } else if (std::strcmp(key, "max_user_num") == 0) {
        max_user_num_ = std::atoi(value);
    } else if (std::strcmp(key, "channel_name") == 0) {
        set_channel_name(value);
    } else if (std::strcmp(key, "exchange_server_ip") == 0) {
        copy_cstr(server_envir_.exchange_server_ip, sizeof(server_envir_.exchange_server_ip), value,
                  sizeof(server_envir_.exchange_server_ip) - 1);
    } else if (std::strcmp(key, "exchange_server_port") == 0) {
        server_envir_.exchange_server_port = std::atoi(value);
    } else if (std::strcmp(key, "lls_keys") == 0) {
        // IDA: tag "lls_keys" copies with strncpy(this+0x318, value, 0x2D).
        copy_cstr(server_envir_.lls_key, sizeof(server_envir_.lls_key), value, 0x2Du);
    } else if (std::strcmp(key, "relay_ip") == 0) {
        // IDA: CEnvironment::parse @0x82906BE, strncpy(this+0x345, value, 0x10).
        std::strncpy(server_envir_.relay_ip, value, sizeof(server_envir_.relay_ip));
    } else if (std::strcmp(key, "relay_tcp_port") == 0) {
        // IDA: CEnvironment::parse @0x82906BE, *(int*)(this+0x358) = atoi(value).
        server_envir_.relay_tcp_port = std::atoi(value);
    } else if (std::strcmp(key, "relay_udp_port") == 0) {
        // IDA: CEnvironment::parse @0x82906BE, *(int*)(this+0x35C) = atoi(value).
        server_envir_.relay_udp_port = std::atoi(value);
    } else if (std::strcmp(key, "stun_ip") == 0) {
        // IDA: CEnvironment::parse @0x82906BE zeros a local SK_IP, copies
        // exactly 16 bytes, then appends every repeated "stun_ip" entry.
        SK_IP entry;
        std::strncpy(entry.text, value, sizeof(entry.text));
        server_envir_.sk_ip_list.push_back(entry);
    } else if (std::strcmp(key, "stun_port") == 0) {
        // IDA: CEnvironment::parse @0x82906BE appends every repeated port.
        server_envir_.sk_port_values.push_back(std::atoi(value));
    } else if (std::strcmp(key, "garena_publickey") == 0) {
        char buf[256];
        std::memset(buf, 0, sizeof(buf));
        std::strncpy(buf, value, 0x80u);
        buf[sizeof(buf) - 1] = '\0';
        server_envir_.garena_publickey = buf;
    } else if (std::strcmp(key, "script_dir") == 0) {
        char buf[256];
        std::memset(buf, 0, sizeof(buf));
        std::strncpy(buf, value, 0x100u);
        buf[sizeof(buf) - 1] = '\0';
        server_envir_.script_dir = buf;
    } else if (std::strcmp(key, "priority_script_dir") == 0) {
        push_priority_script_dir(value);
    } else if (std::strcmp(key, "scriptpacks") == 0) {
        server_envir_.scriptpacks = IsEnableValue(value);
    } else if (std::strcmp(key, "channel_type") == 0) {
        server_envir_.channel_type = std::atoi(value);
    } else if (std::strcmp(key, "validate_script") == 0) {
        server_envir_.validate_script = IsEnableValue(value);
    }

    return true;
}

// IDA_EA: 0x8290540
// IDA_NAME: _ZN12CEnvironment8fgetlineEP8_IO_FILEPc
// DEMANGLED: CEnvironment::fgetline(_IO_FILE *,char *)
bool CEnvironment::fgetline(FILE* stream, char* line) {
    if (stream == 0 || line == 0) {
        return false;
    }
    std::memset(line, 0, 0x400u);
    for (int index = 0; index <= 1023; ++index) {
        const int ch = std::fgetc(stream);
        if (std::feof(stream)) {
            line[index] = '\0';
            return index > 0;
        }
        line[index] = static_cast<char>(ch);
        if (line[index] == '\n') {
            line[index] = '\0';
            if (index > 0 && line[index - 1] == '\r') {
                line[index - 1] = '\0';
            }
            return true;
        }
    }
    return true;
}

// IDA_EA: 0x8290610
// IDA_NAME: _ZN12CEnvironment13check_commentEPc
// DEMANGLED: CEnvironment::check_comment(char *)
bool CEnvironment::check_comment(const char* line) {
    if (line == 0 || line[0] == '\0') {
        return true;
    }
    if (line[0] == '[' || line[0] == '\r' || line[0] == '\n') {
        return true;
    }
    for (int index = 0; index <= 1021; ++index) {
        if (line[index] == '/' && line[index + 1] == '/') {
            return true;
        }
        if (line[index] == '\0') {
            return false;
        }
    }
    return false;
}

// IDA_EA: 0x817DFF6
// IDA_NAME: _ZN12CEnvironment13set_file_nameEPKc
// DEMANGLED: CEnvironment::set_file_name(char const*)
void CEnvironment::set_file_name(const char* name) {
    std::strcpy(file_name_, name);
}

// IDA_EA: 0x80DA39A
// IDA_NAME: _ZN12CEnvironment13get_file_nameEv
// DEMANGLED: CEnvironment::get_file_name(void)
const char* CEnvironment::get_file_name() {
    return file_name_;
}

// IDA_EA: 0x8292FD4
// IDA_NAME: _ZN12CEnvironment12DecryptValueEPKcPc
// DEMANGLED: CEnvironment::DecryptValue(char const*,char *)
bool CEnvironment::DecryptValue(const char* encrypted_hex, char* dest) {
    uint8_t encrypted[24];
    char decrypted[64];
    std::memset(decrypted, 0, sizeof(decrypted));
    if (!hex_to_binary_24(encrypted_hex, encrypted)) {
        return false;
    }
    if (!decrypt_db_password_block(encrypted, decrypted)) {
        return false;
    }
    std::strncpy(dest, decrypted, 0x14u);
    return true;
}

const DBConnInfoMap& CEnvironment::db_conn_info_map() const {
    return db_conn_info_;
}

DBConnInfoMap& CEnvironment::db_conn_info_map() {
    return db_conn_info_;
}

const TournamentConfigMap& CEnvironment::tournament_config_map() const {
    return tournament_config_values_;
}

TournamentConfigMap& CEnvironment::tournament_config_map() {
    return tournament_config_values_;
}

const ServerGroupDBConnInfoMap&
CEnvironment::server_group_db_conn_info_map() const {
    return server_group_db_conn_info_;
}

ServerGroupDBConnInfoMap&
CEnvironment::server_group_db_conn_info_map() {
    return server_group_db_conn_info_;
}

const char* CEnvironment::get_db_tbl_file() const {
    return db_tbl_file_;
}

// IDA_EA: 0x8106CE0
// IDA_NAME: _ZN12CEnvironment16get_server_groupEv
// DEMANGLED: CEnvironment::get_server_group(void)
int CEnvironment::get_server_group() {
    return server_envir_.server_group;
}

int CEnvironment::get_fatigue_time() const {
    return server_envir_.fatigue_time;
}

int CEnvironment::get_avatar_time() const {
    return server_envir_.avatar_time;
}

// IDA_EA: 0x814AACA
// IDA_NAME: _ZN12CEnvironment14get_channel_noEv
// DEMANGLED: CEnvironment::get_channel_no(void)
int CEnvironment::get_channel_no() {
    return server_envir_.channel_no;
}

void CEnvironment::set_channel_no(int channel_no) {
    server_envir_.channel_no = channel_no;
}

int CEnvironment::get_channel_type() const {
    return server_envir_.channel_type;
}

void CEnvironment::set_channel_type(int channel_type) {
    server_envir_.channel_type = channel_type;
}

// IDA_EA: 0x844DC6E
// IDA_NAME: _ZN12CEnvironment9get_gc_noEv
// DEMANGLED: CEnvironment::get_gc_no(void)
int CEnvironment::get_gc_no() {
    return server_envir_.gc_no;
}

// IDA_EA: 0x8234524
// IDA_NAME: _ZN12CEnvironment18get_gc_no_hardcodeE17ENUM_SERVER_GROUP
// DEMANGLED: CEnvironment::get_gc_no_hardcode(ENUM_SERVER_GROUP)
int CEnvironment::get_gc_no_hardcode(int proxy_index) {
    // Old offset +432 is the channel number in df_game_r; the current
    // source keeps that semantic value in server_envir_.channel_no until the full class
    // packing conflict with InitOnAccept is resolved.
    return 1000 * proxy_index + server_envir_.channel_no;
}

// IDA_EA: 0x82A6D5C
// IDA_NAME: _ZN12CEnvironment16get_channel_nameEv
// DEMANGLED: CEnvironment::get_channel_name(void)
const char* CEnvironment::get_channel_name() {
    return server_envir_.channel_name;
}

// IDA_EA: 0x82A6D3C
// IDA_NAME: _ZN12CEnvironment16set_channel_nameEPc
// DEMANGLED: CEnvironment::set_channel_name(char *)
void CEnvironment::set_channel_name(const char* channel_name) {
    std::strcpy(server_envir_.channel_name, channel_name);
}

// IDA_EA: 0x82A6D6A
// IDA_NAME: _ZN12CEnvironment12get_udp_portEv
// DEMANGLED: CEnvironment::get_udp_port(void)
int CEnvironment::get_udp_port() {
    return server_envir_.udp_port;
}

// IDA_EA: 0x857F428
// IDA_NAME: _ZN12CEnvironment12get_tcp_portEv
// DEMANGLED: CEnvironment::get_tcp_port(void)
int CEnvironment::get_tcp_port() {
    return server_envir_.tcp_port;
}

// IDA_EA: 0x84ED1DC
// IDA_NAME: _ZN12CEnvironment13get_server_ipEv
// DEMANGLED: CEnvironment::get_server_ip(void)
const char* CEnvironment::get_server_ip() {
    return server_envir_.server_ip;
}

int CEnvironment::get_server_type() const {
    return server_envir_.server_type;
}

int CEnvironment::get_max_user_num() const {
    return max_user_num_;
}

int CEnvironment::get_accept_lock_dbqueue_count() const {
    return server_envir_.accept_lock_dbqueue_count;
}

int CEnvironment::get_accept_unlock_dbqueue_count() const {
    return server_envir_.accept_unlock_dbqueue_count;
}

// IDA_EA: 0x8106CEE
// IDA_NAME: _ZN12CEnvironment20get_tcp_port_channelEv
// DEMANGLED: CEnvironment::get_tcp_port_channel(void)
int CEnvironment::get_tcp_port_channel() {
    return server_envir_.channel_bridge_port;
}

// IDA_EA: 0x82A6E4A
// IDA_NAME: _ZN12CEnvironment20get_udp_port_channelEv
// DEMANGLED: CEnvironment::get_udp_port_channel(void)
int CEnvironment::get_udp_port_channel() {
    return server_envir_.channel_bridge_port;
}

// IDA_EA: 0x82A6E58
// IDA_NAME: _ZN12CEnvironment18get_udp_ip_channelEv
// DEMANGLED: CEnvironment::get_udp_ip_channel(void)
const char* CEnvironment::get_udp_ip_channel() {
    return server_envir_.udp_ip_channel;
}

// IDA_EA: 0x82A6D78
// IDA_NAME: _ZN12CEnvironment20get_udp_port_monitorEv
// DEMANGLED: CEnvironment::get_udp_port_monitor(void)
int CEnvironment::get_udp_port_monitor() {
    return server_envir_.udp_port_monitor;
}

// IDA_EA: 0x82A6D86
// IDA_NAME: _ZN12CEnvironment18get_udp_ip_monitorEv
// DEMANGLED: CEnvironment::get_udp_ip_monitor(void)
const char* CEnvironment::get_udp_ip_monitor() {
    return server_envir_.udp_ip_monitor;
}

// IDA_EA: 0x82A6DBE
// IDA_NAME: _ZN12CEnvironment20get_tcp_port_monitorEv
// DEMANGLED: CEnvironment::get_tcp_port_monitor(void)
int CEnvironment::get_tcp_port_monitor() {
    return server_envir_.tcp_port_monitor;
}

// IDA_EA: 0x82A6D94
// IDA_NAME: _ZN12CEnvironment18get_udp_port_guildEv
// DEMANGLED: CEnvironment::get_udp_port_guild(void)
int CEnvironment::get_udp_port_guild() {
    return server_envir_.udp_port_guild;
}

// IDA_EA: 0x82A6DA2
// IDA_NAME: _ZN12CEnvironment16get_udp_ip_guildEv
// DEMANGLED: CEnvironment::get_udp_ip_guild(void)
const char* CEnvironment::get_udp_ip_guild() {
    return server_envir_.udp_ip_guild;
}

// IDA_EA: 0x82A6DB0
// IDA_NAME: _ZN12CEnvironment18get_tcp_port_guildEv
// DEMANGLED: CEnvironment::get_tcp_port_guild(void)
int CEnvironment::get_tcp_port_guild() {
    return server_envir_.tcp_port_guild;
}

// IDA_EA: 0x82A6DCC
// IDA_NAME: _ZN12CEnvironment22get_udp_port_statisticEv
// DEMANGLED: CEnvironment::get_udp_port_statistic(void)
int CEnvironment::get_udp_port_statistic() {
    return server_envir_.udp_port_statistic;
}

// IDA_EA: 0x82A6DDA
// IDA_NAME: _ZN12CEnvironment20get_udp_ip_statisticEv
// DEMANGLED: CEnvironment::get_udp_ip_statistic(void)
const char* CEnvironment::get_udp_ip_statistic() {
    return server_envir_.udp_ip_statistic;
}

// IDA_EA: 0x82A6DE8
// IDA_NAME: _ZN12CEnvironment23get_udp_port_doublecheckEv
// DEMANGLED: CEnvironment::get_udp_port_doublecheck(void)
int CEnvironment::get_udp_port_doublecheck() {
    return server_envir_.udp_port_doublecheck;
}

// IDA_EA: 0x82A6DF6
// IDA_NAME: _ZN12CEnvironment21get_udp_ip_doublecheckEv
// DEMANGLED: CEnvironment::get_udp_ip_doublecheck(void)
const char* CEnvironment::get_udp_ip_doublecheck() {
    return server_envir_.udp_ip_doublecheck;
}

// IDA_EA: 0x82A6E04
// IDA_NAME: _ZN12CEnvironment16get_udp_ip_hadesEv
// DEMANGLED: CEnvironment::get_udp_ip_hades(void)
const char* CEnvironment::get_udp_ip_hades() {
    return server_envir_.udp_ip_hades;
}

// IDA_EA: 0x82A6E12
// IDA_NAME: _ZN12CEnvironment18get_udp_port_hadesEv
// DEMANGLED: CEnvironment::get_udp_port_hades(void)
int CEnvironment::get_udp_port_hades() {
    return server_envir_.udp_port_hades;
}

// IDA_EA: 0x82A6E20
// IDA_NAME: _ZN12CEnvironment18get_tcp_port_hadesEv
// DEMANGLED: CEnvironment::get_tcp_port_hades(void)
int CEnvironment::get_tcp_port_hades() {
    return server_envir_.tcp_port_hades;
}

// IDA_EA: 0x82A6E2E
// IDA_NAME: _ZN12CEnvironment20get_school_server_ipEv
// DEMANGLED: CEnvironment::get_school_server_ip(void)
const char* CEnvironment::get_school_server_ip() {
    return server_envir_.school_server_ip;
}

// IDA_EA: 0x82A6E3C
// IDA_NAME: _ZN12CEnvironment22get_school_server_portEv
// DEMANGLED: CEnvironment::get_school_server_port(void)
int CEnvironment::get_school_server_port() {
    return server_envir_.school_server_port;
}

// IDA_EA: 0x82A6E82
// IDA_NAME: _ZN12CEnvironment22get_pcroom_server_portEv
// DEMANGLED: CEnvironment::get_pcroom_server_port(void)
int CEnvironment::get_pcroom_server_port() {
    return server_envir_.pcroom_server_port;
}

// IDA_EA: 0x82A6E90
// IDA_NAME: _ZN12CEnvironment20get_pcroom_server_ipEv
// DEMANGLED: CEnvironment::get_pcroom_server_ip(void)
const char* CEnvironment::get_pcroom_server_ip() {
    return server_envir_.pcroom_server_ip;
}

// IDA_EA: 0x82A6E9E
// IDA_NAME: _ZN12CEnvironment25get_community_server_portEv
// DEMANGLED: CEnvironment::get_community_server_port(void)
int CEnvironment::get_community_server_port() {
    return server_envir_.community_server_port;
}

// IDA_EA: 0x82A6EAC
// IDA_NAME: _ZN12CEnvironment23get_community_server_ipEv
// DEMANGLED: CEnvironment::get_community_server_ip(void)
const char* CEnvironment::get_community_server_ip() {
    return server_envir_.community_server_ip;
}

// IDA_EA: 0x82A6EBA
// IDA_NAME: _ZN12CEnvironment23get_auction_server_portEv
// DEMANGLED: CEnvironment::get_auction_server_port(void)
int CEnvironment::get_auction_server_port() {
    return server_envir_.auction_server_port;
}

// IDA_EA: 0x82A6EC8
// IDA_NAME: _ZN12CEnvironment21get_auction_server_ipEv
// DEMANGLED: CEnvironment::get_auction_server_ip(void)
const char* CEnvironment::get_auction_server_ip() {
    return server_envir_.auction_server_ip;
}

// IDA_EA: 0x82A6ED6
// IDA_NAME: _ZN12CEnvironment28get_cera_auction_server_portEv
// DEMANGLED: CEnvironment::get_cera_auction_server_port(void)
int CEnvironment::get_cera_auction_server_port() {
    return server_envir_.cera_auction_server_port;
}

// IDA_EA: 0x82A6EE4
// IDA_NAME: _ZN12CEnvironment26get_cera_auction_server_ipEv
// DEMANGLED: CEnvironment::get_cera_auction_server_ip(void)
const char* CEnvironment::get_cera_auction_server_ip() {
    return server_envir_.cera_auction_server_ip;
}

// IDA_EA: 0x857F436
// IDA_NAME: _ZN12CEnvironment24get_system_monitoring_ipEv
// DEMANGLED: CEnvironment::get_system_monitoring_ip(void)
const char* CEnvironment::get_system_monitoring_ip() {
    return server_envir_.system_monitoring_ip;
}

// IDA_EA: 0x8234516
// IDA_NAME: _ZNK12CEnvironment18getProcessSequenceEv
// DEMANGLED: CEnvironment::getProcessSequence(void)const
int CEnvironment::getProcessSequence() const {
    return server_envir_.process_sequence;
}

// IDA_EA: 0x8293084
// IDA_NAME: _ZN12CEnvironment28adjust_base_script_directoryEv
// DEMANGLED: CEnvironment::adjust_base_script_directory(void)
void CEnvironment::adjust_base_script_directory() {
    if (server_envir_.script_dir.empty()) {
        server_envir_.script_dir = "./Script/";
    }
}

// IDA_EA: 0x82930B8
// IDA_NAME: _ZN12CEnvironment17parse_se_event_dbEPcS0_
// DEMANGLED: CEnvironment::parse_se_event_db(char *,char *)
bool CEnvironment::parse_se_event_db(char* key, char* value) {
    if (key == 0 || value == 0) {
        return false;
    }
    server_group_db_conn_info_[
        static_cast<ENUM_SERVER_GROUP>(server_envir_.server_group)][DB_HANDLE_0] =
        db_conn_info_[DB_HANDLE_0];
    return true;
}

// IDA_EA: 0x82A6CCC
// IDA_NAME: _ZN12CEnvironment4lockEv
// DEMANGLED: CEnvironment::lock(void)
int CEnvironment::lock() {
    return mutex_.lock();
}

// IDA_EA: 0x82A6CE4
// IDA_NAME: _ZN12CEnvironment6unlockEv
// DEMANGLED: CEnvironment::unlock(void)
int CEnvironment::unlock() {
    return mutex_.unlock();
}

const std::string& CEnvironment::script_dir() const {
    return server_envir_.script_dir;
}

const std::vector<std::string>& CEnvironment::priority_script_dirs() const {
    return server_envir_.priority_script_dirs;
}

bool CEnvironment::scriptpacks() const {
    return server_envir_.scriptpacks;
}

bool CEnvironment::validate_script() const {
    return server_envir_.validate_script;
}

// IDA_EA: 0x82A6E74
// IDA_NAME: _ZN12CEnvironment22get_exchange_server_ipEv
// DEMANGLED: CEnvironment::get_exchange_server_ip(void)
const char* CEnvironment::get_exchange_server_ip() {
    return server_envir_.exchange_server_ip;
}

// IDA_EA: 0x82A6E66
// IDA_NAME: _ZN12CEnvironment24get_exchange_server_portEv
// DEMANGLED: CEnvironment::get_exchange_server_port(void)
int CEnvironment::get_exchange_server_port() {
    return server_envir_.exchange_server_port;
}

const char* CEnvironment::get_lls_key() const {
    return server_envir_.lls_key;
}

const char* CEnvironment::get_garena_publickey() const {
    return server_envir_.garena_publickey.c_str();
}

// IDA_EA: 0x82A6CFC
// IDA_NAME: _ZN12CEnvironment18set_operation_typeE19eStartArgumentType_t
// DEMANGLED: CEnvironment::set_operation_type(eStartArgumentType_t)
void CEnvironment::set_operation_type(eStartArgumentType_t operation_type) {
    operation_type_ = operation_type;
}

// IDA_EA: 0x8375F4C
// IDA_NAME: _ZN12CEnvironment18get_operation_typeEv
// DEMANGLED: CEnvironment::get_operation_type(void)
eStartArgumentType_t CEnvironment::get_operation_type() {
    return operation_type_;
}

eStartArgumentType_t CEnvironment::operation_type() const {
    return operation_type_;
}

// IDA_EA: 0x82A6D0A
// IDA_NAME: _ZN12CEnvironment17set_suspend_stateEb
// DEMANGLED: CEnvironment::set_suspend_state(bool)
void CEnvironment::set_suspend_state(bool suspend) {
    suspend_state_ = suspend;
}

// IDA_EA: 0x857F418
// IDA_NAME: _ZN12CEnvironment17get_suspend_stateEv
// DEMANGLED: CEnvironment::get_suspend_state(void)
bool CEnvironment::get_suspend_state() {
    return suspend_state_;
}

bool CEnvironment::suspend_state() const {
    return suspend_state_;
}

int CEnvironment::get_small_stream_pool_count() const {
    return server_envir_.small_stream_pool_count;
}

int CEnvironment::get_stream_pool_count() const {
    return server_envir_.stream_pool_count;
}

int CEnvironment::get_big_stream_pool_count() const {
    return server_envir_.big_stream_pool_count;
}

uint8_t CEnvironment::get_accept_handshake_flag_888() const {
    return static_cast<uint8_t>(server_envir_.server_group);
}

uint8_t CEnvironment::get_accept_handshake_flag_432() const {
    return static_cast<uint8_t>(server_envir_.channel_no);
}

uint8_t CEnvironment::get_accept_handshake_flag_424() const {
    return static_cast<uint8_t>(server_envir_.server_type);
}

const std::vector<SK_IP>& CEnvironment::get_sk_ip_list() const {
    return server_envir_.sk_ip_list;
}

const std::vector<int>& CEnvironment::get_sk_port_values() const {
    return server_envir_.sk_port_values;
}

bool CEnvironment::decrypt_db_password_block(const uint8_t* src, char* dest) {
    if (src == 0 || dest == 0) {
        return false;
    }
    // IDA @0x8292FD4 calls CTEA::Decrypt on the physical CTEA subobject at
    // CEnvironment+0x4.  Do not duplicate the cipher or maintain a side key.
    password_decryptor_.Decrypt(reinterpret_cast<const char*>(src), dest, 24u);
    return true;
}

// IDA_EA: 0x8292F64
// IDA_NAME: _ZN12CEnvironment13IsEnableValueEPKc
// DEMANGLED: CEnvironment::IsEnableValue(char const*)
bool CEnvironment::IsEnableValue(const char* value) {
    if (value == 0) {
        return false;
    }
    return std::strcmp(value, "yes") == 0 ||
           std::strcmp(value, "YES") == 0 ||
           std::strcmp(value, "Yes") == 0 ||
           std::strcmp(value, "1") == 0;
}

void CEnvironment::push_priority_script_dir(const char* value) {
    char buf[256];
    std::memset(buf, 0, sizeof(buf));
    if (value != 0) {
        std::strncpy(buf, value, 0x100u);
        buf[sizeof(buf) - 1] = '\0';
    }
    const std::size_t len = std::strlen(buf);
    if (len > 0 && len < sizeof(buf) - 1 &&
        buf[len - 1] != '\\' && buf[len - 1] != '/') {
        buf[len] = '/';
        buf[len + 1] = '\0';
    }
    server_envir_.priority_script_dirs.push_back(std::string(buf));
}

void CEnvironment::CheckRestoredOffsets() {
    // The vtable at 0x8C1BA38 contains exactly one callable entry,
    // CEnvironment::parse @0x82906BE.  The absence of D0 is intentional.
    static_assert(std::is_polymorphic<CEnvironment>::value,
                  "CEnvironment must retain the parse vtable");
    static_assert(!std::has_virtual_destructor<CEnvironment>::value,
                  "CEnvironment destructor is non-virtual in the 0627 ELF");
#if defined(__i386__)
    static_assert(sizeof(CTEA) == 0x48u, "CTEA ABI drift");
    static_assert(sizeof(Mutex) == 0x18u, "Mutex ABI drift");
    static_assert(sizeof(Server_Envir) == 0x20Au, "Server_Envir ABI drift");

    static_assert(offsetof(Server_Envir, gc_no) == 0x000u, "gc_no offset");
    static_assert(offsetof(Server_Envir, server_type) == 0x010u,
                  "server_type offset");
    static_assert(offsetof(Server_Envir, channel_no) == 0x018u,
                  "channel_no offset");
    static_assert(offsetof(Server_Envir, server_ip) == 0x01Cu,
                  "server_ip offset");
    static_assert(offsetof(Server_Envir, udp_ip_channel) == 0x0A8u,
                  "channel endpoint offset");
    static_assert(offsetof(Server_Envir, exchange_server_ip) == 0x0C0u,
                  "exchange endpoint offset");
    static_assert(offsetof(Server_Envir, system_monitoring_ip) == 0x110u,
                  "system monitoring offset");
    static_assert(offsetof(Server_Envir, ipg_ip) == 0x120u,
                  "ipg offset");
    static_assert(offsetof(Server_Envir, script_dir) == 0x160u,
                  "script_dir offset");
    static_assert(offsetof(Server_Envir, garena_publickey) == 0x17Cu,
                  "public key offset");
    static_assert(offsetof(Server_Envir, lls_key) == 0x180u,
                  "lls key offset");
    static_assert(offsetof(Server_Envir, relay_ip) == 0x1ADu,
                  "relay endpoint offset");
    static_assert(offsetof(Server_Envir, sk_ip_list) == 0x1C8u,
                  "STUN IP vector offset");
    static_assert(offsetof(Server_Envir, server_group) == 0x1E0u,
                  "server group offset");
    static_assert(offsetof(Server_Envir, channel_name) == 0x1ECu,
                  "channel name offset");

    static_assert(offsetof(CEnvironment, password_decryptor_) == 0x004u,
                  "CTEA subobject offset");
    static_assert(offsetof(CEnvironment, operation_type_) == 0x04Cu,
                  "operation type offset");
    static_assert(offsetof(CEnvironment, tournament_config_values_) == 0x050u,
                  "tournament map offset");
    static_assert(offsetof(CEnvironment, db_conn_info_) == 0x068u,
                  "DB map offset");
    static_assert(offsetof(CEnvironment, server_group_db_conn_info_) == 0x080u,
                  "server-group DB map offset");
    static_assert(offsetof(CEnvironment, db_tbl_file_) == 0x098u,
                  "DB table filename offset");
    static_assert(offsetof(CEnvironment, server_envir_) == 0x198u,
                  "Server_Envir subobject offset");
    static_assert(offsetof(CEnvironment, file_name_) == 0x3A2u,
                  "config filename offset");
    static_assert(offsetof(CEnvironment, max_user_num_) == 0x3C0u,
                  "max user count offset");
    static_assert(offsetof(CEnvironment, mutex_) == 0x3C4u,
                  "Mutex subobject offset");
    static_assert(offsetof(CEnvironment, suspend_state_) == 0x3DCu,
                  "suspend state offset");
    static_assert(sizeof(CEnvironment) == 0x3E0u,
                  "CEnvironment ABI drift");
#endif
}

CDBEnviroment::CDBEnviroment() : CEnvironment(), current_server_group_(0) {
    CheckRestoredOffsets();
}

CDBEnviroment::~CDBEnviroment() {}

bool CDBEnviroment::parse(char* line) {
    char* key = std::strtok(line, "=");
    char* value = std::strtok(0, "=");
    key = trim(key);
    value = trim(value);
    if (key == 0 || value == 0 || key[0] == '\0') {
        return true;
    }

    CEnvironment* global_env = G_CEnvironment();
    if (std::strcmp(key, "game_db_type") == 0) {
        current_server_group_ = game_db_type_to_server_group(value);
        if (current_server_group_ == 0) {
            std::fprintf(stderr, "Unknown value (%s)\n", value);
            return false;
        }
        global_env->tournament_config_map()[
            static_cast<ENUM_SERVER_GROUP>(current_server_group_)] = ConfigTagMap();
        return true;
    }

    if (current_server_group_ == 0) {
        return true;
    }
    global_env->tournament_config_map()[
        static_cast<ENUM_SERVER_GROUP>(current_server_group_)][key] = value;
    return true;
}

int CDBEnviroment::current_server_group() const {
    return current_server_group_;
}

void CDBEnviroment::CheckRestoredOffsets() {
    static_assert(std::is_polymorphic<CDBEnviroment>::value,
                  "CDBEnviroment must override the parse vtable slot");
    static_assert(!std::has_virtual_destructor<CDBEnviroment>::value,
                  "CDBEnviroment destructor is non-virtual in the 0627 ELF");
#if defined(__i386__)
    static_assert(offsetof(CDBEnviroment, current_server_group_) == 0x3E0u,
                  "CDBEnviroment parser context offset");
    static_assert(sizeof(CDBEnviroment) == 0x3E4u,
                  "CDBEnviroment ABI drift");
#endif
}

const char* GetConfigFileString(const ConfigTagMap& values, const char* tag) {
    ConfigTagMap::const_iterator it = values.find(tag == 0 ? "" : tag);
    if (it != values.end() && !it->second.empty()) {
        return it->second.c_str();
    }
    std::fprintf(stderr, "%s tag error\n", tag == 0 ? "" : tag);
    return "";
}

bool CEnvironment::decrypt_or_copy_db_password(const char* value, char* dest) {
    if (value == 0 || dest == 0) {
        return false;
    }
    if (std::strlen(value) >= 48 && DecryptValue(value, dest)) {
        return true;
    }
    copy_cstr(dest, 21, value, 0x14u);
    return true;
}

template <>
CEnvironment* GlobalInstance<CEnvironment>::inst_ptr();
template <>
CEnvironment* GlobalInstance<CEnvironment>::create();
template <>
CEnvironment* GlobalInstance<CEnvironment>::inst();
template <>
void GlobalInstance<CEnvironment>::destroy();
template <>
CEnvironment* GlobalInstance<CEnvironment>::m_p = 0;

// IDA_EA: 0x80CD5A2
// IDA_NAME: _ZN14GlobalInstanceI12CEnvironmentE8inst_ptrEv
// DEMANGLED: GlobalInstance<CEnvironment>::inst_ptr(void)
template <>
CEnvironment* GlobalInstance<CEnvironment>::inst_ptr() {
    GlobalInstance<CEnvironment>::create();
    return GlobalInstance<CEnvironment>::m_p;
}

// IDA_EA: 0x80CF089
// IDA_NAME: _ZN14GlobalInstanceI12CEnvironmentE6createEv
// DEMANGLED: GlobalInstance<CEnvironment>::create(void)
template <>
CEnvironment* GlobalInstance<CEnvironment>::create() {
    if (GlobalInstance<CEnvironment>::m_p == 0) {
        GlobalInstance<CEnvironment>::m_p = new CEnvironment();
    }
    return GlobalInstance<CEnvironment>::m_p;
}

// IDA_EA: 0x82AB6E2
// IDA_NAME: _ZN14GlobalInstanceI12CEnvironmentE4instEv
// DEMANGLED: GlobalInstance<CEnvironment>::inst(void)
template <>
CEnvironment* GlobalInstance<CEnvironment>::inst() {
    GlobalInstance<CEnvironment>::create();
    return GlobalInstance<CEnvironment>::m_p;
}

// IDA_EA: 0x82AB6F4
// IDA_NAME: _ZN14GlobalInstanceI12CEnvironmentE7destroyEv
// DEMANGLED: GlobalInstance<CEnvironment>::destroy(void)
template <>
void GlobalInstance<CEnvironment>::destroy() {
    if (GlobalInstance<CEnvironment>::m_p != 0) {
        delete GlobalInstance<CEnvironment>::m_p;
        GlobalInstance<CEnvironment>::m_p = 0;
    }
}

// IDA_EA: 0x80CC181
// IDA_NAME: _Z14G_CEnvironmentv
// DEMANGLED: G_CEnvironment(void)
CEnvironment* G_CEnvironment() {
    return GlobalInstance<CEnvironment>::inst_ptr();
}
