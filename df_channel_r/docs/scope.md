# Scope

- auth: 用户提供并授权分析的本地 DNF 0828 服务端组件。
- in_scope: `artifacts/channel_0828/df_channel_r`、`channel_info_0828_authoritative.etc`、DNF12 CN 客户端频道握手消息 17/18、恢复业务核心及 `df_channel_r/` 的可替换 POSIX 守护进程外壳。
- out_of_scope: 第三方 OpenSSL、zlib、pthread/POSIX socket 内部源码；主工程其他目录。
- network_profile: 离线静态分析；本机只做核心回归，ELF32 生命周期与网络集成验证在用户自有 Linux 服务端完成。本次消息 17/18 子任务不修改或部署远端文件。
