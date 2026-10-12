#ifndef OLDSRV_DATAMANAGER_FIELD_LENGTH_CHECKER_H_
#define OLDSRV_DATAMANAGER_FIELD_LENGTH_CHECKER_H_

// Canonical B-owned DB field-length checker for PVF synchronization.

#include <string>
#include <vector>

// IDA unit: 99_misc_global/CFieldLengthChecker.  The checker is used by the
// PVF-to-MySQL item synchronizers to retain the original observed DB lengths.
class CFieldLengthChecker {
public:
    CFieldLengthChecker();
    ~CFieldLengthChecker();

    int AddField(const char* query);

    static std::vector<std::string> fieldNames_;
    static std::vector<unsigned int> fieldLength_;

private:
    std::vector<std::string> fields_;
};

#endif  // OLDSRV_DATAMANAGER_FIELD_LENGTH_CHECKER_H_
