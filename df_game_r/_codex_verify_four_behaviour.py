from __future__ import print_function
import os
import subprocess
import sys


def function(path, marker):
    with open(path, 'r') as handle:
        text = handle.read()
    start = text.index(marker)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


root = sys.argv[1]
reward_function = function(os.path.join(root, 'src/old_data/PVF/legacy_datamanager/data_manager_init.cpp'),
                           'int CDataManager::set_mob_reward()')
cart_function = function(os.path.join(root, 'src/old_game/dungeon/escort/golden_commerce.cpp'),
                         'void ShowDeliveredResult(')
source = r'''
#include <cassert>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <map>
#include <vector>
#include <utility>
enum ENUM_DB_HANDLE_IDX { DB2=2 };
enum ENUM_SERVER_GROUP { GROUP0=0 };
struct MySQL {
 std::vector<std::pair<int,unsigned> > rows; int pos; bool fail_fetch;
 MySQL():pos(-1),fail_fetch(false){}
 void set_query(const char*){pos=-1;}
 bool exec(bool){return true;} int get_n_rows(){return rows.size();}
 bool fetch(){return !fail_fetch && ++pos < (int)rows.size();}
 bool get_short(int,short& out){out=rows[pos].first;return true;}
 bool get_uint(int,unsigned& out){out=rows[pos].second;return true;}
};
struct DBMgr { MySQL db; MySQL* GetDBHandle(ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP){return &db;} };
struct Clock { unsigned getCurTickCount(){return 10000;} };
namespace GlobalData { DBMgr* s_db_mgr; Clock c; Clock* system_time(){return &c;} }
struct LogManager { template<class... T> static void logFormat(T...){ } };
struct CDataManager { unsigned mob_rewards_[200]; unsigned original_mob_rewards_[200]; int set_mob_reward(); };
'''
source += reward_function
source += r'''
enum { kDungeonUserState=5,kOrdinaryPartySeatCount=4,ENUM_PACKETCLASS_NOTIPACKET=0,
       ENUM_NOTIPACKET_PLAY_RESULT=34,ENUM_NOTIPACKET_CLEAR_DUNGEON_REWARD=35,TIMER_MESSAGE_EPLP_COMMAND_WAIT=21 };
struct PacketGuard {
 int id; std::vector<unsigned char> bytes;
 void put_header(int,int n){id=n;}
 void put_byte(int n){bytes.push_back(n);}
 void put_short(int n){put_byte(n);put_byte(n>>8);}
 void put_int(unsigned n){for(int i=0;i<4;++i)put_byte(n>>(i*8));}
 void finalize(bool){}
};
struct Field { int dungeon,map; int get_dungeon_index(){return dungeon;} int GetCurrentMapIndex(){return map;} };
struct Battle { uint64_t elapsed; uint64_t& dungeon_playing_time(){return elapsed;} };
struct CParty;
struct CUser {
 CParty* party; int state,id,playing,fail_next; std::vector<PacketGuard> packets;
 CUser():party(0),state(5),id(0),playing(3057),fail_next(0){}
 CParty* GetParty(){return party;} int get_state(){return state;}
 void setPlayingDungeonIndex(int n){playing=n;}
 int Send(const PacketGuard& p){if(fail_next){--fail_next;return 0;}packets.push_back(p);return 1;}
};
struct CParty {
 Field field; Battle battle; CUser* users[4]; int clear_state,eplp,state,index,keys;
 CParty():clear_state(0),eplp(1),state(2),index(7),keys(0){std::memset(users,0,sizeof(users));field.dungeon=3057;field.map=12159;}
 Field& battle_field(){return field;} Battle& battle_data(){return battle;}
 CUser* get_user(int n){return users[n];}
 void makeBestClearTimePacket(PacketGuard& p,unsigned time){p.put_byte(0);int count=0;for(int i=0;i<4;++i)if(users[i])++count;p.put_byte(count);for(int i=0;i<4;++i)if(users[i]){p.put_short(users[i]->id);p.put_int(time);p.put_byte(0);}}
 void set_dungeon_clear_state(int n){clear_state=n;} void SetEPLPState(int n){eplp=n;}
 void set_state(int n){state=n;} int GetPartyIndex(){return index;} int gen_timer_key(int){return ++keys;}
};
struct Timer {int count;Timer():count(0){} void InsertTimer(int,int,int,int,unsigned,int){++count;}} timer;
Timer* G_TimerQueue(){return &timer;}
struct Mutex{} g_golden_commerce_mutex;
struct MutexGuard {explicit MutexGuard(Mutex*){}};
struct cMyTrace {cMyTrace(const char*,int,int){} template<class...T> void operator()(const char*,T...){}};
struct RewardClaimMessage {unsigned run_token_hi,run_token_lo,charac_no,cart_unique_id,reward_gold;int dungeon_index;unsigned char rank_index,authoritative_percent;};
struct PartyMemberState {unsigned charac_no;bool result_ui_sent;PartyMemberState():charac_no(0),result_ui_sent(false){}};
struct PartySession {
 bool completed,terminal_room_bound,result_ui_complete;unsigned run_token_hi,run_token_lo,cart_unique_id;
 int dungeon_index,current_map_index;uint64_t dungeon_start_tick;PartyMemberState members[4];
 PartySession():completed(true),terminal_room_bound(true),result_ui_complete(false),run_token_hi(10),run_token_lo(20),cart_unique_id(300),dungeon_index(3057),current_map_index(12159),dungeon_start_tick(1000){}
};
typedef std::map<CParty*,PartySession> PartySessionMap;
PartySessionMap g_sessions;
int FindMemberSeat(const PartySession& s,CUser* u){for(int i=0;i<4;++i)if(s.members[i].charac_no==(unsigned)u->id)return i;return -1;}
bool IsCurrentMember(CParty* p,int i,const PartyMemberState& m,CUser*& u){u=p->users[i];return u&&m.charac_no==(unsigned)u->id;}
'''
source += cart_function
source += r'''
void add(CParty& p,CUser& u,int seat,int id){u.party=&p;u.id=id;p.users[seat]=&u;g_sessions[&p].members[seat].charac_no=id;}
RewardClaimMessage claim(int id){RewardClaimMessage x={10,20,(unsigned)id,300,92000,3057,3,46};return x;}
int main(){
 DBMgr db; GlobalData::s_db_mgr=&db; CDataManager dm; std::memset(&dm,0xAB,sizeof(dm));
 db.db.rows.push_back(std::make_pair(99,3555u));db.db.rows.push_back(std::make_pair(85,2697u));
 assert(dm.set_mob_reward()==1);assert(dm.mob_rewards_[100]==3555);assert(dm.mob_rewards_[199]==3555);
 assert(dm.mob_rewards_[85]==2697);assert(dm.mob_rewards_[50]==0);assert(dm.mob_rewards_[0]==0);
 assert(!std::memcmp(dm.mob_rewards_,dm.original_mob_rewards_,sizeof(dm.mob_rewards_)));
 db.db.rows.push_back(std::make_pair(100,0u));assert(dm.set_mob_reward()==1);assert(dm.mob_rewards_[100]==0&&dm.mob_rewards_[101]==0);
 db.db.rows.back().second=70000u;assert(dm.set_mob_reward()==1);assert(dm.mob_rewards_[100]==70000&&dm.mob_rewards_[199]==70000);
 db.db.rows.push_back(std::make_pair(200,9u));assert(dm.set_mob_reward()==0);assert(dm.mob_rewards_[100]==70000);
 db.db.rows.pop_back();db.db.fail_fetch=true;assert(dm.set_mob_reward()==0);assert(dm.mob_rewards_[100]==70000);
 puts("PASS reward table: upper-tail clamp, explicit zero, interior gap, uint32, atomic invalid/fetch failure");
 CParty p;CUser a,b;add(p,a,0,5);add(p,b,1,7);
 RewardClaimMessage ca=claim(5),cb=claim(7);RewardClaimMessage stale=ca;stale.run_token_lo=21;
 ShowDeliveredResult(&a,stale);assert(a.packets.empty());
 ShowDeliveredResult(&a,ca);assert(a.packets.size()==2&&a.packets[0].id==34&&a.packets[1].id==35);
 assert(a.packets[1].bytes.size()==121);for(unsigned i=0;i<a.packets[1].bytes.size();++i)assert(a.packets[1].bytes[i]==0);
 assert(p.state==2&&p.eplp==1&&timer.count==0);ShowDeliveredResult(&a,ca);assert(a.packets.size()==2);
 ShowDeliveredResult(&b,cb);assert(b.packets.size()==2&&p.state==1&&p.eplp==2&&p.clear_state==2&&timer.count==1);
 assert(p.battle.elapsed==9000&&a.playing==-1&&b.playing==-1);
 ShowDeliveredResult(&b,cb);assert(b.packets.size()==2&&timer.count==1);
 CParty p2;CUser c;add(p2,c,0,9);c.fail_next=1;RewardClaimMessage cc=claim(9);
 ShowDeliveredResult(&c,cc);assert(c.packets.empty()&&!g_sessions[&p2].members[0].result_ui_sent&&p2.state==2);
 ShowDeliveredResult(&c,cc);assert(c.packets.size()==2&&p2.state==1&&timer.count==2);
 puts("PASS cart: stale run rejected, NOTI34/35 grammar, no duplicate UI, multiplayer completion once");
}
'''
binary = os.path.join(root, '_codex_four_behaviour_test')
compiler = '/opt/gcc-4.9/bin/g++'
p = subprocess.Popen([compiler, '-m32', '-std=gnu++11', '-O0', '-x', 'c++', '-', '-o', binary], stdin=subprocess.PIPE)
p.communicate(source.encode('utf-8'))
if p.returncode:
    sys.exit(p.returncode)
sys.exit(subprocess.call([binary]))
