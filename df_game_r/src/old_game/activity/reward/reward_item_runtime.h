#ifndef OLDSRV_GLOBAL_REWARD_ITEM_RUNTIME_H_
#define OLDSRV_GLOBAL_REWARD_ITEM_RUNTIME_H_

class CUser;

// IDA_EA: 0x84BFB84
// IDA_NAME: _Z10RewardItemP5CUserllljPKcS2_j
// DEMANGLED: RewardItem(
//     CUser *,long,long,long,unsigned int,char const*,char const*,unsigned int)
int RewardItem(CUser* user,
               long start_time,
               long end_time,
               long expiration_time,
               unsigned int item_id,
               const char* title_string_key,
               const char* body_string_key,
               unsigned int occurrence_time);

#endif  // OLDSRV_GLOBAL_REWARD_ITEM_RUNTIME_H_
