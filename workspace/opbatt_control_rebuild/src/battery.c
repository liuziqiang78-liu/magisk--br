/**
 * @file battery.c
 * @brief 电池状态监控实现
 * 
 * 提供电池状态读取和监控功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "battery.h"
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* 读取电池状态 */
int battery_read_status(battery_status_t *status) {
    int ret;
    int temp_raw;
    
    if (!status) {
        LOG_ERROR("Invalid parameters: status=%p", status);
        return -1;
    }
    
    memset(status, 0, sizeof(battery_status_t));
    
    /* 读取温度 */
    if (sysfs_read_int(SYSFS_BATTERY_TEMP, &temp_raw) == 0) {
        status->temperature = temp_raw;
    }
    
    /* 读取电流 */
    if (sysfs_read_int(SYSFS_BATTERY_CURRENT_NOW, &temp_raw) == 0) {
        status->current_ma = current_to_ma(temp_raw);
    }
    
    /* 读取电压 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_VBAT_UV, &temp_raw) == 0) {
        status->voltage_mv = voltage_to_mv(temp_raw);
    }
    
    /* 读取电量 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_CHIP_SOC, &temp_raw) == 0) {
        status->soc_percent = temp_raw;
    }
    
    /* 读取容量 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_FCC, &temp_raw) == 0) {
        status->capacity_mah = temp_raw;
    }
    
    /* 读取当前容量 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_CC, &temp_raw) == 0) {
        status->current_cap_mah = temp_raw;
    }
    
    /* 读取剩余容量 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_RM, &temp_raw) == 0) {
        status->remaining_mah = temp_raw;
    }
    
    /* 读取健康度 */
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_SOH, &temp_raw) == 0) {
        status->health_percent = temp_raw;
    }
    
    /* 读取充电功率 */
    if (sysfs_read_int(SYSFS_OPLUS_COMMON_CPA_POWER, &temp_raw) == 0) {
        status->power_mw = power_to_mw(temp_raw);
    }
    
    /* 读取预估充满时间 */
    if (sysfs_read_int(SYSFS_BATTERY_TIME_TO_FULL, &temp_raw) == 0) {
        status->time_to_full_sec = temp_raw;
    }
    
    /* 读取制造日期 */
    sysfs_read_string(SYSFS_OPLUS_BATTERY_MANU_DATE, status->manu_date, sizeof(status->manu_date));
    
    /* 读取电池类型 */
    sysfs_read_string(SYSFS_OPLUS_BATTERY_TYPE, status->battery_type, sizeof(status->battery_type));
    
    /* 读取充电器类型 */
    sysfs_read_string(SYSFS_OPLUS_BATTERY_LOG, status->charge_type, sizeof(status->charge_type));
    
    /* 检查是否在充电 */
    battery_is_charging(&status->is_charging);
    
    return 0;
}

/* 读取电池温度 */
int battery_read_temperature(int *temp) {
    if (!temp) {
        LOG_ERROR("Invalid parameters: temp=%p", temp);
        return -1;
    }
    
    return sysfs_read_int(SYSFS_BATTERY_TEMP, temp);
}

/* 读取电池电流 */
int battery_read_current(int *current_ma) {
    int raw_current;
    
    if (!current_ma) {
        LOG_ERROR("Invalid parameters: current_ma=%p", current_ma);
        return -1;
    }
    
    if (sysfs_read_int(SYSFS_BATTERY_CURRENT_NOW, &raw_current) != 0) {
        return -1;
    }
    
    *current_ma = current_to_ma(raw_current);
    return 0;
}

/* 读取电池电压 */
int battery_read_voltage(int *voltage_mv) {
    int raw_voltage;
    
    if (!voltage_mv) {
        LOG_ERROR("Invalid parameters: voltage_mv=%p", voltage_mv);
        return -1;
    }
    
    if (sysfs_read_int(SYSFS_OPLUS_BATTERY_VBAT_UV, &raw_voltage) != 0) {
        return -1;
    }
    
    *voltage_mv = voltage_to_mv(raw_voltage);
    return 0;
}

/* 读取电池电量 */
int battery_read_soc(int *soc_percent) {
    if (!soc_percent) {
        LOG_ERROR("Invalid parameters: soc_percent=%p", soc_percent);
        return -1;
    }
    
    return sysfs_read_int(SYSFS_OPLUS_BATTERY_CHIP_SOC, soc_percent);
}

/* 检查是否在充电 */
int battery_is_charging(bool *charging) {
    int online;
    
    if (!charging) {
        LOG_ERROR("Invalid parameters: charging=%p", charging);
        return -1;
    }
    
    if (sysfs_read_int(SYSFS_USB_ONLINE, &online) != 0) {
        *charging = false;
        return -1;
    }
    
    *charging = (online == 1);
    return 0;
}

/* 读取电池容量 */
int battery_read_capacity(int *capacity_mah) {
    if (!capacity_mah) {
        LOG_ERROR("Invalid parameters: capacity_mah=%p", capacity_mah);
        return -1;
    }
    
    return sysfs_read_int(SYSFS_OPLUS_BATTERY_FCC, capacity_mah);
}

/* 读取电池健康度 */
int battery_read_health(int *health_percent) {
    if (!health_percent) {
        LOG_ERROR("Invalid parameters: health_percent=%p", health_percent);
        return -1;
    }
    
    return sysfs_read_int(SYSFS_OPLUS_BATTERY_SOH, health_percent);
}

/* 读取充电功率 */
int battery_read_power(int *power_mw) {
    int raw_power;
    
    if (!power_mw) {
        LOG_ERROR("Invalid parameters: power_mw=%p", power_mw);
        return -1;
    }
    
    if (sysfs_read_int(SYSFS_OPLUS_COMMON_CPA_POWER, &raw_power) != 0) {
        return -1;
    }
    
    *power_mw = power_to_mw(raw_power);
    return 0;
}

/* 读取预估充满时间 */
int battery_read_time_to_full(int *time_sec) {
    if (!time_sec) {
        LOG_ERROR("Invalid parameters: time_sec=%p", time_sec);
        return -1;
    }
    
    return sysfs_read_int(SYSFS_BATTERY_TIME_TO_FULL, time_sec);
}

/* 打印电池状态 */
void battery_print_status(battery_status_t *status) {
    if (!status) {
        return;
    }
    
    printf("=== Battery Status ===\n");
    printf("SOC: %d%%\n", status->soc_percent);
    printf("Voltage: %dmV\n", status->voltage_mv);
    printf("Current: %dmA\n", status->current_ma);
    printf("Temperature: %d°C\n", temp_to_celsius(status->temperature));
    printf("Capacity: %dmAh\n", status->capacity_mah);
    printf("Remaining: %dmAh\n", status->remaining_mah);
    printf("Health: %d%%\n", status->health_percent);
    printf("Power: %dmW\n", status->power_mw);
    printf("Charging: %s\n", status->is_charging ? "Yes" : "No");
    printf("Type: %s\n", status->battery_type);
    printf("Manufacture Date: %s\n", status->manu_date);
    printf("=====================\n");
}
