/**
 * battery_control - 电池控制程序
 * 纯电池控制版本（无网络、无验证）
 */

#ifndef BATTERY_CONTROL_H
#define BATTERY_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

/* 版本信息 */
#define BATTERY_VERSION "1.0.0"
#define BATTERY_BUILD_DATE "2026-03-10"

/* 错误码 */
typedef enum {
    BATTERY_SUCCESS = 0,
    BATTERY_ERROR = -1,
    BATTERY_ERROR_INIT = -2,
    BATTERY_ERROR_READ = -3,
    BATTERY_ERROR_WRITE = -4,
    BATTERY_ERROR_NOT_FOUND = -5
} battery_error_t;

/* 电池状态 */
typedef struct {
    uint32_t voltage_mv;      /* 电压 (mV) */
    int32_t  current_ma;      /* 电流 (mA) 正数为充电，负数为放电 */
    uint32_t capacity_mah;    /* 容量 (mAh) */
    uint32_t level_percent;   /* 电量百分比 (0-100) */
    uint8_t  status;          /* 状态：0=空闲, 1=充电, 2=放电 */
    uint8_t  health;          /* 健康度 (0-100) */
    int32_t  temperature_c;   /* 温度 (°C) */
    uint32_t cycle_count;     /* 循环次数 */
    uint32_t full_charge_capacity; /* 满充容量 (mAh) */
} battery_status_t;

/* 充电配置 */
typedef struct {
    bool enable;              /* 是否启用充电 */
    uint32_t current_ma;      /* 充电电流 (mA) */
    uint32_t voltage_mv;      /* 充电电压 (mV) */
    uint32_t max_temp_c;      /* 最高温度 (°C) */
    uint32_t min_temp_c;      /* 最低温度 (°C) */
} charge_config_t;

/* 电池信息 */
typedef struct {
    char model[64];           /* 电池型号 */
    char manufacturer[64];    /* 制造商 */
    char serial[64];          /* 序列号 */
    uint32_t design_capacity; /* 设计容量 (mAh) */
    uint32_t design_voltage;  /* 设计电压 (mV) */
    char chemistry[32];       /* 化学类型 */
} battery_info_t;

/* 全局上下文 */
typedef struct {
    battery_status_t status;
    charge_config_t charge_config;
    battery_info_t info;
    bool initialized;
    bool simulation_mode;
} battery_context_t;

/* 函数声明 */

/* 初始化和清理 */
int battery_init(battery_context_t *ctx);
int battery_cleanup(battery_context_t *ctx);

/* 电池状态 */
int battery_get_status(battery_context_t *ctx, battery_status_t *status);
int battery_get_info(battery_context_t *ctx, battery_info_t *info);

/* 充电控制 */
int battery_set_charge(battery_context_t *ctx, bool enable);
int battery_set_charge_current(battery_context_t *ctx, uint32_t current_ma);
int battery_set_charge_voltage(battery_context_t *ctx, uint32_t voltage_mv);
int battery_get_charge_config(battery_context_t *ctx, charge_config_t *config);

/* 电池保护 */
int battery_set_temp_limits(battery_context_t *ctx, int32_t min_temp, int32_t max_temp);
int battery_check_protection(battery_context_t *ctx);

/* 电池健康 */
int battery_calculate_health(battery_context_t *ctx);
int battery_estimate_remaining_cycles(battery_context_t *ctx, uint32_t *cycles);

/* 工具函数 */
void battery_log(const char *format, ...);
void battery_print_status(const battery_status_t *status);
void battery_print_info(const battery_info_t *info);

#endif /* BATTERY_CONTROL_H */
