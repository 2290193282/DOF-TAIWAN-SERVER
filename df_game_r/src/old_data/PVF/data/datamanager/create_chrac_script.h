#ifndef OLDSRV_DATAMANAGER_CREATE_CHRAC_SCRIPT_H_
#define OLDSRV_DATAMANAGER_CREATE_CHRAC_SCRIPT_H_

#include <map>
#include <string>
#include <vector>


// IDA_EA: 0x83DD574, 0x83ECCBC, 0x83EFC90, 0x8962460, 0x83CF71C, 0x84E91FA, 0x8962B4C, 0x8373042, 0x837C3D4, 0x895EDA1.
// IDA unit: 12_pvf_datamanager_scripts/createChracScript.
class createChracScript {
public:
    struct rewardInfo {
        rewardInfo();
        rewardInfo(const rewardInfo& other);
        ~rewardInfo();
        rewardInfo& operator=(const rewardInfo& other);

        std::vector<int> item_indexes;
        long start_time;
        long end_time;
        std::string create_title;
        std::string create_text;
    };

    struct rewardInfoList {
        rewardInfoList();
        rewardInfoList(const rewardInfoList& other);
        ~rewardInfoList();

        std::vector<rewardInfo> rewards;
    };

    typedef std::map<int, rewardInfoList> rewardMap;

    createChracScript();
    ~createChracScript();

    rewardMap rewards_by_job;

private:
    createChracScript(const createChracScript&);
    createChracScript& operator=(const createChracScript&);
};

int importCharacterCreateGiftScript(createChracScript* script,
                                    const char* filename);

static_assert(sizeof(createChracScript::rewardInfo) == 0x1Cu,
              "createChracScript::rewardInfo IDA ABI size must stay 0x1c");
static_assert(sizeof(createChracScript::rewardInfoList) == 0x0Cu,
              "createChracScript::rewardInfoList IDA ABI size must stay 0x0c");
static_assert(sizeof(createChracScript) == 0x18u,
              "createChracScript IDA ABI size must stay 0x18");

#endif  // OLDSRV_DATAMANAGER_CREATE_CHRAC_SCRIPT_H_
