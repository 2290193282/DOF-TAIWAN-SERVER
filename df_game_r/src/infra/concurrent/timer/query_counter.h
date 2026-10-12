#ifndef OLDSRV_GLOBAL_QUERY_COUNTER_H_
#define OLDSRV_GLOBAL_QUERY_COUNTER_H_

#include <map>
#include <string>


// IDA unit: 99_misc_global/CQueryCounter.
class CQueryCounter {
public:
    struct QUERY_INFO {
        QUERY_INFO();

        int count;             // offset +0 inside QUERY_INFO.
        double response_time;  // offset +4 inside QUERY_INFO on i386.
    };

    typedef std::map<int, QUERY_INFO> QueryCountTable;
    typedef QueryCountTable::iterator QueryCountIterator;

    CQueryCounter();
    ~CQueryCounter();

    CQueryCounter* setLogDB(bool is_log_db);
    QueryCountTable* GetQueryCountTable();
    CQueryCounter* Reset();
    void IncreaseQueryCount(const std::string& query);
    void UpdateQueryCounter();
    bool SetResponseTime(double elapsed_seconds);
    bool LoadQueryTable();
    bool SaveQuery(const char* query, int& query_id);
    bool RegisterQuery(const char* query, int query_id, int initial_count);

private:
    bool log_db_;                         // offset +0.
    QueryCountTable query_count_table_;   // offset +4.
    std::map<std::string, QueryCountIterator> query_id_map_;  // offset +28.
    QueryCountIterator last_query_;       // offset +52.
    int dirty_query_count_;               // offset +56.
};


#endif  // OLDSRV_GLOBAL_QUERY_COUNTER_H_
