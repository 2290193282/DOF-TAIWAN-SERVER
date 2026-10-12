#include "common/manager/app_load_checker.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_data/db/statistics_log/statistics_log_payloads.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// This specialization is provided by stream_memberization_batch_001_56.cpp.
template <>
SIG_INSERT_SERVER_LOAD_STATISTIC*
CStreamGuard::GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>();


// IDA_EA: 0x85FFC6A
// IDA_NAME: _ZN15CAppLoadCheckerC2Ev
// DEMANGLED: CAppLoadChecker::CAppLoadChecker(void)
CAppLoadChecker::CAppLoadChecker()
    : dbq_baseline_(0),
      logdbq_baseline_(0),
      timerq_baseline_(0),
      packetq_baseline_(0),
      dbq_pressure_(0),
      logdbq_pressure_(0),
      timerq_pressure_(0),
      packetq_pressure_(0) {}

// IDA_EA: 0x85FFCB2
// IDA_NAME: _ZN15CAppLoadChecker13checkLoad_dbqEi
// DEMANGLED: CAppLoadChecker::checkLoad_dbq(int)
int CAppLoadChecker::checkLoad_dbq(int dbq_count) {
    const int upward_delta = dbq_count - dbq_baseline_;
    const int downward_delta = dbq_baseline_ - dbq_count;

    if (dbq_pressure_ > 0 || upward_delta <= 10) {
        if (dbq_pressure_ > 1 || upward_delta <= 25) {
            if (dbq_pressure_ > 2 || upward_delta <= 40) {
                if (dbq_pressure_ > 3 || upward_delta <= 80) {
                    if (dbq_pressure_ > 4 || upward_delta <= 150) {
                        if (dbq_pressure_ > 5 || upward_delta <= 250) {
                            if (dbq_pressure_ == 6 && upward_delta > 250) {
                                return true;
                            }
                            if (dbq_pressure_ < 0 || downward_delta <= 10) {
                                if (dbq_pressure_ < -1 || downward_delta <= 25) {
                                    if (dbq_pressure_ < -2 || downward_delta <= 40) {
                                        if (dbq_pressure_ < -3 || downward_delta <= 80) {
                                            if (dbq_pressure_ < -4 || downward_delta <= 150) {
                                                if (dbq_pressure_ < -5 || downward_delta <= 250) {
                                                    return dbq_pressure_ == -6 &&
                                                           downward_delta > 250;
                                                }
                                                dbq_pressure_ = -6;
                                                return true;
                                            }
                                            dbq_pressure_ = -5;
                                            return true;
                                        }
                                        dbq_pressure_ = -4;
                                        return true;
                                    }
                                    dbq_pressure_ = -3;
                                    return true;
                                }
                                dbq_pressure_ = -2;
                                return true;
                            }
                            dbq_pressure_ = -1;
                            return true;
                        }
                        dbq_pressure_ = 6;
                        return true;
                    }
                    dbq_pressure_ = 5;
                    return true;
                }
                dbq_pressure_ = 4;
                return true;
            }
            dbq_pressure_ = 3;
            return true;
        }
        dbq_pressure_ = 2;
        return true;
    }
    dbq_pressure_ = 1;
    return true;
}

// IDA_EA: 0x85FFEB0
// IDA_NAME: _ZN15CAppLoadChecker16checkLoad_logdbqEi
// DEMANGLED: CAppLoadChecker::checkLoad_logdbq(int)
int CAppLoadChecker::checkLoad_logdbq(int logdbq_count) {
    const int upward_delta = logdbq_count - logdbq_baseline_;
    const int downward_delta = logdbq_baseline_ - logdbq_count;

    if (logdbq_pressure_ > 0 || upward_delta <= 10) {
        if (logdbq_pressure_ > 1 || upward_delta <= 25) {
            if (logdbq_pressure_ > 2 || upward_delta <= 40) {
                if (logdbq_pressure_ > 3 || upward_delta <= 80) {
                    if (logdbq_pressure_ > 4 || upward_delta <= 150) {
                        if (logdbq_pressure_ > 5 || upward_delta <= 250) {
                            if (logdbq_pressure_ == 6 && upward_delta > 250) {
                                return true;
                            }
                            if (logdbq_pressure_ < 0 || downward_delta <= 10) {
                                if (logdbq_pressure_ < -1 || downward_delta <= 25) {
                                    if (logdbq_pressure_ < -2 || downward_delta <= 40) {
                                        if (logdbq_pressure_ < -3 || downward_delta <= 80) {
                                            if (logdbq_pressure_ < -4 || downward_delta <= 150) {
                                                if (logdbq_pressure_ < -5 || downward_delta <= 250) {
                                                    return logdbq_pressure_ == -6 &&
                                                           downward_delta > 250;
                                                }
                                                logdbq_pressure_ = -6;
                                                return true;
                                            }
                                            logdbq_pressure_ = -5;
                                            return true;
                                        }
                                        logdbq_pressure_ = -4;
                                        return true;
                                    }
                                    logdbq_pressure_ = -3;
                                    return true;
                                }
                                logdbq_pressure_ = -2;
                                return true;
                            }
                            logdbq_pressure_ = -1;
                            return true;
                        }
                        logdbq_pressure_ = 6;
                        return true;
                    }
                    logdbq_pressure_ = 5;
                    return true;
                }
                logdbq_pressure_ = 4;
                return true;
            }
            logdbq_pressure_ = 3;
            return true;
        }
        logdbq_pressure_ = 2;
        return true;
    }
    logdbq_pressure_ = 1;
    return true;
}

// IDA_EA: 0x86000B0
// IDA_NAME: _ZN15CAppLoadChecker16checkLoad_timerqEi
// DEMANGLED: CAppLoadChecker::checkLoad_timerq(int)
int CAppLoadChecker::checkLoad_timerq(int timerq_count) {
    const int upward_delta = timerq_count - timerq_baseline_;
    const int downward_delta = timerq_baseline_ - timerq_count;

    if (timerq_pressure_ > 0 || upward_delta <= 10) {
        if (timerq_pressure_ > 1 || upward_delta <= 25) {
            if (timerq_pressure_ > 2 || upward_delta <= 40) {
                if (timerq_pressure_ > 3 || upward_delta <= 80) {
                    if (timerq_pressure_ > 4 || upward_delta <= 150) {
                        if (timerq_pressure_ > 5 || upward_delta <= 250) {
                            if (timerq_pressure_ == 6 && upward_delta > 250) {
                                return true;
                            }
                            if (timerq_pressure_ < 0 || downward_delta <= 10) {
                                if (timerq_pressure_ < -1 || downward_delta <= 25) {
                                    if (timerq_pressure_ < -2 || downward_delta <= 40) {
                                        if (timerq_pressure_ < -3 || downward_delta <= 80) {
                                            if (timerq_pressure_ < -4 || downward_delta <= 150) {
                                                if (timerq_pressure_ < -5 || downward_delta <= 250) {
                                                    return timerq_pressure_ == -6 &&
                                                           downward_delta > 250;
                                                }
                                                timerq_pressure_ = -6;
                                                return true;
                                            }
                                            timerq_pressure_ = -5;
                                            return true;
                                        }
                                        timerq_pressure_ = -4;
                                        return true;
                                    }
                                    timerq_pressure_ = -3;
                                    return true;
                                }
                                timerq_pressure_ = -2;
                                return true;
                            }
                            timerq_pressure_ = -1;
                            return true;
                        }
                        timerq_pressure_ = 6;
                        return true;
                    }
                    timerq_pressure_ = 5;
                    return true;
                }
                timerq_pressure_ = 4;
                return true;
            }
            timerq_pressure_ = 3;
            return true;
        }
        timerq_pressure_ = 2;
        return true;
    }
    timerq_pressure_ = 1;
    return true;
}

// IDA_EA: 0x86002B0
// IDA_NAME: _ZN15CAppLoadChecker17checkLoad_packetqEi
// DEMANGLED: CAppLoadChecker::checkLoad_packetq(int)
int CAppLoadChecker::checkLoad_packetq(int packetq_count) {
    const int upward_delta = packetq_count - packetq_baseline_;
    const int downward_delta = packetq_baseline_ - packetq_count;

    if (packetq_pressure_ > 0 || upward_delta <= 10) {
        if (packetq_pressure_ > 1 || upward_delta <= 25) {
            if (packetq_pressure_ > 2 || upward_delta <= 40) {
                if (packetq_pressure_ > 3 || upward_delta <= 80) {
                    if (packetq_pressure_ > 4 || upward_delta <= 150) {
                        if (packetq_pressure_ > 5 || upward_delta <= 250) {
                            if (packetq_pressure_ == 6 && upward_delta > 250) {
                                return true;
                            }
                            if (packetq_pressure_ < 0 || downward_delta <= 10) {
                                if (packetq_pressure_ < -1 || downward_delta <= 25) {
                                    if (packetq_pressure_ < -2 || downward_delta <= 40) {
                                        if (packetq_pressure_ < -3 || downward_delta <= 80) {
                                            if (packetq_pressure_ < -4 || downward_delta <= 150) {
                                                if (packetq_pressure_ < -5 || downward_delta <= 250) {
                                                    return packetq_pressure_ == -6 &&
                                                           downward_delta > 250;
                                                }
                                                packetq_pressure_ = -6;
                                                return true;
                                            }
                                            packetq_pressure_ = -5;
                                            return true;
                                        }
                                        packetq_pressure_ = -4;
                                        return true;
                                    }
                                    packetq_pressure_ = -3;
                                    return true;
                                }
                                packetq_pressure_ = -2;
                                return true;
                            }
                            packetq_pressure_ = -1;
                            return true;
                        }
                        packetq_pressure_ = 6;
                        return true;
                    }
                    packetq_pressure_ = 5;
                    return true;
                }
                packetq_pressure_ = 4;
                return true;
            }
            packetq_pressure_ = 3;
            return true;
        }
        packetq_pressure_ = 2;
        return true;
    }
    packetq_pressure_ = 1;
    return true;
}

// IDA_EA: 0x86004B0
// IDA_NAME: _ZN15CAppLoadChecker8setQueueEiiii
// DEMANGLED: CAppLoadChecker::setQueue(int,int,int,int)
CAppLoadChecker* CAppLoadChecker::setQueue(int dbq_count, int logdbq_count,
                                            int timerq_count,
                                            int packetq_count) {
    dbq_baseline_ = dbq_count;
    logdbq_baseline_ = logdbq_count;
    timerq_baseline_ = timerq_count;
    packetq_baseline_ = packetq_count;
    return this;
}

// IDA_EA: 0x86004D8
// IDA_NAME: _ZN15CAppLoadChecker9CheckLoadEiiii
// DEMANGLED: CAppLoadChecker::CheckLoad(int,int,int,int)
int CAppLoadChecker::CheckLoad(int dbq_count, int logdbq_count,
                               int netq_count, int packetq_count) {
    const int dbq_changed = checkLoad_dbq(dbq_count);
    const int logdbq_changed = checkLoad_logdbq(logdbq_count);
    const int timerq_changed = checkLoad_timerq(netq_count);
    const int packetq_changed = checkLoad_packetq(packetq_count);
    if (!dbq_changed && !logdbq_changed && !timerq_changed &&
        !packetq_changed) {
        return 0;
    }

    setQueue(dbq_count, logdbq_count, netq_count, packetq_count);
    return 1;
}

// IDA_EA: 0x8600580
// IDA_NAME: _ZN15CAppLoadChecker9RequestDBEiiii
// DEMANGLED: CAppLoadChecker::RequestDB(int,int,int,int)
void CAppLoadChecker::RequestDB(int dbq_count, int logdbq_count,
                                 int netq_count, int packetq_count) {
    CStreamGuard stream_guard(
        GlobalData::s_stream_pool->Acquire("ServerLoadChecker.cpp", 437),
        true);
    (*stream_guard) << 387;
    (*stream_guard) << -1;

    SIG_INSERT_SERVER_LOAD_STATISTIC* payload =
        stream_guard->GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>();
    payload->server_id = static_cast<int8_t>(
        G_CEnvironment()->get_server_group());
    payload->channel_no = static_cast<int8_t>(
        G_CEnvironment()->get_channel_no());
    payload->dbq_cnt = static_cast<int16_t>(dbq_count);
    payload->logdbq_cnt = static_cast<int16_t>(logdbq_count);
    payload->netq_cnt = static_cast<int16_t>(netq_count);
    payload->packetq_cnt = static_cast<int16_t>(packetq_count);

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream_guard);
}

// IDA_EA: 0x85FFC24
// IDA_NAME: _Z23CAppLoadCheckerInstancev
// DEMANGLED: CAppLoadCheckerInstance(void)
CAppLoadChecker* CAppLoadCheckerInstance() {
    static CAppLoadChecker instance;
    return &instance;
}
