# Garena 140828 `df_game_r` 源码工程

本仓库以 Garena 140828 客户端、对应 PVF/RDAR、客户端 IDA 证据和服务器业务调用链为唯一目标，按 Neople 的类、dispatcher、DBTask、PVF importer 和运行时状态机组织源码。

> **版本边界**
>
> - 只生成 140828 原生协议和数据布局。
> - 客户端 CMD/NOTI 编号直接使用 [`PacketEnum.h`](src/infra/network/packet/PacketEnum.h)，不经过编号映射。
> - 不依赖客户端 hook、服务端包体重写、双协议分支、模拟数据或恒成功处理器。
> - 字段语义必须来自客户端 reader/writer、PVF、数据库和运行时调用链。只有字节形状而没有业务 owner 的协议保持禁用。

## 快速索引

| 目标 | 文档或源码 |
|---|---|
| 架构、类职责、协议字段和新增业务 | [`docs/GAME_DEVELOPMENT_GUIDE.md`](docs/GAME_DEVELOPMENT_GUIDE.md) |
| CMD/NOTI 唯一枚举 | [`src/infra/network/packet/PacketEnum.h`](src/infra/network/packet/PacketEnum.h) |
| 人工审计后的请求、响应和 owner | [`docs/0828_upgrade/packet_native_audit.tsv`](docs/0828_upgrade/packet_native_audit.tsv) |
| 客户端注册表与字段读取轨迹 | [`docs/0828_upgrade/packet_truth_matrix.tsv`](docs/0828_upgrade/packet_truth_matrix.tsv) |
| 服务端注册、实现和阻断状态 | [`docs/0828_upgrade/packet_implementation_matrix.tsv`](docs/0828_upgrade/packet_implementation_matrix.tsv) |
| 登录、选角、创建角色和 USERINFO | [`docs/0828_upgrade/phase1_session_spine.md`](docs/0828_upgrade/phase1_session_spine.md) |
| PVF 字段与 importer 结论 | [`docs/0828_upgrade/pvf_native_migration_summary.md`](docs/0828_upgrade/pvf_native_migration_summary.md) |
| Packet 到 DB 队列/表的路由 | [`docs/0828_upgrade/native_0828_packet_db_route_report.md`](docs/0828_upgrade/native_0828_packet_db_route_report.md) |
| 数据库迁移清单与 SHA | [`docs/0828_upgrade/database_migration_inventory_0828.tsv`](docs/0828_upgrade/database_migration_inventory_0828.tsv) |
| 当前变更记录 | [`docs/UPDATES.md`](docs/UPDATES.md) |

## 工程结构

| 路径 | 责任边界 |
|---|---|
| `src/entry/` | `main`、初始化、主循环和关闭流程。 |
| `src/common/` | 全局配置、共享值类型、管理器 ABI 和正式常量。等级、职业、装备槽和背包容量以 `config.h` 为唯一来源。 |
| `src/core/` | 线程、时间、内存、日志和服务端基础设施。 |
| `src/infra/network/` | Socket、Stream、PacketBuf、封包头、dispatcher 注册和协议完整性。 |
| `src/infra/security/` | 加密、AntiBot/DPROTO、策略和动态库 ABI 边界。 |
| `src/old_data/PVF/` | PVF/RDAR 打开、token 读取以及装备、物品、角色、地图、地下城、任务、技能和活动 importer。 |
| `src/old_data/db/` | MySQL、DBTask、SIG/Inter、事务、队列、角色与业务持久化。 |
| `src/old_data/Login/` | 登录、鉴权、角色列表和会话数据装载。 |
| `src/old_data/channel/` | 频道状态、人数、代理连接和频道切换。 |
| `src/old_game/player/` | `CUser`、角色选择、创建、USERINFO 和玩家会话状态。 |
| `src/old_game/world/` | `GameWorld`、城镇、区域、广播、传送和进入世界。 |
| `src/old_game/economy/` | 背包、装备、仓库、商店、邮件、交易、拍卖和商城。 |
| `src/old_game/progression/` | 任务、技能、转职、觉醒和成长。 |
| `src/old_game/social/` | 组队、好友和公会。 |
| `src/old_game/dungeon/` | 普通/特殊副本、房间、怪物、掉落、塔和结算。 |
| `src/old_game/pvp/` | PvP 匹配、房间、战场、同步和赛事。 |
| `src/old_game/activity/` | 活动日程、条件、奖励和账户级领取状态。 |
| `src/old_game/appearance/` | 时装、武器装扮、宠物及外观状态。 |
| `src/runtime/` | 全局实例、启动接线和运行时协调。 |
| `src/ida_full_restore/` | IDA 导出证据树，只用于追溯，不作为正式业务实现目录。 |
| `manifest_A.mk` / `manifest_B.mk` / `manifest_C.mk` | 当前正式翻译单元清单。新增或迁移 `.cpp` 必须同步登记且只能有一个 provider。 |

## 环境要求

### 固定 ABI

| 项目 | 要求 |
|---|---|
| 构建系统 | CentOS 6 x86_64 |
| 输出 | ELF32 i386 |
| 编译器 | GCC/G++ 4.9.x，推荐 `/opt/gcc-4.9/bin/g++` |
| 语言标准 | GNU C++11 |
| 默认参数 | `-m32 -O2 -g -std=gnu++11 -Wall -Wextra -Wno-unused-parameter` |
| Boost | 1.48.0，默认 `/opt/boost_1_48_0` |
| MySQL ABI | ELF32 i386 `libmysqlclient.so.18` |
| MySQL 配套库 | 同目录 ELF32 `libssl.so.1.0.0`、`libcrypto.so.1.0.0` |
| 运行目录 | `/home/neople/game` |

构建机可以是 64 位，但编译器、头文件和所有直接链接库都必须支持 i386。不能通过去掉 `-m32` 或把 64 位库软链接成 32 位名称绕过 ABI 检查。

### 安装基础包

CentOS 6 已停止官方维护，应使用可信的内部镜像或 CentOS Vault，并保持 RPM 签名校验。准备好可用 yum 源后执行：

```bash
yum groupinstall -y "Development Tools"
yum install -y \
  git make cmake autoconf automake libtool tar xz patch gdb \
  glibc-devel.i686 libgcc.i686 libstdc++.i686 libstdc++-devel.i686 \
  zlib-devel.i686 openssl-devel.i686 mysql-devel.i686 \
  GeoIP.i686 GeoIP-devel.i686
```

如果归档源缺少 32 位 GeoIP 包，只能从同版本、已验证的 CentOS 6 环境迁移 `libGeoIP.so.1` 和开发链接文件。

### 安装 GCC 4.9

先检查固定工具链：

```bash
/opt/gcc-4.9/bin/g++ --version
```

若服务器没有该工具链，可从经过 SHA 校验的 GCC 4.9.4 源码构建：

```bash
tar xf gcc-4.9.4.tar.xz
cd gcc-4.9.4
./contrib/download_prerequisites
mkdir build
cd build

../configure \
  --prefix=/opt/gcc-4.9 \
  --enable-languages=c,c++ \
  --enable-multilib \
  --disable-bootstrap

make -j8
make install
```

验证 32 位 C++11 输出：

```bash
cat >/tmp/check_gcc49.cpp <<'EOF'
#include <iostream>
int main() { std::cout << sizeof(void*) << "\n"; }
EOF

/opt/gcc-4.9/bin/g++ -m32 -std=gnu++11 \
  /tmp/check_gcc49.cpp -o /tmp/check_gcc49
/tmp/check_gcc49
file /tmp/check_gcc49
```

正确结果应输出 `4`，且 `file` 显示 `ELF 32-bit`。

### 安装 Boost 1.48

```bash
cd /opt
tar xf boost_1_48_0.tar.bz2
test -f /opt/boost_1_48_0/boost/version.hpp
grep BOOST_LIB_VERSION /opt/boost_1_48_0/boost/version.hpp
```

输出必须包含 `1_48`。不要直接替换成新版 Boost 后继续沿用当前对象布局。

### 部署 32 位 MySQL 客户端

推荐将匹配的一组库放在同一目录：

```text
/home/neople/game/lib/mysql32/
  libmysqlclient.so.18
  libssl.so.1.0.0
  libcrypto.so.1.0.0
```

逐个检查：

```bash
file -L /home/neople/game/lib/mysql32/libmysqlclient.so.18
file -L /home/neople/game/lib/mysql32/libssl.so.1.0.0
file -L /home/neople/game/lib/mysql32/libcrypto.so.1.0.0
readelf -d /home/neople/game/lib/mysql32/libmysqlclient.so.18 | grep SONAME
```

三者都必须为 `ELF 32-bit`、`Intel 80386`，MySQL 客户端 SONAME 必须为 `libmysqlclient.so.18`。项目解析器会按以下顺序查找：

1. `MYSQLCLIENT_LIB` 指定的绝对文件；
2. `MYSQLCLIENT_LIB_DIR` 指定的目录列表；
3. `/home/neople/game/lib/mysql32`；
4. `/opt/lampp/lib`、`/usr/lib/mysql`、`/usr/lib`、`/lib`。

验证自动解析：

```bash
cd /path/to/oldsrv_rebuild
MYSQLCLIENT_LIB_DIR=/home/neople/game/lib/mysql32 \
  sh tools/find_mysqlclient_elf32.sh --path
```

MySQL 服务端版本可以独立升级；`df_game_r` 运行时依赖的是客户端动态库 ABI。数据库 SQL 兼容不代表可以加载任意 SONAME 或任意位数的客户端库。

### 部署游戏动态库

直接链接边界：

```text
libmysqlclient.so.18
libssl.so.1.0.0
libcrypto.so.1.0.0
libdl.so
libpthread.so
libz.so
libGeoIP.so.1
libnxencryption.so
```

以下私有库按程序运行路径动态加载，不应加入普通静态替代实现：

```text
/home/neople/game/libantisvrimport.so
/home/neople/game/libdp2.so
/home/neople/game/libdp2pre.so
/home/neople/game/lib/libdp2game.so
```

检查文件位数和依赖：

```bash
file -L /home/neople/game/libnxencryption.so
file -L /home/neople/game/libantisvrimport.so
ldd /home/neople/game/libantisvrimport.so
```

### 配置 shell

```bash
export PATH=/opt/gcc-4.9/bin:$PATH
export MYSQLCLIENT_LIB_DIR=/home/neople/game/lib/mysql32
export LD_LIBRARY_PATH=/opt/gcc-4.9/lib:/opt/gcc-4.9/lib64:/home/neople/game:/home/neople/game/lib:/home/neople/game/lib/mysql32:/usr/lib:/lib:$LD_LIBRARY_PATH
```

不要把 `/usr/lib64` 放到 32 位私有库之前。

## 编译教程

### 第一次构建

```bash
cd /path/to/oldsrv_rebuild
export PATH=/opt/gcc-4.9/bin:$PATH
export MYSQLCLIENT_LIB_DIR=/home/neople/game/lib/mysql32

make audit_static
make clean
make -j8 restored_objects
make -j8 link_probe
make release
```

如果远端 Python 3 不在 PATH 中：

```bash
make audit_static PYTHON3=/opt/python3/bin/python3
```

各目标作用：

| 目标 | 输出或检查 |
|---|---|
| `make audit_static` | 检查版本常量、manifest 覆盖和静态约束。 |
| `make restored_objects` | 编译三个 manifest 中的全部正式 `.cpp`。 |
| `make link_probe` | 链接带 DWARF 和完整符号表的 `df_game_r`。 |
| `make incremental_link_probe` | 只使用现有对象重新链接，适合定位链接错误。 |
| `make release` | 从同一成功链接产物生成剥离版 `df_game_r.release`。 |
| `make clean` | 删除 `src` 下 `.o/.d` 和链接产物。 |

### 开发期增量构建

修改少量文件后直接执行：

```bash
make -j8 restored_objects
make incremental_link_probe
```

需要缩短错误探针时间时可以临时降低优化和调试信息：

```bash
make -k -j8 \
  CXXFLAGS='-m32 -O0 -g0 -pipe -std=gnu++11 -w' \
  restored_objects
make incremental_link_probe
```

快速探针不能代替发布验收。提交候选程序前必须重新执行静态门禁、`make clean`、正式对象编译和完整链接。

### 查看编译进度

```bash
watch -n 2 '
date
echo "=== make/g++ process ==="
pgrep -af "make|g\+\+"

echo "=== object progress ==="
find /path/to/oldsrv_rebuild/src -name "*.o" 2>/dev/null | wc -l
'
```

对象数量只用于观察进度；最终结果以 make 退出码、完整链接和符号审计为准。

### 验证产物

```bash
file df_game_r df_game_r.release
readelf -h df_game_r | grep -E 'Class|Machine'
readelf -d df_game_r | grep NEEDED
ldd ./df_game_r
nm -C ./df_game_r | grep ' U ' | head -100
```

验收条件：

- 两个程序均为 ELF32 i386。
- `ldd` 不出现 `not found` 或 `wrong ELF class`。
- 没有项目业务符号的未定义引用。
- 没有重复 provider。
- 诊断版与发行版来自同一成功链接输入。

### 生成可审计发布包

需要冻结源码、SQL、工具和静态门禁结果时执行：

```bash
python3 tools/package_active_release.py \
  --root . \
  --output-dir /tmp/df_game_release \
  --rid 140828_$(date +%Y%m%d_%H%M%S)
```

输出包含确定性 `.tar.gz` 和逐文件 SHA-256 的 `.inventory.tsv`。同一个 RID 不允许覆盖，远端 stage 与 clean-build 脚本也要求独立目录和一次性 RID。

## 运行与调试

### 启动前检查

```bash
cd /home/neople/game
export MYSQLCLIENT_LIB_DIR=/home/neople/game/lib/mysql32
export LD_LIBRARY_PATH=/opt/gcc-4.9/lib:/opt/gcc-4.9/lib64:/home/neople/game:/home/neople/game/lib:/home/neople/game/lib/mysql32:/usr/lib:/lib:$LD_LIBRARY_PATH

file ./df_game_r
ldd ./df_game_r
```

确认全部依赖正确后：

```bash
./df_game_r siroco11 start
```

测试候选程序时先在隔离目录前台启动，不要在未完成登录、频道、选角、进入世界和关键业务回归前覆盖生产程序。

### GDB

```bash
cd /home/neople/game
ulimit -c unlimited
gdb -q ./df_game_r
```

```gdb
set pagination off
set print pretty on
set args siroco11 start
run
```

崩溃后采集：

```gdb
set logging file /tmp/df_game_r_gdb.log
set logging overwrite on
set logging on
thread apply all bt full
info registers
x/16i $pc-16
x/32wx $sp
set logging off
```

必须使用产生 core 时完全相同的未剥离 `df_game_r` 和动态库。

## 常见问题

| 错误 | 处理 |
|---|---|
| `wrong ELF class: ELFCLASS64` | 实际加载了 64 位依赖。检查 `file -L`、`ldd` 和 `LD_LIBRARY_PATH` 顺序。 |
| `libmysqlclient.so.18: cannot open` | 设置 `MYSQLCLIENT_LIB_DIR` 与运行时 `LD_LIBRARY_PATH`，确认文件存在且 SONAME 正确。 |
| `cannot find -l:libGeoIP.so.1` | 安装/部署 32 位 GeoIP，不要链接 `/usr/lib64` 的文件。 |
| `GLIBCXX_* not found` | 运行时没有优先加载 GCC 4.9 的 32 位 `libstdc++.so.6`。 |
| `undefined reference` | 按原始类和签名补唯一真实 provider，不能增加包装别名掩盖错误 owner。 |
| `multiple definition` | 删除非 owner 副本；一个业务函数只能有一个实现。 |
| 多个构建任务互相锁住 | 每个任务使用独立源码树、远端根目录和 RID。 |
| 增量链接通过但干净构建失败 | 旧对象掩盖了依赖或 manifest 问题，执行完整干净构建。 |

## 协议文档索引

### 核心会话与角色

| 范围 | 入口 |
|---|---|
| CMD1 登录、CMD4 选角、CMD5 创建角色 | [`开发指南：核心协议`](docs/GAME_DEVELOPMENT_GUIDE.md#核心会话协议) |
| CMD8 与 NOTI2 USERINFO | [`开发指南：userinfo-与角色外观`](docs/GAME_DEVELOPMENT_GUIDE.md#userinfo-与角色外观) |
| 职业、等级、装备类型和背包槽 | [`开发指南：共享数据布局`](docs/GAME_DEVELOPMENT_GUIDE.md#共享数据布局) |

### 游戏业务

| 范围 | 入口 |
|---|---|
| NOTI9 组队、NOTI29 进图、NOTI35/37 结算与经验 | [`开发指南：世界组队与副本`](docs/GAME_DEVELOPMENT_GUIDE.md#世界组队与副本) |
| NOTI13、物品公共尾、成长装备、武器装扮 | [`开发指南：物品装备与背包`](docs/GAME_DEVELOPMENT_GUIDE.md#物品装备与背包) |
| 匹配、PvP、双人 Raid、竞速和任务副本 | [`开发指南：匹配与特殊副本`](docs/GAME_DEVELOPMENT_GUIDE.md#匹配与特殊副本) |
| 活动、奖励、账户领取和高编号协议 | [`开发指南：活动与高编号协议`](docs/GAME_DEVELOPMENT_GUIDE.md#活动与高编号协议) |
| PVF/RDAR 新字段 | [`开发指南：pvf-与-rdar`](docs/GAME_DEVELOPMENT_GUIDE.md#pvf-与-rdar) |
| Packet 到 DBTask/SQL 的持久化链 | [`开发指南：数据库与事务`](docs/GAME_DEVELOPMENT_GUIDE.md#数据库与事务) |

### 全量查询

不要在 README 中维护第二份 0..END 字典。全量查询按以下事实源进行：

1. `PacketEnum.h`：编号和枚举名称。
2. `packet_truth_matrix.tsv`：客户端注册、reader 轨迹和证据位置。
3. `packet_native_audit.tsv`：人工确认的字段名、分支、owner 和待验证项。
4. `packet_implementation_matrix.tsv`：服务端注册与实现状态。
5. `native_0828_packet_db_route_matrix.tsv`：需要持久化的 Packet 到 DB 路由。

PowerShell 查询单个 Packet：

```powershell
Import-Csv docs/0828_upgrade/packet_native_audit.tsv -Delimiter "`t" |
  Where-Object {
    $_.packet_class -eq 'CMD' -and $_.packet_id_0828 -eq '562'
  } |
  Format-List
```

Linux 查询：

```bash
awk -F '\t' 'NR == 1 || ($1 == "CMD" && $2 == 562)' \
  docs/0828_upgrade/packet_native_audit.tsv
```

## 开发与验收规则

1. 现有还原函数保留 `IDA_EA`、`IDA_NAME` 和 `DEMANGLED`；一个 IDA 函数对应一个源码函数。
2. 新增版本功能放入原业务 owner，不创建带版本后缀的平行 `CUser`、`CInventory`、dispatcher 或 serializer。
3. `.h` 保存跨翻译单元使用的正式声明和布局，`.cpp` 保存函数体；不得在调用方复制结构体。
4. PVF 读取失败、DB 失败、状态不完整或协议证据不足时 fail closed，不能生成默认物品、角色、任务或奖励。
5. 每次修改 Packet 同时检查客户端 writer、客户端 reader、服务端 read/process/write、错误分支、PVF/DB owner 和重连恢复。
6. 每次修改持久化结构同时检查 load、save、事务回滚、重复请求、断线重投和登录恢复。
7. 发布门禁依次为：静态审计、ELF32 对象编译、完整链接、依赖检查、登录/频道/选角/进世界回归、功能与重连回归。
