#ifndef OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_RUNTIME_H_

#include <map>
#include <stdint.h>
#include <string>

#include "old_data/db/inventory/achievement_payload.h"
#include "old_game/player/core/charac_expand_cdata.h"


class CUser;
class Inven_Item;
class Quest;
struct stRewardPostal;
struct stTitleSection;
enum ENUM_TITLE_BOOK_CATEGORY : int;

class CAchievement : public charac_expand::CData {
public:
    // IDA_EA: 0x828B614
    // IDA_NAME: _ZN12CAchievementC2Ev
    // DEMANGLED: CAchievement::CAchievement(void)
    CAchievement();

    // IDA_EA: 0x828F8AA
    // IDA_NAME: _ZN12CAchievementD2Ev
    // DEMANGLED: CAchievement::~CAchievement()
    // IDA_EA: 0x828F90C
    // IDA_NAME: _ZN12CAchievementD0Ev
    // DEMANGLED: CAchievement::~CAchievement()
    virtual ~CAchievement();

    // IDA_EA: 0x828B678
    // IDA_NAME: _ZN12CAchievement10setTriggerEjttt
    // DEMANGLED: CAchievement::setTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
    void setTrigger(unsigned int achievement_id,
                    unsigned short first,
                    unsigned short second,
                    unsigned short third);

    // IDA_EA: 0x828C4DE
    // IDA_NAME: _ZN12CAchievement8sendListEv
    // DEMANGLED: CAchievement::sendList(void)
    void sendList();

    // IDA_EA: 0x828C6A4
    // IDA_NAME: _ZNK12CAchievement7getDataEPc
    // DEMANGLED: CAchievement::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x828C8B8
    // IDA_NAME: _ZN12CAchievement8loadDataEP5CUserPc
    // DEMANGLED: CAchievement::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x828C976
    // IDA_NAME: _ZN12CAchievement6adjustEv
    // DEMANGLED: CAchievement::adjust(void)
    charac_expand::CData* adjust();

    // IDA_EA: 0x828CBE4
    // IDA_NAME: _ZN12CAchievement20isClearedAchievementEj
    // DEMANGLED: CAchievement::isClearedAchievement(unsigned int)
    bool isClearedAchievement(unsigned int achievement_id);

    // IDA_EA: 0x828D338
    // IDA_NAME: _ZN12CAchievement8_gm_initEv
    // DEMANGLED: CAchievement::_gm_init(void)
    charac_expand::CData* _gm_init();

    // IDA_EA: 0x828D450
    // IDA_NAME: _ZN12CAchievement10_gm_reportERSs
    // DEMANGLED: CAchievement::_gm_report(std::string &)
    bool _gm_report(std::string& report);

    // IDA_EA: 0x828D738
    // IDA_NAME: _ZN12CAchievement12_gm_clearAllEv
    // DEMANGLED: CAchievement::_gm_clearAll(void)
    bool _gm_clearAll();

    // IDA_EA: 0x828BCB8
    // IDA_NAME: _ZN12CAchievement14_getSectionIdxE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CAchievement::_getSectionIdx(ENUM_TITLE_BOOK_CATEGORY,int)
    static int _getSectionIdx(ENUM_TITLE_BOOK_CATEGORY category, int slot);

protected:
    // IDA_EA: 0x828C774
    // IDA_NAME: _ZN12CAchievement9_saveDataEP5CUser
    // DEMANGLED: CAchievement::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x828C676
    // IDA_NAME: _ZN12CAchievement6_resetEv
    // DEMANGLED: CAchievement::_reset(void)
    virtual int _reset();

private:
    // IDA_EA: 0x828B800
    // IDA_NAME: _ZN12CAchievement14_verifyTriggerEjttt
    // DEMANGLED: CAchievement::_verifyTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
    bool _verifyTrigger(unsigned int achievement_id,
                        unsigned short first,
                        unsigned short second,
                        unsigned short third);

    // IDA_EA: 0x828B896
    // IDA_NAME: _ZN12CAchievement11_setTriggerEjttt
    // DEMANGLED: CAchievement::_setTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
    void _setTrigger(unsigned int achievement_id,
                     unsigned short first,
                     unsigned short second,
                     unsigned short third);

    // IDA_EA: 0x828BB14
    // IDA_NAME: _ZN12CAchievement11_getTriggerEj
    // DEMANGLED: CAchievement::_getTrigger(unsigned int)
    stAchievement _getTrigger(unsigned int achievement_id);

    // IDA_EA: 0x828BBAE
    // IDA_NAME: _ZN12CAchievement14_checkCompleteEj
    // DEMANGLED: CAchievement::_checkComplete(unsigned int)
    void _checkComplete(unsigned int achievement_id);

    // IDA_EA: 0x828BD5E
    // IDA_NAME: _ZN12CAchievement16_getPostalRewardEP14stTitleSection
    // DEMANGLED: CAchievement::_getPostalReward(stTitleSection *)
    stRewardPostal* _getPostalReward(stTitleSection* section);

    // IDA_EA: 0x828BDC6
    // IDA_NAME: _ZN12CAchievement15_getTabCompleteEP14stTitleSection24ENUM_TITLE_BOOK_CATEGORY
    // DEMANGLED: CAchievement::_getTabComplete(stTitleSection *,ENUM_TITLE_BOOK_CATEGORY)
    bool _getTabComplete(stTitleSection* section,
                         ENUM_TITLE_BOOK_CATEGORY category);

    // IDA_EA: 0x828BF2A
    // IDA_NAME: _ZN12CAchievement19_sendCompleteRewardEPK5Quest24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CAchievement::_sendCompleteReward(Quest const*,ENUM_TITLE_BOOK_CATEGORY,int)
    void _sendCompleteReward(const Quest* quest,
                             ENUM_TITLE_BOOK_CATEGORY category,
                             int slot);

    // IDA_EA: 0x828C086
    // IDA_NAME: _ZN12CAchievement22_sendEachAchieveRewardEPK5Quest
    // DEMANGLED: CAchievement::_sendEachAchieveReward(Quest const*)
    void _sendEachAchieveReward(const Quest* quest);

    // IDA_EA: 0x828C152
    // IDA_NAME: _ZN12CAchievement11_sendPostalEiiSsSs
    // DEMANGLED: CAchievement::_sendPostal(int,int,std::string,std::string)
    int _sendPostal(int item_id,
                    int add_info,
                    std::string title,
                    std::string text);

    // IDA_EA: 0x828C252
    // IDA_NAME: _ZN12CAchievement15_sendSetTriggerEjh
    // DEMANGLED: CAchievement::_sendSetTrigger(unsigned int,unsigned char)
    void _sendSetTrigger(unsigned int achievement_id, unsigned char result);

    // IDA_EA: 0x828C3A4
    // IDA_NAME: _ZN12CAchievement13_sendCompleteEji24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CAchievement::_sendComplete(unsigned int,int,ENUM_TITLE_BOOK_CATEGORY,int)
    void _sendComplete(unsigned int achievement_id,
                       int title_reward,
                       ENUM_TITLE_BOOK_CATEGORY category,
                       int slot);

    // IDA_EA: 0x828CC56
    // IDA_NAME: _ZN12CAchievement11_forceClearEj
    // DEMANGLED: CAchievement::_forceClear(unsigned int)
    charac_expand::CData* _forceClear(unsigned int achievement_id);

    // IDA_EA: 0x828CE40
    // IDA_NAME: _ZN12CAchievement11_forceResetEj
    // DEMANGLED: CAchievement::_forceReset(unsigned int)
    intptr_t _forceReset(unsigned int achievement_id);

    // IDA_EA: 0x828D092
    // IDA_NAME: _ZN12CAchievement18_verifyAchievementEj
    // DEMANGLED: CAchievement::_verifyAchievement(unsigned int)
    charac_expand::CData* _verifyAchievement(unsigned int last_update_time);

    CUser* owner_;                                      // IDA +0x08
    std::map<unsigned int, stAchievement> achievements_; // IDA +0x0c
    uint8_t no_clear_;                                  // IDA +0x24
    uint8_t state_25_;                                  // IDA +0x25
};

#if defined(__i386__)
static_assert(sizeof(CAchievement) == 0x28u,
              "CAchievement old-server object size must stay 0x28");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_RUNTIME_H_
