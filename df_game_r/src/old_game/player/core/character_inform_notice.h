#ifndef OLDSRV_OLD_GAME_USERINFO_CHARACTER_INFORM_NOTICE_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARACTER_INFORM_NOTICE_H_

#include <stdint.h>

#include <vector>

#include "old_game/player/core/charac_expand_cdata.h"


// IDA unit: 04_character_select_create/CCharacInformNotice.
// Canonical C-owned slot-19 gameplay provider.
class CCharacInformNotice : public charac_expand::CData {
public:
    CCharacInformNotice();
    virtual ~CCharacInformNotice();

    void handleInformNoticeFlag(uint32_t flag_index);
    bool checkInformNoticeFlag(uint32_t bit, uint32_t byte_index);
    int getInformNoticeFlag(int byte_index);
    bool getInformNoticeFlagIndex(std::vector<uint8_t>& indexes);
    void sendInformNoticeFlag(CUser* user);
    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* dest) const;

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    uint8_t flags_[0x10];  // IDA +0x05, reuses CData tail padding
};

#endif  // OLDSRV_OLD_GAME_USERINFO_CHARACTER_INFORM_NOTICE_H_
