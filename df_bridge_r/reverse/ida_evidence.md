# df_bridge_r IDA 恢复证据

样本：`artifacts/channel_0828/df_bridge_r`

- SHA-256：`30a5741015b3701cee2d9b388140ac1711345bbe8d879b6f152e74a9334ea6b0`
- ELF 32-bit x86，有符号；IDA 9.1 当前数据库识别 4,123 个函数。
- 本文件仅记录 Neople 业务代码证据；MySQL/STL/线程等库函数没有反编译复制。

## 数据库到 channel_info

`ChannelServiceApp::ScriptThread::loop(void*)`：`0x08063936`，大小 `0xb40`。

关键行为：

1. `select * from ch_script_version`，把第一列保存为当前版本。
2. `select group_name, group_gc_no from ch_gc_info order by group_gc_no asc`，构造 GC 表。
3. 写 `./cfg/channel_info.etc`。
4. `select kind_name, dungeon_name from ch_dungeon_data`，再按 `kind_name` 查询 `ch_dungeon_list`。
5. `select * from ch_server_data where is_use='1' order by server_id,channel_number`。
6. 原二进制的 `SELECT *` 下标映射：`row[3]=channel_number`、`[4]=channel_name`、`[5]=channel_kind`、`[6]=kind_name`、`[7]=exp_rate`、`[8..17]=exp_1..exp_10`。
7. 调用 `ChannelScript::ReloadScript()` 后，每 `0x3d0900` 微秒（4 秒）检查版本，变化后重新生成。

恢复实现位于 `src/channel_info.cpp`、`src/script_thread.cpp` 和 `src/mysql_repository.cpp`。改用显式字段名，消除表列顺序耦合；输出的 15 个业务字段不变。

## 脚本缓存与热更新

`ChannelServiceApp::ChannelScript::ReloadScript()`：`0x08056efe`。

- 读取 `cfg/channel_info.etc`。
- 原缓冲区大小 `0x100000`（1 MiB）。
- 去除末尾 CRLF/LF，脚本响应不附加 NUL。

恢复实现用 `std::string` 消除固定缓冲区越界，并通过临时文件改名发布，线上的脚本字节语义不变。

## 协议分发表

`registerProtocols()`：`0x08053f4c`。

- id 4 → `onCS_NOTICE_CHANNEL_SERVER`
- id 5 → `onCS_CHECK_SCRIPT_VERSION`
- id 9 → `onCS_GET_SCRIPT`
- id 13 → `onCS_GET_GC_INFO`

`registerProtocolsExtra()`：`0x08054040`。

- Extra id 2 → `onCS_UPDATE_CHANNEL_INFO`

恢复表位于 `RestoredProtocolTable()`。

## 线结构

IDA 类型系统直接给出的 `pack(1)` 布局：

- `tagPacketHeader`：11 字节；`u8 category, u8 id, u32 length, u32 checksum, u8 ack`。
- `tagCS_CHECK_SCRIPT_VERSION`：27 字节；头 + `char version[16]`。
- `tagSC_CHECK_SCRIPT_VERSION`：31 字节；头 + `int valid` + `char version[16]`。
- `tagSC_GET_GC_INFO`：15 字节；头 + `int count`。
- `tServerGcInfo`：24 字节；`char group_name[20] + int gc_no`。
- `tagCS_UPDATE_CHANNEL_INFO`：51 字节；头 + 5 个 int + `char ip[16]` + port int。
- `tagCS_NOTICE_CHANNEL_SERVER`：35 字节；头 + id int + `char ip[16]` + port int。

构造函数证据：

- `tagSC_CHECK_SCRIPT_VERSION::ctor` `0x0805699a` → category 124、id 6、size 31。
- `tagSC_GET_SCRIPT::ctor` `0x080569e6` → category 124、id 10、size 11。
- `tagSC_GET_GC_INFO::ctor` `0x08056a32` → category 124、id 14、size 15。
- `tagCS_UPDATE_CHANNEL_INFO::ctor` `0x080636c0` → category 123、id 2、size 51。

`onCS_GET_SCRIPT()` 在 `0x08055ffe`：先写 11 字节 SC 头，再附加 `getScriptFileSize()` 个原始字节，最后由 `PAD()` 把头长度改成总长度。`PAD()` 在 `0x08056aaa`，并不添加对齐字节。

## GC 映射与频道注册

`onCS_UPDATE_CHANNEL_INFO()`：`0x08054e76`。

- 只要 `gc_no > 0`，遍历 GC 表。
- 首个满足 `group_gc_no - gc_no > 0 && <= 999` 的组命中。
- 包中的 `group` 字段不参与这一步；所以 `server_group=17` 与 `gc_no=3186` 不能互相替代。
- 每组最多 4,096 个游戏频道状态，以 `gc_no` 更新/插入。

`onCS_NOTICE_CHANNEL_SERVER()`：`0x080555c6`，最多登记 256 个频道服务端点，按 id 更新。

`onCS_GET_GC_INFO()`：`0x080561cc`，序列化 `ch_gc_info`，并非从 `channel_info.etc` 推导。

## 版本检查

`onCS_CHECK_SCRIPT_VERSION()`：`0x08055d9a`。

原代码使用：

```cpp
strncmp(current, peer, strlen(current)) == 0
```

因此恢复代码保留“当前版本是对端版本前缀即有效”的兼容行为，同时对版本长度加 15 字节上限，避免原实现向 16 字节响应字段溢出。

## 可替换守护进程外部契约

下列地址来自同一原始 ELF，并用于约束 `src/service.cpp`，但恢复实现不复制旧通用类 ABI：

- `main` `0x08050134` → `LinuxService::processCommandLine` `0x0805183c`：
  `df_bridge_r <cfg-name> start|run|test|stop`；实例名同时决定
  `./cfg/<name>.cfg` 和 `./pid/<name>.pid`。
- `LinuxService::start` `0x08051cac`：`start` 单次 `fork`、`setsid`、
  `chdir("./")`、`umask(0)` 并保存 PID；`run` 留在前台。
- `Neof_registerSignalHandlers` `0x0805102c`：SIGTERM 控制退出，SIGPIPE 忽略，
  crash/core 路径先设置 `RLIMIT_CORE`。
- `ChannelService::startup` `0x080548b2`：创建 Info/Warn/Error/Cri 四类文件日志，
  依次启动 Script、UDP、TCP accept、TCP reactor 和 Check 线程。
- `TCPAcceptThread::loop` `0x0805cf5a`：bind `INADDR_ANY`，listen backlog
  `0x400`；TCP/UDP 端口来自配置。
- `TCPUser::onRead_` `0x0805f0bc`：接收/重组上限 `0xa0000`，先等 11 字节
  `tagPacketHeader`，再按 header size 做流式分帧。
- `TCPHandlerRelay::dispatch` `0x0805d690` 与
  `UDPHandlerRelay::dispatch` `0x08060d8a`：普通表走 TCP，Extra id 2 走 UDP；
  两者都只读取 header byte +1 的 packet id，不按 byte +0 classification 拒绝，也不校验
  checksum。未知 id 的 handler 为 null，原 TCPUser 保持连接且静默丢弃该帧。
- `CMsgCell::PAD` `0x08056aaa`：只把 header size 改为实际总长，不添加对齐字节。
- `CheckThread::loop` `0x0806268e`：Game 超过 20 秒未更新即删除，Channel notice
  超过 200 秒即注销；每 3 秒把所有存活 Game 的 id 2 状态包通过该 Channel
  已存在的 TCP 会话发送，外发 `group` 固定为 0。

恢复实现对应关系：

- `src/config.cpp`：cfg/实例名契约。
- `src/service.cpp`：LinuxService 生命周期、四级日志、TCP/UDP poll 循环和 Check 广播。
- `src/protocol.cpp::FrameDecoder`：11 字节头、`0xa0000` 上限和连续/分片帧。
- `src/bridge_core.cpp::ExpireStale`：20 秒 Game 与 200 秒 Channel 过期。
- `src/script_thread.cpp` reload callback：DB 版本更新后同步 `ChannelRegistry` GC 表。

替换壳另外增加两项不改变 wire 的安全约束：PID 文件用 `flock` 持锁，stop 先核对
`/proc/<pid>/exe`；MySQL 后台线程用 Repository worker hook 成对调用
`mysql_thread_init/mysql_thread_end`。GC 表只有在排序后的名称或 upper boundary 发生
变化时才清空旧 Game 状态，并且 reload callback 成功后才提交新的 cache 版本。

## 权威 channel_info 字节保真

远端 DB 与权威 0828 文件已证明业务语义相同，但原 Bridge renderer 会改变顺序、CRLF、
inline `[server] N` 和尾部 sentinel，导致文件哈希变化。恢复实现因此在 ChannelInfo 层
增加格式无关比较：

- dungeon 以 normalized kind 为键，比较 display name 与 dungeon id 集合；
- channel 以 `(server_group, channel_number)` 为键，比较去外层 wrapper 的 name、type、
  normalized kind、exp rate 与 10 个 bonus；
- 忽略行/块顺序、空白/CRLF、inline/legacy server 格式、非业务 sentinel 与 inactive 行。

`ScriptThread::ReloadOnce()` 先解析现有输出。语义等价时不调用 writer/rename，cache 直接
使用现有文件的原始二进制字节，同时提交 DB version 与 GC groups；不等价时才 render
并原子发布。reload callback 仍在 cache 版本提交前完成，失败不会吞掉下一轮重试。

## Game → Bridge 7000 真实入口

生产隔离日志曾在 Game 接入时记录 `rejected malformed bridge frame`。恢复实现的
FrameDecoder 已按同一 header size 切出完整帧，因此该日志只能由额外的
`classification == 123` 条件触发。原 ELF 的证据如下：

- `TCPUser::onAccept @ 0x0805eea8` 只清空收发队列，没有认证前导或第二套帧格式；
- `TCPUser::onRead_ @ 0x0805f0bc` 只要求累计至少 11 字节，并从 `header+2` 读取 size；
- `TCPHandlerRelay::dispatch @ 0x0805d690` 只用 `getPacketID()`（byte +1）查表；
- `registerProtocols @ 0x08053f4c` 只注册 4/5/9/13；未知 1/11 不代理也不断线；
- 5/9/13 handler 均调用当前 `TCPUser::onWrite2Buffer`，直接在同一 Game 会话回复；
- `onCS_NOTICE @ 0x080555c6` 保存当前 Channel `TCPUser*`；CheckThread 通过该 fd 发 id 2；
- 原业务代码对 `TCPSocket::connect @ 0x08052cda` 没有调用点，不存在到 advertised
  Channel 7101 的 transparent relay。

因此兼容实现删除 TCP/UDP classification 门，保留 length 安全检查、按 id 分派和未知 id
静默保持连接；没有新增错误的 7000→7101 出站代理。

原始 ELF 是 i386 ET_EXEC，解释器 `/lib/ld-linux.so.2`；动态依赖中没有
`libmysqlclient`，因为旧 MySQL Connector/C 6.0.2 被静态并入。新生产构建改为目标机
现有 `libmysqlclient.so.18`，因此发布验收必须额外检查 ELF32、rpath、`ldd` 和
GLIBC/GLIBCXX 版本闭包。
