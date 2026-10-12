#ifndef OLDSRV_PROTOCOL_STREAM_GUARD_H_
#define OLDSRV_PROTOCOL_STREAM_GUARD_H_

#include "infra/network/packet/stream.h"


class CStreamGuard {
public:
    CStreamGuard(Stream* stream, bool auto_free);
    explicit CStreamGuard(bool auto_free);
    ~CStreamGuard();

    CStreamGuard& operator*();
    CStreamGuard* operator->();
    CStreamGuard* Set(Stream* stream);
    Stream* Get() const;

    CStreamGuard& operator<<(bool value);
    CStreamGuard& operator<<(char value);
    CStreamGuard& operator<<(short value);
    CStreamGuard& operator<<(int value);
    CStreamGuard& operator<<(long value);
    CStreamGuard& operator<<(char* value);
    CStreamGuard& operator<<(const char* value);
    CStreamGuard& operator<<(unsigned char value);
    CStreamGuard& operator<<(unsigned short value);
    CStreamGuard& operator<<(unsigned int value);
    CStreamGuard& operator<<(unsigned long value);
    int put_binary(void* src, int bytes);
    void* GetInBufferUntyped(int bytes);

    template <typename T>
    T* GetInBuffer();

private:
    Stream* stream_;
    bool auto_free_;
};


#endif  // OLDSRV_PROTOCOL_STREAM_GUARD_H_
