#ifndef OLDSRV_PROTOCOL_STREAM_BUFFER_H_
#define OLDSRV_PROTOCOL_STREAM_BUFFER_H_


// IDA: CStreamBuffer is a fixed 0xA000 byte text accumulation buffer.
class CStreamBuffer {
public:
    CStreamBuffer();

    char* c_str();
    static char* AppendString(char* dest, int capacity, const char* src);

    CStreamBuffer& operator<<(const char* value);
    CStreamBuffer& operator<<(int value);
    CStreamBuffer& operator<<(unsigned int value);
    CStreamBuffer& operator<<(float value);

    void Insert(const char* format, const char* value);
    void Insert(const char* format, int value);
    void Insert(const char* format, unsigned int value);
    void Insert(const char* format, float value);

private:
    char buffer_[0xA000];
};


#endif  // OLDSRV_PROTOCOL_STREAM_BUFFER_H_
