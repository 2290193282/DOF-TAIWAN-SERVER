#ifndef OLDSRV_RUNTIME_MESSAGE_COUNTER_H_
#define OLDSRV_RUNTIME_MESSAGE_COUNTER_H_

#include <stdint.h>

#include <map>


// IDA units: message_counter/CMessageCounter, CTimeInterval,
// CUnixTimeInterval and CUserPacketCounter.  DispatchThread owns these
// objects on its stack; their layouts are therefore part of the i386 ABI.
namespace message_counter {

struct MessageKey {
    MessageKey(uint32_t signal_value,
               uint8_t category_value,
               uint16_t code_value);

    bool operator<(const MessageKey& rhs) const;

    uint32_t signal;
    uint8_t category;
    uint8_t reserved;
    uint16_t code;
};

struct MessageValue {
    MessageValue();

    uint32_t count;
    int64_t total_msec;
    double average_msec;
};

class CMessageCounter {
public:
    // IDA_EA: 0x844DE36 / 0x845000E.
    explicit CMessageCounter(int code_offset);
    ~CMessageCounter();

    // IDA_EA: 0x856F9C4 / 0x856FBB0.
    int IncreaseMessage(uint32_t signal,
                        uint8_t category,
                        uint16_t code,
                        int64_t elapsed_msec);
    void WriteLog();

private:
    std::map<MessageKey, MessageValue> values_;
    int code_offset_;
};


class CTimeInterval {
public:
    // IDA_EA: 0x856F974 / 0x856F9B0 / 0x85702C6.
    static CTimeInterval* Create();
    static void Destory(CTimeInterval* interval);

    CTimeInterval();

    // IDA virtual slots +0 and +4.  There is deliberately no virtual
    // destructor: the old Destory implementation invokes operator delete.
    virtual void SetLastTime();
    virtual int64_t GetTimeInterval();
};


class CUnixTimeInterval : public CTimeInterval {
public:
    // IDA_EA: 0x85702D4 / 0x8570236 / 0x8570254.
    CUnixTimeInterval();
    virtual void SetLastTime();
    virtual int64_t GetTimeInterval();

private:
    int32_t last_second_;
    int32_t last_usec_;
};


class CUserPacketCounter {
public:
    // IDA_EA: 0x847E8D2 / 0x847E916.
    CUserPacketCounter(char* file_name, char* counter_name);
    ~CUserPacketCounter();

    // IDA_EA: 0x856FF24 / 0x856FFF0.
    CUserPacketCounter* IncreasePacket(uint32_t account_id);
    int WriteLog();

private:
    std::map<uint32_t, int> values_;
    char filename_[256];
    int total_count_;
};

}  // namespace message_counter


#if defined(__i386__)
static_assert(sizeof(message_counter::MessageKey) == 0x08u,
              "MessageKey must retain the i386 packet-stat key ABI");
static_assert(sizeof(message_counter::MessageValue) == 0x14u,
              "MessageValue must retain the i386 packet-stat value ABI");
static_assert(sizeof(message_counter::CMessageCounter) == 0x1cu,
              "CMessageCounter must retain the old 28-byte ABI");
static_assert(sizeof(message_counter::CTimeInterval) == 0x04u,
              "CTimeInterval must retain its one-vptr ABI");
static_assert(sizeof(message_counter::CUnixTimeInterval) == 0x0cu,
              "CUnixTimeInterval must retain the old 12-byte ABI");
static_assert(sizeof(message_counter::CUserPacketCounter) == 0x11cu,
              "CUserPacketCounter must retain the old 284-byte ABI");
#endif

#endif  // OLDSRV_RUNTIME_MESSAGE_COUNTER_H_
