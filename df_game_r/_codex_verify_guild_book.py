from __future__ import print_function
import os
import re
import subprocess
import sys
import tempfile


def read(path):
    with open(path, 'r') as stream:
        return stream.read()


def function_or_case(text, marker):
    start = text.index(marker)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


root, baseline, pvf = sys.argv[1:4]
relative = 'old_game/player/core/cuser_increase_status_runtime.cpp'
before = read(os.path.join(baseline, relative))
after = read(os.path.join(root, 'src', relative))
old_case = function_or_case(before, 'case 690000220:')
new_case = function_or_case(after, 'case 690000220:')
# This fix must not replace the ordinary-book fallback or the later debit/send logic.
assert before[:before.index(old_case)] == after[:after.index(new_case)]
assert before[before.index(old_case) + len(old_case):] == after[after.index(new_case) + len(new_case):]
params = read(os.path.join(root, 'src/old_data/PVF/etc/guild_parameter_script.cpp'))
lookup = function_or_case(params, 'int32_t GuildParameterScript::getGuildExpBook(')
pvf_text = read(pvf)
books = re.search(r'\[guild exp book\]\s*((?:\s*\d+\s+\d+\s*)+)', pvf_text).group(1)
numbers = list(map(int, re.findall(r'\d+', books)))
assert len(numbers) == 6
pairs = list(zip(numbers[0::2], numbers[1::2]))
assert dict(pairs)[690000220] == 20000
cpp = r'''
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <map>
struct STGuildLevelUpParam {int value_04;};
struct STGuildDBInfoOnly {uint32_t guild_experience;};
struct GuildParameterScript {std::map<int,int> guild_exp_book;STGuildLevelUpParam next;bool level_exists;GuildParameterScript():level_exists(true){next.value_04=100000;}int32_t getGuildExpBook(int32_t);STGuildLevelUpParam* getGuildLevelUpParam(int){return level_exists?&next:0;} };
'''
cpp += lookup + '\n'
cpp += r'''
GuildParameterScript params;
struct Data {GuildParameterScript* script;Data():script(&params){}GuildParameterScript* GetGuildParameterScriptForBuild(){return script;} } data;
Data* G_CDataManager(){return &data;}
struct PacketGuard {};
struct LogManager {template<class...T>static void logFormat(T...){}};
struct CUser {bool master,has_info;STGuildDBInfoOnly info;int status_out,value_out,error;CUser():master(true),has_info(true),status_out(-1),value_out(-1),error(0){info.guild_experience=0;}bool IsGuildMaster(){return master;}int GetGuildLevel(){return 1;}STGuildDBInfoOnly* GetGuildDBInfo(){return has_info?&info:0;}int getCurCharacNo(){return 42;}void Before(int);void After(int);void reset(){status_out=value_out=-1;error=0;} };
void SendUseStatusError(CUser* user,PacketGuard&,int e){user->error=e;}
'''
for name, case in [('Before', old_case), ('After', new_case)]:
    cpp += 'void CUser::' + name + '(int item_id){PacketGuard packet;int status=-1,value=0;switch(item_id){' + case + '}status_out=status;value_out=value;}\n'
cpp += 'int main(){\n'
for key, value in pairs:
    cpp += 'params.guild_exp_book[%d]=%d;\n' % (key, value)
cpp += r'''
 CUser user;user.Before(690000220);assert(!user.error&&user.status_out==16&&user.value_out==0);
 printf("REPRODUCED: old full item ID narrowed to %d and resolved EXP=0\n",int(static_cast<char>(690000220)));
 user.reset();user.After(690000220);assert(!user.error&&user.status_out==16&&user.value_out==20000);
 assert(params.getGuildExpBook(1201)==2000&&params.getGuildExpBook(1202)==3000);
 params.guild_exp_book[static_cast<char>(690000220)]=777;
 user.reset();user.After(690000220);assert(user.value_out==20000); // no low-byte alias
 params.guild_exp_book.erase(690000220);user.reset();user.After(690000220);assert(user.error==17&&user.status_out==-1);
 params.guild_exp_book[690000220]=0;user.reset();user.After(690000220);assert(user.error==17&&user.status_out==-1);
 params.guild_exp_book[690000220]=-1;user.reset();user.After(690000220);assert(user.error==17&&user.status_out==-1);
 params.guild_exp_book[690000220]=20000;
 user.master=false;user.reset();user.After(690000220);assert(user.error==219&&user.status_out==-1);user.master=true;
 user.info.guild_experience=100000;user.reset();user.After(690000220);assert(user.error==210&&user.status_out==-1);user.info.guild_experience=0;
 params.level_exists=false;user.reset();user.After(690000220);assert(user.error==210&&user.status_out==-1);params.level_exists=true;
 user.has_info=false;user.reset();user.After(690000220);assert(user.error==210&&user.status_out==-1);user.has_info=true;
 data.script=0;user.reset();user.After(690000220);assert(user.error==210&&user.status_out==-1);
 puts("PASS: full ID => 20000; ordinary map keys unchanged; no byte alias; missing/zero/negative config rejected; guild-master/EXP-cap/max-level/null guards retained");
 puts("PASS: ordinary-book fallback and downstream debit/guild-send source unchanged; invalid configuration returns before debit");
}
'''
temp = tempfile.mkdtemp(prefix='codex_guild_book_')
source, binary = os.path.join(temp, 'test.cpp'), os.path.join(temp, 'test')
with open(source, 'w') as stream:
    stream.write(cpp)
subprocess.check_call(['/opt/gcc-4.9/bin/g++', '-m32', '-std=gnu++11', '-O0', source, '-o', binary])
subprocess.check_call([binary])
print('HARNESS=' + source)
