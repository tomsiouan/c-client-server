CC      = clang
WIN_CC  = x86_64-w64-mingw32-gcc
CFLAGS  = -Wall -Wextra -std=c17 -g -MMD -MP -Icommon

BUILD_DIR = build
BIN_DIR   = bin

CLIENT_SRC = $(wildcard client/*.c)
SERVER_SRC = $(wildcard server/*.c)

CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/%.o)
SERVER_OBJ = $(SERVER_SRC:%.c=$(BUILD_DIR)/%.o)

CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

# Windows client, cross-compiled with MinGW-w64 (brew install mingw-w64)
WIN_CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/windows/%.o)
WIN_CLIENT_BIN = $(BIN_DIR)/client.exe

.PHONY: all windows clean

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_BIN): $(SERVER_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

windows: $(WIN_CLIENT_BIN)

# -static so the .exe runs on a Windows PC without extra MinGW DLLs
$(WIN_CLIENT_BIN): $(WIN_CLIENT_OBJ)
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CFLAGS) -static -o $@ $^ -lws2_32

$(BUILD_DIR)/windows/%.o: %.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

-include $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d) $(WIN_CLIENT_OBJ:.o=.d)
