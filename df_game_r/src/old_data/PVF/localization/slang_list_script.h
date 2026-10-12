#ifndef OLDSRV_OLD_DATA_PVF_LOCALIZATION_SLANG_LIST_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_LOCALIZATION_SLANG_LIST_SCRIPT_H_

#include <string>
#include <vector>


// IDA_EA: 0x88FFAA9
// IDA_NAME: _Z15importSlangListPcPSt6vectorISsSaISsEE
// DEMANGLED: importSlangList(char *,std::vector<std::string,std::allocator<std::string>> *)
int importSlangList(char* filename, std::vector<std::string>* slang_list)
    __asm__("_Z15importSlangListPcPSt6vectorISsSaISsEE");

#endif  // OLDSRV_OLD_DATA_PVF_LOCALIZATION_SLANG_LIST_SCRIPT_H_
