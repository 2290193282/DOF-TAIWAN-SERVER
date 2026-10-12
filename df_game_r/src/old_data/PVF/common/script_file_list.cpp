#include "old_data/PVF/common/script_file_list.h"

// IDA_EA: 0x88BACF8
// IDA_NAME: _ZN16STScriptFileListC2Ev
// DEMANGLED: STScriptFileList::STScriptFileList(void)
STScriptFileList::STScriptFileList() { Clear(); }

// IDA_EA: 0x80F1352
// IDA_NAME: _ZN16STScriptFileListD2Ev
// DEMANGLED: STScriptFileList::~STScriptFileList()
STScriptFileList::~STScriptFileList() {}

// IDA_EA: 0x88BAD3A
// IDA_NAME: _ZN16STScriptFileList5ClearEv
// DEMANGLED: STScriptFileList::Clear(void)
int STScriptFileList::Clear() {
    clear();
    return 0;
}

// IDA_EA: 0x88BAD4E
// IDA_NAME: _ZNK16STScriptFileList11GetFileNameEi
// DEMANGLED: STScriptFileList::GetFileName(int)const
const char* STScriptFileList::GetFileName(int index) const {
    const_iterator it = find(index);
    if (it == end()) {
        return 0;
    }
    return it->second.c_str();
}

// IDA_EA: 0x88BADBA
// IDA_NAME: _ZN16STScriptFileList8GetIndexEPKc
// DEMANGLED: STScriptFileList::GetIndex(char const*)
int STScriptFileList::GetIndex(const char* file_name) {
    for (iterator it = begin(); it != end(); ++it) {
        if (it->second == file_name) {
            return it->first;
        }
    }
    return -1;
}

// IDA_EA: 0x88BAE56
// IDA_NAME: _ZNK16STScriptFileList19FindIndexByFullScanEPKc
// DEMANGLED: STScriptFileList::FindIndexByFullScan(char const*)const
int STScriptFileList::FindIndexByFullScan(const char* file_name) const {
    for (const_iterator it = begin(); it != end(); ++it) {
        if (it->second == file_name) {
            return it->first;
        }
    }
    return -1;
}

// IDA_EA: 0x88BAEF2
// IDA_NAME: _ZN10STNameListC2Ev
// DEMANGLED: STNameList::STNameList(void)
STNameList::STNameList() { Clear(); }

// IDA_EA: 0x88BAF34
// IDA_NAME: _ZN10STNameList5ClearEv
// DEMANGLED: STNameList::Clear(void)
int STNameList::Clear() {
    clear();
    return 0;
}

// IDA_EA: 0x88BAF48
// IDA_NAME: _ZNK10STNameList5GetIDEPKc
// DEMANGLED: STNameList::GetID(char const*)const
int STNameList::GetID(const char* name) const {
    const_iterator it = find(name);
    if (it == end()) {
        return -1;
    }
    return it->second;
}
