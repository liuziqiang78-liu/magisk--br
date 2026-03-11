/**
 * opbatt_control - 电池控制模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "opbatt_control.h"

/* 电池设备路径 */
#define BATTERY_SYS_PATH "/sys/class/power_supply/battery"
#define BATTERY_STATUS_FILE BATTERY_SYS_PATH "/status"
#define BATTERY_CAPACITY_FILE BATTERY_SYS_PATH "/capacity"
#define BATTERY_VOLTAGE_FILE BATTERY_SYS_PATH "/voltage_now"
#define BATTERY_CURRENT_FILE BATTERY_SYS_PATH "/current_now"
#define BATTERY_TEMP_FILE BATTERY_SYS_PATH "/temp"

/* 读取文件内容 */
static int read_file(const char *path, char *buffer, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        opbatt_log("无法打开文件 %s: %s", path, strerror(errno));
        return OPBATT_ERROR;
    }
    
    ssize_t len = read(fd, buffer, size - 1);
    if (len < 0) {
        opbatt_log("读取文件 %s 失败: %s", path, strerror(errno));
        close(fd);
        return OPBATT_ERROR;
    }
    
    close(fd);
    buffer[len] = '\0';
    
    /* 去除换行符 */
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return OPBATT_SUCCESS;
}

/* 写入文件内容 */
static int write_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        opbatt_log("无法打开文件 %s: %s", path, strerror(errno));
        return OPBATT_ERROR;
    }
    
    ssize_t len = write(fd, value, strlen(value));
    if (len < 0) {
        opbatt_log("写入文件 %s 失败: %s", path, strerror(errno));
        close(fd);
        return OPBATT_ERROR;
    }
    
    close(fd);
    return OPBATT_SUCCESS;
}

/* 初始化电池控制 */
int opbatt_battery_init(opbatt_context_t *ctx) {
    opbatt_log("初始化电池控制模块...");
    
    /* 检查电池设备是否存在 */
    struct stat st;
    if (stat(BATTERY_SYS_PATH, &st) != 0) {
        opbatt_log("警告: 电池设备不存在，使用模拟模式");
        /* 使用模拟值 */
        ctx->battery.voltage_mv = 3700;
        ctx->battery.current_ma = 0;
        ctx->battery.capacity_mah = 3000;
        ctx->battery.level_percent = 50;
        ctx->battery.status = 0;
        ctx->battery.health = 100;
        ctx->battery.temperature_c = 25;
        return OPBATT_SUCCESS;
    }
    
    /* 读取初始状态 */
    return opbatt_battery_get_status(ctx, &ctx->battery);
}

/* 获取电池状态 */
int opbatt_battery_get_status(opbatt_context_t *ctx, battery_status_t *status) {
    char buffer[256];
    int ret;
    
    /* 读取状态 */
    ret = read_file(BATTERY_STATUS_FILE, buffer, sizeof(buffer));
    if (ret == OPBATT_SUCCESS) {
        if (strcmp(buffer, "Charging") == 0) {
            status->status = 1;
        } else if (strcmp(buffer, "Discharging") == 0) {
            status->status = 2;
        } else {
            status->status = 0;
        }
    }
    
    /* 读取电量百分比 */
    ret = read_file(BATTERY_CAPACITY_FILE, buffer, sizeof(buffer));
    if (ret == OPBATT_SUCCESS) {
        status->level_percent = atoi(buffer);
    }
    
    /* 读取电压 (µV -> mV) */
    ret = read_file(BATTERY_VOLTAGE_FILE, buffer, sizeof(buffer));
    if (ret == OPBATT_SUCCESS) {
        status->voltage_mv = atoi(buffer) / 1000;
    }
    
    /* 读取电流 (µA -> mA) */
    ret = read_file(BATTERY_CURRENT_FILE, buffer, sizeof(buffer));
    if (ret == OPBATT_SUCCESS) {
        status->current_ma = atoi(buffer) / 1000;
    }
    
    /* 读取温度 (0.1°C -> °C) */
    ret = read_file(BATTERY_TEMP_FILE, buffer, sizeof(buffer));
    if (ret == OPBATT_SUCCESS) {
        status->temperature_c = atoi(buffer) / 10;
    }
    
    opbatt_log("电池状态: 电压=%dmV, 电流=%dmA, 电量=%d%%, 温度=%d°C",
               status->voltage_mv, status->current_ma, 
               status->level_percent, status->temperature_c);
    
    return OPBATT_SUCCESS;
}

/* 设置充电状态 */
int opbatt_battery_set_charge(opbatt_context_t *ctx, bool enable) {
    char value[16];
    snprintf(value, sizeof(value), "%d", enable ? 1 : 0);
    
    opbatt_log("设置充电状态: %s", enable ? "开启" : "关闭");
    
    /* 写入充电控制文件 */
    const char *charge_control_path = BATTERY_SYS_PATH "/charge_control";
    if (write_file(charge_control_path, value) != OPBATT_SUCCESS) {
        opbatt_log("警告: 无法设置充电状态");
        return OPBATT_ERROR;
    }
    
    ctx->battery.status = enable ? 1 : 0;
    return OPBATT_SUCCESS;
}

/* 设置充电电流 */
int opbatt_battery_set_current(opbatt_context_t *ctx, uint32_t current_ma) {
    char value[32];
    snprintf(value, sizeof(value), "%u", current_ma);
    
    opbatt_log("设置充电电流: %umA", current_ma);
    
    /* 写入电流限制文件 */
    const char *current_limit_path = BATTERY_SYS_PATH "/current_max";
    if (write_file(current_limit_path, value) != OPBATT_SUCCESS) {
        opbatt_log("警告: 无法设置充电电流");
        return OPBATT_ERROR;
    }
    
    ctx->battery.current_ma = current_ma;
    return OPBATT_SUCCESS;
}

/* 模拟电池状态变化（用于测试） */
void opbatt_battery_simulate(opbatt_context_t *ctx) {
    /* 模拟充电 */
    if (ctx->battery.status == 1) {
        if (ctx->battery.level_percent < 100) {
            ctx->battery.level_percent++;
            ctx->battery.current_ma = 2000;
            ctx->battery.voltage_mv = 4200;
        } else {
            ctx->battery.status = 0;
            ctx->battery.current_ma = 0;
            ctx->battery.voltage_mv = 4100;
        }
    }
    /* 模拟放电 */
    else if (ctx->battery.status == 2) {
        if (ctx->battery.level_percent > 0) {
            ctx->battery.level_percent--;
            ctx->battery.current_ma = -1000;
            ctx->battery.voltage_mv = 3700;
        } else {
            ctx->battery.status = 0;
            ctx->battery.current_ma = 0;
            ctx->battery.voltage_mv = 3600;
        }
    }
    /* 空闲状态 */
    else {
        ctx->battery.current_ma = 0;
        ctx->battery.voltage_mv = 3800;
    }
    
    /* 模拟温度变化 */
    if (ctx->battery.current_ma > 0) {
        ctx->battery.temperature_c = 30 + (ctx->battery.current_ma / 1000) * 2;
    } else {
        ctx->battery.temperature_c = 25;
    }
}
