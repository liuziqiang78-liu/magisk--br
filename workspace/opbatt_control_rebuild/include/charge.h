/**
 * @file charge.h
 * @brief 充电控制模块
 * 
 * 提供充电控制功能，包括充放电控制、电流电压设置等
 */

#ifndef CHARGE_H
#define CHARGE_H

#include <stdint.h>
#include <stdbool.h>

/* 充电状态 */
typedef enum {
    CHARGE_STATUS_UNKNOWN = 0,
    CHARGE_STATUS_DISCHARGING = 1,
    CHARGE_STATUS_CHARGING = 2,
    CHARGE_STATUS_FULL = 3,
    CHARGE_STATUS_NOT_CHARGING = 4
} charge_status_t;

/* 充电控制结构体 */
typedef struct {
    bool enabled;           /* 充电开关 */
    int current_ma;         /* 充电电流 (mA) */
    int voltage_mv;         /* 充电电压 (mV) */
    charge_status_t status; /* 充电状态 */
} charge_control_t;

/* 初始化充电控制 */
int charge_init(void);

/* 设置充电开关 */
int charge_set_enable(bool enable);

/* 获取充电开关状态 */
int charge_get_enable(bool *enabled);

/* 设置充电电流 */
int charge_set_current(int current_ma);

/* 获取充电电流 */
int charge_get_current(int *current_ma);

/* 设置充电电压 */
int charge_set_voltage(int voltage_mv);

/* 获取充电电压 */
int charge_get_voltage(int *voltage_mv);

/* 获取充电状态 */
int charge_get_status(char *status_str, size_t size);

/* 检查充电器是否连接 */
int charge_is_online(bool *online);

/* 停止充电 */
int charge_stop(void);

/* 开始充电 */
int charge_start(void);

#endif /* CHARGE_H */
