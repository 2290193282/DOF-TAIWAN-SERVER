#ifndef OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_FILE_LIST_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_FILE_LIST_H_

#include <map>
#include <string>

// IDA unit: 12_pvf_datamanager_scripts/STScriptFileList.
// IDA_EA: 0x80F1352, 0x88BACF8, 0x88BAD3A, 0x88BAD4E, 0x88BADBA, 0x88BAE56.
class STScriptFileList : public std::map<int, std::string> {
public:
    STScriptFileList();
    ~STScriptFileList();

    int Clear();
    const char* GetFileName(int index) const;
    int GetIndex(const char* file_name);
    int FindIndexByFullScan(const char* file_name) const;
};

// IDA unit: 12_pvf_datamanager_scripts/STNameList.
// IDA_EA: 0x88BAEF2, 0x88BAF34, 0x88BAF48.
class STNameList : public std::map<std::string, int> {
public:
    STNameList();

    int Clear();
    int GetID(const char* name) const;
};

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_FILE_LIST_H_
