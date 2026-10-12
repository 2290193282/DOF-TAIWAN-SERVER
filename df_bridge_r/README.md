# df_bridge_r 业务源码恢复（0828）

这是对台服 0828 有符号 `df_bridge_r` 的可编译业务逻辑恢复，并已补齐不依赖旧类 ABI
的 Linux/POSIX 守护进程包装层。第三方/系统库源码不复制进目录；生产构建直接链接
目标环境的 `mysqlclient`，线程、socket、poll、PID 与信号使用标准 C++11/POSIX 实现。

## 已恢复范围

- 从 `d_channel` 读取 `ch_script_version`、`ch_gc_info`、`ch_dungeon_data`、`ch_dungeon_list`、`ch_server_data`。
- `ScriptThread` 生成 `cfg/channel_info.etc`，每 4 秒检查版本并热更新。
- 保留原始 15 个频道业务字段：频道号、名称、`channel_kind`、副本组、总经验率与 10 个经验修正值。数据库表仍是 17 个命名列（另含 `server_id`、`is_use`），type 19/20/23 不需要增加新字段。
- 协议 4/5/9/13 及 Extra 2 的注册、脚本版本检查、脚本文本返回、GC 映射、频道服务注册、游戏频道状态更新。
- 支持类型 19（Zombie）、20（Tournament）和 23（Raid）；类型 21 是无效哨兵、22 是保留值，解析器和验证器都会拒绝 21/22。
- 接受官方 `[server] 17` 和旧 Bridge `[server]\n17` 两种文本布局，输出保持原 Bridge 的线协议布局。

没有复制原程序静态链接进去的 Reactor、内存池和日志类实现；替换程序在外部契约上复现
它们的行为，而不复用旧 C++ 类布局。

## 原版守护进程兼容入口

必须从 Bridge 工作目录（线上为 `/home/neople/bridge`）运行：

```bash
./df_bridge_r bridge test   # 只解析 ./cfg/bridge.cfg，不连 DB、不监听
./df_bridge_r bridge run    # 前台运行，便于隔离验收
./df_bridge_r bridge start  # 单次 fork + setsid，写 ./pid/bridge.pid
./df_bridge_r bridge stop   # SIGTERM，等待退出并清理 PID
```

`start/run` 都会对 `./pid/bridge.pid` 执行非阻塞 `flock` 并持锁到进程退出，拒绝并行
实例。`stop` 在发信号前还会确认 `/proc/<pid>/exe` 的 basename 是 `df_bridge_r`，避免
陈旧 PID 被复用后误杀其他进程。`start` 与原版一样在父进程
返回后由子进程继续初始化，因此现有运维脚本仍应等待 TCP 7000 就绪后再启动 Channel。
SIGPIPE 被忽略，SIGTERM/SIGINT 只设置停机标志；启动时将 core limit 提升为 unlimited。

现网配置格式：

```ini
[server]
max_client = 1000
this_ip = 192.168.200.131
this_tcp_port = 7000
this_udp_port = 7000

[DB]
db_ip = 127.0.0.1
db_name = d_channel
db_id = game
db_pwd = <existing-production-secret>
# db_port = 3306  # 可选；原版固定 3306
```

配置中的密码只用于连接，不写入 stdout 或任何日志。监听行为与原版一致：`this_ip` 作为
声明/advertise 信息记录，TCP/UDP 均 bind `INADDR_ANY` 的 `this_tcp_port`/
`this_udp_port`。运行代码没有硬编码生产 7000、Channel 7101 或测试端口；启动日志和
每连接首帧摘要打印的均是本次 cfg 实际 TCP/UDP 端口。

运行时建立 `./log/Info_bridge.log`、`Warn_bridge.log`、`Error_bridge.log`、
`Cri_bridge.log`。TCP backlog 为 1024，连接数受 `max_client` 限制；长度帧上限为
`0xa0000`。与原 `TCPHandlerRelay/UDPHandlerRelay` 一致，接收路由只使用 packet id，
不以 classification 拒绝旧 Game。TCP 处理普通协议 4/5/9/13，未知 id（包括旧 Game
可发送的 1/11）静默消费并保持连接；UDP 处理 Extra 2。checksum 字段不参与接收校验，
发送时 `PAD` 等价行为是回填总长度。每条 TCP 连接仅记录一次 category/id/size/role
脱敏首帧摘要，不记录 payload。

Bridge 7000 是 Channel 与 Game 共用的应用层协议端点，不是到 Channel 7101 的透明代理：
Game 的版本、脚本和 GC 请求由 Bridge 在原 TCP 会话直接响应；Channel 的 id 4 注册同样
保留该入站 TCP 会话，Check 循环再通过它发送 id 2 状态。

启动时同步加载 MySQL snapshot，成功后才开放端口；后台每 4 秒检查版本。若现有
`./cfg/channel_info.etc` 与 DB snapshot 在 dungeon/channel 业务字段上语义等价，程序
不会重写文件，而是把现有文件的原始二进制字节直接放入响应 cache。因此首次启动可保留
权威 0828 文件的 CRLF、inline server、顺序、尾部 sentinel 及
`7c86d75b...522d79` 哈希；DB 任一真实业务字段变化时才重新 render 并原子替换。
MySQL worker 进入/退出后台线程时分别调用 `mysql_thread_init/end`，同一连接不会
被主线程与 worker 并发使用。GC reload 会在提交新 cache 版本前同步注册表；GC 名称或
upper boundary 变化会清空旧 Game 状态，完全相同（含排序后相同）的 GC 表则保留状态。
Game 状态超过 20 秒未刷新即移除；Channel
notice 超过 200 秒即注销；每 3 秒将全部有效 Game 状态以 id 2、`group=0` 通过该
Channel 已注册的原 TCP 连接发送。非空广播在 Info 日志留下脱敏计数摘要。

## 0828 Zombie 约束

权威文件是：

`../artifacts/channel_0828/channel_info_0828_authoritative.etc`

- SHA-256：`7c86d75b627f8939241d14e5e1aa78b4d5784d7412efc38e4d19e9fb28522d79`
- 21 个 dungeon 组、130 个 dungeon id、475 条频道、12 个 server group。
- Zombie 位于 `server_group=17/98/99`，频道 `186/187/188`，`channel_kind=19`。
- 当前要启动的 Zombie 实例严格使用官方 `server_group=17`，不投影到 group 3。
- `gc_no=3186` 由 `ch_gc_info` 的 Siroco 上界区间归类；`gc_no` 与 `server_group` 是两套独立标识。
- Tournament 位于 group 98/99 的频道 164，`channel_kind=20`。

`ChannelRegistry` 忠实保留原程序的 GC 判定：首个满足 `1 <= group_gc_no - gc_no <= 999` 的组命中。因此 `3186` 能否归入 Siroco 取决于 `ch_gc_info`，不是 `ch_server_data.server_id`。

## d_channel 字段契约

| 表 | 字段 | 作用 |
|---|---|---|
| `ch_script_version` | `script_version` | 热更新触发值和发给游戏服的版本 |
| `ch_gc_info` | `group_name`, `group_gc_no` | 物理区服名称与 GC 上界；不能从频道文件推导 |
| `ch_dungeon_data` | `kind_name`, `dungeon_name` | `[dungeon]` 的组标识和本地化名称 |
| `ch_dungeon_list` | `kind_name`, `dungeon_id` | 各 dungeon 组的数字列表 |
| `ch_server_data` | `server_id`, `is_use`, `channel_number`, `channel_name`, `channel_kind`, `kind_name`, `exp_rate`, `exp_1..exp_10` | `[server]` 内容 |

恢复源码使用显式列名，不再依赖原程序脆弱的 `SELECT *` 下标。type 19/20/23 仍只是 `channel_kind` 的数值，不改变表结构。

## 构建

默认构建不依赖 MySQL，可运行解析、协议和注册逻辑测试：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

目标机只有 GCC 4.9 时直接使用随附 Makefile（严格 `gnu++11`）：

```bash
make -j2
make test
```

上述默认目标不要求 MySQL，可运行 CLI、配置、帧、协议与过期逻辑测试，但 `start/run`
会明确拒绝。生成可直接部署且文件名与原版一致的 MySQL 生产目标：

```bash
make clean
make -j2 test
make production
file build-gcc49/df_bridge_r
ldd build-gcc49/df_bridge_r
```

Linux 生产构建（以 Debian/Ubuntu 包名为例）：

```bash
sudo apt-get install build-essential cmake default-libmysqlclient-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DDFBRIDGE_WITH_MYSQL=ON
cmake --build build -j2
```

在现有 `/opt/lampp` 开发环境也可只构建带 MySQL 的未改名目标：

```bash
make mysql
```

该目标为 `build-gcc49/df_bridge_r_mysql`；`make production` 将同一产物发布为
`build-gcc49/df_bridge_r`。

当前远端运行库为 MySQL 5.5.27 的 `libmysqlclient.so.18`。Makefile 默认从隔离目录
`/root/deps/mysql-5.5.27/include` 使用 Oracle 官方 5.5.27 开发头，并链接
`/opt/lampp/lib/libmysqlclient.so.18`；第三方源码不复制进本目录。其他系统可安装
`mysql-devel`、`default-libmysqlclient-dev` 或 `mariadb-connector-c-devel`，再覆盖
`MYSQL_CPPFLAGS/MYSQL_LDFLAGS`。CMake 查找 `mysql.h` 与 `libmysqlclient`/`libmysql`。

## 验证权威 0828 文件

```powershell
.\build\Release\df_bridge_r.exe validate "..\..\channel\0828\channel_info.etc"
```

预期摘要：

```text
dungeons=21 dungeon_ids=130 channels=475 server_groups=12
groups=... kinds=... zombie_channels=<类型 19 启用行数> raid_channels=<类型 23 启用行数>
```

## 从数据库生成一次

生产构建启用 MySQL 后，用环境变量传递密码，避免出现在命令行历史或日志里：

```bash
export DBRIDGE_DB_PASSWORD='...'
./df_bridge_r db-once 127.0.0.1 d_channel game DBRIDGE_DB_PASSWORD ./cfg/channel_info.etc
```

长期运行入口会先调用 `ReloadOnce()` 建立可服务快照，再调用 `Start()` 做 4 秒热更新。
语义等价时 `ReloadOnce()` 保留已有原始字节；不等价时才采用同目录临时文件后改名，
Linux 上不会让游戏服读到半写入文件。两条路径都在 reload callback 成功后才提交新的
cache 版本和 DB version。

## 协议兼容性

所有结构均 `pack(1)` 并有编译期大小断言：

| 方向 | category/id | 大小/内容 |
|---|---|---|
| CS | 123/4 | 35 字节，频道服务注册 |
| CS | 123/5 | 27 字节，脚本版本检查 |
| SC | 124/6 | 31 字节，版本检查结果 |
| CS | 123/9 | 11 字节，请求脚本 |
| SC | 124/10 | 11 字节头 + 原始脚本文本 |
| CS | 123/13 | 11 字节，请求 GC 表 |
| SC | 124/14 | 15 字节头 + N×24 字节 GC 条目 |
| CS Extra | 123/2 | 51 字节，游戏频道状态更新 |

版本比较保留原程序的前缀行为；GC 条目仍是 `char group_name[20] + int32 gc_no`。
网络层按原版只使用 header length 分帧，不拒绝 checksum/ack 的任意值；本服务发出的
响应将二者置零。

## 目录

- `include/dfbridge/`：恢复的数据模型、协议、注册表、Repository 接口。
- `src/channel_info.cpp`：频道文件解析与生成。
- `src/config.cpp`：原版 `[server]/[DB]` 配置解析与校验。
- `src/mysql_repository.cpp`：只在 `DFBRIDGE_WITH_MYSQL=ON` 时编译的 d_channel 适配器。
- `src/script_thread.cpp`：版本轮询与热更新。
- `src/bridge_core.cpp`：GC 映射和频道注册。
- `src/service.cpp`：CLI、daemon/PID、信号、日志、TCP/UDP 和状态广播。
- `reverse/ida_evidence.md`：IDA 地址、结构大小和伪代码到源码的对应证据。
- `tests/bridge_tests.cpp`：type 0..20/23、21/22 拒绝、协议尺寸、GC 3186 的回归测试。
