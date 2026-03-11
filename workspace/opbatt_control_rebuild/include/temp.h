/**
 * @file temp.h
 * @brief 温度控制模块
 * 
 * 提供温度监控和温度伪装功能
 */

#ifndef TEMP_H
#define TEMP_H

#include <stdint.h>
#include <stdbool.h>

/* 温度控制配置 */
typedef struct {
    bool fake_temp_enabled;     /* 温度伪装开关 */
    int fake_temp;              /* 伪装温度 (0.1°C) */
    int over_temp_threshold;    /* 过温阈值 (0.1°C) */
    int high_temp_threshold;    /* 高温阈值 (0.1°C) */
    int normal_high_temp;       /* 正常高温阈值 (0.1°C) */
    int normal_low_temp;        /* 正常低温阈值 (0.1°C) */
} temp_control_t;

/* 温度策略 */
typedef enum {
    TEMP_STRATEGY_EXTREME_LOW = 0,   /* 极低温 */
    TEMP_STRATEGY_LOW = 1,           /* 低温 */
    TEMP_STRATEGY_LITTLE_COLD = 2,   /* 微冷 */
    TEMP_STRATEGY_NORMAL = 3,        /* 正常 */
    TEMP_STRATEGY_NORMAL_HIGH = 4,   /* 正常高温 */
    TEMP_STRATEGY_HIGH = 5,          /* 高温 */
    TEMP_STRATEGY_OVER_HIGH = 6,     /* 过高温 */
    TEMP_STRATEGY_CRITICAL = 7       /* 危险温度 */
} temp_strategy_t;

/* 初始化温度控制模块 */
int temp_init(void);

/* 设置温度伪装开关 */
int temp_set_fake_enable(bool enable);

/* 获取温度伪装开关状态 */
int temp_get_fake_enable(bool *enabled);

/* 设置伪装温度 */
int temp_set_fake_temp(int temp);

/* 获取伪装温度 */
int temp_get_fake_temp(int *temp);

/* 获取有效温度（考虑温度伪装） */
int temp_get_effective_temp(int actual_temp, bool fake_enabled, int fake_temp);

/* 获取温度策略 */
temp_strategy_t temp_get_strategy(int temp);

/* 根据温度策略获取充电电流限制 */
int temp_get_current_limit(temp_strategy_t strategy);

/* 检查温度是否过高 */
bool temp_is_over_temp(int temp);

/* 检查温度是否在安全范围内 */
bool temp_is_safe(int temp);

#endif /* TEMP_H */
