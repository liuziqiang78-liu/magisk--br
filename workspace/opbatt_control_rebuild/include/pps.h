/**
 * @file pps.h
 * @brief PPS（Programmable Power Supply）协议控制
 * 
 * 提供 PPS 快充协议的控制功能
 */

#ifndef PPS_H
#define PPS_H

#include <stdint.h>
#include <stdbool.h>

/* PPS 配置参数 */
typedef struct {
    bool enabled;               /* PPS 协议开关 */
    int max_current_ma;         /* PPS 最大电流 (mA) */
    int soc_monitor[2];         /* PPS SOC 监控点 */
    int interval_ms[2];         /* PPS 采样间隔 (ms) */
} pps_config_t;

/* PPS 温度策略 */
typedef enum {
    PPS_TEMP_EXTREME_LOW = 0,   /* 极低温 (<50°C) */
    PPS_TEMP_LOW = 1,           /* 低温 (50-120°C) */
    PPS_TEMP_LITTLE_COLD = 2,   /* 微冷 (120-160°C) */
    PPS_TEMP_LITTLE_COLD_HIGH = 3, /* 微冷高温 (160-210°C) */
    PPS_TEMP_NORMAL_LOW = 4,    /* 正常低温 (210-400°C) */
    PPS_TEMP_NORMAL_HIGH = 5,   /* 正常高温 (400-480°C) */
    PPS_TEMP_HIGH = 6,          /* 高温 (480-530°C) */
    PPS_TEMP_OVER_HIGH = 7      /* 过高温 (>530°C) */
} pps_temp_strategy_t;

/* PPS 充电策略 */
typedef enum {
    PPS_CHARGE_FAST = 0,        /* 快速充电 */
    PPS_CHARGE_NORMAL = 1,      /* 正常充电 */
    PPS_CHARGE_CONSTANT_VOLTAGE = 2, /* 恒压充电 */
    PPS_CHARGE_TRICKLE = 3,     /* 涓流充电 */
    PPS_CHARGE_FULL = 4,        /* 充满 */
    PPS_CHARGE_STOP = 5         /* 停止充电 */
} pps_charge_strategy_t;

/* PPS 充电参数 */
typedef struct {
    int target_vbus_mv;         /* 目标电压 (mV) */
    int curr_max_ma;            /* 最大电流 (mA) */
    int min_voltage_mv;         /* 最小电压 (mV) */
    int max_voltage_mv;         /* 最大电压 (mV) */
} pps_charge_params_t;

/* 初始化 PPS 模块 */
int pps_init(void);

/* 设置 PPS 协议开关 */
int pps_set_enable(bool enable);

/* 获取 PPS 协议开关状态 */
int pps_get_enable(bool *enabled);

/* 设置 PPS 最大电流 */
int pps_set_max_current(int current_ma);

/* 获取 PPS 最大电流 */
int pps_get_max_current(int *current_ma);

/* 根据 SOC 和温度获取 PPS 充电电流 */
int pps_get_charge_current(int soc, int temp);

/* 获取 PPS 温度策略 */
pps_temp_strategy_t pps_get_temp_strategy(int temp);

/* 获取 PPS 充电策略 */
pps_charge_strategy_t pps_get_charge_strategy(int soc, int temp);

/* 检查 PPS 是否可用 */
int pps_is_available(bool *available);

#endif /* PPS_H */
