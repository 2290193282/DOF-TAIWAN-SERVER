#ifndef OLDSRV_DATAMANAGER_MODULE_TAG_TABLE_H_
#define OLDSRV_DATAMANAGER_MODULE_TAG_TABLE_H_

#include <string>

// IDA data: 0x949B2C0, initialized by
// __static_initialization_and_destruction_0 @0x88C3055. The binary builds
// sixteen string pairs while processModuleTagSet scans index 16 as well; the
// final empty pair reproduces that non-matching BSS tail without dereferencing
// an invalid std::string object.
extern const char* const g_module_type_start_tags[17];
extern const char* const g_module_type_end_tags[17];

// IDA_EA: 0x8A97637
// IDA_NAME: _Z19processModuleTagSetRiS_PSsib
// DEMANGLED: processModuleTagSet(int &,int &,std::string *,int,bool)
int processModuleTagSet(int& tag_index,
                        int& parsed_module_type,
                        std::string* tag,
                        int module_type,
                        bool is_equipment);

#endif  // OLDSRV_DATAMANAGER_MODULE_TAG_TABLE_H_
