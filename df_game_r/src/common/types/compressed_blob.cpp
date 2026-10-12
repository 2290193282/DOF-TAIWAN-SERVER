#include "common/types/compressed_blob.h"

#include "old_data/db/core/mysql.h"

#include <new>
#include <string.h>
#include <zlib.h>

namespace {

const unsigned int kCompressedBlobRawSizeMask = 0x3FFFFFFFu;

}  // namespace

// IDA_EA: 0x86B201F
// IDA_NAME: _Z12compress_zipPcRjPKcj
// DEMANGLED: compress_zip(char*, unsigned int&, char const*, unsigned int)
int compress_zip(char* dest,
                 unsigned int& dest_len,
                 const char* source,
                 unsigned int source_len) {
    if (source_len == 0 || dest == 0 || source == 0) {
        return 0;
    }

    if (dest_len <= 4) {
        return 0;
    }

    uLongf compressed_len = dest_len - 4;
    const int z_result = compress2(
        reinterpret_cast<Bytef*>(dest + 4),
        &compressed_len,
        reinterpret_cast<const Bytef*>(source),
        source_len,
        Z_DEFAULT_COMPRESSION);
    if (z_result != Z_OK || dest_len < compressed_len) {
        return 0;
    }

    *reinterpret_cast<unsigned int*>(dest) =
        source_len & kCompressedBlobRawSizeMask;
    dest_len = static_cast<unsigned int>(compressed_len) + 4;
    return 1;
}

// IDA_EA: 0x86B2102
// IDA_NAME: _Z14uncompress_zipPcRjPKcj
// DEMANGLED: uncompress_zip(char*, unsigned int&, char const*, unsigned int)
int uncompress_zip(char* dest,
                   unsigned int& dest_len,
                   const char* source,
                   unsigned int source_len) {
    if (source_len == 0) {
        dest_len = 0;
        return 1;
    }
    if (dest == 0 || source == 0) {
        return 0;
    }

    uLongf raw_len =
        *reinterpret_cast<const unsigned int*>(source) &
        kCompressedBlobRawSizeMask;
    const int z_result = uncompress(
        reinterpret_cast<Bytef*>(dest),
        &raw_len,
        reinterpret_cast<const Bytef*>(source + 4),
        source_len);
    dest_len = static_cast<unsigned int>(raw_len);
    return z_result == Z_OK;
}

// IDA_EA: 0x844DE52
// IDA_NAME: _ZN15CCompressedBlobC2Ev
// DEMANGLED: CCompressedBlob::CCompressedBlob(void)
CCompressedBlob::CCompressedBlob() : buffer_(0), capacity_(0) {}

// IDA_EA: 0x844DE6A
// IDA_NAME: _ZN15CCompressedBlobD2Ev
// DEMANGLED: CCompressedBlob::~CCompressedBlob()
CCompressedBlob::~CCompressedBlob() {
    if (buffer_ != 0) {
        delete[] buffer_;
        buffer_ = 0;
    }
}

// IDA_EA: 0x844DE9A
// IDA_NAME: _ZN15CCompressedBlob8compressEP5MySQLiPciPS2_
// DEMANGLED: CCompressedBlob::compress(MySQL*, int, char*, int, char**)
int CCompressedBlob::compress(MySQL* db_handle,
                              int column,
                              char* source,
                              int source_len,
                              char** out_blob) {
    if (db_handle == 0 || source == 0 || out_blob == 0 || source_len <= 0) {
        return 0;
    }

    unsigned int size =
        static_cast<unsigned int>(source_len + source_len / 5 + 12);
    resize(static_cast<int>(size));
    reset();
    if (compress_zip(buffer_, size, source,
                     static_cast<unsigned int>(source_len)) != 1) {
        return 0;
    }
    *out_blob = db_handle->blob_to_str(column, buffer_, static_cast<int>(size));
    return 1;
}

// IDA_EA: 0x844DF40
// IDA_NAME: _ZN15CCompressedBlob10uncompressEP5MySQLiPcii
// DEMANGLED: CCompressedBlob::uncompress(MySQL*, int, char*, int, int)
int CCompressedBlob::uncompress(MySQL* db_handle,
                                int column,
                                char* dest,
                                int dest_len,
                                int unit_size) {
    if (db_handle == 0 || dest == 0 || dest_len < 0) {
        return 0;
    }

    unsigned int n =
        static_cast<unsigned int>(dest_len + dest_len / 5 + 12);
    resize(static_cast<int>(n));
    reset();

    const unsigned int binary_length =
        static_cast<unsigned int>(db_handle->get_binary_length(column));
    if (db_handle->get_binary(column, buffer_, static_cast<int>(n)) != true) {
        return 0;
    }
    if (uncompress_zip(dest, n, buffer_, binary_length) != 1) {
        return 0;
    }
    return unit_size == 0 || n % static_cast<unsigned int>(unit_size) == 0 ||
           n == 0;
}

// IDA_EA: 0x844E02A
// IDA_NAME: _ZN15CCompressedBlob6resizeEi
// DEMANGLED: CCompressedBlob::resize(int)
CCompressedBlob* CCompressedBlob::resize(int size) {
    if (capacity_ < size) {
        if (buffer_ != 0) {
            delete[] buffer_;
            buffer_ = 0;
        }
        buffer_ = new char[size];
        capacity_ = size;
    }
    return this;
}

// IDA_EA: 0x844E080
// IDA_NAME: _ZN15CCompressedBlob5resetEv
// DEMANGLED: CCompressedBlob::reset(void)
void* CCompressedBlob::reset() {
    if (buffer_ == 0 || capacity_ <= 0) {
        return buffer_;
    }
    return memset(buffer_, 0, static_cast<unsigned int>(capacity_));
}

// IDA_EA: 0x83FBC79
// IDA_NAME: _Z24get_compressed_blob_dataP5MySQLiPcii
// DEMANGLED: get_compressed_blob_data(MySQL*, int, char*, int, int)
int get_compressed_blob_data(MySQL* db_handle,
                             int column,
                             char* dest,
                             int dest_len,
                             int unit_size) {
    static CCompressedBlob compressed_blob;
    return compressed_blob.uncompress(db_handle, column, dest, dest_len,
                                      unit_size);
}

// IDA_EA: 0x83FBD06
// IDA_NAME: _Z24put_compressed_blob_dataP5MySQLiPciPS1_
// DEMANGLED: put_compressed_blob_data(MySQL*, int, char*, int, char**)
int put_compressed_blob_data(MySQL* db_handle,
                             int column,
                             char* source,
                             int source_len,
                             char** out_blob) {
    static CCompressedBlob compressed_blob;
    return compressed_blob.compress(db_handle, column, source, source_len,
                                    out_blob);
}
