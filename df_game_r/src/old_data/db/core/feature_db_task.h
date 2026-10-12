#ifndef OLDSRV_OLD_DATA_DB_CORE_FEATURE_DB_TASK_H_
#define OLDSRV_OLD_DATA_DB_CORE_FEATURE_DB_TASK_H_


class DBDispatcher;
class Stream;

// Dispatcher registry owned exclusively by NATIVE_0828_DB_Q.  It is kept
// separate from DBTask so native packets cannot consume the legacy DB_Q or
// mutate DBMgr::user_idx_.
class FeatureDBTask {
public:
    FeatureDBTask();
    ~FeatureDBTask();

    bool ready() const;
    int HandleEvents(bool& had_event);

private:
    enum {
        kDispatcherCount = 852,
    };

    bool RegistDispatcher();
    bool RegisterDispatcher(int signal, DBDispatcher* dispatcher);
    void UnregistDispatcher();
    int DBProcess(Stream* stream, int& signal);
    void FreeStream(Stream* stream, int stream_size, int pool_kind,
                    int signal);

    DBDispatcher* dispatchers_[kDispatcherCount];
    bool ready_;
};


#endif  // OLDSRV_OLD_DATA_DB_CORE_NATIVE_0828_DB_TASK_H_
