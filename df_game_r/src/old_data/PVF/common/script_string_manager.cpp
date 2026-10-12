#include "old_data/PVF/common/script_string_manager.h"

#include <cstdio>
#include <cstring>


namespace {

int read_u32_le(const unsigned char* data) {
    return static_cast<int>(data[0]) |
           (static_cast<int>(data[1]) << 8) |
           (static_cast<int>(data[2]) << 16) |
           (static_cast<int>(data[3]) << 24);
}

void write_u32_le(unsigned char* data, int value) {
    data[0] = static_cast<unsigned char>(value & 0xFF);
    data[1] = static_cast<unsigned char>((value >> 8) & 0xFF);
    data[2] = static_cast<unsigned char>((value >> 16) & 0xFF);
    data[3] = static_cast<unsigned char>((value >> 24) & 0xFF);
}

}  // namespace

// IDA_EA: 0x8ACF110
// IDA_NAME: _ZN19ScriptStringManagerC2Ev
// DEMANGLED: ScriptStringManager::ScriptStringManager(void)
ScriptStringManager::ScriptStringManager() : strings_(), string_map_() {}

// IDA_EA: 0x8ACF5F0
// IDA_NAME: _ZN19ScriptStringManagerD2Ev
// DEMANGLED: ScriptStringManager::~ScriptStringManager()
// IDA_EA: 0x8ACF700
// IDA_NAME: _ZN19ScriptStringManagerD0Ev
// DEMANGLED: ScriptStringManager::~ScriptStringManager() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
ScriptStringManager::~ScriptStringManager() {}

// IDA_EA: 0x8ACF160
// IDA_NAME: _ZNK19ScriptStringManager9getStringEiPSs
// DEMANGLED: ScriptStringManager::getString(int,std::string *)const
bool ScriptStringManager::getString(int index,
                                    std::string* output_string) const {
    if (output_string == 0) {
        return false;
    }
    output_string->clear();
    if (index < 0 ||
        index >= static_cast<int>(strings_.size())) {
        return false;
    }
    output_string->assign(strings_[index]);
    return true;
}

// IDA_EA: 0x8ACF1F0
// IDA_NAME: _ZNK19ScriptStringManager9getStringEi
// DEMANGLED: ScriptStringManager::getString(int)const
const std::string& ScriptStringManager::getString(int index) const {
    static const std::string DEFAULT_STRING("");
    if (index < 0 ||
        index >= static_cast<int>(strings_.size())) {
        return DEFAULT_STRING;
    }
    return strings_[index];
}

// IDA_EA: 0x8ACF2A0
// IDA_NAME: _ZNK19ScriptStringManager12exportToFileEPKc
// DEMANGLED: ScriptStringManager::exportToFile(char const*)const
bool ScriptStringManager::exportToFile(const char* file_name) const {
    FILE* file = std::fopen(file_name, "wb");
    if (file == 0) {
        return false;
    }

    const int count = static_cast<int>(strings_.size());
    unsigned int offset = static_cast<unsigned int>(4 * count + 4);
    if (std::fwrite(&count, 4, 1, file) != 1) {
        std::fclose(file);
        return false;
    }
    for (int i = 0; i < count; ++i) {
        if (std::fwrite(&offset, 4, 1, file) != 1) {
            std::fclose(file);
            return false;
        }
        offset += static_cast<unsigned int>(strings_[i].size());
    }
    if (std::fwrite(&offset, 4, 1, file) != 1) {
        std::fclose(file);
        return false;
    }
    for (int i = 0; i < count; ++i) {
        const std::string& value = strings_[i];
        if (!value.empty() &&
            std::fwrite(value.data(), value.size(), 1, file) != 1) {
            std::fclose(file);
            return false;
        }
    }
    std::fclose(file);
    return true;
}

// IDA_EA: 0x8ACF420
// IDA_NAME: _ZNK19ScriptStringManager14exportToBufferEPhi
// DEMANGLED: ScriptStringManager::exportToBuffer(unsigned char *,int)const
int ScriptStringManager::exportToBuffer(unsigned char* buffer,
                                        int buff_size) const {
    const int count = static_cast<int>(strings_.size());
    if (buffer == 0 || buff_size <= 3) {
        return -1;
    }

    write_u32_le(buffer, count);
    unsigned char* out = buffer + 4;
    int write_size = 8;
    int offset = 4 * count + 4;
    for (int i = 0; i < count; ++i) {
        if (buff_size < write_size) {
            return -1;
        }
        write_u32_le(out, offset);
        out += 4;
        write_size += 4;
        offset += static_cast<int>(strings_[i].size());
    }
    if (buff_size < write_size) {
        return -1;
    }
    write_u32_le(out, offset);
    out += 4;

    int total_written = write_size;
    for (int i = 0; i < count; ++i) {
        const std::string& value = strings_[i];
        const int size = static_cast<int>(value.size());
        total_written += size;
        if (buff_size < total_written) {
            return -1;
        }
        if (size != 0) {
            std::memcpy(out, value.data(), size);
            out += size;
        }
    }
    return static_cast<int>(out - buffer);
}

// IDA_EA: 0x8ACF550
// IDA_NAME: _ZN19ScriptStringManager5clearEv
// DEMANGLED: ScriptStringManager::clear(void)
void ScriptStringManager::clear() {
    strings_.clear();
    string_map_.clear();
}

// IDA_EA: 0x8ACF720
// IDA_NAME: _ZN19ScriptStringManager12insertStringEPKc
// DEMANGLED: ScriptStringManager::insertString(char const*)
int ScriptStringManager::insertString(const char* string) {
    StringIntMap::const_iterator found = string_map_.find(string);
    if (found != string_map_.end()) {
        return found->second;
    }
    strings_.push_back(string);
    const int index = static_cast<int>(strings_.size()) - 1;
    string_map_.insert(std::make_pair(std::string(string), index));
    return index;
}

// IDA_EA: 0x8ACF970
// IDA_NAME: _ZN19ScriptStringManager16importFromBufferEPKhi
// DEMANGLED: ScriptStringManager::importFromBuffer(unsigned char const*,int)
bool ScriptStringManager::importFromBuffer(const unsigned char* buffer,
                                           int buff_size) {
    if (buffer == 0 || buff_size <= 3) {
        return false;
    }

    const int count = read_u32_le(buffer);
    if (count < 0) {
        return false;
    }

    std::vector<int> offsets;
    offsets.reserve(static_cast<size_t>(count) + 1);
    int read_size = 8;
    const unsigned char* ptr = buffer + 4;
    for (int i = 0; i < count; ++i) {
        if (buff_size < read_size) {
            return false;
        }
        offsets.push_back(read_u32_le(ptr));
        ptr += 4;
        read_size += 4;
    }
    if (buff_size < read_size) {
        return false;
    }
    offsets.push_back(read_u32_le(ptr));
    ptr += 4;

    strings_.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        const int length = offsets[i + 1] - offsets[i];
        if (length < 0) {
            return false;
        }
        read_size += length;
        if (buff_size < read_size) {
            return false;
        }
        const int copy_length = length <= 1023 ? length : 1023;
        char temp_buffer[1024];
        if (copy_length > 0) {
            std::memcpy(temp_buffer, ptr, static_cast<size_t>(copy_length));
        }
        temp_buffer[copy_length] = 0;
        strings_.push_back(temp_buffer);
        ptr += length;
    }
    return true;
}

// IDA_EA: 0x8ACFD00
// IDA_NAME: _ZN19ScriptStringManager14importFromFileEPKc
// DEMANGLED: ScriptStringManager::importFromFile(char const*)
bool ScriptStringManager::importFromFile(const char* file_name) {
    FILE* file = std::fopen(file_name, "rb");
    if (file == 0) {
        return false;
    }
    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return false;
    }
    const long file_size = std::ftell(file);
    if (file_size < 0 || std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file);
        return false;
    }
    std::vector<unsigned char> buffer(static_cast<size_t>(file_size));
    if (file_size != 0 &&
        std::fread(&buffer[0], static_cast<size_t>(file_size), 1, file) != 1) {
        std::fclose(file);
        return false;
    }
    std::fclose(file);
    importFromBuffer(file_size == 0 ? 0 : &buffer[0],
                     static_cast<int>(file_size));
    return true;
}
