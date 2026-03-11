# Makefile for opbatt_control
# Target: ARM64 (aarch64-linux-gnu)

# 项目名称
PROJECT_NAME = opbatt_control

# 源文件
SRCS = src/main.c \
       src/sysfs.c \
       src/logger.c \
       src/config.c \
       src/battery.c \
       src/charge.c \
       src/ufcs.c \
       src/pps.c \
       src/svooc.c \
       src/temp.c \
       src/cv.c \
       src/ddrc.c \
       src/soc.c \
       src/daemon.c

# 目标文件
OBJS = $(SRCS:.c=.o)

# 头文件目录
INCLUDE_DIR = include

# 编译器
CC = gcc
ARMCROSS = aarch64-linux-gnu-gcc

# 编译选项
CFLAGS = -Wall -Wextra -O2 -std=c11 -I$(INCLUDE_DIR)
LDFLAGS = -lpthread

# ARM64 编译选项
ARM_CFLAGS = $(CFLAGS) -static
ARM_LDFLAGS = $(LDFLAGS)

# 目标文件
TARGET = $(PROJECT_NAME)
ARM_TARGET = $(PROJECT_NAME)_arm64

# 默认目标
all: $(TARGET)

# 本地编译
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# 编译源文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ARM64 交叉编译
arm64: clean
	$(ARMCROSS) $(ARM_CFLAGS) $(SRCS) -o $(ARM_TARGET) $(ARM_LDFLAGS)
	@echo "ARM64 build complete: $(ARM_TARGET)"

# 清理
clean:
	rm -f $(OBJS) $(TARGET) $(ARM_TARGET)
	@echo "Clean complete"

# 安装
install: $(TARGET)
	install -d /usr/local/bin
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Install complete"

# 卸载
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# 帮助
help:
	@echo "Available targets:"
	@echo "  all      - Build for local system (default)"
	@echo "  arm64    - Cross-compile for ARM64"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  uninstall- Uninstall from /usr/local/bin"
	@echo "  help     - Show this help message"

# 依赖关系
src/main.o: src/main.c include/*.h
src/sysfs.o: src/sysfs.c include/sysfs.h include/sysfs_paths.h include/logger.h
src/logger.o: src/logger.c include/logger.h
src/config.o: src/config.c include/config.h include/logger.h
src/battery.o: src/battery.c include/battery.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/charge.o: src/charge.c include/charge.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/ufcs.o: src/ufcs.c include/ufcs.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/pps.o: src/pps.c include/pps.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/svooc.o: src/svooc.c include/svooc.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/temp.o: src/temp.c include/temp.h include/sysfs.h include/sysfs_paths.h include/logger.h
src/cv.o: src/cv.c include/cv.h include/config.h include/logger.h
src/ddrc.o: src/ddrc.c include/ddrc.h include/logger.h
src/soc.o: src/soc.c include/soc.h include/logger.h
src/daemon.o: src/daemon.c include/daemon.h include/logger.h

.PHONY: all arm64 clean install uninstall help
