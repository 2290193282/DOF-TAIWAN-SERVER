#ifndef OLDSRV_SECURITY_CODEHACK_CHECKSUM_H_
#define OLDSRV_SECURITY_CODEHACK_CHECKSUM_H_

#include <stdint.h>
#include <vector>


// 32-byte code-hack checksum blob compared with memcmp(..., 0x20).
struct CodeHackChecksum {
    uint8_t bytes[32];

    ~CodeHackChecksum();
};

// Code region checked by the old anti-hack checksum route.
struct CodeHackCheckRange {
    uint32_t start;
    uint32_t size;
};

// Vector wrapper used by old df_game_r for allowed code checksums.
class CodeHackChecksumData {
public:
    CodeHackChecksumData();
    CodeHackChecksumData(const CodeHackChecksumData& rhs);
    ~CodeHackChecksumData();

    int Clear();
    int Reserve(long count);
    void AddCodeHackChecksum(const CodeHackChecksum& checksum);
    int IsRightChecksum(const CodeHackChecksum* checksum) const;

protected:
    std::vector<CodeHackChecksum> checksums_;
};

// Extended checksum record loaded from serialized anti-hack script data.
class CodeHackChecksumEx : public CodeHackChecksumData {
public:
    CodeHackChecksumEx();
    CodeHackChecksumEx(const CodeHackChecksumEx& rhs);
    ~CodeHackChecksumEx();

    char* SetFileName(const char* src);
    char* GetFileName();
    int SetCodeHackCheckRange(const CodeHackCheckRange& range);
    CodeHackCheckRange* GetCodeHackCheckRange();
    bool setData(const char* src);
    int IsRightChecksum(const CodeHackChecksum* checksum) const;

private:
    char file_name_[0x104];
    CodeHackCheckRange range_;
};


#endif  // OLDSRV_SECURITY_CODEHACK_CHECKSUM_H_
