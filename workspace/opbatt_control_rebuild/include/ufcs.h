/**
 * @file ufcs.h
 * @brief UFCS（Unified Fast Charging Specification）协议控制
 * 
 * 提供 UFCS 快充协议的控制功能
 */

#ifndef UFCS_H
#define UFCS_H

#include <stdint.h>
#include <stdbool.h>

/* UFCS 配置参数 */
typedef struct {
    bool enabled;               /* UFCS 协议开关 */
    int max_current_ma;         /* UFCS 最大电流 (mA) */
    int soc_monitor[2];         /* UFCS SOC 监控点 */
    int interval_ms[2];         /* UFCS 采样间隔 (ms) */
    int reset_count;            /* UFCS 降功率重置次数 */
    int reset_delay_sec;        /* UFCS 重置延迟 (秒) */
} ufcs_config_t;

/* UFCS 温度策略 */
typedef enum {
    UFCS_TEMP_EXTREME_LOW = 0,  /* 极低温 (<50°C) */
    UFCS_TEMP_LOW = 1,          /* 低温 (50-120°C) */
    UFCS_TEMP_LITTLE_COLD = 2,  /* 微冷 (120-160°C) */
    UFCS_TEMP_LITTLE_COLD_HIGH = 3, /* 微冷高温 (160-210°C) */
    UFCS_TEMP_NORMAL_LOW = 4,   /* 正常低温 (210-400°C) */
    UFCS_TEMP_NORMAL_HIGH = 5,  /* 正常高温 (400-480°C) */
    UFCS_TEMP_HIGH = 6,         /* 高温 (480-530°C) */
    UFCS_TEMP_OVER_HIGH = 7     /* 过高温 (>530°C) */
} ufcs_temp_strategy_t;

/* UFCS 充电策略 */
typedef enum {
    UFCS_CHARGE_FAST = 0,       /* 快速充电 */
    UFCS_CHARGE_NORMAL = 1,     /* 正常充电 */
    UFCS_CHARGE_CONSTANT_VOLTAGE = 2, /* 恒压充电 */
    UFCS_CHARGE_TRICKLE = 3,    /* 涓流充电 */
    UFCS_CHARGE_FULL = 4,       /* 充满 */
    UFCS_CHARGE_STOP = 5        /* 停止充电 */
} ufcs_charge_strategy_t;

/* UFCS 充电参数 */
typedef struct {
    int target_vbus_mv;         /* 目标电压 (mV) */
    int curr_max_ma;            /* 最大电流 (mA) */
    int full_cool_sw_vbat_mv;   /* 低温充满电压 (mV) */
    int full_normal_sw_vbat_mv; /* 正常充满电压 (mV) */
    int full_warm_vbat_mv;      /* 高温充满电压 (mV) */
} ufcs_charge_params_t;

/* 初始化 UFCS 模块 */
int ufcs_init(void);

/* 设置 UFCS 协议开关 */
int ufcs_set_enable(bool enable);

/* 获取 UFCS 协议开关状态 */
int ufcs_get_enable(bool *enabled);

/* 设置 UFCS 最大电流 */
int ufcs_set_max_current(int current_ma);

/* 获取 UFCS 最大电流 */
int ufcs_get_max_current(int *current_ma);

/* 根据 SOC 和温度获取 UFCS 充电电流 */
int ufcs_get_charge_current(int soc, int temp);

/* 获取 UFCS 温度策略 */
ufcs_temp_strategy_t ufcs_get_temp_strategy(int temp);

/* 获取 UFCS 充电策略 */
ufcs_charge_strategy_t ufcs_get_charge_strategy(int soc, int temp);

/* 重置 UFCS 充电器 */
int ufcs_reset_charger(void);

/* 检查 UFCS 是否可用 */
int ufcs_is_available(bool *available);

#endif /* UFCS_H */
