ifeq ($(origin CXX), default)
CXX := /opt/gcc-4.9/bin/g++
endif
BUILD_DIR ?= build-gcc49

CPPFLAGS += -Iinclude
CXXFLAGS ?= -m32 -O2 -g -std=gnu++11 -Wall -Wextra -Wpedantic -pthread
LDLIBS += -pthread

CORE_SOURCES := \
	src/bridge_core.cpp \
	src/channel_info.cpp \
	src/config.cpp \
	src/protocol.cpp \
	src/script_thread.cpp
CORE_OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(CORE_SOURCES))
SERVICE_SOURCE := src/service.cpp
SERVICE_OBJECT := $(BUILD_DIR)/service.o
MAIN_OBJECT := $(BUILD_DIR)/main.o
TEST_OBJECT := $(BUILD_DIR)/bridge_tests.o
CLI := $(BUILD_DIR)/df_bridge_r
TEST_BIN := $(BUILD_DIR)/dfbridge_tests

.PHONY: all test mysql production clean

all: $(CLI) $(TEST_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(MAIN_OBJECT): src/main.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(TEST_OBJECT): tests/bridge_tests.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(CLI): $(CORE_OBJECTS) $(SERVICE_OBJECT) $(MAIN_OBJECT)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

$(TEST_BIN): $(CORE_OBJECTS) $(TEST_OBJECT)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

# Production adapter: use the target machine's MySQL/MariaDB development kit.
MYSQL_CPPFLAGS ?= -I/root/deps/mysql-5.5.27/include
MYSQL_CXXFLAGS ?= -m32 -O2 -g -std=gnu++11 -Wall -Wextra -Wpedantic -pthread
MYSQL_LDFLAGS ?= -m32 -L/opt/lampp/lib -Wl,-rpath,/opt/lampp/lib
MYSQL_LIBS ?= -lmysqlclient -lpthread -ldl -lz

mysql: | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(MYSQL_CPPFLAGS) $(MYSQL_CXXFLAGS) -DDFBRIDGE_WITH_MYSQL=1 \
		$(CORE_SOURCES) $(SERVICE_SOURCE) src/mysql_repository.cpp src/main.cpp \
		$(MYSQL_LDFLAGS) $(MYSQL_LIBS) -o $(BUILD_DIR)/df_bridge_r_mysql

# Clean production builds expose the exact historical executable name.
production: mysql
	cp $(BUILD_DIR)/df_bridge_r_mysql $(BUILD_DIR)/df_bridge_r

clean:
	rm -rf $(BUILD_DIR)

-include $(CORE_OBJECTS:.o=.d) $(SERVICE_OBJECT:.o=.d) $(MAIN_OBJECT:.o=.d) $(TEST_OBJECT:.o=.d)
