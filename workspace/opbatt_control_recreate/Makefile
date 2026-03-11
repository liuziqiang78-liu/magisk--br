# Makefile for opbatt_control
# 电池控制程序重新实现版本

# 编译器和工具
CC = gcc
AR = ar
RANLIB = ranlib

# 编译选项
CFLAGS = -Wall -Wextra -O2 -g -std=c11
CFLAGS += -I./include
CFLAGS += -D_GNU_SOURCE

# 链接选项
LDFLAGS = -lpthread -lm

# OpenSSL 支持（可选）
# CFLAGS += -DUSE_OPENSSL
# LDFLAGS += -lssl -lcrypto

# 目标文件
TARGET = opbatt_control
STATIC_LIB = libopbatt.a

# 源文件
SRCS = src/main.c \
       src/battery.c \
       src/network.c \
       src/license.c \
       src/utils.c \
       src/main_loop.c \
       src/init.c

# 目标文件
OBJS = $(SRCS:.c=.o)

# 头文件依赖
DEPS = include/opbatt_control.h

# 默认目标
all: $(TARGET)

# 编译主程序
$(TARGET): $(OBJS)
	@echo "链接 $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "编译完成: $@"

# 编译静态库
$(STATIC_LIB): $(OBJS)
	@echo "创建静态库 $@..."
	$(AR) rcs $@ $^
	$(RANLIB) $@

# 编译源文件
%.o: %.c $(DEPS)
	@echo "编译 $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	@echo "清理..."
	rm -f $(OBJS) $(TARGET) $(STATIC_LIB)
	rm -f src/*.o
	@echo "清理完成"

# 安装
install: $(TARGET)
	@echo "安装 $(TARGET)..."
	install -d /usr/local/bin
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "安装完成"

# 卸载
uninstall:
	@echo "卸载 $(TARGET)..."
	rm -f /usr/local/bin/$(TARGET)
	@echo "卸载完成"

# 运行
run: $(TARGET)
	./$(TARGET) -s -p 8888 -v

# 调试
debug: $(TARGET)
	gdb ./$(TARGET)

# 帮助
help:
	@echo "可用的目标:"
	@echo "  all       - 编译程序（默认）"
	@echo "  clean     - 清理编译文件"
	@echo "  install   - 安装程序到 /usr/local/bin"
	@echo "  uninstall - 从 /usr/local/bin 卸载程序"
	@echo "  run       - 编译并运行程序"
	@echo "  debug     - 使用 gdb 调试程序"
	@echo "  help      - 显示此帮助信息"

.PHONY: all clean install uninstall run debug help
