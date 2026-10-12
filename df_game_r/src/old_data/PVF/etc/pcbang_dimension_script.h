#ifndef OLDSRV_OLD_DATA_PVF_ETC_PCBANG_DIMENSION_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_PCBANG_DIMENSION_SCRIPT_H_

#include "common/manager/cdata_manager_tail_public_abi.h"

#include <string>
#include <vector>


// IDA_EA: 0x88945E2
// IDA_NAME: _Z11tokenizeStrRKSsRSt6vectorISsSaISsEES0_
// DEMANGLED: tokenizeStr(std::string const&,std::vector<std::string>&,std::string const&)
int tokenizeStr(const std::string& text,
                std::vector<std::string>& tokens,
                const std::string& separator);

// IDA_EA: 0x8915606
// IDA_NAME: _Z27importPcBangEquipmentRendarPKcP20PcBangItemRentarData
// DEMANGLED: importPcBangEquipmentRendar(char const*,PcBangItemRentarData*)
int importPcBangEquipmentRendar(const char* filename,
                                PcBangItemRentarData* data);

// IDA_EA: 0x89160F0
// IDA_NAME: _Z29importDimensionActivationDataPKcP23DimensionActivationData
// DEMANGLED: importDimensionActivationData(char const*,DimensionActivationData*)
int importDimensionActivationData(const char* filename,
                                  DimensionActivationData* data);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_PCBANG_DIMENSION_SCRIPT_H_
