#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"

#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"

// IDA_EA: 0x8451BF6
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_LOAD_DNF_HACK_LOGEEPT_v
// DEMANGLED: SIG_LOAD_DNF_HACK_LOG * Stream::GetOutBuffer<SIG_LOAD_DNF_HACK_LOG>(void)
template <>
SIG_LOAD_DNF_HACK_LOG* Stream::GetOutBuffer<SIG_LOAD_DNF_HACK_LOG>() {
    const int bytes = sizeof(SIG_LOAD_DNF_HACK_LOG);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_DNF_HACK_LOG* result =
        reinterpret_cast<SIG_LOAD_DNF_HACK_LOG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452A84
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_GOBLIN_UPDATE_VALIDITYTIMEEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_VALIDITYTIME * Stream::GetOutBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>(void)
template <>
SIG_GOBLIN_UPDATE_VALIDITYTIME*
Stream::GetOutBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>() {
    const int bytes = sizeof(SIG_GOBLIN_UPDATE_VALIDITYTIME);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_GOBLIN_UPDATE_VALIDITYTIME* result =
        reinterpret_cast<SIG_GOBLIN_UPDATE_VALIDITYTIME*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x823B05A
// IDA_NAME: _ZN6Stream11GetInBufferI27SIG_DOUBLE_CHECK_GUILD_NAMEEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_NAME * Stream::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_NAME*
Stream::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>() {
    const int bytes = sizeof(SIG_DOUBLE_CHECK_GUILD_NAME);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_DOUBLE_CHECK_GUILD_NAME* const result =
        reinterpret_cast<SIG_DOUBLE_CHECK_GUILD_NAME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823B0C4
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_DOUBLE_CHECK_GUILD_ADDRESSEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_ADDRESS * Stream::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_ADDRESS*
Stream::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>() {
    const int bytes = sizeof(SIG_DOUBLE_CHECK_GUILD_ADDRESS);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_DOUBLE_CHECK_GUILD_ADDRESS* const result =
        reinterpret_cast<SIG_DOUBLE_CHECK_GUILD_ADDRESS*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8237948
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_DOUBLE_CHECK_GUILD_NAMEEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_NAME * CStreamGuard::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_NAME*
CStreamGuard::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>() {
    return stream_->GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>();
}

// IDA_EA: 0x823795E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_DOUBLE_CHECK_GUILD_ADDRESSEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_ADDRESS * CStreamGuard::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_ADDRESS*
CStreamGuard::GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>() {
    return stream_->GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>();
}

// IDA_EA: 0x84524A2
// IDA_NAME: _ZN6Stream12GetOutBufferI27SIG_DOUBLE_CHECK_GUILD_NAMEEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_NAME * Stream::GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_NAME*
Stream::GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>() {
    const int bytes = sizeof(SIG_DOUBLE_CHECK_GUILD_NAME);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_DOUBLE_CHECK_GUILD_NAME* const result =
        reinterpret_cast<SIG_DOUBLE_CHECK_GUILD_NAME*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84524F6
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_DOUBLE_CHECK_GUILD_ADDRESSEEPT_v
// DEMANGLED: SIG_DOUBLE_CHECK_GUILD_ADDRESS * Stream::GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>(void)
template <>
SIG_DOUBLE_CHECK_GUILD_ADDRESS*
Stream::GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>() {
    const int bytes = sizeof(SIG_DOUBLE_CHECK_GUILD_ADDRESS);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_DOUBLE_CHECK_GUILD_ADDRESS* const result =
        reinterpret_cast<SIG_DOUBLE_CHECK_GUILD_ADDRESS*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84537BC
// IDA_NAME: _ZN6Stream12GetOutBufferI36SIG_UPDATE_COMPOUND_EMBLEM_STATISTICEEPT_v
// DEMANGLED: SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC * Stream::GetOutBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>(void)
template <>
SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC*
Stream::GetOutBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>() {
    const int bytes = sizeof(SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC* result =
        reinterpret_cast<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453A7A
// IDA_NAME: _ZN6Stream12GetOutBufferI20SIG_CHECK_JOIN_GUILDEEPT_v
// DEMANGLED: SIG_CHECK_JOIN_GUILD * Stream::GetOutBuffer<SIG_CHECK_JOIN_GUILD>(void)
template <>
SIG_CHECK_JOIN_GUILD* Stream::GetOutBuffer<SIG_CHECK_JOIN_GUILD>() {
    const int bytes = sizeof(SIG_CHECK_JOIN_GUILD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_JOIN_GUILD* result =
        reinterpret_cast<SIG_CHECK_JOIN_GUILD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845591C
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_LOAD_DNF_HACK_LOGEEPT_v
// DEMANGLED: SIG_LOAD_DNF_HACK_LOG * Stream::GetInBuffer<SIG_LOAD_DNF_HACK_LOG>(void)
template <>
SIG_LOAD_DNF_HACK_LOG* Stream::GetInBuffer<SIG_LOAD_DNF_HACK_LOG>() {
    const int bytes = sizeof(SIG_LOAD_DNF_HACK_LOG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_DNF_HACK_LOG* result =
        reinterpret_cast<SIG_LOAD_DNF_HACK_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456820
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_GOBLIN_UPDATE_VALIDITYTIMEEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_VALIDITYTIME * Stream::GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>(void)
template <>
SIG_GOBLIN_UPDATE_VALIDITYTIME*
Stream::GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>() {
    const int bytes = sizeof(SIG_GOBLIN_UPDATE_VALIDITYTIME);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GOBLIN_UPDATE_VALIDITYTIME* result =
        reinterpret_cast<SIG_GOBLIN_UPDATE_VALIDITYTIME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456A9C
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_IP_MONITOR_PUNISHEEPT_v
// DEMANGLED: SIG_IP_MONITOR_PUNISH * Stream::GetInBuffer<SIG_IP_MONITOR_PUNISH>(void)
template <>
SIG_IP_MONITOR_PUNISH* Stream::GetInBuffer<SIG_IP_MONITOR_PUNISH>() {
    const int bytes = sizeof(SIG_IP_MONITOR_PUNISH);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_IP_MONITOR_PUNISH* result =
        reinterpret_cast<SIG_IP_MONITOR_PUNISH*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8457210
// IDA_NAME: _ZN6Stream11GetInBufferI36SIG_UPDATE_COMPOUND_EMBLEM_STATISTICEEPT_v
// DEMANGLED: SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC * Stream::GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>(void)
template <>
SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC*
Stream::GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>() {
    const int bytes = sizeof(SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC* result =
        reinterpret_cast<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84575CA
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_CHECK_JOIN_GUILD_DB_REPLYEEPT_v
// DEMANGLED: SIG_CHECK_JOIN_GUILD_DB_REPLY * Stream::GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>(void)
template <>
SIG_CHECK_JOIN_GUILD_DB_REPLY*
Stream::GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>() {
    const int bytes = sizeof(SIG_CHECK_JOIN_GUILD_DB_REPLY);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_JOIN_GUILD_DB_REPLY* result =
        reinterpret_cast<SIG_CHECK_JOIN_GUILD_DB_REPLY*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8457634
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_CHECK_JOIN_GUILDEEPT_v
// DEMANGLED: SIG_CHECK_JOIN_GUILD * Stream::GetInBuffer<SIG_CHECK_JOIN_GUILD>(void)
template <>
SIG_CHECK_JOIN_GUILD* Stream::GetInBuffer<SIG_CHECK_JOIN_GUILD>() {
    const int bytes = sizeof(SIG_CHECK_JOIN_GUILD);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_JOIN_GUILD* result =
        reinterpret_cast<SIG_CHECK_JOIN_GUILD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8457772
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_JOIN_GUILD_INFOEEPT_v
// DEMANGLED: SIG_JOIN_GUILD_INFO * Stream::GetInBuffer<SIG_JOIN_GUILD_INFO>(void)
template <>
SIG_JOIN_GUILD_INFO* Stream::GetInBuffer<SIG_JOIN_GUILD_INFO>() {
    const int bytes = sizeof(SIG_JOIN_GUILD_INFO);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_JOIN_GUILD_INFO* result =
        reinterpret_cast<SIG_JOIN_GUILD_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84577DC
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_GUILD_JOIN_LISTEEPT_v
// DEMANGLED: SIG_GUILD_JOIN_LIST * Stream::GetInBuffer<SIG_GUILD_JOIN_LIST>(void)
template <>
SIG_GUILD_JOIN_LIST* Stream::GetInBuffer<SIG_GUILD_JOIN_LIST>() {
    const int bytes = sizeof(SIG_GUILD_JOIN_LIST);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GUILD_JOIN_LIST* result =
        reinterpret_cast<SIG_GUILD_JOIN_LIST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451C4A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_LOAD_DNF_HACK_LOGEEPT_v
// DEMANGLED: SIG_LOAD_DNF_HACK_LOG * CStreamGuard::GetInBuffer<SIG_LOAD_DNF_HACK_LOG>(void)
template <>
SIG_LOAD_DNF_HACK_LOG*
CStreamGuard::GetInBuffer<SIG_LOAD_DNF_HACK_LOG>() {
    return stream_->GetInBuffer<SIG_LOAD_DNF_HACK_LOG>();
}

// IDA_EA: 0x8452AD8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_GOBLIN_UPDATE_VALIDITYTIMEEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_VALIDITYTIME * CStreamGuard::GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>(void)
template <>
SIG_GOBLIN_UPDATE_VALIDITYTIME*
CStreamGuard::GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>() {
    return stream_->GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>();
}

// IDA_EA: 0x8453048
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_IP_MONITOR_PUNISHEEPT_v
// DEMANGLED: SIG_IP_MONITOR_PUNISH * CStreamGuard::GetInBuffer<SIG_IP_MONITOR_PUNISH>(void)
template <>
SIG_IP_MONITOR_PUNISH*
CStreamGuard::GetInBuffer<SIG_IP_MONITOR_PUNISH>() {
    return stream_->GetInBuffer<SIG_IP_MONITOR_PUNISH>();
}

// IDA_EA: 0x8453810
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI36SIG_UPDATE_COMPOUND_EMBLEM_STATISTICEEPT_v
// DEMANGLED: SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC * CStreamGuard::GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>(void)
template <>
SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC*
CStreamGuard::GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>() {
    return stream_->GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>();
}

// IDA_EA: 0x8453ACE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_CHECK_JOIN_GUILD_DB_REPLYEEPT_v
// DEMANGLED: SIG_CHECK_JOIN_GUILD_DB_REPLY * CStreamGuard::GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>(void)
template <>
SIG_CHECK_JOIN_GUILD_DB_REPLY*
CStreamGuard::GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>() {
    return stream_->GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>();
}

// IDA_EA: 0x8453AE4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_CHECK_JOIN_GUILDEEPT_v
// DEMANGLED: SIG_CHECK_JOIN_GUILD * CStreamGuard::GetInBuffer<SIG_CHECK_JOIN_GUILD>(void)
template <>
SIG_CHECK_JOIN_GUILD* CStreamGuard::GetInBuffer<SIG_CHECK_JOIN_GUILD>() {
    return stream_->GetInBuffer<SIG_CHECK_JOIN_GUILD>();
}

// IDA_EA: 0x8453B7A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_JOIN_GUILD_INFOEEPT_v
// DEMANGLED: SIG_JOIN_GUILD_INFO * CStreamGuard::GetInBuffer<SIG_JOIN_GUILD_INFO>(void)
template <>
SIG_JOIN_GUILD_INFO* CStreamGuard::GetInBuffer<SIG_JOIN_GUILD_INFO>() {
    return stream_->GetInBuffer<SIG_JOIN_GUILD_INFO>();
}

// IDA_EA: 0x8453B90
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_GUILD_JOIN_LISTEEPT_v
// DEMANGLED: SIG_GUILD_JOIN_LIST * CStreamGuard::GetInBuffer<SIG_GUILD_JOIN_LIST>(void)
template <>
SIG_GUILD_JOIN_LIST* CStreamGuard::GetInBuffer<SIG_GUILD_JOIN_LIST>() {
    return stream_->GetInBuffer<SIG_GUILD_JOIN_LIST>();
}

// IDA_EA: 0x8456C44
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_LOAD_POWER_WAR_STATUE_INFOEEPT_v
// DEMANGLED: SIG_LOAD_POWER_WAR_STATUE_INFO * Stream::GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>(void)
template <>
SIG_LOAD_POWER_WAR_STATUE_INFO*
Stream::GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>() {
    const int bytes = sizeof(SIG_LOAD_POWER_WAR_STATUE_INFO);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_POWER_WAR_STATUE_INFO* result =
        reinterpret_cast<SIG_LOAD_POWER_WAR_STATUE_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453148
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_LOAD_POWER_WAR_STATUE_INFOEEPT_v
// DEMANGLED: SIG_LOAD_POWER_WAR_STATUE_INFO * CStreamGuard::GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>(void)
template <>
SIG_LOAD_POWER_WAR_STATUE_INFO*
CStreamGuard::GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>() {
    return stream_->GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>();
}

// IDA_EA: 0x84578B0
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_TOD_RELOADED_USER_APCEEPT_v
// DEMANGLED: SIG_TOD_RELOADED_USER_APC * Stream::GetInBuffer<SIG_TOD_RELOADED_USER_APC>(void)
template <>
SIG_TOD_RELOADED_USER_APC*
Stream::GetInBuffer<SIG_TOD_RELOADED_USER_APC>() {
    const int bytes = sizeof(SIG_TOD_RELOADED_USER_APC);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_TOD_RELOADED_USER_APC* result =
        reinterpret_cast<SIG_TOD_RELOADED_USER_APC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453CB8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_TOD_RELOADED_USER_APCEEPT_v
// DEMANGLED: SIG_TOD_RELOADED_USER_APC * CStreamGuard::GetInBuffer<SIG_TOD_RELOADED_USER_APC>(void)
template <>
SIG_TOD_RELOADED_USER_APC*
CStreamGuard::GetInBuffer<SIG_TOD_RELOADED_USER_APC>() {
    return stream_->GetInBuffer<SIG_TOD_RELOADED_USER_APC>();
}
