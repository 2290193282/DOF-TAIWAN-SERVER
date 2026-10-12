#ifndef OLDSRV_SHUTDOWN_SHUTDOWN_MANAGER_H_
#define OLDSRV_SHUTDOWN_SHUTDOWN_MANAGER_H_


class CUser;

// IDA: CShutdowManager functions @0x82A35A6..0x82A6FFF.
class CShutdowManager {
public:
    // IDA: CShutdowManager::CShutdowManager @0x82A6FA8.
    CShutdowManager();

    // IDA: CShutdowManager::SendShutdownSignal @0x82A35A6.
    int SendShutdownSignal();

    // IDA: CShutdowManager::OnAllUserLogout @0x82A36BC.
    void OnAllUserLogout();

    // IDA: CShutdowManager::SendLastMsgDBQueue @0x82A37C8.
    // The CUser* argument is present in the original signature but is not
    // consumed by this function; it only queues the two final DB messages.
    int SendLastMsgDBQueue(CUser* user);

    // IDA: CShutdowManager::WaitUntilExitThread @0x82A396E.
    int WaitUntilExitThread();

    // IDA: CShutdowManager::EnableShutdown @0x82A6F70.
    CShutdowManager* EnableShutdown(bool enabled);

    // IDA: CShutdowManager::Reset @0x82A6F88.
    CShutdowManager* Reset();

    // IDA: CShutdowManager::IncrementLastMsgResponseCount @0x84ED1EA.
    CShutdowManager* IncrementLastMsgResponseCount();

    // IDA: CShutdowManager::GetLastMsgCount @0x84ED1FE.
    int GetLastMsgCount() const;

    // IDA: CShutdowManager::GetLastMsgResponseCount @0x84ED20A.
    int GetLastMsgResponseCount() const;

    bool shutdown_enabled() const;

private:
    bool shutdown_enabled_;        // IDA byte at this+0, shutdown gate.
    int last_msg_count_;           // IDA dword at this+4, queued final DB messages.
    int last_msg_response_count_;  // IDA dword at this+8, responses received.
};

// IDA: GetInstanceShutdowManager @0x82A6FBB.
CShutdowManager* GetInstanceShutdowManager();


#endif  // OLDSRV_SHUTDOWN_SHUTDOWN_MANAGER_H_
