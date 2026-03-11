# Makefile for battery_control
# 纯电池控制版本（无网络、无验证）

# 编译器和工具
CC = gcc
AR = ar
RANLIB = ranlib

# 编译选项
CFLAGS = -Wall -Wextra -O2 -g -std=c11
CFLAGS += -I./include
CFLAGS += -D_GNU_SOURCE

# 链接选项
LDFLAGS = -lm

# 目标文件
TARGET = battery_control
STATIC_LIB = libbattery.a

# 源文件
SRCS = src/main.c \
       src/battery.c \
       src/utils.c

# 目标文件
OBJS = $(SRCS:.c=.o)

# 头文件依赖
DEPS = include/battery_control.h

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
	./$(TARGET) -s

# 监控
monitor: $(TARGET)
	./$(TARGET) -m

# 帮助
help:
	@echo "可用的目标:"
	@echo "  all       - 编译程序（默认）"
	@echo "  clean     - 清理编译文件"
	@echo "  install   - 安装程序到 /usr/local/bin"
	@echo "  uninstall - 从 /usr/local/bin 卸载程序"
	@echo "  run       - 编译并运行程序（显示状态）"
	@echo "  monitor   - 编译并运行程序（监控模式）"
	@echo "  help      - 显示此帮助信息"

.PHONY: all clean install uninstall run monitor help
