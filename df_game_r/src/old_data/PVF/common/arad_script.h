#ifndef OLDSRV_DATAMANAGER_ARAD_SCRIPT_H_
#define OLDSRV_DATAMANAGER_ARAD_SCRIPT_H_

#include <stdint.h>
#include <map>
#include <string>


// IDA unit: 12_pvf_datamanager_scripts/Arad_Script.
class Arad_Script {
public:
    Arad_Script();
    explicit Arad_Script(const char* file_name);
    ~Arad_Script();

    static char* make_file_path(const char* file_name);

    int Create();
    int ExpandCol();
    int ExpandRow();
    int Load(const char* buffer, const char* section_name);
    int Load(const char* section_name);

    int GetIdx4Col(char* col_name);
    int GetIdx4Row(char* row_name);

    char* GetString(int row, int col);
    char* GetString(char* row_name, char* col_name);
    char* GetString(char* row_name, int col);
    char* GetString(int row, char* col_name);

    int GetInt(int row, int col);
    int GetInt(char* row_name, char* col_name);
    int GetInt(char* row_name, int col);
    int GetInt(int row, char* col_name);

    float GetFloat(int row, int col);
    float GetFloat(char* row_name, char* col_name);
    float GetFloat(char* row_name, int col);
    float GetFloat(int row, char* col_name);

    double GetDouble(int row, int col);
    double GetDouble(char* row_name, char* col_name);
    double GetDouble(char* row_name, int col);
    double GetDouble(int row, char* col_name);

    char* SetString(int row, int col, char* value);
    char* SetString(char* row_name, char* col_name, char* value);
    char* SetString(char* row_name, int col, char* value);
    char* SetString(int row, char* col_name, char* value);

    int SetInt(int row, int col, int value);
    int SetInt(char* row_name, char* col_name, int value);
    int SetInt(char* row_name, int col, int value);
    int SetInt(int row, char* col_name, int value);

    int GetRowNum();

    int32_t col_capacity_;       // +0x00
    int32_t row_capacity_;       // +0x04
    int32_t col_count_;          // +0x08
    int32_t row_count_;          // +0x0c
    int32_t buffer_size_;        // +0x10
    char* cells_;                // +0x14
    std::string scratch_0_;      // +0x18
    std::string scratch_1_;      // +0x1c
    std::string file_path_;      // +0x20
    std::map<std::string, int> col_index_;  // +0x24
    std::map<std::string, int> row_index_;  // +0x3c
};

#endif  // OLDSRV_DATAMANAGER_ARAD_SCRIPT_H_
