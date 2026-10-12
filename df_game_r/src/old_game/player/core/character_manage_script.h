#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_MANAGE_SCRIPT_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_MANAGE_SCRIPT_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x88E0674 in the companion source.


#include <map>
#include <string>
#include <vector>


// IDA unit: 04_character_select_create/CharacManageScript.
struct ManagePointBonus {
    int begin_level;
    int end_level;
    int point;
};

struct ContentsOptionRevision {
    int contents_type;
    int option_value;
};

class CharacManageScript {
public:
    static CharacManageScript* getInsance();

    CharacManageScript();
    ~CharacManageScript();

    int clear();
    int importScript(const char* filename);
    int GetPoint(int level);
    int GetLevel(int point);
    long double GetExpBonus(int level);
    long double GetGoldBonus(int level);
    char GetExpBonus(int level, int* value);
    char GetGoldBonus(int level, int* value);
    int GetLevelUpRequirePoint(unsigned int level);
    std::string GetName(unsigned int level);
    int GetOptionValue(int key);
    int GetMinimumPointCharacterLevel();

private:
    friend class CUser;

    std::vector<ManagePointBonus> point_bonus_;
    std::vector<int> manage_level_point_;
    std::map<int, int> exp_bonus_;
    std::map<int, int> gold_bonus_;
    int manage_level_max_;
    std::map<int, std::string> manage_basic_explain_;
    std::map<int, int> manage_option_;
    std::vector<ContentsOptionRevision> contents_option_revision_;

    static CharacManageScript* instance_;
};


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_MANAGE_SCRIPT_H_
