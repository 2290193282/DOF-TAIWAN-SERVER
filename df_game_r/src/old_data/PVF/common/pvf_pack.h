#ifndef OLDSRV_OLD_DATA_PVF_COMMON_PVF_PACK_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_PVF_PACK_H_

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <utility>
#include <vector>


// IDA uses ``unsigned long`` for PVF hashes.  It is 32 bits on the original
// i386 server, but remains a distinct Itanium ABI type from unsigned int.
typedef unsigned long pack_hash_t;

enum pack_ret_t {
    PACK_OK = 0,
    PACK_ERROR = 1
};

enum pack_load_type_t {
    PACK_LOAD_FULL = 0,
    PACK_LOAD_ONLY_INDEX = 1
};

// IDA_EA: 0x8AC3B9A
// IDA_NAME: _Z17_correct_filenamePKcPcjS1_j
// DEMANGLED: _correct_filename(char const*,char *,unsigned int,char *,unsigned int)
pack_ret_t _correct_filename(const char* filename,
                             char* dir,
                             uint32_t dir_size,
                             char* file,
                             uint32_t file_size);

class PackSet;
class Crc32 {
public:
    static const uint32_t* getCrc32Table();
    static uint32_t crc32(uint32_t crc, const char* buffer, uint32_t size);
};

class IPackFileProcess {
public:
    IPackFileProcess();
    virtual ~IPackFileProcess();
    virtual int load(const char* filename,
                     unsigned char** content,
                     uint32_t* content_size) = 0;
    unsigned char* getCommonBuffer();
    unsigned char* getCommonBuffer(uint32_t need_size);

private:
    static unsigned char* s_common_buffer_;
    static uint32_t s_common_buffer_size_;
    static int s_ref_count_;
};

// IDA unit: 99_misc_global/IndexEntry, restored locally for B-owned PVF use.
// IDA_EA: 0x8AC3DE2..0x8AC5880, 0x8AC7A50, 0x8AC9D92.
class IndexEntry {
public:
    enum entry_type_t {
        ENTRY_PACK = 0,
        ENTRY_FILE = 1
    };

    IndexEntry();
    IndexEntry(const IndexEntry& other);
    ~IndexEntry();
    IndexEntry& operator=(const IndexEntry& other);

    void clear();
    entry_type_t type() const;
    const std::string& name() const;
    uint32_t size() const;
    uint32_t aligned_size() const;
    uint32_t crc() const;
    uint32_t offset() const;
    unsigned char* content() const;
    pack_ret_t get_content(PackSet* owner,
                           pack_hash_t code,
                           unsigned char* dst,
                           uint32_t dst_size,
                           uint32_t* content_size);
    pack_ret_t get_content(PackSet* owner,
                           pack_hash_t code,
                           unsigned char** dst,
                           uint32_t* content_size);
    pack_ret_t get_contentEncryptedFile(PackSet* owner,
                                        pack_hash_t code,
                                        unsigned char* dst,
                                        uint32_t dst_size,
                                        uint32_t* content_size);
    pack_ret_t read_info(unsigned char*& mem);
    pack_ret_t write_info(FILE* fp);
    pack_ret_t update_content_from_memory(unsigned char* mem, PackSet* owner);
    pack_ret_t update_content_from_file(const char* name,
                                        PackSet* owner,
                                        IPackFileProcess* process);

private:
    entry_type_t type_;
    std::string name_;
    uint32_t size_;
    uint32_t aligned_size_;
    uint32_t crc_;
    uint32_t offset_;
    unsigned char* content_;

    friend class PackSet;
};

// IDA unit: 99_misc_global/PackSet, restored locally for B-owned PVF use.
// IDA_EA: 0x8AC47F4..0x8AC7680.
class PackSet {
public:
    typedef std::pair<pack_hash_t, IndexEntry> index_pair_t;
    typedef std::pair<pack_hash_t, IndexEntry*> index_pair2_t;
    typedef std::vector<index_pair_t> index_vector_t;
    typedef std::vector<index_pair2_t> index_vector2_t;
    typedef index_vector_t::iterator index_vector_it;
    typedef index_vector_t::const_iterator index_vector_const_it;
    typedef index_vector2_t::iterator index_vector2_it;

    PackSet();
    ~PackSet();

    void clear();
    uint32_t revision();
    uint32_t header_size();
    const index_vector_t* indexes() const;
    index_vector_t* indexes();
    bool is_ready() const;
    bool is_dirty() const;
    bool is_exist(const char* name);
    void close();
    void closeEncryptedFile();
    FILE* pack_file();
    bool UsingEncryptedFile();
    index_vector_const_it find_const(const char* name);
    index_vector_it find(const char* name);
    pack_ret_t get(const char* name,
                   unsigned char* buffer,
                   uint32_t buffer_size,
                   uint32_t* content_size);
    pack_ret_t get(const char* name,
                   unsigned char** content,
                   uint32_t* content_size);
    pack_ret_t deleted(const char* name);
    void freeContent(const char* name);
    pack_ret_t modified(const char* name, IPackFileProcess* process);
    pack_ret_t added(const char* name, IPackFileProcess* process);
    pack_ret_t sort();
    pack_ret_t open(const char* name, const char* tag, pack_load_type_t type);
    pack_ret_t openEncryptedFile(const char* name,
                                 const char* tag,
                                 pack_load_type_t type);
    pack_ret_t write(const char* name, const char* tag, uint32_t rev);
    pack_ret_t output_file_list(const char* output_path) const;
    pack_ret_t verify();
    pack_ret_t verifyEncryptedFile();
    void UseEncryptedFile();

private:
    FILE* pack_file_;
    uint32_t revision_;
    uint32_t header_size_;
    pack_load_type_t type_;
    index_vector_t indexes_;
    index_vector_t addeds_;
    bool m_bUseEncryptedFile;

    PackSet(const PackSet&);
    PackSet& operator=(const PackSet&);
};

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_PVF_PACK_H_
