/**
 * @file sysfs.h
 * @brief sysfs 接口层
 * 
 * 提供统一的 sysfs 文件访问接口
 */

#ifndef SYSFS_H
#define SYSFS_H

#include <stdint.h>
#include <stddef.h>

/* 读取 sysfs 整数值 */
int sysfs_read_int(const char *path, int *value);

/* 读取 sysfs 字符串 */
int sysfs_read_string(const char *path, char *buffer, size_t size);

/* 写入 sysfs 整数值 */
int sysfs_write_int(const char *path, int value);

/* 写入 sysfs 字符串 */
int sysfs_write_string(const char *path, const char *value);

/* 检查文件是否存在 */
int sysfs_exists(const char *path);

/* 数据转换函数 */
static inline int temp_to_celsius(int raw) { return raw / 10; }
static inline int current_to_ma(int raw) { return raw / 1000; }
static inline int voltage_to_mv(int raw) { return raw / 1000; }
static inline int power_to_mw(int raw) { return raw / 1000; }

#endif /* SYSFS_H */
