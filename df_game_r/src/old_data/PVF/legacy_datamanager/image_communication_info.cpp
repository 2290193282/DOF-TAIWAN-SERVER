#include "old_data/PVF/data/datamanager/image_communication_info.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>


typedef char ImageCommunicationData_Size_Must_Be_0x04[
    (sizeof(ImageCommunicationData) == 0x04) ? 1 : -1];

// IDA_EA: 0x83712FE
// IDA_NAME: _ZN22ImageCommunicationDataC2Ev
// DEMANGLED: ImageCommunicationData::ImageCommunicationData(void)
ImageCommunicationData::ImageCommunicationData() : charge(0) {}

// IDA_EA: 0x8913DBC
// IDA_NAME: _Z28importImageCommunicationInfoPKcP22ImageCommunicationData
// DEMANGLED: importImageCommunicationInfo(char const*,ImageCommunicationData *)
int importImageCommunicationInfo(const char* filename,
                                 ImageCommunicationData* data) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importImageCommunicationInfo",
                              0x3233,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    while (1) {
        bool is_type = false;
        bool success = false;
        std::string text;
        ScanTypeOrInt(is_type, text, success, true);
        if (!success) {
            return 0;
        }

        if (is_type && text == "[charge]") {
            data->charge = ScanInt(&success);
            return success ? 1 : 0;
        }

        if (is_type && text == "[/image communication]") {
            return 1;
        }
    }
}
