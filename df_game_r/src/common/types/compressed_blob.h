#ifndef OLDSRV_GLOBAL_COMPRESSED_BLOB_H_
#define OLDSRV_GLOBAL_COMPRESSED_BLOB_H_


class MySQL;

// IDA unit: 99_misc_global/CCompressedBlob.
// IDA_EA: ctor @0x844DE52, dtor @0x844DE6A, compress @0x844DE9A,
// uncompress @0x844DF40, resize @0x844E02A, reset @0x844E080.
class CCompressedBlob {
public:
    CCompressedBlob();
    ~CCompressedBlob();

    int compress(MySQL* db_handle,
                 int column,
                 char* source,
                 int source_len,
                 char** out_blob);
    int uncompress(MySQL* db_handle,
                   int column,
                   char* dest,
                   int dest_len,
                   int unit_size);
    CCompressedBlob* resize(int size);
    void* reset();

private:
    char* buffer_;
    int capacity_;

    CCompressedBlob(const CCompressedBlob&);
    CCompressedBlob& operator=(const CCompressedBlob&);
};

int get_compressed_blob_data(MySQL* db_handle,
                             int column,
                             char* dest,
                             int dest_len,
                             int unit_size);
int put_compressed_blob_data(MySQL* db_handle,
                             int column,
                             char* source,
                             int source_len,
                             char** out_blob);


int compress_zip(char* dest, unsigned int& dest_len, const char* source,
                 unsigned int source_len);
int uncompress_zip(char* dest, unsigned int& dest_len, const char* source,
                   unsigned int source_len);

#endif  // OLDSRV_GLOBAL_COMPRESSED_BLOB_H_
