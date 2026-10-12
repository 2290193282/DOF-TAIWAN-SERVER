#include "infra/network/packet/stream.h"

#include "old_data/Login/inter_returns/auto_punish_inter_receivers.h"
#include "old_data/db/mail/letter_stat_payload.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/economy/mall/auction_runtime.h"

#include <cstring>
#include <new>


namespace {

void write_bytes(Stream* stream, const void* src, int bytes) {
    while (!stream->enable_in(bytes)) {
        stream->resize(stream->size() * 2);
    }
    std::memcpy(stream->get_buffer() + stream->length(), src, bytes);
    stream->in_ptr(bytes);
}

template <typename T>
int read_plain(Stream* stream, T* dest) {
    if (!stream->enable_out(static_cast<int>(sizeof(T)))) {
        return 0;
    }
    std::memcpy(dest, stream->get_buffer(), sizeof(T));
    stream->out_ptr(static_cast<int>(sizeof(T)));
    return 1;
}

}  // namespace

// IDA_EA: 0x848FC78
// IDA_NAME: _ZN15stMonsterInfo_tC2Ev
// DEMANGLED: stMonsterInfo_t::stMonsterInfo_t(void)
stMonsterInfo_t::stMonsterInfo_t() : value(0), flag(0) {}

// IDA_EA: 0x848FC8E
// IDA_NAME: _ZN26SIG_LOAD_KILL_MONSTER_INFOC2Ev
// DEMANGLED: SIG_LOAD_KILL_MONSTER_INFO::SIG_LOAD_KILL_MONSTER_INFO(void)
SIG_LOAD_KILL_MONSTER_INFO::SIG_LOAD_KILL_MONSTER_INFO() {
    std::memset(boss_info, 0, sizeof(boss_info));
    std::memset(named_info, 0, sizeof(named_info));
    std::memset(apc_boss_info, 0, sizeof(apc_boss_info));
    charac_no = 0;
}

// IDA_EA: 0x848FA0E
// IDA_NAME: _ZN20CONTINUOUS_ITEM_INFOC2Ev
// DEMANGLED: CONTINUOUS_ITEM_INFO::CONTINUOUS_ITEM_INFO(void)
CONTINUOUS_ITEM_INFO::CONTINUOUS_ITEM_INFO() {
    count = 0;
    for (int index = 0; index <= 49; ++index) {
        entries[index].item_id = 0;
        entries[index].remain_time = 0;
    }
}

// IDA_EA: 0x8491ECA
// IDA_NAME: _ZN38SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFOC2Ev
// DEMANGLED: SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO::SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO(void)
SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO::
    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO()
    : item_info() {}

// IDA_EA: 0x861BC08
// IDA_NAME: _ZN6StreamC2Ev
// DEMANGLED: Stream::Stream(void)
Stream::Stream()
    : size_(0),
      buffer_(0),
      in_ptr_(0),
      out_ptr_(0),
      big_stream_(false),
      align_after_big_(),
      acquire_file_(0),
      acquire_line_(0),
      pool_kind_(0),
      pool_live_(false),
      align_after_live_(),
      pool_dummy_(36641),
      pool_ref_count_(0) {
    resize(0x4000);
}

// IDA_EA: 0x861BC72
// IDA_NAME: _ZN6StreamC2Ei
// DEMANGLED: Stream::Stream(int)
Stream::Stream(int size)
    : size_(0),
      buffer_(0),
      in_ptr_(0),
      out_ptr_(0),
      big_stream_(false),
      align_after_big_(),
      acquire_file_(0),
      acquire_line_(0),
      pool_kind_(0),
      pool_live_(false),
      align_after_live_(),
      pool_dummy_(36641),
      pool_ref_count_(0) {
    resize(size);
}

// IDA_EA: 0x861BCDA
// IDA_NAME: _ZN6StreamC2EPc
// DEMANGLED: Stream::Stream(char *)
Stream::Stream(char* src) : Stream() {
    *this << src;
}

// IDA_EA: 0x861BD14
// IDA_NAME: _ZN6StreamC2ERS_
// DEMANGLED: Stream::Stream(Stream&)
Stream::Stream(Stream& rhs) : Stream() {
    const int unread = rhs.length();
    put_binary(rhs.get_buffer(), unread);
}

// IDA_EA: 0x861BD6C
// IDA_NAME: _ZN6StreamD2Ev
// DEMANGLED: Stream::~Stream()
Stream::~Stream() {
    delete[] buffer_;
    buffer_ = 0;
}

// IDA_EA: 0x861BDAA
// IDA_NAME: _ZN6StreamD0Ev
// DEMANGLED: Stream::~Stream()
// The deleting destructor is compiler-emitted from the virtual destructor.

// IDA_EA: 0x861BDC8
// IDA_NAME: _ZN6Stream4sizeEv
// DEMANGLED: Stream::size(void)
int Stream::size() const {
    return size_;
}

// IDA_EA: 0x861BDD4
// IDA_NAME: _ZN6Stream6lengthEv
// DEMANGLED: Stream::length(void)
int Stream::length() const {
    return in_ptr_ - out_ptr_;
}

// IDA_EA: 0x861BE00
// IDA_NAME: _ZN6Stream6in_ptrEv
// DEMANGLED: Stream::in_ptr(void)
int Stream::in_ptr() const {
    return in_ptr_;
}

// IDA_EA: 0x861BE0C
// IDA_NAME: _ZN6Stream6in_ptrEi
// DEMANGLED: Stream::in_ptr(int)
int Stream::in_ptr(int bytes) {
    in_ptr_ += bytes;
    return in_ptr_;
}

// IDA_EA: 0x861BE28
// IDA_NAME: _ZN6Stream7out_ptrEv
// DEMANGLED: Stream::out_ptr(void)
int Stream::out_ptr() const {
    return out_ptr_;
}

// IDA_EA: 0x861BE34
// IDA_NAME: _ZN6Stream7out_ptrEi
// DEMANGLED: Stream::out_ptr(int)
int Stream::out_ptr(int bytes) {
    if (out_ptr_ + bytes > in_ptr_) {
        return -1;
    }
    out_ptr_ += bytes;
    if (out_ptr_ == in_ptr_) {
        reset();
    }
    return out_ptr_;
}

// IDA_EA: 0x861BE8A
// IDA_NAME: _ZN6Stream5resetEv
// DEMANGLED: Stream::reset(void)
Stream* Stream::reset() {
    in_ptr_ = 0;
    out_ptr_ = 0;
    set_big_stream(false);
    pool_kind_ = 0;
    return this;
}

// IDA_EA: 0x861BEC4
// IDA_NAME: _ZN6Stream14set_big_streamEb
// DEMANGLED: Stream::set_big_stream(bool)
Stream* Stream::set_big_stream(bool enabled) {
    big_stream_ = enabled;
    return this;
}

// IDA_EA: 0x861BEDC
// IDA_NAME: _ZN6Stream13is_big_streamEv
// DEMANGLED: Stream::is_big_stream(void)
int Stream::is_big_stream() const {
    return big_stream_ ? 1 : 0;
}

// IDA_EA: 0x861BEE8
// IDA_NAME: _ZN6Stream6resizeEi
// DEMANGLED: Stream::resize(int)
int Stream::resize(int size) {
    char* dest = new (std::nothrow) char[size];
    if (!dest) {
        return 0;
    }
    const int unread = length();
    if (buffer_ && unread > 0) {
        std::memcpy(dest, buffer_ + out_ptr_, unread);
    }
    delete[] buffer_;
    buffer_ = dest;
    size_ = size;
    out_ptr_ = 0;
    in_ptr_ = unread;
    return 1;
}

// IDA_EA: 0x861BF9E
// IDA_NAME: _ZN6Stream9enable_inEi
// DEMANGLED: Stream::enable_in(int)
bool Stream::enable_in(int bytes) const {
    return in_ptr_ + bytes <= size_;
}

// IDA_EA: 0x861BFDC
// IDA_NAME: _ZN6Stream10enable_outEi
// DEMANGLED: Stream::enable_out(int)
bool Stream::enable_out(int bytes) const {
    return out_ptr_ + bytes <= in_ptr_;
}

// IDA_EA: 0x861C01A
// IDA_NAME: _ZN6StreamlsEb
// DEMANGLED: Stream::operator<<(bool)
Stream& Stream::operator<<(bool value) {
    const char out = value ? 1 : 0;
    write_bytes(this, &out, 1);
    return *this;
}

// IDA_EA: 0x861C0B2
// IDA_NAME: _ZN6StreamlsEc
// DEMANGLED: Stream::operator<<(char)
Stream& Stream::operator<<(char value) {
    write_bytes(this, &value, 1);
    return *this;
}

// IDA_EA: 0x861C12E
// IDA_NAME: _ZN6StreamlsEs
// DEMANGLED: Stream::operator<<(short)
Stream& Stream::operator<<(short value) {
    write_bytes(this, &value, 2);
    return *this;
}

// IDA_EA: 0x861C1BC
// IDA_NAME: _ZN6StreamlsEi
// DEMANGLED: Stream::operator<<(int)
Stream& Stream::operator<<(int value) {
    write_bytes(this, &value, 4);
    return *this;
}

// IDA_EA: 0x861C242
// IDA_NAME: _ZN6StreamlsEl
// DEMANGLED: Stream::operator<<(long)
Stream& Stream::operator<<(long value) {
    const int32_t old_value = static_cast<int32_t>(value);
    write_bytes(this, &old_value, 4);
    return *this;
}

// IDA_EA: 0x861C2C8
// IDA_NAME: _ZN6StreamlsEPc
// DEMANGLED: Stream::operator<<(char *)
Stream& Stream::operator<<(char* value) {
    return *this << static_cast<const char*>(value);
}

Stream& Stream::operator<<(const char* value) {
    const char* text = value ? value : "";
    const int bytes = static_cast<int>(std::strlen(text)) + 1;
    while (!enable_in(bytes)) {
        resize(size_ * 2);
    }
    std::memcpy(buffer_ + in_ptr_, text, bytes);
    in_ptr(bytes);
    return *this;
}

// IDA_EA: 0x861C36A
// IDA_NAME: _ZN6StreamlsEh
// DEMANGLED: Stream::operator<<(unsigned char)
Stream& Stream::operator<<(unsigned char value) {
    return *this << static_cast<char>(value);
}

// IDA_EA: 0x861C392
// IDA_NAME: _ZN6StreamlsEt
// DEMANGLED: Stream::operator<<(unsigned short)
Stream& Stream::operator<<(unsigned short value) {
    return *this << static_cast<short>(value);
}

// IDA_EA: 0x861C3B8
// IDA_NAME: _ZN6StreamlsEj
// DEMANGLED: Stream::operator<<(unsigned int)
Stream& Stream::operator<<(unsigned int value) {
    return *this << static_cast<int>(value);
}

// IDA_EA: 0x861C3D6
// IDA_NAME: _ZN6StreamlsEm
// DEMANGLED: Stream::operator<<(unsigned long)
Stream& Stream::operator<<(unsigned long value) {
    return *this << static_cast<long>(value);
}

// IDA_EA: 0x861C3F4
// IDA_NAME: _ZN6Stream10put_binaryEPvi
// DEMANGLED: Stream::put_binary(void *,int)
int Stream::put_binary(void* src, int bytes) {
    while (!enable_in(bytes)) {
        resize(size_ * 2);
    }
    std::memcpy(buffer_ + in_ptr_, src, bytes);
    in_ptr(bytes);
    return 1;
}

void* Stream::GetInBufferUntyped(int bytes) {
    while (!enable_in(bytes)) {
        resize(size_ * 2);
    }
    void* result = buffer_ + in_ptr_;
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x861C47C
// IDA_NAME: _ZN6StreamrsERb
// DEMANGLED: Stream::operator>>(bool &)
int Stream::operator>>(bool& value) {
    char raw = 0;
    if (!(*this >> raw)) {
        return 0;
    }
    value = raw == 1;
    return 1;
}

// IDA_EA: 0x861C4F0
// IDA_NAME: _ZN6StreamrsERc
// DEMANGLED: Stream::operator>>(char &)
int Stream::operator>>(char& value) {
    return read_plain(this, &value);
}

// IDA_EA: 0x861C552
// IDA_NAME: _ZN6StreamrsERs
// DEMANGLED: Stream::operator>>(short &)
int Stream::operator>>(short& value) {
    return read_plain(this, &value);
}

// IDA_EA: 0x861C5C4
// IDA_NAME: _ZN6StreamrsERi
// DEMANGLED: Stream::operator>>(int &)
int Stream::operator>>(int& value) {
    return read_plain(this, &value);
}

// IDA_EA: 0x861C636
// IDA_NAME: _ZN6StreamrsERl
// DEMANGLED: Stream::operator>>(long &)
int Stream::operator>>(long& value) {
    int32_t old_value = 0;
    if (!read_plain(this, &old_value)) {
        return 0;
    }
    value = old_value;
    return 1;
}

// IDA_EA: 0x861C6A8
// IDA_NAME: _ZN6StreamrsEPc
// DEMANGLED: Stream::operator>>(char *)
int Stream::operator>>(char* value) {
    if (!length()) {
        return 0;
    }
    char* src = buffer_ + out_ptr_;
    const int bytes = str_len(src) + 1;
    std::memcpy(value, src, bytes);
    out_ptr(bytes);
    return 1;
}

// IDA_EA: 0x861C732
// IDA_NAME: _ZN6StreamrsERh
// DEMANGLED: Stream::operator>>(unsigned char &)
int Stream::operator>>(unsigned char& value) {
    char raw = 0;
    const int ok = *this >> raw;
    value = static_cast<unsigned char>(raw);
    return ok;
}

// IDA_EA: 0x861C762
// IDA_NAME: _ZN6StreamrsERt
// DEMANGLED: Stream::operator>>(unsigned short &)
int Stream::operator>>(unsigned short& value) {
    short raw = 0;
    const int ok = *this >> raw;
    value = static_cast<unsigned short>(raw);
    return ok;
}

// IDA_EA: 0x861C796
// IDA_NAME: _ZN6StreamrsERj
// DEMANGLED: Stream::operator>>(unsigned int &)
int Stream::operator>>(unsigned int& value) {
    int raw = 0;
    const int ok = *this >> raw;
    value = static_cast<unsigned int>(raw);
    return ok;
}

// IDA_EA: 0x861C7C8
// IDA_NAME: _ZN6StreamrsERm
// DEMANGLED: Stream::operator>>(unsigned long &)
int Stream::operator>>(unsigned long& value) {
    long raw = 0;
    const int ok = *this >> raw;
    value = static_cast<unsigned long>(raw);
    return ok;
}

// IDA_EA: 0x861C7FA
// IDA_NAME: _ZN6Stream10get_binaryEPvi
// DEMANGLED: Stream::get_binary(void *,int)
int Stream::get_binary(void* dest, int bytes) {
    if (!enable_out(bytes)) {
        return 0;
    }
    std::memcpy(dest, buffer_ + out_ptr_, bytes);
    out_ptr(bytes);
    return 1;
}

// IDA_EA: 0x861C86A
// IDA_NAME: _ZN6Stream7str_lenEPc
// DEMANGLED: Stream::str_len(char *)
int Stream::str_len(char* src) const {
    int result = 0;
    for (int index = 0; index <= 9999999; ++index) {
        if (!src[index]) {
            return result;
        }
        ++result;
    }
    return 0;
}

// IDA_EA: 0x861C8B0
// IDA_NAME: _ZN6Stream10get_bufferEv
// DEMANGLED: Stream::get_buffer(void)
char* Stream::get_buffer() {
    return buffer_ + out_ptr_;
}

void Stream::set_acquire_site(const char* file, int line) {
    acquire_file_ = file;
    acquire_line_ = line;
}

void Stream::set_pool_kind(int kind) {
    pool_kind_ = kind;
}

int Stream::pool_kind() const {
    return pool_kind_;
}

void Stream::set_pool_live(bool live) {
    pool_live_ = live;
}

bool Stream::pool_live() const {
    return pool_live_;
}

// IDA_EA: 0x8454ED2
// IDA_NAME: _ZN6Stream11GetInBufferI16SIG_UPDATE_LOGINEEPT_v
// DEMANGLED: SIG_UPDATE_LOGIN * Stream::GetInBuffer<SIG_UPDATE_LOGIN>(void)
template <>
SIG_UPDATE_LOGIN* Stream::GetInBuffer<SIG_UPDATE_LOGIN>() {
    const int bytes = sizeof(SIG_UPDATE_LOGIN);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_LOGIN* const result =
        reinterpret_cast<SIG_UPDATE_LOGIN*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823AFF0
// IDA_NAME: _ZN6Stream11GetInBufferI16SIG_PARTY_STATUSEEPT_v
// DEMANGLED: SIG_PARTY_STATUS * Stream::GetInBuffer<SIG_PARTY_STATUS>(void)
template <>
SIG_PARTY_STATUS* Stream::GetInBuffer<SIG_PARTY_STATUS>() {
    const int bytes = sizeof(SIG_PARTY_STATUS);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_PARTY_STATUS* const result =
        reinterpret_cast<SIG_PARTY_STATUS*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8455B2E
// IDA_NAME: _ZN6Stream11GetInBufferI15SIG_LETTER_STATEEPT_v
// DEMANGLED: SIG_LETTER_STAT * Stream::GetInBuffer<SIG_LETTER_STAT>(void)
template <>
SIG_LETTER_STAT* Stream::GetInBuffer<SIG_LETTER_STAT>() {
    const int bytes = sizeof(SIG_LETTER_STAT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LETTER_STAT* const result =
        reinterpret_cast<SIG_LETTER_STAT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456466
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_AUTO_PUNISH_RULE_BLACK_IPEEPT_v
// DEMANGLED: SIG_AUTO_PUNISH_RULE_BLACK_IP * Stream::GetInBuffer<SIG_AUTO_PUNISH_RULE_BLACK_IP>(void)
template <>
SIG_AUTO_PUNISH_RULE_BLACK_IP*
Stream::GetInBuffer<SIG_AUTO_PUNISH_RULE_BLACK_IP>() {
    const int bytes = sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUTO_PUNISH_RULE_BLACK_IP* const result =
        reinterpret_cast<SIG_AUTO_PUNISH_RULE_BLACK_IP*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84564D0
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_AUTO_PUNISH_RULE_HACK_TYPEEEPT_v
// DEMANGLED: SIG_AUTO_PUNISH_RULE_HACK_TYPE * Stream::GetInBuffer<SIG_AUTO_PUNISH_RULE_HACK_TYPE>(void)
template <>
SIG_AUTO_PUNISH_RULE_HACK_TYPE*
Stream::GetInBuffer<SIG_AUTO_PUNISH_RULE_HACK_TYPE>() {
    const int bytes = sizeof(SIG_AUTO_PUNISH_RULE_HACK_TYPE);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUTO_PUNISH_RULE_HACK_TYPE* const result =
        reinterpret_cast<SIG_AUTO_PUNISH_RULE_HACK_TYPE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x847DE2E
// IDA_NAME: _ZN6Stream11GetInBufferI34SIG_AUCTION_NOTIFY_AUCTION_SERVICEEEPT_v
// DEMANGLED: SIG_AUCTION_NOTIFY_AUCTION_SERVICE * Stream::GetInBuffer<SIG_AUCTION_NOTIFY_AUCTION_SERVICE>(void)
template <>
SIG_AUCTION_NOTIFY_AUCTION_SERVICE*
Stream::GetInBuffer<SIG_AUCTION_NOTIFY_AUCTION_SERVICE>() {
    const int bytes = sizeof(SIG_AUCTION_NOTIFY_AUCTION_SERVICE);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUCTION_NOTIFY_AUCTION_SERVICE* const result =
        reinterpret_cast<SIG_AUCTION_NOTIFY_AUCTION_SERVICE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8457F5A
// IDA_NAME: _ZN6Stream11GetInBufferIN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoEEEPT_v
// DEMANGLED: SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo * Stream::GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>(void)
template <>
SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*
Stream::GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>() {
    const int bytes = sizeof(SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo* const result =
        reinterpret_cast<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80E35C2
// IDA_NAME: _ZN6Stream11GetInBufferI35SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPEEEPT_v
// DEMANGLED: SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE * Stream::GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>(void)
template <>
SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE*
Stream::GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>() {
    const int bytes = sizeof(SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE* const result =
        reinterpret_cast<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80FA8DC
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_SAVE_CLIENT_HACK_LOGEEPT_v
// DEMANGLED: SIG_SAVE_CLIENT_HACK_LOG * Stream::GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>(void)
template <>
SIG_SAVE_CLIENT_HACK_LOG* Stream::GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>() {
    const int bytes = sizeof(SIG_SAVE_CLIENT_HACK_LOG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_CLIENT_HACK_LOG* const result =
        reinterpret_cast<SIG_SAVE_CLIENT_HACK_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8157E38
// IDA_NAME: _ZN6Stream11GetInBufferI21STBossDungeonTowerLogEEPT_v
// DEMANGLED: STBossDungeonTowerLog * Stream::GetInBuffer<STBossDungeonTowerLog>(void)
template <>
STBossDungeonTowerLog* Stream::GetInBuffer<STBossDungeonTowerLog>() {
    const int bytes = sizeof(STBossDungeonTowerLog);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    STBossDungeonTowerLog* const result =
        reinterpret_cast<STBossDungeonTowerLog*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8239AEC
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_FIND_FACTORY_HUBEEPT_v
// DEMANGLED: SIG_FIND_FACTORY_HUB * Stream::GetInBuffer<SIG_FIND_FACTORY_HUB>(void)
template <>
SIG_FIND_FACTORY_HUB* Stream::GetInBuffer<SIG_FIND_FACTORY_HUB>() {
    const int bytes = sizeof(SIG_FIND_FACTORY_HUB);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_FIND_FACTORY_HUB* const result =
        reinterpret_cast<SIG_FIND_FACTORY_HUB*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823A2F2
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_EXTEND_CHARAC_SLOTEEPT_v
// DEMANGLED: SIG_EXTEND_CHARAC_SLOT * Stream::GetInBuffer<SIG_EXTEND_CHARAC_SLOT>(void)
template <>
SIG_EXTEND_CHARAC_SLOT* Stream::GetInBuffer<SIG_EXTEND_CHARAC_SLOT>() {
    const int bytes = sizeof(SIG_EXTEND_CHARAC_SLOT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_EXTEND_CHARAC_SLOT* const result =
        reinterpret_cast<SIG_EXTEND_CHARAC_SLOT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823AF1C
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_FORBID_USER_TO_PLAYEEPT_v
// DEMANGLED: SIG_FORBID_USER_TO_PLAY * Stream::GetInBuffer<SIG_FORBID_USER_TO_PLAY>(void)
template <>
SIG_FORBID_USER_TO_PLAY* Stream::GetInBuffer<SIG_FORBID_USER_TO_PLAY>() {
    const int bytes = sizeof(SIG_FORBID_USER_TO_PLAY);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_FORBID_USER_TO_PLAY* const result =
        reinterpret_cast<SIG_FORBID_USER_TO_PLAY*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823B368
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_AUCTION_MONEY_SUBEEPT_v
// DEMANGLED: SIG_AUCTION_MONEY_SUB * Stream::GetInBuffer<SIG_AUCTION_MONEY_SUB>(void)
template <>
SIG_AUCTION_MONEY_SUB* Stream::GetInBuffer<SIG_AUCTION_MONEY_SUB>() {
    const int bytes = sizeof(SIG_AUCTION_MONEY_SUB);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUCTION_MONEY_SUB* const result =
        reinterpret_cast<SIG_AUCTION_MONEY_SUB*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x82BC896
// IDA_NAME: _ZN6Stream11GetInBufferI36SIG_UPDATE_LINK_CHARAC_CONNECT_STATEEEPT_v
// DEMANGLED: SIG_UPDATE_LINK_CHARAC_CONNECT_STATE * Stream::GetInBuffer<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE>(void)
template <>
SIG_UPDATE_LINK_CHARAC_CONNECT_STATE*
Stream::GetInBuffer<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE>() {
    const int bytes = sizeof(SIG_UPDATE_LINK_CHARAC_CONNECT_STATE);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_LINK_CHARAC_CONNECT_STATE* const result =
        reinterpret_cast<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454B1C
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_LOAD_EVENTITEMEEPT_v
// DEMANGLED: SIG_LOAD_EVENTITEM * Stream::GetInBuffer<SIG_LOAD_EVENTITEM>(void)
template <>
SIG_LOAD_EVENTITEM* Stream::GetInBuffer<SIG_LOAD_EVENTITEM>() {
    const int bytes = sizeof(SIG_LOAD_EVENTITEM);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_EVENTITEM* const result =
        reinterpret_cast<SIG_LOAD_EVENTITEM*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x82A3D38
// IDA_NAME: _ZN11SmallStreamC2Ev
// DEMANGLED: SmallStream::SmallStream(void)
SmallStream::SmallStream() : Stream(0x1000) {}

// IDA_EA: 0x82AD248
// IDA_NAME: _ZN11SmallStreamD2Ev
// DEMANGLED: SmallStream::~SmallStream()
SmallStream::~SmallStream() {}

// IDA_EA: 0x82AD278
// IDA_NAME: _ZN11SmallStreamD0Ev
// DEMANGLED: SmallStream::~SmallStream()
// The deleting destructor is compiler-emitted from the virtual destructor.

// IDA_EA: 0x82A3D5C
// IDA_NAME: _ZN9BigStreamC2Ev
// DEMANGLED: BigStream::BigStream(void)
BigStream::BigStream() : Stream(0x40000) {}

// IDA_EA: 0x82AD97E
// IDA_NAME: _ZN9BigStreamD2Ev
// DEMANGLED: BigStream::~BigStream()
BigStream::~BigStream() {}

// IDA_EA: 0x82AD9AE
// IDA_NAME: _ZN9BigStreamD0Ev
// DEMANGLED: BigStream::~BigStream()
// The deleting destructor is compiler-emitted from the virtual destructor.
