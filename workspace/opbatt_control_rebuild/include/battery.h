/**
 * @file battery.h
 * @brief 电池状态监控
 * 
 * 提供电池状态读取和监控功能
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>
#include <stdbool.h>

/* 电池状态结构体 */
typedef struct {
    /* 基础状态 */
    int temperature;        /* 电池温度 (0.1°C) */
    int current_ma;         /* 电流 (mA) */
    int voltage_mv;         /* 电压 (mV) */
    int soc_percent;        /* 电量百分比 */
    bool is_charging;       /* 是否在充电 */
    
    /* 容量信息 */
    int capacity_mah;       /* 满充容量 (mAh) */
    int current_cap_mah;    /* 当前容量 (mAh) */
    int remaining_mah;      /* 剩余容量 (mAh) */
    
    /* 健康信息 */
    int health_percent;     /* 健康度 (%) */
    int cycle_count;        /* 循环次数 */
    
    /* 其他信息 */
    char manu_date[32];     /* 制造日期 */
    char battery_type[32];  /* 电池类型 */
    char charge_type[64];   /* 充电器类型 */
    
    /* 充电信息 */
    int power_mw;           /* 充电功率 (mW) */
    int time_to_full_sec;   /* 预估充满时间 (秒) */
    
    /* 双电芯信息 */
    int cell1_voltage_mv;   /* 电芯1电压 (mV) */
    int cell2_voltage_mv;   /* 电芯2电压 (mV) */
    int voltage_diff_mv;    /* 压差 (mV) */
    
    /* 其他 */
    int shell_temp;         /* 外壳温度 (0.1°C) */
    int shutdown_voltage_mv;/* 关机电压 (mV) */
} battery_status_t;

/* 读取电池状态 */
int battery_read_status(battery_status_t *status);

/* 读取电池温度 */
int battery_read_temperature(int *temp);

/* 读取电池电流 */
int battery_read_current(int *current_ma);

/* 读取电池电压 */
int battery_read_voltage(int *voltage_mv);

/* 读取电池电量 */
int battery_read_soc(int *soc_percent);

/* 检查是否在充电 */
int battery_is_charging(bool *charging);

/* 读取电池容量 */
int battery_read_capacity(int *capacity_mah);

/* 读取电池健康度 */
int battery_read_health(int *health_percent);

/* 读取充电功率 */
int battery_read_power(int *power_mw);

/* 读取预估充满时间 */
int battery_read_time_to_full(int *time_sec);

/* 打印电池状态 */
void battery_print_status(battery_status_t *status);

#endif /* BATTERY_H */
