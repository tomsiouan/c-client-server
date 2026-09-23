CC      = clang
CFLAGS  = -Wall -Wextra -std=c17 -g -MMD -MP

BUILD_DIR = build
BIN_DIR   = bin

CLIENT_SRC = $(wildcard client/*.c)
SERVER_SRC = $(wildcard server/*.c)

CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/%.o)
SERVER_OBJ = $(SERVER_SRC:%.c=$(BUILD_DIR)/%.o)

CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

.PHONY: all clean

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_BIN): $(SERVER_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

-include $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)
