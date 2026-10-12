#include "old_data/PVF/data/datamanager/script_tag.h"

namespace script_tag {

// IDA_EA: 0x8A0C8B2
// IDA_NAME: _ZN10script_tag13isCloseStringERSs
// DEMANGLED: script_tag::isCloseString(std::string &)
int isCloseString(std::string& value) {
    if (value.size() > 2) {
        return value.at(1) == '/';
    }
    return 0;
}

// IDA_EA: 0x8A0C8FA
// IDA_NAME: _ZN10script_tag12isTypeStringERSs
// DEMANGLED: script_tag::isTypeString(std::string &)
int isTypeString(std::string& value) {
    if (value.size() > 1) {
        return value.at(0) == '[';
    }
    return 0;
}

}  // namespace script_tag
