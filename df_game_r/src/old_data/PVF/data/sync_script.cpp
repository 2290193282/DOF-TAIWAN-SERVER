#include "old_data/PVF/data/sync_script.h"

#include "old_data/PVF/data/field_length_checker.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"

#include <string>

std::vector<std::string> CFieldLengthChecker::fieldNames_;
std::vector<unsigned int> CFieldLengthChecker::fieldLength_;

// IDA_EA: 0x8627C94
// IDA_NAME: _ZN19CFieldLengthCheckerC2Ev
// DEMANGLED: CFieldLengthChecker::CFieldLengthChecker(void)
CFieldLengthChecker::CFieldLengthChecker() : fields_() {}

// IDA_EA: 0x8627CA8
// IDA_NAME: _ZN19CFieldLengthCheckerD2Ev
// DEMANGLED: CFieldLengthChecker::~CFieldLengthChecker()
CFieldLengthChecker::~CFieldLengthChecker() {}

// IDA_EA: 0x86272E8
// IDA_NAME: _ZN19CFieldLengthChecker8AddFieldEPKc
// DEMANGLED: CFieldLengthChecker::AddField(char const*)
int CFieldLengthChecker::AddField(const char* query) {
    if (fieldLength_.empty()) {
        const std::string statement(query);
        const std::string::size_type open = statement.find("(", 0);
        const std::string columns = statement.substr(open + 1);
        const std::string::size_type close = columns.find(")", 0);
        std::string remaining = columns.substr(0, close);
        for (;;) {
            const std::string::size_type comma = remaining.find(",", 0);
            if (comma == std::string::npos) {
                break;
            }
            fieldNames_.push_back(remaining.substr(0, comma));
            fieldLength_.push_back(0);
            remaining = remaining.substr(comma + 1);
        }
        fieldNames_.push_back(remaining);
        fieldLength_.push_back(0);
    }

    if (!fields_.empty()) {
        return 0;
    }

    const std::string statement(query);
    const std::string::size_type values_open = statement.find("values(", 0);
    const std::string values_and_tail = statement.substr(values_open + 7);
    const std::string::size_type values_close = values_and_tail.find(")", 0);
    std::string remaining = values_and_tail.substr(0, values_close);

    for (unsigned int index = 0; index < fieldLength_.size(); ++index) {
        std::string::size_type comma = remaining.find(",", 0);
        if (comma == std::string::npos) {
            break;
        }

        std::string value;
        if (remaining[0] == '\'') {
            comma = 0;
            do {
                comma = remaining.find("'", comma + 1);
                if (comma == std::string::npos) {
                    return 0;
                }
            } while (remaining[comma - 1] == '\\');
            ++comma;
            value = remaining.substr(1, comma - 1);
        } else {
            value = remaining.substr(0, comma);
        }
        trim(value);
        if (fieldLength_[index] < value.length()) {
            fieldLength_[index] = value.length();
        }
        remaining = remaining.substr(comma + 1);
    }
    return 0;
}

namespace sync_script {

// IDA_EA: 0x861CA94
// IDA_NAME: _ZN11sync_script16DisplayMaxLengthEv
// DEMANGLED: sync_script::DisplayMaxLength(void)
bool DisplayMaxLength() {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(5),static_cast<ENUM_SERVER_GROUP>( 0));
    bool result;
    for (unsigned int index = 0;; ++index) {
        result = CFieldLengthChecker::fieldNames_.size() > index;
        if (!result) {
            break;
        }
        if (CFieldLengthChecker::fieldLength_[index] != 0) {
            db_handle->set_query("seLect max(length(%s)) from dnf_item_info",
                                 CFieldLengthChecker::fieldNames_[index].c_str());
            if (db_handle->exec(true) && db_handle->fetch()) {
                unsigned int length = 0;
                if (db_handle->get_uint(0, length)) {
                    CFieldLengthChecker::fieldLength_[index] = length;
                }
            }
        }
    }
    return result;
}

// IDA_EA: 0x861CBB4
// IDA_NAME: _ZN11sync_script21GetInstanceSyncScriptEv
// DEMANGLED: sync_script::GetInstanceSyncScript(void)
CSyncScript* GetInstanceSyncScript() {
    static CSyncScript instance;
    return &instance;
}

}  // namespace sync_script
