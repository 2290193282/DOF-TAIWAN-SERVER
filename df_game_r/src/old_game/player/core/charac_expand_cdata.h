#ifndef OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_CDATA_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_CDATA_H_

#include <stdint.h>


class CUser;

namespace charac_expand {

// IDA unit: 04_character_select_create/charac_expand/charac_expand_CData.
class CData {
public:
    CData();

    CData* alter();
    virtual void ResetDailyMidnight();
    virtual bool loadData(CUser* user, char* data) = 0;
    virtual void getData(char* data) const;
    virtual void ResetDaily();
    virtual ~CData();

    int isSave();
    int saveData(CUser* user);
    int reset();

protected:
    virtual int _saveData(CUser* user) = 0;
    virtual int _reset() = 0;

private:
    uint8_t save_flag_;  // IDA +0x04
};

}  // namespace charac_expand

#endif  // OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_CDATA_H_
