#include "old_data/PVF/localization/invalid_name_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>


// IDA_EA: 0x8908338
// IDA_NAME: _Z17importInvalidNamePcP15InvalidCharData
// DEMANGLED: importInvalidName(char *,InvalidCharData *)
int importInvalidName(char* filename, InvalidCharData* invalid_char_data) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[one byte range]") {
            while (1) {
                bool begin_success = false;
                const int begin = ScanInt(&begin_success);
                if (!begin_success) {
                    break;
                }

                bool end_success = false;
                const int end = ScanInt(&end_success);
                if (!begin_success || !end_success) {
                    break;
                }

                InvalidCharData::stRangeElement<unsigned char> range(
                    static_cast<unsigned char>(begin),
                    static_cast<unsigned char>(end));
                invalid_char_data->one_byte_ranges.push_back(range);
            }
        }

        if (tag == "[one byte]") {
            while (1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }

                const unsigned char invalid_char =
                    static_cast<unsigned char>(value);
                invalid_char_data->one_byte_chars.push_back(invalid_char);
            }
        }

        if (tag == "[two byte range]") {
            while (1) {
                bool begin_success = false;
                const int begin = ScanInt(&begin_success);
                if (!begin_success) {
                    break;
                }

                bool end_success = false;
                const int end = ScanInt(&end_success);
                if (!begin_success || !end_success) {
                    break;
                }

                InvalidCharData::stRangeElement<unsigned short> range(
                    static_cast<unsigned short>(begin),
                    static_cast<unsigned short>(end));
                invalid_char_data->two_byte_ranges.push_back(range);
            }
        }

        if (tag == "[two byte]") {
            while (1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }

                const unsigned short invalid_char =
                    static_cast<unsigned short>(value);
                invalid_char_data->two_byte_chars.push_back(invalid_char);
            }
        }
    }

    return 1;
}
