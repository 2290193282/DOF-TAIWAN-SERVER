# Timeline

- 2026-08-18：确认 ELF32 i386，保留 `.symtab` 和完整 DWARF。
- 2026-08-18：恢复 11 字节包头、消息 1–14 和频道/GC wire 结构。
- 2026-08-18：恢复脚本版本、完整脚本下发、Bridge GC 同步与频道列表流程。
- 2026-08-18：增加 0828 type 19/20 语义和 group 17 Zombie 启动回退。
- 2026-08-18：产出 GNU++11 核心、Makefile/CMake 和回归测试。
- 2026-08-18：补齐原版 CLI、七键配置、daemon/PID/信号、四级日志和可执行入口。
- 2026-08-18：补齐 TCP/UDP、Bridge 重连与转发、GC-ready 门、40 秒淘汰和每日 AES 换钥。
- 2026-08-18：增加 authoritative 21/130/475 校验与真实进程 Mock Bridge 集成测试；等待目标机单线程验收。
- 2026-08-18：替换前 P1 收口：单快照 ASK、SC14 结构校验/5 秒门、PID 身份锁、15 字节 IP 与崩溃回滚恢复。
- 2026-08-18：隔离 smoke 证实 SC14 为 VARCHAR wire 顺序；撤销数值递增门并固定回归 3186 首命中 4000。
- 2026-08-27：从 DNF12 CN 客户端恢复消息 17 的 35 字节请求，以及消息 18 的 zlib/AES 响应和 `u16 + u32 + N*48` 明文布局。
- 2026-08-27：确认 parser server group `1 -> cain`，`265 -> SERVER_GROUP_TYPE_MAX` 且客户端拒绝 265；在 `df_channel_r` 实现 17/18、错误门和压缩加密回环测试。
