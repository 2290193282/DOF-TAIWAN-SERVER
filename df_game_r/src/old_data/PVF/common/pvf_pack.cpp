#include "old_data/PVF/common/pvf_pack.h"

#include <algorithm>
#include <deque>
#include <new>
#include <stack>
#include <string.h>
#include <time.h>

static uint32_t crc_table[256] = {};
// IDA global XOR_KEY @0x950189C is .bss in the original df_game_r: it starts
// at zero and its only references in the executable are the two read sites in
// _encrypt/_decrypt_N_crc32.  Keeping it at 0xFFFFFFFF changes the index key
// and rejects the authentic Script.pvf header CRC.
static uint32_t XOR_KEY = 0u;
static uint32_t g_AdditionalKey = 0x81A79011u;

unsigned char* IPackFileProcess::s_common_buffer_ = 0;
uint32_t IPackFileProcess::s_common_buffer_size_ = 0;
int IPackFileProcess::s_ref_count_ = 0;

// IDA_EA: 0x8AC768C
// IDA_NAME: _ZN16IPackFileProcessC2Ev
// DEMANGLED: IPackFileProcess::IPackFileProcess(void)
IPackFileProcess::IPackFileProcess() {
    ++s_ref_count_;
}

// IDA_EA: 0x8AC76A8
// IDA_NAME: _ZN16IPackFileProcessD2Ev
// DEMANGLED: IPackFileProcess::~IPackFileProcess()
// IDA_EA: 0x8AC7716
// IDA_NAME: _ZN16IPackFileProcessD0Ev
// DEMANGLED: IPackFileProcess::~IPackFileProcess()
// The virtual C++ destructor below is the one canonical body; GCC emits the
// original deleting-destructor ABI entry point from it.
IPackFileProcess::~IPackFileProcess() {
    if (--s_ref_count_ <= 0) {
        delete[] s_common_buffer_;
        s_common_buffer_ = 0;
        s_common_buffer_size_ = 0;
    }
}

// IDA_EA: 0x8AC7734
// IDA_NAME: _ZN16IPackFileProcess15getCommonBufferEv
// DEMANGLED: IPackFileProcess::getCommonBuffer(void)
unsigned char* IPackFileProcess::getCommonBuffer() {
    return s_common_buffer_;
}

// IDA_EA: 0x8AC773E
// IDA_NAME: _ZN16IPackFileProcess15getCommonBufferEj
// DEMANGLED: IPackFileProcess::getCommonBuffer(unsigned int)
unsigned char* IPackFileProcess::getCommonBuffer(uint32_t need_size) {
    if (need_size > s_common_buffer_size_) {
        delete[] s_common_buffer_;
        s_common_buffer_ = new unsigned char[need_size];
        s_common_buffer_size_ = s_common_buffer_ != 0 ? need_size : 0;
    }
    return s_common_buffer_;
}

static uint32_t ror32(uint32_t value, unsigned int count) {
    count &= 31u;
    return (value >> count) | (value << (32u - count));
}

static int ss_strcpy(char* dest, size_t count, const char* src) {
    if (dest == 0 || count == 0) {
        return 0;
    }
    if (src == 0) {
        dest[0] = 0;
        return 0;
    }
    strncpy(dest, src, count - 1);
    dest[count - 1] = 0;
    return 1;
}

static uint32_t read_u32(const unsigned char* data) {
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
}

static void write_u32(unsigned char* data, uint32_t value) {
    data[0] = static_cast<unsigned char>(value & 0xFFu);
    data[1] = static_cast<unsigned char>((value >> 8) & 0xFFu);
    data[2] = static_cast<unsigned char>((value >> 16) & 0xFFu);
    data[3] = static_cast<unsigned char>((value >> 24) & 0xFFu);
}

// IDA_EA: 0x8AC38BE
// IDA_NAME: _ZL8_encryptjPhj
// DEMANGLED: _encrypt(unsigned int,unsigned char *,unsigned int)
static __attribute__((noinline, used))
pack_ret_t _encrypt(uint32_t key, unsigned char* buffer, uint32_t size) {
    if ((size & 3u) != 0) {
        return PACK_ERROR;
    }
    if (XOR_KEY != 0) {
        key ^= XOR_KEY;
    }
    for (unsigned char* p = buffer; p != &buffer[4 * (size >> 2)]; p += 4) {
        uint32_t word = read_u32(p);
        word = key ^ ror32(word, 26);
        if (g_AdditionalKey != 0) {
            word ^= g_AdditionalKey;
        }
        write_u32(p, word);
    }
    return PACK_OK;
}

// IDA_EA: 0x8AD3420
// IDA_NAME: _ZL14make_crc_tablev
// DEMANGLED: make_crc_table(void)
static __attribute__((noinline, used)) void make_crc_table() {
    uint32_t h = 1;
    crc_table[0] = 0;
    for (uint32_t i = 128; i != 0; i >>= 1) {
        uint32_t poly = 0;
        if ((h & 1u) != 0) {
            poly = 0xEDB88320u;
        }
        h = (h >> 1) ^ poly;
        for (uint32_t j = 0; j <= 0xFFu; j += 2 * i) {
            crc_table[j + i] = h ^ crc_table[j];
        }
    }
}

// IDA_EA: 0x8AD3504
// IDA_NAME: _ZN5Crc3213getCrc32TableEv
// DEMANGLED: Crc32::getCrc32Table(void)
const uint32_t* Crc32::getCrc32Table() {
    if (crc_table[255] == 0) {
        make_crc_table();
    }
    return crc_table;
}

// IDA_EA: 0x8AD34AE
// IDA_NAME: _ZN5Crc325crc32EjPKcj
// DEMANGLED: Crc32::crc32(unsigned int,char const*,unsigned int)
uint32_t Crc32::crc32(uint32_t crc, const char* buf, uint32_t len) {
    const uint32_t* table = getCrc32Table();
    uint32_t crca = ~crc;
    while (len-- != 0) {
        crca = (crca >> 8) ^
               table[static_cast<unsigned char>(crca ^ *buf++)];
    }
    return ~crca;
}

// IDA_EA: 0x8AC398B
// IDA_NAME: _ZL16_decrypt_N_crc32mmmPhj
// DEMANGLED: _decrypt_N_crc32(unsigned long,unsigned long,unsigned long,unsigned char *,unsigned int)
static __attribute__((noinline, used))
int _decrypt_N_crc32(unsigned long crc,
                     unsigned long checksum,
                     unsigned long key,
                     unsigned char* buffer,
                     uint32_t size) {
    if ((size & 3u) != 0) {
        return 0;
    }

    const uint32_t* table = Crc32::getCrc32Table();
    uint32_t crca = ~crc;
    if (XOR_KEY != 0) {
        key ^= XOR_KEY;
    }

    for (unsigned char* p = buffer; p != &buffer[4 * (size >> 2)]; p += 4) {
        uint32_t word = read_u32(p);
        if (g_AdditionalKey != 0) {
            word ^= g_AdditionalKey;
        }
        word = ror32(key ^ word, 6);
        write_u32(p, word);

        uint32_t crcb = (crca >> 8) ^ table[static_cast<unsigned char>(crca ^ p[0])];
        uint32_t crcc = (crcb >> 8) ^ table[static_cast<unsigned char>(crcb ^ p[1])];
        uint32_t crcd = (crcc >> 8) ^ table[static_cast<unsigned char>(crcc ^ p[2])];
        crca = (crcd >> 8) ^ table[static_cast<unsigned char>(crcd ^ p[3])];
    }

    return ~crca == checksum;
}

// IDA_EA: 0x8AC3B04
// IDA_NAME: _ZL5_hashPKc
// DEMANGLED: _hash(char const*)
static __attribute__((noinline, used)) pack_hash_t _hash(const char* p) {
    char c = *p;
    uint32_t code = 5381;
    while (c != 0) {
        c = *p++;
        code = 33 * code + static_cast<unsigned char>(c);
    }
    return code;
}

// IDA_EA: 0x8AC3B55
// IDA_NAME: _ZL11_write_hashPKcS0_
// DEMANGLED: _write_hash(char const*,char const*)
static __attribute__((noinline, used))
pack_hash_t _write_hash(const char* dirname, const char* filename) {
    const pack_hash_t dcode = _hash(dirname);
    const pack_hash_t fcode = _hash(filename);
    return (dcode & 0xFFFF0000u) | ((fcode >> 16) & 0xFFFFu);
}

// IDA_EA: 0x8AC3714
// IDA_NAME: _ZL11_fwrite_strPKcP8_IO_FILE
// DEMANGLED: _fwrite_str(char const*,FILE *)
static __attribute__((noinline, used))
pack_ret_t _fwrite_str(const char* src, FILE* fp) {
    if (src == 0 || fp == 0) {
        return PACK_ERROR;
    }
    unsigned char len_bytes[4];
    const uint32_t len = static_cast<uint32_t>(strlen(src));
    write_u32(len_bytes, len);
    fwrite(len_bytes, 4, 1, fp);
    if (len != 0) {
        fwrite(src, len, 1, fp);
    }
    return PACK_OK;
}

// IDA_EA: 0x8AC37F3
// IDA_NAME: _ZL10_fread_strRSsP8_IO_FILE
// DEMANGLED: _fread_str(std::string &,FILE *)
static __attribute__((noinline, used))
pack_ret_t _fread_str(std::string& str, FILE* fp) {
    unsigned char length_bytes[4] = {};
    if (fp == 0) {
        return PACK_ERROR;
    }
    if (fread(length_bytes, 4, 1, fp) != 1) {
        return PACK_ERROR;
    }
    const uint32_t len = read_u32(length_bytes);
    if (len > 0x103u) {
        return PACK_ERROR;
    }
    char mbcs[260];
    if (len != 0 && fread(mbcs, len, 1, fp) != 1) {
        return PACK_ERROR;
    }
    mbcs[len] = 0;
    str = mbcs;
    return PACK_OK;
}

// IDA_EA: 0x8AC3832
// IDA_NAME: _ZL10_mread_strRSsRPh
// DEMANGLED: _mread_str(std::string &,unsigned char *&)
static __attribute__((noinline, used))
pack_ret_t _mread_str(std::string& str, unsigned char*& mem) {
    if (mem == 0) {
        return PACK_ERROR;
    }
    const uint32_t len = read_u32(mem);
    mem += 4;
    if (len > 0x103u) {
        return PACK_ERROR;
    }
    char mbcs[260];
    memcpy(mbcs, mem, len);
    mem += len;
    mbcs[len] = 0;
    str = mbcs;
    return PACK_OK;
}

// IDA_EA: 0x8AC3B9A
// IDA_NAME: _Z17_correct_filenamePKcPcjS1_j
// DEMANGLED: _correct_filename(char const*,char *,unsigned int,char *,unsigned int)
__attribute__((noinline, used))
pack_ret_t _correct_filename(const char* filename,
                             char* dir,
                             uint32_t dir_size,
                             char* file,
                             uint32_t file_size) {
    if (filename == 0 || dir == 0 || dir_size == 0) {
        return PACK_ERROR;
    }

    char* d_p = dir;
    const char* s_p = filename;
    std::deque<char*> deque_data;
    std::stack<char*, std::deque<char*> > dir_stack(deque_data);
    dir_stack.push(d_p);

    char c = 0;
    while (1) {
        while (1) {
            c = *s_p++;
            if (c == 0) {
                break;
            }
            if (c == '/' || c == '\\') {
                if (static_cast<uint32_t>(d_p - dir) + 1 >= dir_size) {
                    *dir = 0;
                    return PACK_ERROR;
                }
                *d_p++ = '/';
                break;
            }
            if (static_cast<uint32_t>(d_p - dir) + 1 >= dir_size) {
                *dir = 0;
                return PACK_ERROR;
            }
            *d_p++ = (c > 64 && c <= 90) ? static_cast<char>(c + 32) : c;
        }

        if (c == 0) {
            break;
        }

        char* top = dir_stack.top();
        if (d_p - top == 1) {
            d_p = top;
        } else if (d_p - top == 2 && top[0] == '.') {
            d_p = top;
        } else if (d_p - top == 3 && top[0] == '.' && top[1] == '.') {
            if (dir_stack.empty()) {
                return PACK_ERROR;
            }
            dir_stack.pop();
            if (dir_stack.empty()) {
                return PACK_ERROR;
            }
            d_p = dir_stack.top();
        } else {
            dir_stack.push(d_p);
        }
    }

    *d_p = 0;
    if (file != 0) {
        char* top = dir_stack.top();
        ss_strcpy(file, file_size, top);
        if (top == dir) {
            *top = 0;
        } else {
            *(top - 1) = 0;
        }
    }

    return PACK_OK;
}

// IDA_EA: 0x8AC3ADC
// IDA_NAME: _ZL11_index_lessRKSt4pairIm10IndexEntryES3_
// DEMANGLED: _index_less(std::pair<unsigned long,IndexEntry> const&,std::pair<unsigned long,IndexEntry> const&)
static __attribute__((noinline, used))
bool _index_less(const PackSet::index_pair_t& e1,
                 const PackSet::index_pair_t& e2) {
    return e1.first < e2.first;
}

// IDA_EA: 0x8AC3AF0
// IDA_NAME: _ZL12_index_less2RKSt4pairImP10IndexEntryES4_
// DEMANGLED: _index_less2(std::pair<unsigned long,IndexEntry *> const&,std::pair<unsigned long,IndexEntry *> const&)
static __attribute__((noinline, used))
bool _index_less2(const PackSet::index_pair2_t& e1,
                  const PackSet::index_pair2_t& e2) {
    return e1.first < e2.first;
}

// Keep PackSet's original normalized/hash lookup as the primary path.  Some
// PVF revisions retain mixed-case index names even though the original
// _correct_filename ABI folds caller input to ASCII lowercase; only those
// otherwise-unreachable entries need this compatibility fallback.
static bool _path_equal_folded(const char* canonical_name,
                               const char* indexed_name) {
    if (canonical_name == 0 || indexed_name == 0) {
        return false;
    }

    while (*canonical_name != 0 && *indexed_name != 0) {
        char indexed = *indexed_name;
        if (indexed == '\\') {
            indexed = '/';
        } else if (indexed >= 'A' && indexed <= 'Z') {
            indexed = static_cast<char>(indexed + ('a' - 'A'));
        }
        if (*canonical_name != indexed) {
            return false;
        }
        ++canonical_name;
        ++indexed_name;
    }
    return *canonical_name == 0 && *indexed_name == 0;
}

// A few authentic PVF indexes contain names whose stored hash cannot be
// reproduced by the server's path normalizer (notably legacy DBCS names).
// Falling back to a complete indexes_ scan keeps those files reachable, but a
// missing optional script then compares every path in a 400k-entry PVF.  Keep
// a sidecar sorted by the normalized hash instead.  The sidecar deliberately
// lives outside PackSet so the original i386 class layout and ABI do not move.
struct FoldedIndexRecord {
    FoldedIndexRecord(pack_hash_t value, uint32_t original_position)
        : hash(value), position(original_position) {}

    pack_hash_t hash;
    uint32_t position;
};

struct FoldedIndexSlot {
    FoldedIndexSlot() : owner(0), records() {}

    const PackSet* owner;
    std::vector<FoldedIndexRecord> records;
};

static uint32_t _folded_path_hash(const char* path) {
    if (path == 0) {
        return 0;
    }

    // Match _hash(), including its final multiply/add of the terminating NUL.
    unsigned char ch = static_cast<unsigned char>(*path);
    uint32_t code = 5381u;
    while (ch != 0) {
        ch = static_cast<unsigned char>(*path++);
        if (ch == static_cast<unsigned char>('\\')) {
            ch = static_cast<unsigned char>('/');
        } else if (ch >= static_cast<unsigned char>('A') &&
                   ch <= static_cast<unsigned char>('Z')) {
            ch = static_cast<unsigned char>(ch + ('a' - 'A'));
        }
        code = 33u * code + ch;
    }
    return code;
}

static bool _folded_index_less(const FoldedIndexRecord& lhs,
                               const FoldedIndexRecord& rhs) {
    if (lhs.hash != rhs.hash) {
        return lhs.hash < rhs.hash;
    }
    // The former linear fallback returned the first entry in indexes_.  Keep
    // that priority even if a future PVF contains folded-path duplicates.
    return lhs.position < rhs.position;
}

static std::vector<FoldedIndexSlot>& _folded_index_registry() {
    // PackSet is itself process-global in the game server.  A deliberately
    // process-lifetime registry avoids cross-translation-unit destruction
    // order hazards when that PackSet's destructor calls close().  Individual
    // record vectors are still erased by PackSet::clear().
    static std::vector<FoldedIndexSlot>* registry =
        new std::vector<FoldedIndexSlot>();
    return *registry;
}

static FoldedIndexSlot* _find_folded_index_slot(const PackSet* owner) {
    std::vector<FoldedIndexSlot>& registry = _folded_index_registry();
    for (std::vector<FoldedIndexSlot>::iterator it = registry.begin();
         it != registry.end(); ++it) {
        if (it->owner == owner) {
            return &*it;
        }
    }
    return 0;
}

static void _erase_folded_index(const PackSet* owner) {
    std::vector<FoldedIndexSlot>& registry = _folded_index_registry();
    for (std::vector<FoldedIndexSlot>::iterator it = registry.begin();
         it != registry.end(); ++it) {
        if (it->owner == owner) {
            registry.erase(it);
            return;
        }
    }
}

static void _rebuild_folded_index(PackSet* owner) {
    if (owner == 0) {
        return;
    }

    _erase_folded_index(owner);
    std::vector<FoldedIndexSlot>& registry = _folded_index_registry();
    registry.push_back(FoldedIndexSlot());
    FoldedIndexSlot& slot = registry.back();
    slot.owner = owner;

    const PackSet::index_vector_t* indexes = owner->indexes();
    slot.records.reserve(indexes->size());
    for (uint32_t position = 0;
         position < static_cast<uint32_t>(indexes->size()); ++position) {
        slot.records.push_back(FoldedIndexRecord(
            _folded_path_hash((*indexes)[position].second.name().c_str()),
            position));
    }
    std::sort(slot.records.begin(), slot.records.end(), _folded_index_less);
}

static bool _find_folded_index_position(const PackSet* owner,
                                        const char* canonical_name,
                                        pack_hash_t hash,
                                        uint32_t* position) {
    if (owner == 0 || canonical_name == 0 || position == 0) {
        return false;
    }

    FoldedIndexSlot* slot = _find_folded_index_slot(owner);
    if (slot == 0) {
        return false;
    }

    const FoldedIndexRecord key(hash, 0);
    std::vector<FoldedIndexRecord>::const_iterator it =
        std::lower_bound(slot->records.begin(), slot->records.end(), key,
                         _folded_index_less);
    const PackSet::index_vector_t* indexes = owner->indexes();
    for (; it != slot->records.end() && it->hash == hash; ++it) {
        if (it->position >= indexes->size()) {
            continue;
        }
        if (_path_equal_folded(
                canonical_name,
                (*indexes)[it->position].second.name().c_str())) {
            *position = it->position;
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x8AC3DE2
// IDA_NAME: _ZN10IndexEntryC2Ev
// DEMANGLED: IndexEntry::IndexEntry(void)
IndexEntry::IndexEntry() : name_() { clear(); }

// IDA_EA: 0x8AC7A50
// IDA_NAME: _ZN10IndexEntryC2ERKS_
// DEMANGLED: IndexEntry::IndexEntry(IndexEntry const&)
IndexEntry::IndexEntry(const IndexEntry& other)
    : type_(other.type_),
      name_(other.name_),
      size_(other.size_),
      aligned_size_(other.aligned_size_),
      crc_(other.crc_),
      offset_(other.offset_),
      content_(other.content_) {}

// IDA_EA: 0x8AC3E04
// IDA_NAME: _ZN10IndexEntryD2Ev
// DEMANGLED: IndexEntry::~IndexEntry()
IndexEntry::~IndexEntry() {}

// IDA_EA: 0x8AC9D92
// IDA_NAME: _ZN10IndexEntryaSERKS_
// DEMANGLED: IndexEntry::operator=(IndexEntry const&)
IndexEntry& IndexEntry::operator=(const IndexEntry& other) {
    type_ = other.type_;
    name_ = other.name_;
    size_ = other.size_;
    aligned_size_ = other.aligned_size_;
    crc_ = other.crc_;
    offset_ = other.offset_;
    content_ = other.content_;
    return *this;
}

// IDA_EA: 0x8AC3E1A
// IDA_NAME: _ZN10IndexEntry5clearEv
// DEMANGLED: IndexEntry::clear(void)
void IndexEntry::clear() {
    type_ = ENTRY_PACK;
    size_ = 0;
    aligned_size_ = 0;
    crc_ = 0;
    offset_ = 0;
    content_ = 0;
}

// IDA_EA: 0x8AC3E5A
// IDA_NAME: _ZNK10IndexEntry4typeEv
// DEMANGLED: IndexEntry::type(void)const
IndexEntry::entry_type_t IndexEntry::type() const { return type_; }

// IDA_EA: 0x8AC3E64
// IDA_NAME: _ZNK10IndexEntry4nameEv
// DEMANGLED: IndexEntry::name(void)const
const std::string& IndexEntry::name() const { return name_; }

// IDA_EA: 0x8AC3E70
// IDA_NAME: _ZNK10IndexEntry4sizeEv
// DEMANGLED: IndexEntry::size(void)const
uint32_t IndexEntry::size() const { return size_; }

// IDA_EA: 0x8AC3E7C
// IDA_NAME: _ZNK10IndexEntry12aligned_sizeEv
// DEMANGLED: IndexEntry::aligned_size(void)const
uint32_t IndexEntry::aligned_size() const { return aligned_size_; }

// IDA_EA: 0x8AC3E88
// IDA_NAME: _ZNK10IndexEntry3crcEv
// DEMANGLED: IndexEntry::crc(void)const
uint32_t IndexEntry::crc() const { return crc_; }

// IDA_EA: 0x8AC3E94
// IDA_NAME: _ZNK10IndexEntry6offsetEv
// DEMANGLED: IndexEntry::offset(void)const
uint32_t IndexEntry::offset() const { return offset_; }

// IDA_EA: 0x8AC3EA0
// IDA_NAME: _ZNK10IndexEntry7contentEv
// DEMANGLED: IndexEntry::content(void)const
unsigned char* IndexEntry::content() const { return content_; }

// IDA_EA: 0x8AC3EAC
// IDA_NAME: _ZN10IndexEntry11get_contentEP7PackSetmPhjPj
// DEMANGLED: IndexEntry::get_content(PackSet *,unsigned long,unsigned char *,unsigned int,unsigned int *)
pack_ret_t IndexEntry::get_content(PackSet* owner,
                                   pack_hash_t code,
                                   unsigned char* dst,
                                   uint32_t dst_size,
                                   uint32_t* content_size) {
    if (owner == 0 || dst == 0 || content_size == 0) {
        return PACK_ERROR;
    }
    if (owner->UsingEncryptedFile()) {
        return get_contentEncryptedFile(owner, code, dst, dst_size,
                                        content_size);
    }
    *content_size = 0;
    if (owner->pack_file() == 0) {
        return PACK_ERROR;
    }
    if (size_ > dst_size) {
        return PACK_ERROR;
    }
    if (content_ != 0) {
        memcpy(dst, content_, aligned_size_);
    } else {
        if (type_ == ENTRY_FILE) {
            return PACK_ERROR;
        }
        if (type_ == ENTRY_PACK && aligned_size_ != 0) {
            const long off =
                static_cast<long>(offset_ + owner->header_size());
            FILE* stream = owner->pack_file();
            fseek(stream, off, SEEK_SET);
            stream = owner->pack_file();
            if (fread(dst, aligned_size_, 1, stream) != 1) {
                return PACK_ERROR;
            }
        }
    }
    *content_size = size_;
    if (_decrypt_N_crc32(code, crc_, crc_, dst, aligned_size_)) {
        return PACK_OK;
    }
    return PACK_ERROR;
}

// IDA_EA: 0x8AC41F6
// IDA_NAME: _ZN10IndexEntry24get_contentEncryptedFileEP7PackSetmPhjPj
// DEMANGLED: IndexEntry::get_contentEncryptedFile(PackSet *,unsigned long,unsigned char *,unsigned int,unsigned int *)
pack_ret_t IndexEntry::get_contentEncryptedFile(PackSet* owner,
                                                pack_hash_t code,
                                                unsigned char* dst,
                                                uint32_t dst_size,
                                                uint32_t* content_size) {
    (void)owner;
    (void)code;
    (void)dst;
    (void)dst_size;
    (void)content_size;
    return PACK_ERROR;
}

// IDA_EA: 0x8AC4200
// IDA_NAME: _ZN10IndexEntry9read_infoERPh
// DEMANGLED: IndexEntry::read_info(unsigned char *&)
pack_ret_t IndexEntry::read_info(unsigned char*& mem) {
    if (_mread_str(name_, mem) == PACK_ERROR) {
        return PACK_ERROR;
    }
    size_ = read_u32(mem);
    mem += 4;
    aligned_size_ = (size_ + 3u) & 0xFFFFFFFCu;
    crc_ = read_u32(mem);
    mem += 4;
    offset_ = read_u32(mem);
    mem += 4;
    return PACK_OK;
}

// IDA_EA: 0x8AC4284
// IDA_NAME: _ZN10IndexEntry10write_infoEP8_IO_FILE
// DEMANGLED: IndexEntry::write_info(_IO_FILE *)
pack_ret_t IndexEntry::write_info(FILE* fp) {
    if (fp == 0) {
        return PACK_ERROR;
    }
    _fwrite_str(name_.c_str(), fp);
    unsigned char bytes[4];
    write_u32(bytes, size_);
    fwrite(bytes, 4, 1, fp);
    write_u32(bytes, crc_);
    fwrite(bytes, 4, 1, fp);
    write_u32(bytes, offset_);
    fwrite(bytes, 4, 1, fp);
    return PACK_OK;
}

// IDA_EA: 0x8AC4378
// IDA_NAME: _ZN10IndexEntry26update_content_from_memoryEPhP7PackSet
// DEMANGLED: IndexEntry::update_content_from_memory(unsigned char *,PackSet *)
pack_ret_t IndexEntry::update_content_from_memory(unsigned char* mem,
                                                  PackSet* owner) {
    if (mem == 0) {
        return PACK_ERROR;
    }
    if (type_ == ENTRY_FILE) {
        return PACK_ERROR;
    }
    if (type_ == ENTRY_PACK) {
        content_ = new unsigned char[aligned_size_];
        memcpy(content_, &mem[offset_ + owner->header_size()], aligned_size_);
    }
    return PACK_OK;
}

// IDA_EA: 0x8AC4476
// IDA_NAME: _ZN10IndexEntry24update_content_from_fileEPKcP7PackSetP16IPackFileProcess
// DEMANGLED: IndexEntry::update_content_from_file(char const*,PackSet *,IPackFileProcess *)
pack_ret_t IndexEntry::update_content_from_file(const char* name,
                                                PackSet* owner,
                                                IPackFileProcess* process) {
    (void)owner;
    char dir[260];
    if (_correct_filename(name, dir, sizeof(dir), 0, 0) == PACK_ERROR) {
        return PACK_ERROR;
    }

    FILE* fp = fopen(dir, "rb");
    if (fp == 0) {
        return PACK_ERROR;
    }

    type_ = ENTRY_FILE;
    name_ = dir;
    fseek(fp, 0, SEEK_END);
    const long file_size = ftell(fp);
    if (file_size < 0) {
        fclose(fp);
        return PACK_ERROR;
    }

    size_ = static_cast<uint32_t>(file_size);
    fseek(fp, 0, SEEK_SET);
    aligned_size_ = (size_ + 3u) & 0xFFFFFFFCu;
    if (size_ != 0) {
        if (process != 0) {
            fclose(fp);
            fp = 0;
            if (process->load(name, &content_, &size_) == PACK_ERROR) {
                return PACK_ERROR;
            }
            aligned_size_ = (size_ + 3u) & 0xFFFFFFFCu;
        } else {
            content_ = new unsigned char[aligned_size_];
            memset(&content_[size_], 0, aligned_size_ - size_);
            if (fread(content_, size_, 1, fp) != 1) {
                fclose(fp);
                return PACK_ERROR;
            }
        }
    } else {
        content_ = new unsigned char[4];
        write_u32(content_, 0);
    }

    if (fp != 0) {
        fclose(fp);
    }

    const pack_hash_t crc_key = _hash(dir);
    crc_ = Crc32::crc32(static_cast<uint32_t>(crc_key),
                        (const char*)content_, aligned_size_);
    if (_encrypt(crc_, content_, aligned_size_) == PACK_ERROR) {
        return PACK_ERROR;
    }
    return PACK_OK;
}

// IDA_EA: 0x8AC5880
// IDA_NAME: _ZN10IndexEntry11get_contentEP7PackSetmPPhPj
// DEMANGLED: IndexEntry::get_content(PackSet *,unsigned long,unsigned char **,unsigned int *)
pack_ret_t IndexEntry::get_content(PackSet* owner,
                                   pack_hash_t code,
                                   unsigned char** dst,
                                   uint32_t* content_size) {
    if (dst == 0) {
        return PACK_ERROR;
    }
    *dst = new unsigned char[aligned_size_];
    if (*dst != 0) {
        return get_content(owner, code, *dst, aligned_size_, content_size);
    }
    return PACK_ERROR;
}

// IDA_EA: 0x8AC47F4
// IDA_NAME: _ZN7PackSetC2Ev
// DEMANGLED: PackSet::PackSet(void)
PackSet::PackSet()
    : pack_file_(0),
      revision_(0),
      header_size_(0),
      type_(PACK_LOAD_FULL),
      indexes_(),
      addeds_(),
      m_bUseEncryptedFile(false) {
    clear();
}

// IDA_EA: 0x8AC4862
// IDA_NAME: _ZN7PackSetD2Ev
// DEMANGLED: PackSet::~PackSet()
PackSet::~PackSet() { close(); }

// IDA_EA: 0x8AC48D2
// IDA_NAME: _ZN7PackSet5clearEv
// DEMANGLED: PackSet::clear(void)
void PackSet::clear() {
    _erase_folded_index(this);
    pack_file_ = 0;
    revision_ = 0;
    header_size_ = 0;
    type_ = PACK_LOAD_FULL;
    indexes_.clear();
    addeds_.clear();
    m_bUseEncryptedFile = false;
}

// IDA_EA: 0x8AC4924
// IDA_NAME: _ZN7PackSet8revisionEv
// DEMANGLED: PackSet::revision(void)
uint32_t PackSet::revision() { return revision_; }

// IDA_EA: 0x8AC4930
// IDA_NAME: _ZN7PackSet11header_sizeEv
// DEMANGLED: PackSet::header_size(void)
uint32_t PackSet::header_size() { return header_size_; }

// IDA_EA: 0x8AC493C
// IDA_NAME: _ZNK7PackSet7indexesEv
// DEMANGLED: PackSet::indexes(void)const
const PackSet::index_vector_t* PackSet::indexes() const { return &indexes_; }

// IDA_EA: 0x8AC4948
// IDA_NAME: _ZN7PackSet7indexesEv
// DEMANGLED: PackSet::indexes(void)
PackSet::index_vector_t* PackSet::indexes() { return &indexes_; }

// IDA_EA: 0x8AC4954
// IDA_NAME: _ZNK7PackSet8is_readyEv
// DEMANGLED: PackSet::is_ready(void)const
bool PackSet::is_ready() const { return indexes_.size() != 0; }

// IDA_EA: 0x8AC4970
// IDA_NAME: _ZNK7PackSet8is_dirtyEv
// DEMANGLED: PackSet::is_dirty(void)const
bool PackSet::is_dirty() const { return addeds_.size() != 0; }

// IDA_EA: 0x8AC498C
// IDA_NAME: _ZN7PackSet8is_existEPKc
// DEMANGLED: PackSet::is_exist(char const*)
bool PackSet::is_exist(const char* name) {
    return find(name) != indexes_.end();
}

// IDA_EA: 0x8AC49DA
// IDA_NAME: _ZN7PackSet5closeEv
// DEMANGLED: PackSet::close(void)
void PackSet::close() {
    if (UsingEncryptedFile()) {
        closeEncryptedFile();
        return;
    }
    if (pack_file_ != 0) {
        fclose(pack_file_);
    }
    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        delete[] it->second.content_;
        it->second.content_ = 0;
    }
    for (index_vector_it it = addeds_.begin(); it != addeds_.end(); ++it) {
        delete[] it->second.content_;
        it->second.content_ = 0;
    }
    clear();
}

// IDA_EA: 0x8AC4B66
// IDA_NAME: _ZN7PackSet18closeEncryptedFileEv
// DEMANGLED: PackSet::closeEncryptedFile(void)
void PackSet::closeEncryptedFile() {}

// IDA_EA: 0x8AC4B6C
// IDA_NAME: _ZN7PackSet10find_constEPKc
// DEMANGLED: PackSet::find_const(char const*)
PackSet::index_vector_const_it PackSet::find_const(const char* name) {
    if (name == 0) {
        return indexes_.end();
    }
    char dir[260];
    if (_correct_filename(name, dir, sizeof(dir), 0, 0) == PACK_ERROR) {
        return indexes_.end();
    }
    const pack_hash_t code = _hash(dir);
    index_pair_t dummy(code, IndexEntry());
    std::pair<index_vector_const_it, index_vector_const_it> range =
        std::equal_range(indexes_.begin(), indexes_.end(), dummy, _index_less);
    if (range.first != indexes_.end() && range.first->first == code) {
        for (index_vector_const_it it = range.first; it != range.second; ++it) {
            if (it->first != code) {
                break;
            }
            if (strcmp(it->second.name_.c_str(), dir) == 0) {
                return it;
            }
        }
    }
    uint32_t folded_position = 0;
    if (_find_folded_index_position(this, dir, code, &folded_position)) {
        return indexes_.begin() + folded_position;
    }
    return indexes_.end();
}

// IDA_EA: 0x8AC4E00
// IDA_NAME: _ZN7PackSet4findEPKc
// DEMANGLED: PackSet::find(char const*)
PackSet::index_vector_it PackSet::find(const char* name) {
    if (name == 0) {
        return indexes_.end();
    }
    char dir[260];
    if (_correct_filename(name, dir, sizeof(dir), 0, 0) == PACK_ERROR) {
        return indexes_.end();
    }
    const pack_hash_t code = _hash(dir);
    index_pair_t dummy(code, IndexEntry());
    std::pair<index_vector_it, index_vector_it> range =
        std::equal_range(indexes_.begin(), indexes_.end(), dummy, _index_less);
    if (range.first != indexes_.end() && range.first->first == code) {
        for (index_vector_it it = range.first; it != range.second; ++it) {
            if (it->first != code) {
                break;
            }
            if (strcmp(it->second.name_.c_str(), dir) == 0) {
                return it;
            }
        }
    }
    uint32_t folded_position = 0;
    if (_find_folded_index_position(this, dir, code, &folded_position)) {
        return indexes_.begin() + folded_position;
    }
    return indexes_.end();
}

// IDA_EA: 0x8AC5060
// IDA_NAME: _ZN7PackSet3getEPKcPhjPj
// DEMANGLED: PackSet::get(char const*,unsigned char *,unsigned int,unsigned int *)
pack_ret_t PackSet::get(const char* name,
                        unsigned char* buffer,
                        uint32_t buffer_size,
                        uint32_t* content_size) {
    if (name == 0 || buffer == 0 || content_size == 0) {
        return PACK_ERROR;
    }
    index_vector_it it = find(name);
    if (it == indexes_.end()) {
        return PACK_ERROR;
    }
    return it->second.get_content(this, it->first, buffer, buffer_size,
                                  content_size);
}

// IDA_EA: 0x8AC51CE
// IDA_NAME: _ZN7PackSet3getEPKcPPhPj
// DEMANGLED: PackSet::get(char const*,unsigned char **,unsigned int *)
pack_ret_t PackSet::get(const char* name,
                        unsigned char** content,
                        uint32_t* content_size) {
    if (name == 0 || content == 0 || content_size == 0) {
        return PACK_ERROR;
    }
    index_vector_it it = find(name);
    if (it == indexes_.end()) {
        return PACK_ERROR;
    }
    return it->second.get_content(this, it->first, content, content_size);
}

// IDA_EA: 0x8AC5332
// IDA_NAME: _ZN7PackSet7deletedEPKc
// DEMANGLED: PackSet::deleted(char const*)
pack_ret_t PackSet::deleted(const char* name) {
    if (type_ != PACK_LOAD_FULL) {
        return PACK_ERROR;
    }
    index_vector_it it = find(name);
    if (it != indexes_.end()) {
        delete[] it->second.content_;
        indexes_.erase(it);
        _rebuild_folded_index(this);
    }
    return PACK_OK;
}

// IDA_EA: 0x8AC5448
// IDA_NAME: _ZN7PackSet11freeContentEPKc
// DEMANGLED: PackSet::freeContent(char const*)
void PackSet::freeContent(const char* name) {
    index_vector_it it = find(name);
    if (it != indexes_.end() && it->second.content_ != 0) {
        delete[] it->second.content_;
        it->second.content_ = 0;
    }
}

// IDA_EA: 0x8AC54EE
// IDA_NAME: _ZN7PackSet8modifiedEPKcP16IPackFileProcess
// DEMANGLED: PackSet::modified(char const*,IPackFileProcess *)
pack_ret_t PackSet::modified(const char* name, IPackFileProcess* process) {
    if (type_ != PACK_LOAD_FULL) {
        return PACK_ERROR;
    }
    index_vector_it it = find(name);
    if (it == indexes_.end()) {
        return PACK_ERROR;
    }
    IndexEntry* entry = &it->second;
    if (entry->content_ != 0) {
        delete[] entry->content_;
        entry->content_ = 0;
    }
    if (entry->update_content_from_file(name, this, process) == PACK_ERROR) {
        return PACK_ERROR;
    }
    return PACK_OK;
}

// IDA_EA: 0x8AC56A2
// IDA_NAME: _ZN7PackSet5addedEPKcP16IPackFileProcess
// DEMANGLED: PackSet::added(char const*,IPackFileProcess *)
pack_ret_t PackSet::added(const char* name, IPackFileProcess* process) {
    index_pair_t entry;
    if (type_ != PACK_LOAD_FULL) {
        return PACK_ERROR;
    }
    if (find(name) != indexes_.end()) {
        return PACK_ERROR;
    }
    if (entry.second.update_content_from_file(name, this, process) ==
        PACK_ERROR) {
        return PACK_ERROR;
    }
    entry.first = _hash(entry.second.name_.c_str());
    addeds_.push_back(entry);
    return PACK_OK;
}

// IDA_EA: 0x8AC58EE
// IDA_NAME: _ZN7PackSet4sortEv
// DEMANGLED: PackSet::sort(void)
pack_ret_t PackSet::sort() {
    if (type_ != PACK_LOAD_FULL) {
        return PACK_ERROR;
    }
    indexes_.reserve(addeds_.size());
    for (index_vector_it it = addeds_.begin(); it != addeds_.end(); ++it) {
        indexes_.push_back(*it);
    }
    std::sort(indexes_.begin(), indexes_.end(), _index_less);
    addeds_.clear();
    _rebuild_folded_index(this);
    return PACK_OK;
}

// IDA_EA: 0x8AC5A52
// IDA_NAME: _ZN7PackSet4openEPKcS1_16pack_load_type_t
// DEMANGLED: PackSet::open(char const*,char const*,pack_load_type_t)
pack_ret_t PackSet::open(const char* name,
                         const char* tag,
                         pack_load_type_t type) {
    if (UsingEncryptedFile()) {
        return openEncryptedFile(name, tag, type);
    }
    if (is_ready()) {
        return PACK_ERROR;
    }

    FILE* fp = fopen(name, "rb");
    if (fp == 0) {
        return PACK_ERROR;
    }

    std::string index_tag;
    if (_fread_str(index_tag, fp) == PACK_ERROR) {
        fclose(fp);
        return PACK_ERROR;
    }
    if (tag != 0 && strcmp(index_tag.c_str(), tag) != 0) {
        fclose(fp);
        return PACK_ERROR;
    }

    unsigned char bytes[4];
    if (fread(bytes, 4, 1, fp) != 1) {
        fclose(fp);
        return PACK_ERROR;
    }
    revision_ = read_u32(bytes);
    if (fread(bytes, 4, 1, fp) != 1) {
        fclose(fp);
        return PACK_ERROR;
    }
    const uint32_t aligned_index_header_size = read_u32(bytes);
    if (fread(bytes, 4, 1, fp) != 1) {
        fclose(fp);
        return PACK_ERROR;
    }
    const uint32_t index_header_crc = read_u32(bytes);
    if (fread(bytes, 4, 1, fp) != 1) {
        fclose(fp);
        return PACK_ERROR;
    }
    const uint32_t index_size = read_u32(bytes);

    unsigned char* index_header_data =
        new (std::nothrow) unsigned char[aligned_index_header_size];
    if (index_header_data == 0) {
        fclose(fp);
        return PACK_ERROR;
    }
    if (aligned_index_header_size != 0 &&
        fread(index_header_data, aligned_index_header_size, 1, fp) != 1) {
        delete[] index_header_data;
        fclose(fp);
        return PACK_ERROR;
    }

    header_size_ = static_cast<uint32_t>(ftell(fp));
    pack_file_ = fp;
    if (!_decrypt_N_crc32(index_size, index_header_crc, index_header_crc,
                          index_header_data, aligned_index_header_size)) {
        delete[] index_header_data;
        pack_file_ = 0;
        fclose(fp);
        return PACK_ERROR;
    }

    indexes_.resize(index_size);
    unsigned char* tmp = index_header_data;
    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        it->first = read_u32(tmp);
        tmp += 4;
        it->second.read_info(tmp);
    }
    _rebuild_folded_index(this);
    delete[] index_header_data;

    if (type != PACK_LOAD_FULL) {
        fseek(fp, 0, SEEK_END);
        const long size_0 = ftell(fp);
        if (size_0 > 0) {
            fseek(fp, 0, SEEK_SET);
            unsigned char* content_0 =
                new (std::nothrow) unsigned char[static_cast<size_t>(size_0)];
            if (content_0 != 0 &&
                fread(content_0, static_cast<size_t>(size_0), 1, fp) == 1) {
                for (index_vector_it it = indexes_.begin();
                     it != indexes_.end(); ++it) {
                    if (it->second.name_.find(".ani") != std::string::npos) {
                        it->second.update_content_from_memory(content_0, this);
                    }
                }
            }
            delete[] content_0;
        }
    } else {
        fseek(fp, 0, SEEK_END);
        const long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        unsigned char* content =
            new (std::nothrow) unsigned char[static_cast<size_t>(size)];
        if (content != 0 &&
            fread(content, static_cast<size_t>(size), 1, fp) == 1) {
            for (index_vector_it it = indexes_.begin(); it != indexes_.end();
                 ++it) {
                it->second.update_content_from_memory(content, this);
            }
        }
        delete[] content;
    }

    type_ = type;
    return PACK_OK;
}

// IDA_EA: 0x8AC60F0
// IDA_NAME: _ZN7PackSet17openEncryptedFileEPKcS1_16pack_load_type_t
// DEMANGLED: PackSet::openEncryptedFile(char const*,char const*,pack_load_type_t)
pack_ret_t PackSet::openEncryptedFile(const char* name,
                                      const char* tag,
                                      pack_load_type_t type) {
    (void)name;
    (void)tag;
    (void)type;
    return PACK_ERROR;
}

// IDA_EA: 0x8AC60FA
// IDA_NAME: _ZN7PackSet5writeEPKcS1_j
// DEMANGLED: PackSet::write(char const*,char const*,unsigned int)
pack_ret_t PackSet::write(const char* name, const char* tag, uint32_t rev) {
    FILE* fp = 0;
    index_vector2_t write_indexes;
    unsigned char* index_header_data = 0;
    const char* temp_header_filename = "header.tmp";

    if (type_ != PACK_LOAD_FULL) {
        return PACK_ERROR;
    }
    if (is_dirty() && sort() == PACK_ERROR) {
        return PACK_ERROR;
    }

    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        char dir[260];
        char file[260];
        if (_correct_filename(it->second.name_.c_str(), dir, sizeof(dir), file,
                              sizeof(file)) == PACK_ERROR) {
            return PACK_ERROR;
        }
        write_indexes.push_back(
            index_pair2_t(_write_hash(dir, file), &it->second));
    }
    std::sort(write_indexes.begin(), write_indexes.end(), _index_less2);

    uint32_t accum_offset = 0;
    for (index_vector2_it it = write_indexes.begin();
         it != write_indexes.end();
         ++it) {
        it->second->offset_ = accum_offset;
        accum_offset += it->second->aligned_size_;
    }

    fp = fopen(temp_header_filename, "wb");
    if (fp == 0) {
        return PACK_ERROR;
    }
    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        unsigned char bytes[4];
        write_u32(bytes, it->first);
        fwrite(bytes, 4, 1, fp);
        if (it->second.write_info(fp) == PACK_ERROR) {
            fclose(fp);
            return PACK_ERROR;
        }
    }
    fclose(fp);
    fp = 0;

    fp = fopen(temp_header_filename, "rb");
    if (fp == 0) {
        return PACK_ERROR;
    }
    fseek(fp, 0, SEEK_END);
    const long index_header_size_long = ftell(fp);
    if (index_header_size_long < 0) {
        fclose(fp);
        return PACK_ERROR;
    }
    fseek(fp, 0, SEEK_SET);
    const uint32_t index_header_size =
        static_cast<uint32_t>(index_header_size_long);
    const uint32_t aligned_index_header_size =
        (index_header_size + 3u) & 0xFFFFFFFCu;
    index_header_data = new unsigned char[aligned_index_header_size];
    memset(index_header_data, 0, aligned_index_header_size);
    if (index_header_size != 0 &&
        fread(index_header_data, index_header_size, 1, fp) != 1) {
        delete[] index_header_data;
        fclose(fp);
        return PACK_ERROR;
    }
    fclose(fp);
    fp = 0;

    const uint32_t index_size = static_cast<uint32_t>(indexes_.size());
    uint32_t index_header_crc = Crc32::crc32(
        index_size, (const char*)index_header_data,
        aligned_index_header_size);
    _encrypt(index_header_crc, index_header_data,
             aligned_index_header_size);

    fp = fopen(name, "wb");
    if (fp == 0) {
        delete[] index_header_data;
        return PACK_ERROR;
    }
    _fwrite_str(tag, fp);
    unsigned char bytes[4];
    write_u32(bytes, rev);
    fwrite(bytes, 4, 1, fp);
    write_u32(bytes, aligned_index_header_size);
    fwrite(bytes, 4, 1, fp);
    write_u32(bytes, index_header_crc);
    fwrite(bytes, 4, 1, fp);
    write_u32(bytes, index_size);
    fwrite(bytes, 4, 1, fp);
    if (aligned_index_header_size != 0) {
        fwrite(index_header_data, aligned_index_header_size, 1, fp);
    }
    delete[] index_header_data;
    index_header_data = 0;

    for (index_vector2_it it = write_indexes.begin();
         it != write_indexes.end();
         ++it) {
        if (it->second->content_ == 0) {
            fclose(fp);
            return PACK_ERROR;
        }
        fwrite(it->second->content_, it->second->aligned_size_, 1, fp);
    }
    fclose(fp);
    return PACK_OK;
}

// IDA_EA: 0x8AC6988
// IDA_NAME: _ZNK7PackSet16output_file_listEPKc
// DEMANGLED: PackSet::output_file_list(char const*)const
pack_ret_t PackSet::output_file_list(const char* output_path) const {
    if (!is_ready()) {
        return PACK_ERROR;
    }
    FILE* file = fopen(output_path, "wt");
    if (file == 0) {
        return PACK_ERROR;
    }
    fwrite("hash,type,name,size,aligned size,crc,offset\n", 1, 0x2Cu,
           file);
    for (index_vector_const_it it = indexes_.begin(); it != indexes_.end();
         ++it) {
        fprintf(file, "\"%d\"", static_cast<int>(it->first));
        fputc(',', file);
        fprintf(file, "\"%d\"", static_cast<int>(it->second.type_));
        fputc(',', file);
        fprintf(file, "\"%s\"", it->second.name_.c_str());
        fputc(',', file);
        fprintf(file, "\"%d\"", static_cast<int>(it->second.size_));
        fputc(',', file);
        fprintf(file, "\"%d\"", static_cast<int>(it->second.aligned_size_));
        fputc(',', file);
        fprintf(file, "\"%d\"", static_cast<int>(it->second.crc_));
        fputc(',', file);
        fprintf(file, "\"%d\"", static_cast<int>(it->second.offset_));
        fputc('\n', file);
    }
    fclose(file);
    return PACK_OK;
}

// IDA_EA: 0x8AC6C8A
// IDA_NAME: _ZN7PackSet6verifyEv
// DEMANGLED: PackSet::verify(void)
pack_ret_t PackSet::verify() {
    if (UsingEncryptedFile()) {
        return verifyEncryptedFile();
    }
    uint32_t buffer_size = 5242880u;
    unsigned char* buffer_file = 0;
    unsigned char* buffer_pack = 0;
    int size_pack = 0;

    if (!is_ready()) {
        return PACK_ERROR;
    }

    buffer_file = new unsigned char[buffer_size];
    buffer_pack = new unsigned char[buffer_size];

    size_t proc_count = 0;
    size_t total_count = indexes_.size();
    size_t proc_size = 0;
    size_t total_size = 0;
    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        total_size += it->second.size_;
    }

    clock_t start_time = clock();
    clock_t update_time = start_time;
    for (index_vector_it it = indexes_.begin(); it != indexes_.end(); ++it) {
        if (it->second.aligned_size_ > buffer_size) {
            delete[] buffer_file;
            delete[] buffer_pack;
            buffer_size = it->second.aligned_size_;
            buffer_file = new unsigned char[buffer_size];
            buffer_pack = new unsigned char[buffer_size];
        }
        *buffer_file = 0;
        *buffer_pack = 0;

        uint32_t content_size = 0;
        if (it->second.get_content(this, it->first, buffer_pack, buffer_size,
                                   &content_size) == PACK_ERROR) {
            delete[] buffer_file;
            delete[] buffer_pack;
            return PACK_ERROR;
        }
        size_pack = static_cast<int>(content_size);

        FILE* fp = fopen(it->second.name_.c_str(), "rb");
        if (fp == 0) {
            delete[] buffer_file;
            delete[] buffer_pack;
            return PACK_ERROR;
        }
        fseek(fp, 0, SEEK_END);
        const long size_file_long = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (size_file_long < 0 ||
            static_cast<uint32_t>(size_file_long) > buffer_size) {
            fclose(fp);
            delete[] buffer_file;
            delete[] buffer_pack;
            return PACK_ERROR;
        }
        const int size_file = static_cast<int>(size_file_long);
        if (size_file != 0 && fread(buffer_file, size_file, 1, fp) != 1) {
            fclose(fp);
            delete[] buffer_file;
            delete[] buffer_pack;
            return PACK_ERROR;
        }
        fclose(fp);

        if (size_file != size_pack ||
            memcmp(buffer_file, buffer_pack, static_cast<size_t>(size_pack)) !=
                0) {
            delete[] buffer_file;
            delete[] buffer_pack;
            return PACK_ERROR;
        }

        proc_size += it->second.size_;
        ++proc_count;
        const clock_t now = clock();
        if ((now - update_time) * 1000 / CLOCKS_PER_SEC > 1000) {
            update_time = now;
            const int run_time =
                static_cast<int>((now - start_time) * 1000 / CLOCKS_PER_SEC);
            const long double avg_time =
                static_cast<long double>(run_time) /
                static_cast<long double>(proc_size);
            const int remain_time = static_cast<int>(
                static_cast<long double>(total_size - proc_size) * avg_time);
            printf("%3d%% (%2dmin %2dsec)/(%2dmin %2dsec) (%d/%d)    \n",
                   static_cast<int>(
                       static_cast<long double>(proc_size) /
                       static_cast<long double>(total_size) * 100.0),
                   run_time / 60000,
                   run_time % 60000 / 1000,
                   remain_time / 60000,
                   remain_time % 60000 / 1000,
                   static_cast<int>(proc_count),
                   static_cast<int>(total_count));
        }
    }

    const clock_t now = clock();
    const int run_time =
        static_cast<int>((now - start_time) * 1000 / CLOCKS_PER_SEC);
    const long double avg_time = static_cast<long double>(run_time) /
                                 static_cast<long double>(proc_size);
    const int remain_time = static_cast<int>(
        static_cast<long double>(total_size - proc_size) * avg_time);
    printf("%3d%% (%2dmin %2dsec)/(%2dmin %2dsec) (%d/%d)    \n",
           static_cast<int>(static_cast<long double>(proc_size) /
                            static_cast<long double>(total_size) * 100.0),
           run_time / 60000,
           run_time % 60000 / 1000,
           remain_time / 60000,
           remain_time % 60000 / 1000,
           static_cast<int>(proc_count),
           static_cast<int>(total_count));

    delete[] buffer_file;
    delete[] buffer_pack;
    return PACK_OK;
}

// IDA_EA: 0x8AC75FE
// IDA_NAME: _ZN7PackSet19verifyEncryptedFileEv
// DEMANGLED: PackSet::verifyEncryptedFile(void)
pack_ret_t PackSet::verifyEncryptedFile() {
    return PACK_ERROR;
}

// IDA_EA: 0x8AC7608
// IDA_NAME: _ZN7PackSet9pack_fileEv
// DEMANGLED: PackSet::pack_file(void)
FILE* PackSet::pack_file() {
    if (m_bUseEncryptedFile) {
        fprintf(stdout, "%s\n", "_C_DNF_SCRIPT_CRYPTER_ is turned on!!");
        fprintf(stdout, "%s(%d) - %s\n", "../../Include/FilePack/Pack.cpp",
                1655, "pack_file");
    }
    return pack_file_;
}

// IDA_EA: 0x8AC7674
// IDA_NAME: _ZN7PackSet18UsingEncryptedFileEv
// DEMANGLED: PackSet::UsingEncryptedFile(void)
bool PackSet::UsingEncryptedFile() { return m_bUseEncryptedFile; }

// IDA_EA: 0x8AC7680
// IDA_NAME: _ZN7PackSet16UseEncryptedFileEv
// DEMANGLED: PackSet::UseEncryptedFile(void)
void PackSet::UseEncryptedFile() {
    m_bUseEncryptedFile = true;
}
