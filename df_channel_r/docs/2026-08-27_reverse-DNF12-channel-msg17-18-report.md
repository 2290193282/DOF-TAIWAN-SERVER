# DNF12 CN 频道消息 17/18 逆向与实现报告

> 分析日期：2026-08-27  
> 工具链：IDA Pro 9.0、Hex-Rays、MSVC 19.44、OpenSSL 3.0.12、zlib 1.2.13、CMake/CTest  
> 状态：客户端 wire 已恢复，`df_channel_r` 实现及本地回归已通过；远端部署不在本子任务范围

## 执行摘要

DNF12 客户端在完成旧消息 11/12 与 9/10 后新增消息 17/18。原
`df_channel_r` 仅注册到消息 14，收到 17 后返回“不支持”并关闭连接，所以客户端日志停在
`recvPacketData(): FAILED TO RECV(0/11)`。本次已恢复精确 wire、修正 CN 大区枚举、实现服务端响应并完成压缩加密回环测试。

最关键的兼容性修正是：CN 大区 1 的 `parser_server_group` 是 `1`，wire 名为
`cain`；`265` 实际是 `SERVER_GROUP_TYPE_MAX` 哨兵，客户端会主动拒绝。

## Scope 摘要

- 授权与范围：[scope.md](scope.md)
- in_scope：DNF12 客户端频道消息 17/18、`df_channel_r` 正式实现、CN 频道配置与本地回归。
- out_of_scope：远端部署、PVF 解析器、登录、Raid 业务包和第三方库内部实现。
- network_profile：本地静态逆向和回环测试；没有修改远端文件。

## 恢复的协议

### C2S 消息 17

`0xAB5210` 构造固定 35 字节帧：

```text
offset  size  field
0x00    11    ChannelPacketHeader (message_id=17, size=35)
0x0B    20    char server_group_name[20]
0x1F     4    uint32 parser_server_group
```

`0x842F10` 从 parser 对象 `+0x88` 取枚举；`0xFD4690` 映射 wire 名称。静态字符串解密确认：

- `1 -> cain`
- `2 -> diregie`
- `265 -> SERVER_GROUP_TYPE_MAX`

### S2C 消息 18

`0xAB5700` 先读 11 字节头，要求 `ack >= 1` 与 `message_id == 18`，然后：

1. 读取最多 `0x20000` 字节 zlib body；
2. 解压到最多 `0x40000` 字节；
3. 校验长度为 Channel cipher block size 的倍数；
4. 解密并对包含零填充的完整明文算 CRC32；
5. 把明文交给 `0xAB6B40` 解析。

明文精确格式：

```text
offset  size  field
0x00     2    uint16 fallback_server_group
0x02     4    uint32 channel_count
0x06    48*N  ChannelServerInfo18Wire[channel_count]
```

每个 48 字节记录为：

```text
char channel_name[20]
uint32 max_user_num
uint32 cur_user_num
char server_ip[16]
uint32 port
```

客户端只解析一个服务器组。`0xAB6CBB` 在读频道数前明确拒绝枚举 265。

## 字符串表边界

消息 18 不是新的 `dname/chn_channel_info` 字符串表。`0xAB5A70` 从 20 字节
`channel_name` 提取十进制频道号，随后客户端用本地 PVF 频道表解析显示名。

权威来源是：

- `D:\BaiduNetdiskDownload\12\PVF_FILE\etc\channel_info.etc`
- SHA256：`037C077A9A20D4C1853A74C4B92418AD509E5EEB61105E1FBE01ACE45684B0E7`

因此服务端写 `#ch.11`、`#ch.200` 这一类可提取数字的引用，中文地下城名和频道名继续由客户端本地
`channel_info_dname_*` / `chn_channel_info_*` 数据负责。把 PVF 文本直接塞进消息 18 会破坏客户端的固定二进制解析。

## 正式实现

实现位置：

- `include/channel/protocol.hpp`：消息 17/18 与 35/6 字节 wire 结构；
- `include/channel/channel_service.hpp`：新增正式处理入口；
- `src/channel_service.cpp:116`：验证请求、按 `cain` 选择动态 registry group、序列化单组频道、Channel codec 加密与 zlib 压缩；
- `tests/channel_core_test.cpp:382`：完整成功路径和失败门回归。

失败门包括：

- `parser_server_group == 0` 或 `>= 265`；
- 20 字节大区名没有 NUL；
- 大区名为空、重复或当前 registry 不存在；
- 编码后的压缩 body 超过客户端 `0x20000` 上限。

## Evidence

### E-001

- title：DNF12 客户端消息 17/18 wire
- observed_at：2026-08-27
- source_type：IDA database
- source_ref：`DNF_12_clean.exe.i64`，`0xAB5210`、`0xAB5700`、`0xAB6B40`
- content_hash：n/a（IDA 工作数据库已保存）
- repro_command：在 IDA 中反编译上述地址并查看已保存的函数名、类型和行注释
- raw_excerpt：`msg17 size=35`；`msg18 plaintext=u16+u32+N*48`
- linked_workitem：n/a
- supersedes：none

### E-002

- title：CN parser server group 映射
- observed_at：2026-08-27
- source_type：IDA database
- source_ref：`0x842F10`、`0xFD4690`、`0xAB6CBB`
- content_hash：n/a
- repro_command：反编译 `ParserSession_GetServerGroupType`、`ServerGroupType_ToWireName`、`ChannelManager_ParseDownloadedChannelList`
- raw_excerpt：`1=cain`；`265=SERVER_GROUP_TYPE_MAX`；`parser_server_group == 265` 进入失败分支
- linked_workitem：n/a
- supersedes：none

### E-003

- title：本地 PVF 频道显示名来源
- observed_at：2026-08-27
- source_type：file
- source_ref：`D:\BaiduNetdiskDownload\12\PVF_FILE\etc\channel_info.etc`
- content_hash：`037C077A9A20D4C1853A74C4B92418AD509E5EEB61105E1FBE01ACE45684B0E7`
- repro_command：`Get-FileHash -Algorithm SHA256 D:\BaiduNetdiskDownload\12\PVF_FILE\etc\channel_info.etc`
- raw_excerpt：大区 1 频道行及中文地下城/频道名称
- linked_workitem：n/a
- supersedes：none

### E-004

- title：`df_channel_r` Release 编译与回环测试
- observed_at：2026-08-27
- source_type：command
- source_ref：`migration/dnf12_cn/evidence/client_channel_protocol/msg17_18/2026-08-27_msg17_18_validation.md`
- content_hash：n/a
- repro_command：按证据文件中的 CMake、build 与 CTest 命令执行
- raw_excerpt：`100% tests passed, 0 tests failed out of 1`
- linked_workitem：n/a
- supersedes：none

## Findings

### F-001

- title：旧 `df_channel_r` 缺少 DNF12 消息 17/18
- severity：n/a_re
- category：reverse_algo
- status：validated
- evidence_ids：E-001
- location：`df_channel_r/src/channel_service.cpp:35-39`
- impact：客户端在消息 17 后被断开，无法取得频道。
- confidence：high
- remediation：注册并实现精确消息 17/18 wire；已完成。

### F-002

- title：265 是不可用的 MAX 哨兵
- severity：n/a_re
- category：design
- status：validated
- evidence_ids：E-002
- location：`DNF.exe:0xFD4690,0xAB6CBB`
- impact：即便服务端返回正确消息 18，parser group 265 仍会由客户端主动判失败。
- confidence：high
- remediation：CN 大区 1 使用 `parser_server_group=1` 与 `cain`；配置已修正。

### F-003

- title：频道显示字符串由本地 PVF 解析
- severity：n/a_re
- category：reverse_algo
- status：validated
- evidence_ids：E-001,E-003
- location：`DNF.exe:0xAB5A70,0xAB6DCD`
- impact：误把 PVF 字符串表当消息 18 payload 会造成二进制布局错误。
- confidence：high
- remediation：消息 18 仅发送数字频道引用与运行时端点；保持 PVF 所有权。

### F-004

- title：正式实现通过 wire、加密和失败门回归
- severity：n/a_re
- category：other
- status：validated
- evidence_ids：E-004
- location：`df_channel_r/src/channel_service.cpp:116-185`
- impact：本地核心已具备 DNF12 消息 17/18 处理能力。
- confidence：high
- remediation：在最终统一部署阶段进入 Linux 全树编译与真实客户端回归。

## Path

### P-001

- title：DNF12 CN 频道获取调用路径
- path_type：callflow
- start：parser session `+0x88`
- goal：客户端建立可用频道对象
- steps：
  1. action：`0x842F10` 读取 group 1，`0xFD4690` 映射 `cain` — evidence：E-002 — finding：F-002
  2. action：`0xAB5210` 发送 35 字节消息 17 — evidence：E-001 — finding：F-001
  3. action：`df_channel_r` 按 `cain` 选择动态 group，序列化 `u16+u32+N*48` — evidence：E-004 — finding：F-004
  4. action：服务端 AES/Channel codec 加密并 zlib 压缩为消息 18 — evidence：E-001,E-004 — finding：F-004
  5. action：客户端解压解密，经 `0xAB6B40` 解析记录，再用本地 PVF 按频道号取显示名 — evidence：E-001,E-003 — finding：F-003
- residual_risks：仍需在最终 Linux 二进制及真实 DNF12 客户端上验证频道 11、200 的端到端运行态。

## 验证结果

```text
Release build: PASS
df_channel_r executable build: PASS
channel_core_test: PASS
CTest: 1/1 PASS (0 failed)
region wiring validation: PASS (8/8)
IDA IDB save: PASS
```

测试覆盖成功路径、AES 零填充、zlib 回环、两个 48 字节记录，以及 265 哨兵、错误大区名、未终止大区名三个失败路径。

## Timeline 摘要

完整记录见 [timeline.md](timeline.md)。

- 2026-08-27：恢复消息 17/18 wire 和压缩加密顺序；
- 2026-08-27：确认 `1 -> cain`、`265 -> SERVER_GROUP_TYPE_MAX`；
- 2026-08-27：实现服务端、保存 IDB、完成 Release/CTest/区域接线验证。
