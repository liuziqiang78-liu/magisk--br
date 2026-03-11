/**
 * @file logger.h
 * @brief 日志系统
 * 
 * 提供日志记录和管理功能
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stddef.h>

/* 日志级别 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3
} log_level_t;

/* 日志文件路径 */
#define LOG_FILE_PATH "/data/opbatt/battchg.log"
#define KERNEL_LOG_PATH "/data/opbatt/kernellog"
#define LOG_MAX_SIZE (5 * 1024 * 1024)  /* 5MB */

/* 初始化日志系统 */
int logger_init(const char *path);

/* 关闭日志系统 */
void logger_close(void);

/* 记录日志 */
void logger_log(log_level_t level, const char *format, ...);

/* 记录电池状态 */
void logger_log_battery_status(int soc, int voltage_mv, int current_ma, int temp);

/* 记录充电事件 */
void logger_log_charge_event(const char *event, int value);

/* 设置日志级别 */
void logger_set_level(log_level_t level);

/* 日志宏定义 */
#define LOG_DEBUG(fmt, ...) logger_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) logger_log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) logger_log(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif /* LOGGER_H */
