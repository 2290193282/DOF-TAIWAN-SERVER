#ifndef OLDSRV_DATAMANAGER_IMAGE_COMMUNICATION_INFO_H_
#define OLDSRV_DATAMANAGER_IMAGE_COMMUNICATION_INFO_H_

#include <stdint.h>


// IDA_EA: 0x83712FE, 0x8913DBC.
// IDA unit: 99_misc_global/ImageCommunicationData and the B-owned
// DataManager/PVF importer call from CDataManager::init.
struct ImageCommunicationData {
    ImageCommunicationData();

    int32_t charge;
};

int importImageCommunicationInfo(const char* filename,
                                 ImageCommunicationData* data);

#endif  // OLDSRV_DATAMANAGER_IMAGE_COMMUNICATION_INFO_H_
