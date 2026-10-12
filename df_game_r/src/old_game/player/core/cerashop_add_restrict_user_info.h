#ifndef OLDSRV_OLD_GAME_USERINFO_CERASHOP_ADD_RESTRICT_USER_INFO_H_
#define OLDSRV_OLD_GAME_USERINFO_CERASHOP_ADD_RESTRICT_USER_INFO_H_

#include <map>

namespace CerashopAddRestrict {

struct paramDaily;

struct UserInfo {
    // IDA_EA: 0x8695B20
    // IDA_NAME: _ZN19CerashopAddRestrict8UserInfoC2Ev
    // DEMANGLED: CerashopAddRestrict::UserInfo::UserInfo(void)
    UserInfo();

    // IDA_EA: 0x8695BB2
    // IDA_NAME: _ZN19CerashopAddRestrict8UserInfoD2Ev
    // DEMANGLED: CerashopAddRestrict::UserInfo::~UserInfo()
    ~UserInfo();

    std::map<unsigned int, paramDaily*> parameter_map_0;
    std::map<unsigned int, paramDaily*> parameter_map_1;
    std::map<unsigned int, unsigned int> value_map_0;
    std::map<unsigned int, unsigned int> value_map_1;
};

static_assert(sizeof(UserInfo) == 0x60u,
              "CerashopAddRestrict::UserInfo i386 layout drift");

}  // namespace CerashopAddRestrict

#endif  // OLDSRV_OLD_GAME_USERINFO_CERASHOP_ADD_RESTRICT_USER_INFO_H_
