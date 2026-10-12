#ifndef OLDSRV_PROCESS_PROCESS_FILES_H_
#define OLDSRV_PROCESS_PROCESS_FILES_H_

#include <cstddef>
#include <sys/types.h>


// Validate an instance before it can reach CEnvironment::set_file_name's
// recovered fixed 30-byte field.  BuildPidPath uses this same boundary.
bool IsSafeProcessInstanceName(const char* instance_name);

// Restored from check_argv/rdar_init/daemon_init/save_pid at
// 0x829EA5A/0x829E5BF/0x829E8B9/0x829EEB2.  The old access()+O_CREAT
// sequence raced concurrent launchers; this boundary atomically owns the PID
// pathname and holds a persistent per-instance advisory lock across the whole
// process lifetime while preserving start/start_all daemonization and nofork.
bool BeginProcessPidLifecycle(const char* operation, const char* instance_name);

// The daemon launcher parent waits until the final child completes Init().
bool CommitProcessStartup();
void AbortProcessStartup();

// While the per-instance lock excludes cooperating restart launchers, remove
// only the inode owned by this process and only when the payload names getpid().
bool RemoveOwnedPidFile();

// Strict PID parsing plus /proc executable and instance validation for signal
// senders.
bool ReadValidatedInstancePid(const char* instance_name,
                              pid_t* pid,
                              char* error_text,
                              std::size_t error_text_size);


// IDA: TerminateServers() @0x82A21F9.
void TerminateServers();


#endif  // OLDSRV_PROCESS_PROCESS_FILES_H_
