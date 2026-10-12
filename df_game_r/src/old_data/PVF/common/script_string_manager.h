#ifndef OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_STRING_MANAGER_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_STRING_MANAGER_H_

#include <map>
#include <string>
#include <vector>


// IDA unit: 12_pvf_datamanager_scripts/ScriptStringManager.
class ScriptStringManager {
public:
    ScriptStringManager();
    virtual ~ScriptStringManager();

    bool getString(int index, std::string* output_string) const;
    const std::string& getString(int index) const;
    bool exportToFile(const char* file_name) const;
    int exportToBuffer(unsigned char* buffer, int buff_size) const;
    void clear();
    int insertString(const char* string);
    bool importFromBuffer(const unsigned char* buffer, int buff_size);
    bool importFromFile(const char* file_name);

private:
    typedef std::vector<std::string> StringVector;
    typedef std::map<std::string, int> StringIntMap;

    StringVector strings_;
    StringIntMap string_map_;
};

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_STRING_MANAGER_H_
