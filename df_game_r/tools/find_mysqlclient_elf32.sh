#!/bin/sh

set -eu

LC_ALL=C
export LC_ALL

soname=${MYSQLCLIENT_SONAME:-libmysqlclient.so.18}
search_dirs=${MYSQLCLIENT_LIB_DIR:-}

usage()
{
    echo "usage: $0 --path | --verify FILE" >&2
    exit 64
}

verify_library()
{
    library=$1

    if [ ! -f "$library" ]; then
        echo "MySQL client does not exist: $library" >&2
        return 1
    fi

    resolved=$(readlink -f "$library") || return 1
    if ! readelf -h "$resolved" 2>/dev/null | grep -q 'Class:[[:space:]]*ELF32'; then
        echo "MySQL client is not ELF32: $resolved" >&2
        return 1
    fi
    if ! readelf -h "$resolved" 2>/dev/null | grep -q 'Machine:[[:space:]]*Intel 80386'; then
        echo "MySQL client is not i386: $resolved" >&2
        return 1
    fi
    if ! readelf -d "$resolved" 2>/dev/null | grep 'SONAME' | grep -Fq "[$soname]"; then
        echo "MySQL client SONAME is not $soname: $resolved" >&2
        return 1
    fi

    printf '%s\n' "$resolved"
}

find_library()
{
    old_ifs=$IFS
    IFS=:
    for directory in $search_dirs; do
        [ -n "$directory" ] || continue
        if verify_library "$directory/$soname" 2>/dev/null; then
            IFS=$old_ifs
            return 0
        fi
    done
    IFS=$old_ifs

    for directory in \
        /home/neople/game/lib/mysql32 \
        /opt/lampp/lib \
        /usr/lib/mysql \
        /usr/lib \
        /lib
    do
        if verify_library "$directory/$soname" 2>/dev/null; then
            return 0
        fi
    done

    echo "No ELF32 i386 $soname was found" >&2
    return 1
}

case ${1:-} in
    --path)
        [ "$#" -eq 1 ] || usage
        find_library
        ;;
    --verify)
        [ "$#" -eq 2 ] || usage
        verify_library "$2"
        ;;
    *)
        usage
        ;;
esac
