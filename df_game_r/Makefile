CXX_AUTO := $(shell if [ -x /opt/gcc-4.9/bin/g++ ]; then echo /opt/gcc-4.9/bin/g++; else echo g++; fi)
ifeq ($(origin CXX), default)
CXX := $(CXX_AUTO)
endif

# This Makefile is intentionally limited to source files restored from the old
# df_game_r IDA export. Do not add source without an old-server IDA boundary.
TARGET := df_game_r
RELEASE_TARGET := df_game_r.release
PYTHON3 ?= python3
VERSION_CONFIG_AUDIT := tools/audit_version_config.py
SOURCE_MANIFEST_AUDIT := tools/audit_source_manifests.py

# Source ownership is explicit. The manifests list canonical paths in the
# semantic src hierarchy; the root build owns only common flags and link rules.
include manifest_A.mk
include manifest_B.mk
include manifest_C.mk

ifeq ($(strip $(A_SRC)),)
$(error manifest_A.mk must define a non-empty A_SRC)
endif
ifeq ($(strip $(B_SRC)),)
$(error manifest_B.mk must define a non-empty B_SRC)
endif
ifeq ($(strip $(C_SRC)),)
$(error manifest_C.mk must define a non-empty C_SRC)
endif

RESTORED_SRC := $(A_SRC) $(B_SRC) $(C_SRC)

RESTORED_OBJ := $(RESTORED_SRC:.cpp=.o)
RESTORED_DEP := $(RESTORED_OBJ:.o=.d)

# The remote original /home/neople/game/df_game_r is a 32-bit i386 ELF. Keep
# the rebuild on G++ 4.9 / C++11, matching the target CentOS 6 host.
CXXFLAGS ?= -m32 -O2 -g -std=gnu++11 -Wall -Wextra -Wno-unused-parameter
DEPFLAGS ?= -MMD -MP
CPPFLAGS := -Isrc -I/opt/boost_1_48_0
LDFLAGS ?= -m32

# The restored executable is i386 and the old binary's direct dependency is
# libmysqlclient.so.18. Resolve that exact ELF32 ABI instead of allowing
# -lmysqlclient to select an ELF64 or a different client SONAME. Override with
# MYSQLCLIENT_LIB=/absolute/path only when the file passes the same gate.
MYSQLCLIENT_SONAME ?= libmysqlclient.so.18
MYSQLCLIENT_SSL_SONAME ?= libssl.so.1.0.0
MYSQLCLIENT_CRYPTO_SONAME ?= libcrypto.so.1.0.0
MYSQLCLIENT_LIB ?=
MYSQLCLIENT_LIB_DIR ?=
MYSQLCLIENT_RESOLVER := tools/find_mysqlclient_elf32.sh

# CentOS6 old-server link probe library boundary.
#
# Keep normal packet Cipher algorithms internal under src/security/crypto_*.cpp.
# Do not link libtomcrypt for packet cipher unless an old df_game_r IDA call-site
# proves a direct external boundary. The old ELF directly needs
# libnxencryption.so; private DP/antibot libraries stay runtime dependencies
# under /home/neople/game and are reached through restored dlopen call-sites,
# not speculative direct links.
OLDSRV_LIB_DIRS := \
	-L/home/neople/game \
	-L/home/neople/game/lib \
	-L/opt/lampp/lib \
	-L/usr/lib \
	-L/lib

OLDSRV_LDLIBS := \
	-ldl \
	-lpthread \
	-lz \
	-l:libGeoIP.so.1 \
	-lnxencryption

define LINK_DF_GAME
	@mysqlclient_lib='$(MYSQLCLIENT_LIB)'; \
	if [ -n "$$mysqlclient_lib" ]; then \
		mysqlclient_lib="$$(MYSQLCLIENT_SONAME='$(MYSQLCLIENT_SONAME)' \
			MYSQLCLIENT_LIB_DIR='$(MYSQLCLIENT_LIB_DIR)' \
			sh $(MYSQLCLIENT_RESOLVER) --verify "$$mysqlclient_lib")" || exit $$?; \
	else \
		mysqlclient_lib="$$(MYSQLCLIENT_SONAME='$(MYSQLCLIENT_SONAME)' \
			MYSQLCLIENT_LIB_DIR='$(MYSQLCLIENT_LIB_DIR)' \
			sh $(MYSQLCLIENT_RESOLVER) --path)" || exit $$?; \
	fi; \
	mysqlclient_dir="$$(dirname "$$mysqlclient_lib")"; \
	mysql_ssl_lib="$$mysqlclient_dir/$(MYSQLCLIENT_SSL_SONAME)"; \
	mysql_crypto_lib="$$mysqlclient_dir/$(MYSQLCLIENT_CRYPTO_SONAME)"; \
	for support_lib in "$$mysql_ssl_lib" "$$mysql_crypto_lib"; do \
		if [ ! -f "$$support_lib" ]; then \
			echo "Missing MySQL client support library: $$support_lib" >&2; \
			exit 1; \
		fi; \
		if ! readelf -h "$$support_lib" 2>/dev/null | \
			grep -q 'Class:[[:space:]]*ELF32'; then \
			echo "MySQL client support library is not ELF32: $$support_lib" >&2; \
			exit 1; \
		fi; \
		if ! readelf -h "$$support_lib" 2>/dev/null | \
			grep -q 'Machine:[[:space:]]*Intel 80386'; then \
			echo "MySQL client support library is not i386: $$support_lib" >&2; \
			exit 1; \
		fi; \
	done; \
	echo "[link] MySQL client: $$mysqlclient_lib"; \
	echo "[link] MySQL SSL: $$mysql_ssl_lib"; \
	echo "[link] MySQL crypto: $$mysql_crypto_lib"; \
	$(CXX) $(LDFLAGS) -o $(TARGET) $(RESTORED_OBJ) \
		"$$mysqlclient_lib" "$$mysql_ssl_lib" "$$mysql_crypto_lib" \
		-Wl,-rpath-link,"$$mysqlclient_dir" \
		$(OLDSRV_LIB_DIRS) $(OLDSRV_LDLIBS)
endef

.PHONY: all audit_static audit_version_config audit_source_manifests \
	restored_objects link_probe \
	incremental_link_probe release clean

all: restored_objects

audit_static: audit_version_config audit_source_manifests

audit_version_config:
	$(PYTHON3) $(VERSION_CONFIG_AUDIT)

audit_source_manifests:
	$(PYTHON3) $(SOURCE_MANIFEST_AUDIT)

restored_objects: $(RESTORED_OBJ)

link_probe: $(TARGET)

# The normal link intentionally keeps DWARF and the complete static symbol
# table for IDA/NM verification.  Deployment builds use the same ELF and
# remove only debug/unneeded sections into a separate file, leaving the
# verified diagnostic artifact untouched.
release: $(TARGET)
	cp $(TARGET) $(RELEASE_TARGET)
	strip --strip-debug --strip-unneeded $(RELEASE_TARGET)

# Reuse an already-verified restored-object cache without letting make scan
# source timestamps and start an unrelated rebuild before the link.  The
# remote incremental probe compiles reviewed changed targets explicitly, then
# invokes this target to collect linker diagnostics from that exact cache.
incremental_link_probe:
	$(LINK_DF_GAME)

$(TARGET): $(RESTORED_OBJ)
	$(LINK_DF_GAME)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

-include $(RESTORED_DEP)

clean:
	find src -type f \( -name '*.o' -o -name '*.d' \) -delete
	rm -f $(TARGET) $(RELEASE_TARGET)
