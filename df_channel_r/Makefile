ifeq ($(origin CXX), default)
CXX := /opt/gcc-4.9/bin/g++
endif
CPPFLAGS ?= -Iinclude
CXXFLAGS ?= -m32 -O2 -g -std=gnu++11 -Wall -Wextra -Wpedantic -pthread
LDFLAGS ?= -m32 -L/opt/lampp/lib -Wl,-rpath,/opt/lampp/lib
LDLIBS ?= -lcrypto -lz -pthread
CHANNEL_INFO ?= ../artifacts/channel_0828/channel_info_0828_authoritative.etc
CHANNEL_TEST_ROOT ?=

CORE_SOURCES := src/protocol.cpp src/script_store.cpp src/channel_registry.cpp \
	src/legacy_codec.cpp src/channel_service.cpp src/channel_client.cpp src/daemon_config.cpp
CORE_OBJECTS := $(CORE_SOURCES:.cpp=.o)
DAEMON_OBJECTS := src/main.o src/channel_daemon.o
DAEMON_TEST_OBJECTS := tests/channel_daemon_integration_test.o

.PHONY: all test clean
all: df_channel_r channel_core_test

df_channel_r: $(CORE_OBJECTS) $(DAEMON_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

channel_core_test: $(CORE_OBJECTS) tests/channel_core_test.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

channel_daemon_integration_test: $(CORE_OBJECTS) $(DAEMON_TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

test: df_channel_r channel_core_test channel_daemon_integration_test
	./channel_core_test $(CHANNEL_INFO)
	@test -n "$(CHANNEL_TEST_ROOT)" || \
		{ echo "CHANNEL_TEST_ROOT must contain cfg/smoke.cfg with caller-selected ports" >&2; exit 2; }
	./channel_daemon_integration_test ./df_channel_r $(CHANNEL_INFO) $(CHANNEL_TEST_ROOT)

clean:
	$(RM) $(CORE_OBJECTS) $(DAEMON_OBJECTS) $(DAEMON_TEST_OBJECTS) tests/channel_core_test.o \
		df_channel_r channel_core_test channel_daemon_integration_test
