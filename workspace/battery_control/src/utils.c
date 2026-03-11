/**
 * battery_control - 工具函数模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "battery_control.h"

/* 获取当前时间戳 */
static const char *get_timestamp(void) {
    static char timestamp[64];
    struct timeval tv;
    struct tm *tm;
    
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000);
    
    return timestamp;
}

/* 日志输出 */
void battery_log(const char *format, ...) {
    va_list args;
    
    printf("[%s] ", get_timestamp());
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}

/* 打印电池状态 */
void battery_print_status(const battery_status_t *status) {
    printf("========================================\n");
    printf("           电池状态\n");
    printf("========================================\n");
    printf("电压:     %u mV\n", status->voltage_mv);
    printf("电流:     %d mA\n", status->current_ma);
    printf("电量:     %u%%\n", status->level_percent);
    printf("容量:     %u mAh\n", status->capacity_mah);
    printf("温度:     %d°C\n", status->temperature_c);
    printf("健康度:   %u%%\n", status->health);
    printf("循环次数: %u\n", status->cycle_count);
    printf("满充容量: %u mAh\n", status->full_charge_capacity);
    
    printf("状态:     ");
    switch (status->status) {
        case 0:
            printf("空闲\n");
            break;
        case 1:
            printf("充电中\n");
            break;
        case 2:
            printf("放电中\n");
            break;
        default:
            printf("未知\n");
            break;
    }
    
    printf("========================================\n");
}

/* 打印电池信息 */
void battery_print_info(const battery_info_t *info) {
    printf("========================================\n");
    printf("           电池信息\n");
    printf("========================================\n");
    printf("型号:     %s\n", info->model);
    printf("制造商:   %s\n", info->manufacturer);
    printf("序列号:   %s\n", info->serial);
    printf("设计容量: %u mAh\n", info->design_capacity);
    printf("设计电压: %u mV\n", info->design_voltage);
    printf("化学类型: %s\n", info->chemistry);
    printf("========================================\n");
}
