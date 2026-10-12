#include "core/lifecycle/serve/process_files.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


bool IsSafeProcessInstanceName(const char* instance_name) {
    if (!instance_name) {
        return false;
    }
    std::size_t length = 0;
    while (length < 30 && instance_name[length] != '\0') {
        const unsigned char value =
            static_cast<unsigned char>(instance_name[length]);
        const bool alpha_numeric =
            (value >= '0' && value <= '9') ||
            (value >= 'A' && value <= 'Z') ||
            (value >= 'a' && value <= 'z');
        if (!alpha_numeric && value != '_' && value != '-' && value != '.') {
            return false;
        }
        ++length;
    }
    return length > 0 && length < 30;
}


namespace {

const unsigned int kStartupMessageMagic = 0x50494431u;  // "PID1"
const char kPidDirectory[] = "./pid";

enum StartupStage {
    STARTUP_STAGE_READY = 0,
    STARTUP_STAGE_INIT = 1,
    STARTUP_STAGE_SETSID = 2,
    STARTUP_STAGE_CHDIR = 3,
    STARTUP_STAGE_WRITE_PID = 4,
    STARTUP_STAGE_FSYNC_PID = 5,
    STARTUP_STAGE_CLOSE_PID = 6
};

struct StartupMessage {
    unsigned int magic;
    int success;
    int stage;
    int error_number;
    pid_t child_pid;
};

struct PidLifecycleState {
    int pid_fd;
    int instance_lock_fd;
    int parent_notify_fd;
    dev_t device;
    ino_t inode;
    pid_t owner_pid;
    bool owns_path;
    bool startup_committed;
    char path[96];
    char lock_path[96];
};

PidLifecycleState g_pid_lifecycle = {
    -1, -1, -1, 0, 0, 0, false, false, {0}, {0}
};

void SetErrorText(char* output, std::size_t output_size,
                  const char* format, ...) {
    if (!output || output_size == 0) {
        return;
    }
    va_list arguments;
    va_start(arguments, format);
    std::vsnprintf(output, output_size, format, arguments);
    va_end(arguments);
    output[output_size - 1] = '\0';
}

bool BuildPidPath(const char* instance_name, char* output,
                  std::size_t output_size) {
    if (!IsSafeProcessInstanceName(instance_name) || !output ||
        output_size == 0) {
        return false;
    }
    const int length = std::snprintf(output, output_size, "%s/%s.pid",
                                     kPidDirectory, instance_name);
    return length > 0 && static_cast<std::size_t>(length) < output_size;
}

bool BuildInstanceLockPath(const char* instance_name, char* output,
                           std::size_t output_size) {
    if (!IsSafeProcessInstanceName(instance_name) || !output ||
        output_size == 0) {
        return false;
    }
    const int length = std::snprintf(output, output_size, "%s/%s.lock",
                                     kPidDirectory, instance_name);
    return length > 0 && static_cast<std::size_t>(length) < output_size;
}

bool EnsurePidDirectory(char* error_text, std::size_t error_text_size) {
    // This directory is only a launcher coordination boundary.  It does not
    // need group/other write access even though the original make_dir helper
    // requested 0777 before applying the caller's umask.
    if (::mkdir(kPidDirectory, 0755) != 0 && errno != EEXIST) {
        SetErrorText(error_text, error_text_size,
                     "mkdir(%s) failed: %s", kPidDirectory,
                     std::strerror(errno));
        return false;
    }
    struct stat status;
    if (::lstat(kPidDirectory, &status) != 0 || !S_ISDIR(status.st_mode) ||
        status.st_uid != ::geteuid()) {
        SetErrorText(error_text, error_text_size,
                     "%s is not an owned PID directory", kPidDirectory);
        return false;
    }

    const int directory_fd =
        ::open(kPidDirectory, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (directory_fd < 0) {
        SetErrorText(error_text, error_text_size,
                     "open(%s) failed: %s", kPidDirectory,
                     std::strerror(errno));
        return false;
    }
    struct stat opened_status;
    int validation_error = 0;
    if (::fstat(directory_fd, &opened_status) != 0) {
        validation_error = errno;
    } else if (opened_status.st_dev != status.st_dev ||
               opened_status.st_ino != status.st_ino ||
               !S_ISDIR(opened_status.st_mode) ||
               opened_status.st_uid != ::geteuid()) {
        validation_error = ESTALE;
    }
    if (validation_error != 0) {
        ::close(directory_fd);
        SetErrorText(error_text, error_text_size,
                     "%s changed while opening: %s", kPidDirectory,
                     std::strerror(validation_error));
        return false;
    }
    if (::fchmod(directory_fd, 0755) != 0) {
        const int saved_errno = errno;
        ::close(directory_fd);
        SetErrorText(error_text, error_text_size,
                     "fchmod(%s) failed: %s", kPidDirectory,
                     std::strerror(saved_errno));
        return false;
    }
    if (::close(directory_fd) != 0) {
        SetErrorText(error_text, error_text_size,
                     "close(%s) failed: %s", kPidDirectory,
                     std::strerror(errno));
        return false;
    }
    return true;
}

void CloseInstanceLockDescriptor() {
    if (g_pid_lifecycle.instance_lock_fd >= 0) {
        // Do not issue LOCK_UN here.  The daemon parent and child inherit the
        // same open-file description; closing just this process's descriptor
        // preserves the child's lock until its own final cleanup.
        ::close(g_pid_lifecycle.instance_lock_fd);
        g_pid_lifecycle.instance_lock_fd = -1;
    }
}

bool AcquireInstanceLifecycleLock(const char* instance_name,
                                  char* error_text,
                                  std::size_t error_text_size) {
    if (g_pid_lifecycle.instance_lock_fd >= 0) {
        SetErrorText(error_text, error_text_size,
                     "PID lifecycle lock is already held");
        return false;
    }
    if (!BuildInstanceLockPath(instance_name, g_pid_lifecycle.lock_path,
                               sizeof(g_pid_lifecycle.lock_path))) {
        SetErrorText(error_text, error_text_size,
                     "invalid df_game_r instance lock name");
        return false;
    }

    const int fd = ::open(g_pid_lifecycle.lock_path,
                          O_CREAT | O_RDWR | O_NOFOLLOW, 0644);
    if (fd < 0) {
        SetErrorText(error_text, error_text_size, "open(%s) failed: %s",
                     g_pid_lifecycle.lock_path, std::strerror(errno));
        return false;
    }

    struct stat fd_status;
    int validation_error = 0;
    if (::fstat(fd, &fd_status) != 0) {
        validation_error = errno;
    } else if (!S_ISREG(fd_status.st_mode) ||
               fd_status.st_uid != ::geteuid()) {
        validation_error = EPERM;
    } else if (::fchmod(fd, 0644) != 0) {
        validation_error = errno;
    } else {
        const int descriptor_flags = ::fcntl(fd, F_GETFD);
        if (descriptor_flags < 0 ||
            ::fcntl(fd, F_SETFD, descriptor_flags | FD_CLOEXEC) != 0) {
            validation_error = errno;
        }
    }
    if (validation_error != 0) {
        ::close(fd);
        SetErrorText(error_text, error_text_size,
                     "%s is not a trusted lifecycle lock: %s",
                     g_pid_lifecycle.lock_path,
                     std::strerror(validation_error));
        return false;
    }

    if (::flock(fd, LOCK_EX | LOCK_NB) != 0) {
        const int saved_errno = errno;
        ::close(fd);
        SetErrorText(error_text, error_text_size,
                     "df_game_r instance lifecycle is already locked: %s (%s)",
                     g_pid_lifecycle.lock_path,
                     std::strerror(saved_errno));
        return false;
    }

    // O_CREAT makes concurrent first-open callers converge on one inode.  The
    // post-lock identity check rejects a pathname swap that happened while the
    // descriptor was being opened/acquired.  Cooperating launchers never
    // unlink this persistent coordination file.
    struct stat path_status;
    int identity_error = 0;
    if (::lstat(g_pid_lifecycle.lock_path, &path_status) != 0) {
        identity_error = errno;
    } else if (!S_ISREG(path_status.st_mode) ||
               path_status.st_dev != fd_status.st_dev ||
               path_status.st_ino != fd_status.st_ino) {
        identity_error = ESTALE;
    }
    if (identity_error != 0) {
        ::close(fd);
        SetErrorText(error_text, error_text_size,
                     "%s changed while locking: %s",
                     g_pid_lifecycle.lock_path,
                     std::strerror(identity_error));
        return false;
    }

    g_pid_lifecycle.instance_lock_fd = fd;
    return true;
}

bool WriteAll(int fd, const void* source, std::size_t size) {
    const char* cursor = static_cast<const char*>(source);
    std::size_t remaining = size;
    while (remaining != 0) {
        const ssize_t written = ::write(fd, cursor, remaining);
        if (written > 0) {
            cursor += written;
            remaining -= static_cast<std::size_t>(written);
            continue;
        }
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written == 0) {
            errno = EIO;
        }
        return false;
    }
    return true;
}

bool ReadAll(int fd, void* destination, std::size_t size) {
    char* cursor = static_cast<char*>(destination);
    std::size_t remaining = size;
    while (remaining != 0) {
        const ssize_t received = ::read(fd, cursor, remaining);
        if (received > 0) {
            cursor += received;
            remaining -= static_cast<std::size_t>(received);
            continue;
        }
        if (received < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
    return true;
}

bool PathStillHasOwnedInode() {
    if (!g_pid_lifecycle.owns_path || g_pid_lifecycle.path[0] == '\0') {
        return false;
    }
    struct stat status;
    if (::lstat(g_pid_lifecycle.path, &status) != 0) {
        return false;
    }
    return S_ISREG(status.st_mode) &&
           status.st_dev == g_pid_lifecycle.device &&
           status.st_ino == g_pid_lifecycle.inode;
}

bool StrictReadPidFile(const char* path, dev_t required_device,
                       ino_t required_inode, bool require_identity,
                       pid_t* pid, char* error_text,
                       std::size_t error_text_size) {
    struct stat path_status;
    if (::lstat(path, &path_status) != 0) {
        SetErrorText(error_text, error_text_size, "lstat(%s) failed: %s",
                     path, std::strerror(errno));
        return false;
    }
    if (!S_ISREG(path_status.st_mode)) {
        SetErrorText(error_text, error_text_size,
                     "%s is not a regular file", path);
        return false;
    }
    if (require_identity &&
        (path_status.st_dev != required_device ||
         path_status.st_ino != required_inode)) {
        SetErrorText(error_text, error_text_size,
                     "%s inode no longer belongs to this process", path);
        return false;
    }

    const int fd = ::open(path, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        SetErrorText(error_text, error_text_size, "open(%s) failed: %s",
                     path, std::strerror(errno));
        return false;
    }
    struct stat fd_status;
    int validation_error = 0;
    if (::fstat(fd, &fd_status) != 0) {
        validation_error = errno;
    } else if (fd_status.st_dev != path_status.st_dev ||
               fd_status.st_ino != path_status.st_ino) {
        validation_error = ESTALE;
    }
    if (validation_error != 0) {
        ::close(fd);
        SetErrorText(error_text, error_text_size,
                     "%s changed while opening: %s", path,
                     std::strerror(validation_error));
        return false;
    }

    char buffer[64];
    std::size_t used = 0;
    bool read_failed = false;
    int read_errno = 0;
    for (;;) {
        if (used == sizeof(buffer)) {
            read_failed = true;
            read_errno = EOVERFLOW;
            break;
        }
        const ssize_t received =
            ::read(fd, buffer + used, sizeof(buffer) - used);
        if (received > 0) {
            used += static_cast<std::size_t>(received);
            continue;
        }
        if (received == 0) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        read_failed = true;
        read_errno = errno;
        break;
    }
    const int close_result = ::close(fd);
    if (read_failed || close_result != 0) {
        SetErrorText(error_text, error_text_size, "read(%s) failed: %s",
                     path, std::strerror(read_failed ? read_errno : errno));
        return false;
    }

    // save_pid writes one positive decimal PID and one trailing LF.
    if (used < 2 || buffer[used - 1] != '\n' || buffer[0] == '0') {
        SetErrorText(error_text, error_text_size,
                     "%s is not one newline-terminated PID", path);
        return false;
    }
    unsigned long value = 0;
    for (std::size_t index = 0; index + 1 < used; ++index) {
        const unsigned char digit =
            static_cast<unsigned char>(buffer[index]);
        if (digit < '0' || digit > '9') {
            SetErrorText(error_text, error_text_size,
                         "%s contains a non-decimal PID", path);
            return false;
        }
        const unsigned long number =
            static_cast<unsigned long>(digit - '0');
        if (value >
            (static_cast<unsigned long>(INT_MAX) - number) / 10u) {
            SetErrorText(error_text, error_text_size,
                         "%s PID is out of range", path);
            return false;
        }
        value = value * 10u + number;
    }
    if (value <= 1u) {
        SetErrorText(error_text, error_text_size,
                     "%s PID is not a signalable process", path);
        return false;
    }
    *pid = static_cast<pid_t>(value);
    return true;
}

bool RemoveOwnedPath(bool require_pid_payload) {
    if (!g_pid_lifecycle.owns_path) {
        return true;
    }
    if (!PathStillHasOwnedInode()) {
        return false;
    }
    if (require_pid_payload) {
        pid_t stored_pid = 0;
        char error_text[160];
        if (!StrictReadPidFile(g_pid_lifecycle.path,
                               g_pid_lifecycle.device,
                               g_pid_lifecycle.inode,
                               true,
                               &stored_pid,
                               error_text,
                               sizeof(error_text)) ||
            stored_pid != ::getpid() || !PathStillHasOwnedInode()) {
            return false;
        }
    }
    if (::unlink(g_pid_lifecycle.path) != 0 && errno != ENOENT) {
        return false;
    }
    g_pid_lifecycle.owns_path = false;
    return true;
}

void ClosePidDescriptor() {
    if (g_pid_lifecycle.pid_fd >= 0) {
        ::close(g_pid_lifecycle.pid_fd);
        g_pid_lifecycle.pid_fd = -1;
    }
}

bool SendStartupMessage(bool success, StartupStage stage,
                        int error_number) {
    if (g_pid_lifecycle.parent_notify_fd < 0) {
        return true;
    }
    StartupMessage message;
    message.magic = kStartupMessageMagic;
    message.success = success ? 1 : 0;
    message.stage = static_cast<int>(stage);
    message.error_number = error_number;
    message.child_pid = ::getpid();
    const bool sent = WriteAll(g_pid_lifecycle.parent_notify_fd,
                               &message, sizeof(message));
    // Once a complete success record has been written, the parent is allowed
    // to report success.  close(2) cannot retract bytes already accepted by a
    // pipe, so treating a later close diagnostic as child-side failure would
    // let parent and child reach opposite startup conclusions.
    ::close(g_pid_lifecycle.parent_notify_fd);
    g_pid_lifecycle.parent_notify_fd = -1;
    return sent;
}

void ReapChild(pid_t child_pid) {
    int status = 0;
    while (::waitpid(child_pid, &status, 0) < 0 && errno == EINTR) {
    }
}

void ParentWaitForStartup(pid_t child_pid, int read_fd) {
    StartupMessage message;
    std::memset(&message, 0, sizeof(message));
    errno = 0;
    const bool received = ReadAll(read_fd, &message, sizeof(message));
    const int read_errno = errno;
    ::close(read_fd);

    if (received && message.magic == kStartupMessageMagic &&
        message.success == 1 &&
        message.stage == static_cast<int>(STARTUP_STAGE_READY) &&
        message.error_number == 0 && message.child_pid == child_pid) {
        ClosePidDescriptor();
        CloseInstanceLockDescriptor();
        std::fflush(0);
        ::_exit(0);
    }

    ReapChild(child_pid);
    // On child death/EOF, remove only the inode this launcher created.  A
    // pre-existing PID file is never opened, modified, or removed.  Keep the
    // inherited creation FD open through this check so an unlinked inode
    // cannot be recycled for a replacement pathname before comparison.
    RemoveOwnedPath(false);
    ClosePidDescriptor();
    CloseInstanceLockDescriptor();
    if (received && message.magic == kStartupMessageMagic) {
        std::fprintf(stderr,
                     "df_game_r child %ld startup failed at stage %d: %s\n",
                     static_cast<long>(child_pid), message.stage,
                     std::strerror(message.error_number != 0
                                       ? message.error_number : EIO));
    } else {
        std::fprintf(stderr,
                     "df_game_r child %ld startup handshake failed: %s\n",
                     static_cast<long>(child_pid),
                     std::strerror(read_errno != 0 ? read_errno : EPIPE));
    }
    std::fflush(0);
    ::_exit(1);
}

void ChildBootstrapFailure(StartupStage stage, int error_number) {
    RemoveOwnedPath(false);
    ClosePidDescriptor();
    SendStartupMessage(false, stage,
                       error_number != 0 ? error_number : EIO);
    CloseInstanceLockDescriptor();
    ::_exit(1);
}

bool WriteCurrentPid(StartupStage* failed_stage) {
    char text[32];
    const int length = std::snprintf(text, sizeof(text), "%ld\n",
                                     static_cast<long>(::getpid()));
    if (length <= 0 || static_cast<std::size_t>(length) >= sizeof(text)) {
        errno = EOVERFLOW;
        *failed_stage = STARTUP_STAGE_WRITE_PID;
        return false;
    }
    if (!WriteAll(g_pid_lifecycle.pid_fd, text,
                  static_cast<std::size_t>(length))) {
        *failed_stage = STARTUP_STAGE_WRITE_PID;
        return false;
    }
    if (::fsync(g_pid_lifecycle.pid_fd) != 0) {
        *failed_stage = STARTUP_STAGE_FSYNC_PID;
        return false;
    }
    // Keep the creation descriptor open until Init commits (or aborts).  It
    // pins this inode during the daemon-parent handshake, preventing an
    // unlink/recreate sequence from recycling the same st_dev/st_ino pair and
    // fooling failure cleanup into deleting somebody else's replacement.
    g_pid_lifecycle.owner_pid = ::getpid();
    return true;
}

bool ReadProcCmdline(pid_t pid, const char* instance_name,
                     char* error_text, std::size_t error_text_size) {
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%ld/cmdline",
                  static_cast<long>(pid));
    const int fd = ::open(path, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        SetErrorText(error_text, error_text_size, "open(%s) failed: %s",
                     path, std::strerror(errno));
        return false;
    }
    char buffer[4096];
    std::size_t used = 0;
    for (;;) {
        if (used == sizeof(buffer)) {
            ::close(fd);
            SetErrorText(error_text, error_text_size,
                         "%s exceeds the validation buffer", path);
            return false;
        }
        const ssize_t received =
            ::read(fd, buffer + used, sizeof(buffer) - used);
        if (received > 0) {
            used += static_cast<std::size_t>(received);
            continue;
        }
        if (received == 0) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        const int saved_errno = errno;
        ::close(fd);
        SetErrorText(error_text, error_text_size, "read(%s) failed: %s",
                     path, std::strerror(saved_errno));
        return false;
    }
    if (::close(fd) != 0 || used == 0 || buffer[used - 1] != '\0') {
        SetErrorText(error_text, error_text_size,
                     "%s is not a complete process command line", path);
        return false;
    }

    const char* arguments[3] = {0, 0, 0};
    std::size_t argument_count = 0;
    std::size_t offset = 0;
    while (offset < used && argument_count < 3) {
        const void* terminator =
            std::memchr(buffer + offset, '\0', used - offset);
        if (!terminator) {
            break;
        }
        arguments[argument_count++] = buffer + offset;
        offset = static_cast<const char*>(terminator) - buffer + 1;
    }
    if (argument_count < 3 ||
        std::strcmp(arguments[1], instance_name) != 0 ||
        (std::strcmp(arguments[2], "start") != 0 &&
         std::strcmp(arguments[2], "start_all") != 0 &&
         std::strcmp(arguments[2], "nofork") != 0)) {
        SetErrorText(error_text, error_text_size,
                     "PID %ld command line is not df_game_r instance %s",
                     static_cast<long>(pid), instance_name);
        return false;
    }
    return true;
}

bool ValidateProcExecutable(pid_t pid, const char* instance_name,
                            char* error_text,
                            std::size_t error_text_size) {
    char process_path[64];
    std::snprintf(process_path, sizeof(process_path), "/proc/%ld",
                  static_cast<long>(pid));
    struct stat process_status;
    if (::stat(process_path, &process_status) != 0 ||
        process_status.st_uid != ::geteuid()) {
        SetErrorText(error_text, error_text_size,
                     "PID %ld is absent or owned by another user",
                     static_cast<long>(pid));
        return false;
    }

    char executable_path[64];
    std::snprintf(executable_path, sizeof(executable_path),
                  "/proc/%ld/exe", static_cast<long>(pid));
    struct stat target_executable;
    struct stat current_executable;
    if (::stat(executable_path, &target_executable) != 0 ||
        ::stat("/proc/self/exe", &current_executable) != 0 ||
        target_executable.st_dev != current_executable.st_dev ||
        target_executable.st_ino != current_executable.st_ino) {
        SetErrorText(error_text, error_text_size,
                     "PID %ld executable is not this df_game_r image",
                     static_cast<long>(pid));
        return false;
    }
    return ReadProcCmdline(pid, instance_name, error_text, error_text_size);
}

}  // namespace


// IDA: rdar_init @0x829E5BF, daemon_init @0x829E8B9,
// check_pid_file @0x829E948, save_pid @0x829EEB2.  The original final child
// PID and daemon topology are preserved, while the PID pathname is acquired
// atomically before fork.  Failure descriptors are closed; the successful
// per-instance lock intentionally remains open until process exit.
bool BeginProcessPidLifecycle(const char* operation,
                              const char* instance_name) {
    const bool daemon_mode = operation &&
        (std::strcmp(operation, "start") == 0 ||
         std::strcmp(operation, "start_all") == 0);
    const bool nofork_mode = operation &&
        std::strcmp(operation, "nofork") == 0;
    if (!daemon_mode && !nofork_mode) {
        return true;
    }

    char error_text[192];
    if (!EnsurePidDirectory(error_text, sizeof(error_text))) {
        std::fprintf(stderr, "%s\n", error_text);
        return false;
    }
    if (!BuildPidPath(instance_name, g_pid_lifecycle.path,
                      sizeof(g_pid_lifecycle.path))) {
        std::fprintf(stderr, "invalid df_game_r instance name for PID file\n");
        return false;
    }
    if (!AcquireInstanceLifecycleLock(instance_name, error_text,
                                      sizeof(error_text))) {
        std::fprintf(stderr, "%s\n", error_text);
        return false;
    }

    int startup_pipe[2] = {-1, -1};
    if (daemon_mode && ::pipe(startup_pipe) != 0) {
        std::fprintf(stderr, "pipe() for daemon startup failed: %s\n",
                     std::strerror(errno));
        CloseInstanceLockDescriptor();
        return false;
    }

    g_pid_lifecycle.pid_fd =
        ::open(g_pid_lifecycle.path, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (g_pid_lifecycle.pid_fd < 0) {
        const int saved_errno = errno;
        if (startup_pipe[0] >= 0) {
            ::close(startup_pipe[0]);
            ::close(startup_pipe[1]);
        }
        CloseInstanceLockDescriptor();
        if (saved_errno == EEXIST) {
            std::fprintf(stderr,
                         "PID file already exists; refusing to change it: %s\n",
                         g_pid_lifecycle.path);
        } else {
            std::fprintf(stderr, "open(%s) failed: %s\n",
                         g_pid_lifecycle.path, std::strerror(saved_errno));
        }
        return false;
    }
    g_pid_lifecycle.owns_path = true;

    struct stat status;
    if (::fstat(g_pid_lifecycle.pid_fd, &status) != 0) {
        const int saved_errno = errno;
        ClosePidDescriptor();
        if (startup_pipe[0] >= 0) {
            ::close(startup_pipe[0]);
            ::close(startup_pipe[1]);
        }
        CloseInstanceLockDescriptor();
        std::fprintf(stderr,
                     "fstat(%s) failed; PID file left fail-closed: %s\n",
                     g_pid_lifecycle.path, std::strerror(saved_errno));
        return false;
    }
    g_pid_lifecycle.device = status.st_dev;
    g_pid_lifecycle.inode = status.st_ino;
    if (::fchmod(g_pid_lifecycle.pid_fd, 0644) != 0) {
        const int saved_errno = errno;
        RemoveOwnedPath(false);
        ClosePidDescriptor();
        if (startup_pipe[0] >= 0) {
            ::close(startup_pipe[0]);
            ::close(startup_pipe[1]);
        }
        CloseInstanceLockDescriptor();
        std::fprintf(stderr, "fchmod(%s) failed: %s\n",
                     g_pid_lifecycle.path, std::strerror(saved_errno));
        return false;
    }

    StartupStage failed_stage = STARTUP_STAGE_WRITE_PID;
    if (nofork_mode) {
        if (!WriteCurrentPid(&failed_stage)) {
            const int saved_errno = errno;
            RemoveOwnedPath(false);
            ClosePidDescriptor();
            CloseInstanceLockDescriptor();
            std::fprintf(stderr,
                         "PID file initialization failed at stage %d: %s\n",
                         static_cast<int>(failed_stage),
                         std::strerror(saved_errno));
            return false;
        }
        return true;
    }

    std::fflush(0);
    const pid_t child_pid = ::fork();
    if (child_pid < 0) {
        const int saved_errno = errno;
        RemoveOwnedPath(false);
        ClosePidDescriptor();
        ::close(startup_pipe[0]);
        ::close(startup_pipe[1]);
        CloseInstanceLockDescriptor();
        std::fprintf(stderr, "fork() failed: %s\n",
                     std::strerror(saved_errno));
        return false;
    }
    if (child_pid > 0) {
        ::close(startup_pipe[1]);
        ParentWaitForStartup(child_pid, startup_pipe[0]);
    }

    ::close(startup_pipe[0]);
    g_pid_lifecycle.parent_notify_fd = startup_pipe[1];
    if (::setsid() < 0) {
        ChildBootstrapFailure(STARTUP_STAGE_SETSID, errno);
    }
    if (::chdir("./") != 0) {
        ChildBootstrapFailure(STARTUP_STAGE_CHDIR, errno);
    }
    ::umask(0);
    if (!WriteCurrentPid(&failed_stage)) {
        ChildBootstrapFailure(failed_stage, errno);
    }
    return true;
}

bool CommitProcessStartup() {
    if (!g_pid_lifecycle.owns_path || g_pid_lifecycle.startup_committed) {
        return true;
    }
    pid_t stored_pid = 0;
    char error_text[160];
    if (g_pid_lifecycle.owner_pid != ::getpid() ||
        !StrictReadPidFile(g_pid_lifecycle.path,
                           g_pid_lifecycle.device,
                           g_pid_lifecycle.inode,
                           true,
                           &stored_pid,
                           error_text,
                           sizeof(error_text)) ||
        stored_pid != ::getpid() || !PathStillHasOwnedInode()) {
        return false;
    }
    if (g_pid_lifecycle.pid_fd >= 0) {
        const int pid_fd = g_pid_lifecycle.pid_fd;
        g_pid_lifecycle.pid_fd = -1;
        if (::close(pid_fd) != 0) {
            const int saved_errno = errno;
            SendStartupMessage(false, STARTUP_STAGE_CLOSE_PID,
                               saved_errno != 0 ? saved_errno : EIO);
            return false;
        }
    }
    if (!SendStartupMessage(true, STARTUP_STAGE_READY, 0)) {
        return false;
    }
    g_pid_lifecycle.startup_committed = true;
    return true;
}

void AbortProcessStartup() {
    if (!g_pid_lifecycle.owns_path || g_pid_lifecycle.startup_committed) {
        if (!g_pid_lifecycle.startup_committed) {
            CloseInstanceLockDescriptor();
        }
        return;
    }
    RemoveOwnedPath(g_pid_lifecycle.owner_pid == ::getpid());
    ClosePidDescriptor();
    SendStartupMessage(false, STARTUP_STAGE_INIT, ECANCELED);
    CloseInstanceLockDescriptor();
}

bool RemoveOwnedPidFile() {
    if (!g_pid_lifecycle.owns_path) {
        return true;
    }
    const bool removed = RemoveOwnedPath(true);
    if (!removed) {
        std::fprintf(stderr, "FAIL TO DELETE OWNED PID FILE: %s\n",
                      g_pid_lifecycle.path);
    }
    // Deliberately retain the successful server's lock descriptor.  main must
    // still run Neof_shutdownGlobalInstances(), GlobalData::Fini() and
    // CLog::destroyInstance() after PID removal in the recovered order.  The
    // kernel releases this final descriptor only when that process exits, so
    // a replacement cannot overlap the old process's last global teardown.
    return removed;
}

bool ReadValidatedInstancePid(const char* instance_name,
                              pid_t* pid,
                              char* error_text,
                              std::size_t error_text_size) {
    if (!pid) {
        SetErrorText(error_text, error_text_size, "null PID output pointer");
        return false;
    }
    struct stat directory_status;
    if (::lstat(kPidDirectory, &directory_status) != 0 ||
        !S_ISDIR(directory_status.st_mode) ||
        directory_status.st_uid != ::geteuid() ||
        (directory_status.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
        SetErrorText(error_text, error_text_size,
                     "%s is not a trusted PID directory", kPidDirectory);
        return false;
    }
    char path[96];
    if (!BuildPidPath(instance_name, path, sizeof(path))) {
        SetErrorText(error_text, error_text_size,
                     "invalid df_game_r instance name");
        return false;
    }
    pid_t candidate = 0;
    if (!StrictReadPidFile(path, 0, 0, false, &candidate,
                           error_text, error_text_size) ||
        !ValidateProcExecutable(candidate, instance_name,
                                error_text, error_text_size)) {
        return false;
    }
    *pid = candidate;
    return true;
}


// IDA_EA: 0x82A21F9
// IDA_NAME: _Z16TerminateServersv
// DEMANGLED: TerminateServers(void)
void TerminateServers() {
    StreamPool* pool = GlobalData::stream_pool();
    MsgQueueMgr* queue = GlobalData::msgq_mgr();
    if (!pool || !queue) {
        return;
    }

    Stream* stream = pool->Acquire("App.cpp", 11818);
    CStreamGuard guard(stream, true);
    guard << 62;
    guard << 0;
    queue->put(MsgQueueMgr::DISPATCH_Q, guard);
}
