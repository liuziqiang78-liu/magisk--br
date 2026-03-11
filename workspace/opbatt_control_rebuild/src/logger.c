/**
 * @file logger.c
 * @brief 日志系统实现
 * 
 * 提供日志记录和管理功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "logger.h"

/* 日志级别字符串 */
static const char *log_level_str[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

/* 全局变量 */
static FILE *log_fp = NULL;
static log_level_t current_log_level = LOG_LEVEL_INFO;
static char log_file_path[256] = LOG_FILE_PATH;

/* 初始化日志系统 */
int logger_init(const char *path) {
    if (path) {
        strncpy(log_file_path, path, sizeof(log_file_path) - 1);
        log_file_path[sizeof(log_file_path) - 1] = '\0';
    }
    
    /* 检查日志文件大小，如果超过限制则清空 */
    struct stat st;
    if (stat(log_file_path, &st) == 0) {
        if (st.st_size > LOG_MAX_SIZE) {
            /* 备份旧日志 */
            char backup_path[512];
            snprintf(backup_path, sizeof(backup_path), "%s.bak", log_file_path);
            rename(log_file_path, backup_path);
        }
    }
    
    /* 打开日志文件 */
    log_fp = fopen(log_file_path, "a");
    if (!log_fp) {
        fprintf(stderr, "Failed to open log file %s: %s\n", log_file_path, strerror(errno));
        return -1;
    }
    
    /* 设置缓冲区 */
    setvbuf(log_fp, NULL, _IOLBF, 0);
    
    LOG_INFO("Logger initialized: %s", log_file_path);
    
    return 0;
}

/* 关闭日志系统 */
void logger_close(void) {
    if (log_fp) {
        LOG_INFO("Logger closed");
        fclose(log_fp);
        log_fp = NULL;
    }
}

/* 记录日志 */
void logger_log(log_level_t level, const char *format, ...) {
    if (!log_fp || level < current_log_level) {
        return;
    }
    
    /* 获取当前时间 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* 写入日志级别和时间 */
    fprintf(log_fp, "[%s] [%s] ", time_str, log_level_str[level]);
    
    /* 写入日志内容 */
    va_list args;
    va_start(args, format);
    vfprintf(log_fp, format, args);
    va_end(args);
    
    /* 写入换行符 */
    fprintf(log_fp, "\n");
    
    /* 刷新缓冲区 */
    fflush(log_fp);
}

/* 记录电池状态 */
void logger_log_battery_status(int soc, int voltage_mv, int current_ma, int temp) {
    LOG_INFO("Battery Status: SOC=%d%%, Voltage=%dmV, Current=%dmA, Temp=%d°C",
             soc, voltage_mv, current_ma, temp / 10);
}

/* 记录充电事件 */
void logger_log_charge_event(const char *event, int value) {
    LOG_INFO("Charge Event: %s=%d", event, value);
}

/* 设置日志级别 */
void logger_set_level(log_level_t level) {
    if (level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_ERROR) {
        current_log_level = level;
        LOG_INFO("Log level set to: %s", log_level_str[level]);
    }
}
