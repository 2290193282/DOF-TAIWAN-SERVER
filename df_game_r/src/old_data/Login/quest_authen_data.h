#ifndef OLDSRV_OLD_DATA_LOGIN_QUEST_AUTHEN_DATA_H_
#define OLDSRV_OLD_DATA_LOGIN_QUEST_AUTHEN_DATA_H_

// IDA unit: 02_login_account/_Quest_Authen_Data.
struct _Quest_Authen_Data {
    _Quest_Authen_Data();

    _Quest_Authen_Data* reset();

    unsigned short quest_id;          // +0x00
    unsigned short clear_count;       // +0x02
    // IDA_EA: UserQuest::set_authen_data @0x86ABDB0 writes this word for
    // QUEST_CONDITION 1, and check_quest_type_condition @0x86ACE46 compares
    // it for trigger type 3.
    unsigned short condition_value;   // +0x04
    unsigned short pad_06;            // +0x06
    unsigned int auth_time;           // +0x08
    unsigned char auth_flag;          // +0x0C
    char pad_0d[3];               // +0x0D
    unsigned int auth_value_10;   // +0x10
    unsigned int auth_value_14;   // +0x14
};

#endif  // OLDSRV_OLD_DATA_LOGIN_QUEST_AUTHEN_DATA_H_
