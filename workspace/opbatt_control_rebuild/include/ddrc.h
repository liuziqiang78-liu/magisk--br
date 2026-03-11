/**
 * @file ddrc.h
 * @brief DDRC（Deep Discharge Recovery）模块
 * 
 * 提供深度放电恢复功能
 */

#ifndef DDRC_H
#define DDRC_H

#include <stdint.h>
#include <stdbool.h>

/* DDRC 配置参数 */
typedef struct {
    int uv_thr;                /* 低压阈值 (mV) */
    int count_thr;             /* 计数阈值 */
    int vbat_soc;              /* 电压SOC阈值 (%) */
    int soc_range[5];          /* SOC范围 (%) */
    int temp_range[3];         /* 温度范围 (0.1°C) */
} ddrc_config_t;

/* DDRC 状态 */
typedef struct {
    bool ddrc_active;          /* DDRC是否激活 */
    int deep_discharge_count;  /* 深度放电计数 */
    int limited_current_ma;    /* 限制电流 (mA) */
} ddrc_state_t;

/* 初始化 DDRC 模块 */
int ddrc_init(void);

/* 检查是否触发 DDRC */
bool ddrc_should_trigger(int voltage_mv, int count);

/* 获取 DDRC 限制电流 */
int ddrc_get_limited_current(int soc, int voltage_mv);

/* 获取深度放电计数 */
int ddrc_get_count(int *count);

/* 重置深度放电计数 */
int ddrc_reset_count(void);

/* 检查 DDRC 是否激活 */
bool ddrc_is_active(void);

/* 获取 DDRC 状态 */
int ddrc_get_state(ddrc_state_t *state);

#endif /* DDRC_H */
