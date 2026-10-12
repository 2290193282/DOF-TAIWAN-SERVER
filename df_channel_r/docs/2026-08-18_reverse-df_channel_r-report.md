# df_channel_r 0828 逆向恢复与可替换守护进程报告

> 分析日期：2026-08-18  
> 目标：`artifacts/channel_0828/df_channel_r`  
> 工具链：IDA 9.1 MCP（现有 GUI 会话）、pyelftools、Capstone、DWARF、CMake/MSVC 配置检查

## Scope 摘要

本次仅恢复用户提供的 Channel 服务业务核心，范围和第三方库边界见 [scope.md](scope.md)。时间记录见 [timeline.md](timeline.md)。

## 目标概述

| 属性 | 值 |
|---|---|
| 文件类型 | ELF32 i386, ET_EXEC, 有符号、含 DWARF |
| 文件大小 | 1,305,013 字节 |
| SHA-256 | `7f7f6bd9eea6452dc463bca2f2e33f07eec7675e9f37e280e92739cfb87747bd` |
| 原编译目录 | `/home/neople/source/ChannelOld/DNFChannelServer` |
| 核心编译单元 | `ChannelService.cpp`, `CheckThread.cpp`, `Script.cpp`, `ScriptData.cpp`, `Rijndael.cpp` |

## Evidence

### E-001

- title: DWARF wire 结构
- observed_at: 2026-08-18
- source_type: file
- source_ref: `artifacts/channel_0828/df_channel_r`
- content_hash: `7f7f6bd9eea6452dc463bca2f2e33f07eec7675e9f37e280e92739cfb87747bd`
- repro_command: `python df_channel_r/tools/dwarf_inspect.py artifacts/channel_0828/df_channel_r --types '^(tagCS_.*|tagSC_.*|tServer.*|tpServerInfo)$'`
- raw_excerpt: `tagPacketHeader=11, tagCS_UPDATE_CHANNEL_INFO=51, tServerGcInfo=24, tpServerInfo=48`
- linked_workitem: n/a
- supersedes: none

### E-002

- title: 协议注册表
- observed_at: 2026-08-18
- source_type: command
- source_ref: `ChannelService::registerProtocols @ 0x0804DC78`
- content_hash: n/a
- repro_command: `python df_channel_r/tools/dwarf_inspect.py artifacts/channel_0828/df_channel_r --functions 'registerProtocols' --disasm`
- raw_excerpt: `1,2,3,5,6,9,10,11,14 handlers; CheckThread constructors prove 4 and 13`
- linked_workitem: n/a
- supersedes: none

### E-003

- title: 固定脚本与版本路径
- observed_at: 2026-08-18
- source_type: command
- source_ref: `ChannelScript::ReloadScript @ 0x0805210A`, `CheckThread::loop @ 0x08060E76`
- content_hash: n/a
- repro_command: `python df_channel_r/tools/dwarf_inspect.py artifacts/channel_0828/df_channel_r --functions 'ChannelScript12ReloadScript|CheckThread4loop' --disasm`
- raw_excerpt: `channel_info/channel_info.etc`, `channel_info/version`
- linked_workitem: n/a
- supersedes: none

### E-004

- title: 0828 频道文件
- observed_at: 2026-08-18
- source_type: file
- source_ref: `artifacts/channel_0828/channel_info_0828_authoritative.etc`
- content_hash: `7c86d75b627f8939241d14e5e1aa78b4d5784d7412efc38e4d19e9fb28522d79`
- repro_command: `./df_channel_r/channel_core_test artifacts/channel_0828/channel_info_0828_authoritative.etc`
- raw_excerpt: `完整 [dungeon]/[server] 表按原始字节加载并校验`
- linked_workitem: n/a
- supersedes: none

### E-005

- title: 原版进程与运维契约
- observed_at: 2026-08-18
- source_type: command
- source_ref: `artifacts/channel_0828/df_channel_r`
- content_hash: `7f7f6bd9eea6452dc463bca2f2e33f07eec7675e9f37e280e92739cfb87747bd`
- repro_command: `python df_channel_r/tools/dwarf_inspect.py artifacts/channel_0828/df_channel_r --functions 'main$|processCommandLine|load_script|LinuxService5start|save_pid|registerSignalHandlers' --disasm`
- raw_excerpt: `./cfg/%s.cfg`; `start/run/test/stop/pause`; `./pid/%s.pid`; fork/setsid/umask；SIGTERM/SIGUSR1/SIGPIPE。
- linked_workitem: n/a
- supersedes: none

### E-006

- title: 原版网络与维护循环契约
- observed_at: 2026-08-18
- source_type: command
- source_ref: `TCPAcceptThread::loop @ 0x0805962E`, `TCPThread::loop @ 0x0805A3D0`, `UDPThread::loop @ 0x0805F6CA`, `CheckThread::loop @ 0x08060E76`
- content_hash: n/a
- repro_command: `python df_channel_r/tools/dwarf_inspect.py artifacts/channel_0828/df_channel_r --functions 'TCPAcceptThread4loop|TCPThread4loop|UDPThread4loop|CheckThread4loop' --disasm`
- raw_excerpt: TCP/UDP bind INADDR_ANY；Bridge 连接后发 13、每 20 秒发 5/4、每秒重连；TCP 上限 `0xA0000`；Game 状态差值大于 40 秒删除；日期变化调用 `createEncKey`。
- linked_workitem: n/a
- supersedes: none

### E-007

- title: 可替换守护进程实现与测试入口
- observed_at: 2026-08-18
- source_type: file
- source_ref: `src/channel_daemon.cpp`, `src/daemon_config.cpp`, `tests/channel_daemon_integration_test.cpp`, `Makefile`
- content_hash: n/a
- repro_command: `cd df_channel_r && make clean all && make test CHANNEL_INFO=../artifacts/channel_0828/channel_info_0828_authoritative.etc`
- raw_excerpt: 构建 `df_channel_r`；Mock Bridge 覆盖 13/14、5/6、4、TCP 转发 id2、UDP bind/无分发、Game CONNECT 与 1/3。
- linked_workitem: n/a
- supersedes: none

## Findings

### F-001

- title: df_channel_r 不从数据库生成 channel_info.etc
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-003]
- location: `0x0805210A`
- impact: 工作目录中的 `channel_info/channel_info.etc` 是 Channel 服务实际下发源；Bridge 版本不一致时会覆盖它。
- confidence: high
- repro_steps: 运行 E-003 命令并检查 `fopen("channel_info/channel_info.etc", "rb")`。
- remediation: 将 0828 文件放在正确工作目录，并同步 version。

### F-002

- title: Channel 同时服务游戏频道并向 Bridge 同步权威脚本
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-001, E-002, E-003]
- location: `0x0805029C-0x080510F3`, `0x08060E76`
- impact: 仅启动 game 而没有 Bridge/GC 响应时，频道更新无法可靠映射到大区；脚本也不会自动升级。
- confidence: high
- repro_steps: 对照消息 13/14、5/6、9/10 和 4 的构造/处理函数。
- remediation: 启动 Bridge 或使用兼容协议模拟器；保持 GC 和脚本版本响应完整。

### F-003

- title: 0828 type 19 与 type 20 需要不同 PvP 语义
- severity: n/a_re
- category: design
- status: validated
- evidence_ids: [E-004]
- location: `include/channel/protocol.hpp`, `src/protocol.cpp`
- impact: 将 Zombie 19 当普通 PvP 会进入错误分支；遗漏 type 20 会使新增联赛频道不可用。
- confidence: high
- repro_steps: 运行 `channel_core_test` 的类型矩阵断言。
- remediation: 保持 type 19 独立、将 type 20 加入 PvP 集合、拒绝 21 哨兵。

### F-004

- title: 旧交付只有业务核心，不能替换原守护进程
- severity: n/a_re
- category: design
- status: validated
- evidence_ids: [E-005, E-006]
- location: 旧版 `CMakeLists.txt`/`Makefile` 与原 ELF 生命周期函数
- impact: 没有可执行入口、监听、PID、信号、日志、重连、拆包和维护定时器，直接部署只会得到测试工具。
- confidence: high
- repro_steps: 对比原 ELF 的 `main/LinuxService/startup/CheckThread` 与旧构建目标。
- remediation: 以 POSIX 外壳复刻可观察契约并保持业务核心独立。

### F-005

- title: 新守护外壳覆盖直接替换所需契约
- severity: n/a_re
- category: design
- status: candidate
- evidence_ids: [E-005, E-006, E-007]
- location: `src/channel_daemon.cpp`, `src/daemon_config.cpp`
- impact: 已形成名为 `df_channel_r` 的可执行目标，并覆盖原 CLI、运行目录、TCP/UDP、Bridge、维护与关闭路径。
- confidence: medium
- repro_steps: 在目标机执行 E-007，随后检查 `file/readelf/ldd` 并用隔离端口运行集成测试。
- remediation: 只有目标机 GCC 4.9/i386、ABI 和隔离网络验收全通过后，才把状态升级为 validated 并替换线上二进制。

## Path

### P-001

- title: Bridge 到客户端频道表的数据流
- path_type: callflow
- start: Channel 连接 Bridge
- goal: 游戏客户端收到 0828 完整频道表和运行频道列表
- steps:
  1. action: Channel 发送 13 获取 GC 分组 — evidence: E-002 — finding: F-002
  2. action: Channel 发送 5 比较本地 version — evidence: E-003 — finding: F-002
  3. action: 版本不同时发送 9，接收 10 并完整写入脚本 — evidence: E-003 — finding: F-001
  4. action: game 用消息 2 上报运行实例，Channel 按 GC/组保存 — evidence: E-001 — finding: F-002
  5. action: game 用消息 1 请求列表，Channel 返回消息 3（AES 后 zlib） — evidence: E-001 — finding: F-003
- residual_risks: 业务数据流已接入新守护外壳；剩余风险是目标机 ELF32 运行库解析与真实 Bridge/Game 隔离端口验证。

### P-002

- title: 原运维命令到 0828 频道发布的数据流
- path_type: callflow
- start: `./df_channel_r <name> start`
- goal: Zombie 186 作为 type 19 出现在 Game 可用频道链
- steps:
  1. action: 读取 `cfg/<name>.cfg`、脚本和 version，fork 后锁定 `pid/<name>.pid` — evidence: E-005,E-007 — finding: F-004,F-005
  2. action: bind TCP/UDP，连接 Bridge 并以 13 获取 GC 权威边界 — evidence: E-006,E-007 — finding: F-005
  3. action: 每 20 秒发送 5/4，处理 14/6/10 和 Bridge TCP 转发 id2 — evidence: E-002,E-006,E-007 — finding: F-002,F-005
  4. action: Bridge TCP 转发 id2 在 GC-ready 后注册 3186，并在超过 40 秒未刷新时淘汰；UDP 仅绑定和排空 — evidence: E-006,E-007 — finding: F-005
  5. action: Game 完成 CONNECT 后通过 1/3 获取 AES+zlib 频道列表，并通过 5/9 获取版本 60 全量脚本 — evidence: E-001,E-004,E-007 — finding: F-001,F-003,F-005
- residual_risks: 目标机依赖版本、supervisor 实际命令和真实流量仍需发布前验收。

## 实现映射

| 原函数/地址 | 恢复源码 |
|---|---|
| `registerProtocols @ 0x0804DC78` | `protocol.hpp`, `ChannelService::HandleGameFrame` |
| `onCS_ASK_CHANNEL_INFO @ 0x0804EE64` | `ChannelRegistry::SerializeAskChannelBody`, `HandleAskChannelInfo` |
| `onCS_UPDATE_CHANNEL_INFO @ 0x0804F786` | `ChannelRegistry::ApplyChannelUpdate` |
| `onCS_CHECK_SCRIPT_VERSION @ 0x0805029C` | `HandleCheckScriptVersion` |
| `onCS_GET_SCRIPT @ 0x080505CE` | `HandleGetScript` |
| `onCS_CONNECT @ 0x08050A5E` | `HandleConnect` |
| `onSC_GET_GC_INFO @ 0x08050C74` | `ApplyGcInfoReply` |
| `onSC_CHECK_SCRIPT_VERSION @ 0x08050D70` | `BridgeProtocolClient::HandleFrame` |
| `onSC_GET_SCRIPT @ 0x08050F82` | `ScriptStore::ReplaceFromBridge` |
| `CheckThread::loop @ 0x08060E76` | `BuildInitialFrames`, `PosixBridgeConnection` |
| `main/LinuxService @ 0x08049EB4-0x0804BC90` | `ChannelDaemonMain`, PID/信号/daemon 控制 |
| `TCPAcceptThread/TCPThread/UDPThread` | `ChannelRuntime::PollOnce`, TCP sessions, UDP id2 |
| `CheckThread` 40 秒清理/每日换钥 | `ExpireStale`, `LegacyCodec::Rotate` |

## 复现和构建

```bash
cd df_channel_r
./build.sh
```

静态证据工具依赖本机 `pyelftools` 和 `capstone`：

```bash
python tools/dwarf_inspect.py ../artifacts/channel_0828/df_channel_r \
  --functions 'on(CS|SC)_(ASK_CHANNEL_INFO|UPDATE_CHANNEL_INFO|CHECK_SCRIPT_VERSION|GET_SCRIPT|CONNECT|GET_GC_INFO)'
```

## 验证状态

- packed 结构尺寸：编译期 `static_assert`。
- 0828 type 19/20 和 21 哨兵：单元断言。
- GC 24 字节记录、Zombie `group=17/gc=3186/channel=186`：单元断言。
- Bridge 13/5/4 初始序列及 6→9→10 脚本同步：单元断言。
- AES-128-ECB 零填充 round-trip、游戏 CONNECT、频道列表响应：单元断言。
- Windows CMake 已编译业务核心与新增可执行入口；`channel_core_test` 使用权威文件验证 `21/130/475`、group 17 的 186/187/188 type 19、Bridge 转发 id2、严格 GC 门和 40 秒淘汰，结果 `PASS`。
- Linux `channel_daemon_integration_test` 已写入构建图；由于本机没有 WSL 发行版且操作者要求降低本机负载，GCC 4.9/i386 编译与真实进程测试必须在目标机单线程执行，未在本机冒充通过。
- Bridge writer 频道行回归覆盖 `exp_rate=0.0`：该字段与后续十个经验修正值统一按有限浮点数解析，避免消息 6→9→10 热更在 `ReplaceFromBridge` 阶段被整数解析器拒绝。
- 替换前 P1 回归增加同一 snapshot 的 ASK count/body 序列化、SC14 空 count/空名/非正值拒绝、每连接 5 秒 GC-ready 门、PID flock+`/proc/<pid>/exe` 身份校验，以及缺失/损坏主 pair 的固定 rollback 恢复。隔离 smoke 进一步证实真实 Bridge 因 VARCHAR 排序发送 `10000,100000,2000,...`；Channel 现保留该 wire 顺序并回归验证 `gc_no=3186` 首个命中 `upper_gc=4000`。

## 边界

恢复结果现包含可执行 POSIX 守护外壳，但不复制旧第三方框架内部源码。实现用 `poll`、标准线程、文件锁、OpenSSL 和 zlib 复刻 CLI、路径、wire 与时序等可观察契约。Channel 不连接 MySQL；目标机必须提供 32 位 OpenSSL/zlib/pthread/C++ 运行库。正式替换的最后门闩是远端 `make test`、`readelf/ldd` 和隔离端口烟测。
