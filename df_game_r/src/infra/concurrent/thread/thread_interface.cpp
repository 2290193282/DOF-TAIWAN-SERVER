#include "infra/concurrent/thread/thread_interface.h"

#include <iostream>


#if defined(__i386__)
typedef char ThreadInterface_Size_Must_Be_12[
    (sizeof(ThreadInterface) == 12) ? 1 : -1];
#endif


// IDA_EA: 0x8630B38
// IDA_NAME: _ZN15ThreadInterfaceC2Ev
// DEMANGLED: ThreadInterface::ThreadInterface(void)
ThreadInterface::ThreadInterface() : thread_(), thread_id_(0) {}

// IDA_EA: 0x8630B46
// IDA_NAME: _ZN15ThreadInterfaceD2Ev
// DEMANGLED: ThreadInterface::~ThreadInterface()
// IDA_EA: 0x8630B6C
// IDA_NAME: _ZN15ThreadInterfaceD0Ev
// DEMANGLED: ThreadInterface::~ThreadInterface()
ThreadInterface::~ThreadInterface() {}

// IDA_EA: 0x8630C14
// IDA_NAME: _ZN15ThreadInterface4stopEv
// DEMANGLED: ThreadInterface::stop(void)
// IDA proves this is an intentional no-op (a single ret instruction).
void ThreadInterface::stop() {}

// IDA_EA: 0x8630C1A
// IDA_NAME: _ZN15ThreadInterface14dispatch_proxyEPv
// DEMANGLED: ThreadInterface::dispatch_proxy(void *)
void* ThreadInterface::dispatch_proxy(void* user_data) {
    ThreadInterface* const thread = static_cast<ThreadInterface*>(user_data);
    thread->dispatch(user_data);
    return 0;
}

// IDA_EA: 0x8630B8A
// IDA_NAME: _ZN15ThreadInterface5beginEv
// DEMANGLED: ThreadInterface::begin(void)
int ThreadInterface::begin() {
    const int result = pthread_create(&thread_, 0, dispatch_proxy, this);
    if (result) {
        std::cout << "[ThreadInterface::begin] Can't begin thread"
                  << " " << result << std::endl;
        thread_id_ = 0;
        return 0;
    }
    thread_id_ = static_cast<int>(thread_);
    return 1;
}

// IDA_EA: 0x8630C46
// IDA_NAME: _ZN15ThreadInterface4joinEv
// DEMANGLED: ThreadInterface::join(void)
bool ThreadInterface::join() {
    return pthread_join(thread_, 0) == 0;
}

// IDA_EA: 0x82A4218
// IDA_NAME: _ZN15ThreadInterface11getThreadIDEv
// DEMANGLED: ThreadInterface::getThreadID(void)
int ThreadInterface::getThreadID() {
    return thread_id_;
}
