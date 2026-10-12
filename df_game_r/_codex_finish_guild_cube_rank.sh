#!/bin/sh
set -eu
build=/root/oldsrv_rebuild_full_0828_native_pvf0828_fresh_20260828_222433_2268
runtime=/home/neople/game/df_game_r
previous=61d11aac404a2349433a6b12fdafe3122075860bdcce6d88f6b618f17f1e5a9e
backup=/home/neople/game/.codex_backups/guild_cube_20260910_rank/df_game_r
cd "$build"
test "$(pwd -P)" = "$build"
test -f df_game_r
test ! -L df_game_r
test ! -e df_game_r.release.guild_cube_new
strip --strip-debug --strip-unneeded -o df_game_r.release.guild_cube_new df_game_r
file df_game_r.release.guild_cube_new
readelf -h df_game_r.release.guild_cube_new | grep 'ELF32'
LD_LIBRARY_PATH=/home/neople/game/lib:/home/neople/game/lib/mysql32:/home/neople/game ldd df_game_r.release.guild_cube_new > /tmp/codex_guild_cube_ldd.log
if grep -q 'not found' /tmp/codex_guild_cube_ldd.log; then
    cat /tmp/codex_guild_cube_ldd.log
    exit 1
fi
strings df_game_r.release.guild_cube_new | grep -F '[GUILD_EXP_BOOK]'
strings df_game_r.release.guild_cube_new | grep -F '[CHRONICLE_ENCHANT][RESULT]'
strings df_game_r.release.guild_cube_new | grep -F '[SPECIAL_TUTORIAL][NORMALIZE]'
strings df_game_r.release.guild_cube_new | grep -F '[CARD_FLOW][GENERATED]'
strings df_game_r.release.guild_cube_new | grep -F '[BIND_CUBE][CHECK]'
strings df_game_r.release.guild_cube_new | grep -F '[GUILD][ALL_MEMBERS_ROUTE]'
strings df_game_r.release.guild_cube_new | grep -F '[GUILD][MEMBER_GRADE_SYNC]'
strings df_game_r.release.guild_cube_new | grep -F '[BIND_CUBE][CANDIDATES_LOADED]'
candidate=$(sha256sum df_game_r.release.guild_cube_new | cut -d ' ' -f1)
if pgrep -x df_game_r >/dev/null; then
    printf 'REFUSING_INSTALL_GAME_IS_RUNNING\n'
    exit 1
fi
test "$(sha256sum "$runtime" | cut -d ' ' -f1)" = "$previous"
test ! -L "$runtime"
test ! -e "$backup"
test ! -e /home/neople/game/df_game_r.guild_cube_new
mkdir -p /home/neople/game/.codex_backups/guild_cube_20260910_rank
cp -p "$runtime" "$backup"
test "$(sha256sum "$backup" | cut -d ' ' -f1)" = "$previous"
install -m 755 df_game_r.release.guild_cube_new /home/neople/game/df_game_r.guild_cube_new
test "$(sha256sum /home/neople/game/df_game_r.guild_cube_new | cut -d ' ' -f1)" = "$candidate"
if pgrep -x df_game_r >/dev/null; then
    printf 'REFUSING_INSTALL_GAME_STARTED_DURING_STAGING\n'
    exit 1
fi
mv -T /home/neople/game/df_game_r.guild_cube_new "$runtime"
mv -T df_game_r.release.guild_cube_new df_game_r.release
sha256sum df_game_r.release "$runtime" "$backup"
stat -c '%s %n' df_game_r.release "$runtime"
printf 'INSTALLED_GAME_LEFT_STOPPED\n'
