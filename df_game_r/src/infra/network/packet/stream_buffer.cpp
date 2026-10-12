#include "infra/network/packet/stream_buffer.h"

#include <algorithm>
#include <cstdio>
#include <cstring>


namespace {

const int kStreamBufferSize = 0xA000;

}  // namespace

// IDA_EA: 0x86269BE
// IDA_NAME: _ZN13CStreamBufferC2Ev
// DEMANGLED: CStreamBuffer::CStreamBuffer(void)
CStreamBuffer::CStreamBuffer() : buffer_() {
    std::memset(buffer_, 0, sizeof(buffer_));
}

// IDA_EA: 0x86269E2
// IDA_NAME: _ZN13CStreamBuffer5c_strEv
// DEMANGLED: CStreamBuffer::c_str(void)
char* CStreamBuffer::c_str() {
    return buffer_;
}

// IDA_EA: 0x86269EA
// IDA_NAME: _ZN13CStreamBuffer12AppendStringEPciS0_
// DEMANGLED: CStreamBuffer::AppendString(char *,int,char *)
char* CStreamBuffer::AppendString(char* dest, int capacity, const char* src) {
    const size_t remain = capacity - std::strlen(dest) - 1;
    const size_t source_len = std::strlen(src);
    const size_t append_len = std::min(source_len, remain);
    return std::strncat(dest, src, append_len);
}

// IDA_EA: 0x8626A4A
// IDA_NAME: _ZN13CStreamBufferlsEPKc
// DEMANGLED: CStreamBuffer::operator<<(char const*)
CStreamBuffer& CStreamBuffer::operator<<(const char* value) {
    Insert("%s", value);
    return *this;
}

// IDA_EA: 0x8626A70
// IDA_NAME: _ZN13CStreamBufferlsEi
// DEMANGLED: CStreamBuffer::operator<<(int)
CStreamBuffer& CStreamBuffer::operator<<(int value) {
    Insert("%d", value);
    return *this;
}

// IDA_EA: 0x8626A96
// IDA_NAME: _ZN13CStreamBufferlsEj
// DEMANGLED: CStreamBuffer::operator<<(unsigned int)
CStreamBuffer& CStreamBuffer::operator<<(unsigned int value) {
    Insert("%u", value);
    return *this;
}

// IDA_EA: 0x8626ABC
// IDA_NAME: _ZN13CStreamBufferlsEf
// DEMANGLED: CStreamBuffer::operator<<(float)
CStreamBuffer& CStreamBuffer::operator<<(float value) {
    Insert("%f", value);
    return *this;
}

// IDA_EA: 0x8627CBC
// IDA_NAME: _ZN13CStreamBuffer6InsertIPKcEEvPcT_
// DEMANGLED: void CStreamBuffer::Insert<char const*>(char *,char const*)
void CStreamBuffer::Insert(const char* format, const char* value) {
    char temp[kStreamBufferSize + 8];
    std::memset(temp, 0, kStreamBufferSize);
    std::snprintf(temp, kStreamBufferSize, format, value);
    AppendString(buffer_, kStreamBufferSize, temp);
}

// IDA_EA: 0x8627D48
// IDA_NAME: _ZN13CStreamBuffer6InsertIiEEvPcT_
// DEMANGLED: void CStreamBuffer::Insert<int>(char *,int)
void CStreamBuffer::Insert(const char* format, int value) {
    char temp[kStreamBufferSize + 8];
    std::memset(temp, 0, kStreamBufferSize);
    std::snprintf(temp, kStreamBufferSize, format, value);
    AppendString(buffer_, kStreamBufferSize, temp);
}

// IDA_EA: 0x8627DD4
// IDA_NAME: _ZN13CStreamBuffer6InsertIjEEvPcT_
// DEMANGLED: void CStreamBuffer::Insert<unsigned int>(char *,unsigned int)
void CStreamBuffer::Insert(const char* format, unsigned int value) {
    char temp[kStreamBufferSize + 8];
    std::memset(temp, 0, kStreamBufferSize);
    std::snprintf(temp, kStreamBufferSize, format, value);
    AppendString(buffer_, kStreamBufferSize, temp);
}

// IDA_EA: 0x8627E60
// IDA_NAME: _ZN13CStreamBuffer6InsertIfEEvPcT_
// DEMANGLED: void CStreamBuffer::Insert<float>(char *,float)
void CStreamBuffer::Insert(const char* format, float value) {
    char temp[kStreamBufferSize + 8];
    std::memset(temp, 0, kStreamBufferSize);
    std::snprintf(temp, kStreamBufferSize, format, value);
    AppendString(buffer_, kStreamBufferSize, temp);
}
