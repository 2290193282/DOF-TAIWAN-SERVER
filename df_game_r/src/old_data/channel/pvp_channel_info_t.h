#ifndef OLDSRV_OLD_DATA_CHANNEL_PVP_CHANNEL_INFO_T_H_
#define OLDSRV_OLD_DATA_CHANNEL_PVP_CHANNEL_INFO_T_H_

// IDA unit: 03_channel_select/pvp_channel_info_t. Old i386 size: 0x24.
struct pvp_channel_info_t {
    pvp_channel_info_t();
    ~pvp_channel_info_t();

    int channel_no;          // dword0
    int min_grade;           // dword1
    int max_grade;           // dword2
    int upgrade_revision;    // dword3
    int channel_type;        // dword4
    int min_level;           // dword5
    int max_level;           // dword6
    int pvp_mode;            // dword7
    int team_count;          // dword8
};

#endif  // OLDSRV_OLD_DATA_CHANNEL_PVP_CHANNEL_INFO_T_H_
