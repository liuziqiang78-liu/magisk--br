/**
 * battery_control - 电池控制核心模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "battery_control.h"

/* 电池设备路径 */
#define BATTERY_SYS_PATH "/sys/class/power_supply/battery"
#define BATTERY_STATUS_FILE BATTERY_SYS_PATH "/status"
#define BATTERY_CAPACITY_FILE BATTERY_SYS_PATH "/capacity"
#define BATTERY_VOLTAGE_FILE BATTERY_SYS_PATH "/voltage_now"
#define BATTERY_CURRENT_FILE BATTERY_SYS_PATH "/current_now"
#define BATTERY_TEMP_FILE BATTERY_SYS_PATH "/temp"
#define BATTERY_CYCLE_COUNT_FILE BATTERY_SYS_PATH "/cycle_count"
#define BATTERY_CHARGE_CONTROL_FILE BATTERY_SYS_PATH "/charge_control"
#define BATTERY_CURRENT_MAX_FILE BATTERY_SYS_PATH "/current_max"
#define BATTERY_VOLTAGE_MAX_FILE BATTERY_SYS_PATH "/voltage_max"
#define BATTERY_HEALTH_FILE BATTERY_SYS_PATH "/health"
#define BATTERY_CAPACITY_FULL_FILE BATTERY_SYS_PATH "/charge_full"

/* 电池信息文件 */
#define BATTERY_MODEL_FILE BATTERY_SYS_PATH "/model"
#define BATTERY_MANUFACTURER_FILE BATTERY_SYS_PATH "/manufacturer"
#define BATTERY_SERIAL_FILE BATTERY_SYS_PATH "/serial_number"
#define BATTERY_CAPACITY_DESIGN_FILE BATTERY_SYS_PATH "/charge_full_design"
#define BATTERY_VOLTAGE_DESIGN_FILE BATTERY_SYS_PATH "/voltage_max_design"
#define BATTERY_CHEMISTRY_FILE BATTERY_SYS_PATH "/technology"

/* 读取文件内容 */
static int read_file(const char *path, char *buffer, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return BATTERY_ERROR_READ;
    }
    
    ssize_t len = read(fd, buffer, size - 1);
    if (len < 0) {
        close(fd);
        return BATTERY_ERROR_READ;
    }
    
    close(fd);
    buffer[len] = '\0';
    
    /* 去除换行符 */
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return BATTERY_SUCCESS;
}

/* 写入文件内容 */
static int write_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return BATTERY_ERROR_WRITE;
    }
    
    ssize_t len = write(fd, value, strlen(value));
    if (len < 0) {
        close(fd);
        return BATTERY_ERROR_WRITE;
    }
    
    close(fd);
    return BATTERY_SUCCESS;
}

/* 初始化电池控制 */
int battery_init(battery_context_t *ctx) {
    battery_log("初始化电池控制模块...");
    
    /* 检查电池设备是否存在 */
    struct stat st;
    if (stat(BATTERY_SYS_PATH, &st) != 0) {
        battery_log("警告: 电池设备不存在，使用模拟模式");
        ctx->simulation_mode = true;
        
        /* 使用模拟值 */
        ctx->status.voltage_mv = 3700;
        ctx->status.current_ma = 0;
        ctx->status.capacity_mah = 3000;
        ctx->status.level_percent = 50;
        ctx->status.status = 0;
        ctx->status.health = 100;
        ctx->status.temperature_c = 25;
        ctx->status.cycle_count = 0;
        ctx->status.full_charge_capacity = 3000;
        
        /* 模拟电池信息 */
        strcpy(ctx->info.model, "SIM-BATTERY-001");
        strcpy(ctx->info.manufacturer, "SIM-MANUFACTURER");
        strcpy(ctx->info.serial, "SIM-SERIAL-001");
        ctx->info.design_capacity = 3000;
        ctx->info.design_voltage = 3700;
        strcpy(ctx->info.chemistry, "Li-ion");
        
        ctx->initialized = true;
        return BATTERY_SUCCESS;
    }
    
    /* 读取初始状态 */
    battery_get_status(ctx, &ctx->status);
    battery_get_info(ctx, &ctx->info);
    
    ctx->initialized = true;
    battery_log("电池控制模块初始化完成");
    
    return BATTERY_SUCCESS;
}

/* 清理电池控制 */
int battery_cleanup(battery_context_t *ctx) {
    battery_log("清理电池控制模块...");
    
    ctx->initialized = false;
    
    return BATTERY_SUCCESS;
}

/* 获取电池状态 */
int battery_get_status(battery_context_t *ctx, battery_status_t *status) {
    char buffer[256];
    int ret;
    
    if (ctx->simulation_mode) {
        /* 模拟模式：返回模拟值 */
        *status = ctx->status;
        return BATTERY_SUCCESS;
    }
    
    /* 读取状态 */
    ret = read_file(BATTERY_STATUS_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
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
    if (ret == BATTERY_SUCCESS) {
        status->level_percent = atoi(buffer);
    }
    
    /* 读取电压 (µV -> mV) */
    ret = read_file(BATTERY_VOLTAGE_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        status->voltage_mv = atoi(buffer) / 1000;
    }
    
    /* 读取电流 (µA -> mA) */
    ret = read_file(BATTERY_CURRENT_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        status->current_ma = atoi(buffer) / 1000;
    }
    
    /* 读取温度 (0.1°C -> °C) */
    ret = read_file(BATTERY_TEMP_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        status->temperature_c = atoi(buffer) / 10;
    }
    
    /* 读取循环次数 */
    ret = read_file(BATTERY_CYCLE_COUNT_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        status->cycle_count = atoi(buffer);
    }
    
    /* 读取满充容量 */
    ret = read_file(BATTERY_CAPACITY_FULL_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        status->full_charge_capacity = atoi(buffer);
    }
    
    /* 读取健康度 */
    ret = read_file(BATTERY_HEALTH_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        if (strcmp(buffer, "Good") == 0) {
            status->health = 100;
        } else if (strcmp(buffer, "Overheat") == 0) {
            status->health = 50;
        } else if (strcmp(buffer, "Dead") == 0) {
            status->health = 0;
        } else {
            status->health = atoi(buffer);
        }
    }
    
    /* 计算容量 */
    if (status->full_charge_capacity > 0) {
        status->capacity_mah = status->full_charge_capacity;
    }
    
    return BATTERY_SUCCESS;
}

/* 获取电池信息 */
int battery_get_info(battery_context_t *ctx, battery_info_t *info) {
    char buffer[256];
    int ret;
    
    if (ctx->simulation_mode) {
        /* 模拟模式：返回模拟值 */
        *info = ctx->info;
        return BATTERY_SUCCESS;
    }
    
    /* 读取型号 */
    ret = read_file(BATTERY_MODEL_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        strncpy(info->model, buffer, sizeof(info->model) - 1);
    }
    
    /* 读取制造商 */
    ret = read_file(BATTERY_MANUFACTURER_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        strncpy(info->manufacturer, buffer, sizeof(info->manufacturer) - 1);
    }
    
    /* 读取序列号 */
    ret = read_file(BATTERY_SERIAL_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        strncpy(info->serial, buffer, sizeof(info->serial) - 1);
    }
    
    /* 读取设计容量 */
    ret = read_file(BATTERY_CAPACITY_DESIGN_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        info->design_capacity = atoi(buffer);
    }
    
    /* 读取设计电压 */
    ret = read_file(BATTERY_VOLTAGE_DESIGN_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        info->design_voltage = atoi(buffer);
    }
    
    /* 读取化学类型 */
    ret = read_file(BATTERY_CHEMISTRY_FILE, buffer, sizeof(buffer));
    if (ret == BATTERY_SUCCESS) {
        strncpy(info->chemistry, buffer, sizeof(info->chemistry) - 1);
    }
    
    return BATTERY_SUCCESS;
}

/* 设置充电状态 */
int battery_set_charge(battery_context_t *ctx, bool enable) {
    char value[16];
    snprintf(value, sizeof(value), "%d", enable ? 1 : 0);
    
    battery_log("设置充电状态: %s", enable ? "开启" : "关闭");
    
    if (ctx->simulation_mode) {
        /* 模拟模式：更新状态 */
        ctx->charge_config.enable = enable;
        ctx->status.status = enable ? 1 : 0;
        return BATTERY_SUCCESS;
    }
    
    /* 写入充电控制文件 */
    int ret = write_file(BATTERY_CHARGE_CONTROL_FILE, value);
    if (ret != BATTERY_SUCCESS) {
        battery_log("警告: 无法设置充电状态");
        return ret;
    }
    
    ctx->charge_config.enable = enable;
    return BATTERY_SUCCESS;
}

/* 设置充电电流 */
int battery_set_charge_current(battery_context_t *ctx, uint32_t current_ma) {
    char value[32];
    snprintf(value, sizeof(value), "%u", current_ma);
    
    battery_log("设置充电电流: %u mA", current_ma);
    
    if (ctx->simulation_mode) {
        /* 模拟模式：更新配置 */
        ctx->charge_config.current_ma = current_ma;
        return BATTERY_SUCCESS;
    }
    
    /* 写入电流限制文件 */
    int ret = write_file(BATTERY_CURRENT_MAX_FILE, value);
    if (ret != BATTERY_SUCCESS) {
        battery_log("警告: 无法设置充电电流");
        return ret;
    }
    
    ctx->charge_config.current_ma = current_ma;
    return BATTERY_SUCCESS;
}

/* 设置充电电压 */
int battery_set_charge_voltage(battery_context_t *ctx, uint32_t voltage_mv) {
    char value[32];
    snprintf(value, sizeof(value), "%u", voltage_mv);
    
    battery_log("设置充电电压: %u mV", voltage_mv);
    
    if (ctx->simulation_mode) {
        /* 模拟模式：更新配置 */
        ctx->charge_config.voltage_mv = voltage_mv;
        return BATTERY_SUCCESS;
    }
    
    /* 写入电压限制文件 */
    int ret = write_file(BATTERY_VOLTAGE_MAX_FILE, value);
    if (ret != BATTERY_SUCCESS) {
        battery_log("警告: 无法设置充电电压");
        return ret;
    }
    
    ctx->charge_config.voltage_mv = voltage_mv;
    return BATTERY_SUCCESS;
}

/* 获取充电配置 */
int battery_get_charge_config(battery_context_t *ctx, charge_config_t *config) {
    *config = ctx->charge_config;
    return BATTERY_SUCCESS;
}

/* 设置温度限制 */
int battery_set_temp_limits(battery_context_t *ctx, int32_t min_temp, int32_t max_temp) {
    battery_log("设置温度限制: %d°C - %d°C", min_temp, max_temp);
    
    ctx->charge_config.min_temp_c = min_temp;
    ctx->charge_config.max_temp_c = max_temp;
    
    return BATTERY_SUCCESS;
}

/* 检查电池保护 */
int battery_check_protection(battery_context_t *ctx) {
    battery_status_t status;
    int ret;
    
    ret = battery_get_status(ctx, &status);
    if (ret != BATTERY_SUCCESS) {
        return ret;
    }
    
    /* 检查温度保护 */
    if (status.temperature_c > ctx->charge_config.max_temp_c) {
        battery_log("警告: 温度过高 (%d°C > %d°C)", 
                   status.temperature_c, ctx->charge_config.max_temp_c);
        battery_set_charge(ctx, false);
        return BATTERY_ERROR;
    }
    
    if (status.temperature_c < ctx->charge_config.min_temp_c) {
        battery_log("警告: 温度过低 (%d°C < %d°C)", 
                   status.temperature_c, ctx->charge_config.min_temp_c);
        battery_set_charge(ctx, false);
        return BATTERY_ERROR;
    }
    
    /* 检查过压保护 */
    if (status.voltage_mv > ctx->charge_config.voltage_mv + 100) {
        battery_log("警告: 电压过高 (%d mV)", status.voltage_mv);
        battery_set_charge(ctx, false);
        return BATTERY_ERROR;
    }
    
    /* 检查过流保护 */
    if (status.current_ma > (int32_t)ctx->charge_config.current_ma + 500) {
        battery_log("警告: 电流过高 (%d mA)", status.current_ma);
        battery_set_charge(ctx, false);
        return BATTERY_ERROR;
    }
    
    return BATTERY_SUCCESS;
}

/* 计算电池健康度 */
int battery_calculate_health(battery_context_t *ctx) {
    battery_status_t status;
    battery_info_t info;
    int ret;
    
    ret = battery_get_status(ctx, &status);
    if (ret != BATTERY_SUCCESS) {
        return ret;
    }
    
    ret = battery_get_info(ctx, &info);
    if (ret != BATTERY_SUCCESS) {
        return ret;
    }
    
    /* 基于容量计算健康度 */
    if (info.design_capacity > 0) {
        status.health = (status.full_charge_capacity * 100) / info.design_capacity;
    }
    
    /* 基于循环次数调整健康度 */
    if (status.cycle_count > 0) {
        uint32_t reference_cycles = 500;
        uint32_t reference_health = 80;
        
        if (status.cycle_count >= reference_cycles) {
            status.health = reference_health;
        } else {
            status.health = 100 - (status.cycle_count * (100 - reference_health) / reference_cycles);
        }
    }
    
    /* 限制健康度范围 */
    if (status.health > 100) status.health = 100;
    if (status.health < 0) status.health = 0;
    
    ctx->status.health = status.health;
    
    return BATTERY_SUCCESS;
}

/* 估算剩余循环次数 */
int battery_estimate_remaining_cycles(battery_context_t *ctx, uint32_t *cycles) {
    battery_status_t status;
    int ret;
    
    ret = battery_get_status(ctx, &status);
    if (ret != BATTERY_SUCCESS) {
        return ret;
    }
    
    /* 假设电池在健康度 < 60% 时寿命结束 */
    uint32_t end_of_life_health = 60;
    
    if (status.health <= end_of_life_health) {
        *cycles = 0;
        return BATTERY_SUCCESS;
    }
    
    /* 计算剩余循环次数 */
    float health_degradation_per_cycle = (100.0 - end_of_life_health) / 500.0;
    float remaining_health = status.health - end_of_life_health;
    *cycles = remaining_health / health_degradation_per_cycle;
    
    return BATTERY_SUCCESS;
}
