/**
 * @file svooc.h
 * @brief SVOOC（Super VOOC）协议控制
 * 
 * 提供 SVOOC 快充协议的控制功能
 */

#ifndef SVOOC_H
#define SVOOC_H

#include <stdint.h>
#include <stdbool.h>

/* SVOOC 配置参数 */
typedef struct {
    bool enabled;               /* SVOOC 协议开关 */
} svooc_config_t;

/* SVOOC 温度阈值 */
typedef enum {
    SVOOC_TEMP_NORMAL = 0,      /* 正常温度 (<42°C) */
    SVOOC_TEMP_NORMAL_HIGH = 1, /* 正常高温 (42-48°C) */
    SVOOC_TEMP_HIGH = 2,        /* 高温 (48-53°C) */
    SVOOC_TEMP_OVER_HIGH = 3,   /* 过高温 (53-55°C) */
    SVOOC_TEMP_CRITICAL = 4     /* 危险温度 (>55°C) */
} svooc_temp_strategy_t;

/* SVOOC 充电策略 */
typedef enum {
    SVOOC_CHARGE_FAST = 0,      /* 快速充电 */
    SVOOC_CHARGE_NORMAL = 1,    /* 正常充电 */
    SVOOC_CHARGE_CONSTANT_VOLTAGE = 2, /* 恒压充电 */
    SVOOC_CHARGE_TRICKLE = 3,   /* 涓流充电 */
    SVOOC_CHARGE_FULL = 4,      /* 充满 */
    SVOOC_CHARGE_STOP = 5       /* 停止充电 */
} svooc_charge_strategy_t;

/* SVOOC 温度阈值参数 */
typedef struct {
    int normal_low_temp;        /* 正常低温阈值 (0.1°C) */
    int normal_high_temp;       /* 正常高温阈值 (0.1°C) */
    int high_temp;              /* 高温阈值 (0.1°C) */
    int over_high_temp;         /* 过高温阈值 (0.1°C) */
} svooc_temp_thresholds_t;

/* 初始化 SVOOC 模块 */
int svooc_init(void);

/* 设置 SVOOC 协议开关 */
int svooc_set_enable(bool enable);

/* 获取 SVOOC 协议开关状态 */
int svooc_get_enable(bool *enabled);

/* 获取 SVOOC 温度策略 */
svooc_temp_strategy_t svooc_get_temp_strategy(int temp);

/* 获取 SVOOC 充电策略 */
svooc_charge_strategy_t svooc_get_charge_strategy(int soc, int temp);

/* 检查 SVOOC 是否可用 */
int svooc_is_available(bool *available);

#endif /* SVOOC_H */
