#ifndef OLDSRV_OLD_DATA_DB_CORE_MYSQL_H_
#define OLDSRV_OLD_DATA_DB_CORE_MYSQL_H_


class CQueryCounter;

// IDA unit: 13_database_dispatch/MySQL.
class MySQL {
public:
    MySQL();
    ~MySQL();

    bool get_int(int column, unsigned int& out);
    bool is_valid_col(int column);
    bool get_uint(int column, unsigned int& out);
    int get_n_rows();
    bool get_str(int column, char* dest, int n);
    bool get_short(int column, short& out);
    bool get_byte(int column, char& out);
    bool get_ubyte(int column, unsigned char& out);
    bool get_int(int column, int& out);
    bool get_ushort(int column, unsigned short& out);
    int getDBError();
    bool get_binary(int column, void* dest, int n);
    int get_binary_length(int column);
    bool get_short(int column, int& out);
    bool get_char(int column, char& out);
    bool get_ulonglong(int column, unsigned long long& out);
    CQueryCounter* GetQueryCounter();

    bool set_compress_option();
    bool set_read_default_grp_option();
    bool set_charset_name_option();
    bool set_reconnect_option();
    bool init_db_handle();
    bool init();
    MySQL* close();
    bool open(char* host, char* database, char* user, char* password);
    bool open(char* host, unsigned int port, char* database, char* user,
              char* password);
    bool set_query(const char* format, ...);
    unsigned long long getAffectedRowCount();
    bool exec(bool log_error);
    bool fetch();
    int ping();
    char* blob_to_str(int index, void* data, int n);
    const char* getServerVersion();
    const char* getClientVersion();
    int select();
    int select(const char* format, ...);
    int insert(const char* format, ...);
    int update(const char* format, ...);
    bool set_query();
    int escape_string(char* dest, const char* src);
    MySQL* SetQueryCounter(CQueryCounter* counter);
    int exec_query();
    MySQL* clear_result_set();
    char* GetQueryString();
    bool get_ulong(int column, unsigned long& out);
    bool get_float(int column, float& out);
    bool get_byte(int column, int& out);
    bool is_valid_data(int column);
    bool get_longlong(int column, long long& out);

private:
    enum {
        kQueryBufferSize = 24577,
        kBlobBufferCount = 10,
        kBlobBufferSize = 24577
    };

    CQueryCounter* query_counter_;  // offset 0
    void* mysql_handle_;            // offset 4
    void* result_;                  // offset 8
    char** row_;                    // offset 12
    unsigned long* lengths_;        // offset 16
    unsigned int rows_;             // offset 20
    unsigned int fields_;           // offset 24
    char query_[kQueryBufferSize];  // offset 28
    char blob_buffers_[kBlobBufferCount][kBlobBufferSize];  // offset 24605
    char blob_pad_;                 // offset 270375
    int query_length_;              // offset 270376
    int db_error_;                  // offset 270380
    char db_host_[16];              // offset 270384
    unsigned int db_port_;          // offset 270400
    char db_user_[20];              // offset 270404
    char db_passwd_[20];            // offset 270424
    char db_name_[30];              // offset 270444
    unsigned char query_counter_enabled_;  // offset 270474
};

#endif  // OLDSRV_OLD_DATA_DB_CORE_MYSQL_H_
