/**
 * @file svooc.c
 * @brief SVOOC（Super VOOC）协议控制实现
 * 
 * 提供 SVOOC 快充协议的控制功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svooc.h"
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* SVOOC 温度阈值参数 */
static svooc_temp_thresholds_t svooc_thresholds = {
    .normal_low_temp = 420,    /* 42°C */
    .normal_high_temp = 480,   /* 48°C */
    .high_temp = 530,          /* 53°C */
    .over_high_temp = 550      /* 55°C */
};

/* SVOOC 配置 */
static svooc_config_t svooc_config = {
    .enabled = false
};

/* 初始化 SVOOC 模块 */
int svooc_init(void) {
    LOG_INFO("SVOOC module initialized");
    return 0;
}

/* 设置 SVOOC 协议开关 */
int svooc_set_enable(bool enable) {
    /* 检查 SVOOC enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_SVOOC_ENABLE)) {
        if (sysfs_write_int(SYSFS_OPLUS_SVOOC_ENABLE, enable ? 1 : 0) != 0) {
            LOG_ERROR("Failed to set SVOOC enable to %d", enable);
            return -1;
        }
    }
    
    svooc_config.enabled = enable;
    LOG_INFO("SVOOC enable set to: %d", enable);
    return 0;
}

/* 获取 SVOOC 协议开关状态 */
int svooc_get_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    /* 检查 SVOOC enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_SVOOC_ENABLE)) {
        int value;
        if (sysfs_read_int(SYSFS_OPLUS_SVOOC_ENABLE, &value) == 0) {
            *enabled = (value == 1);
            return 0;
        }
    }
    
    *enabled = svooc_config.enabled;
    return 0;
}

/* 获取 SVOOC 温度策略 */
svooc_temp_strategy_t svooc_get_temp_strategy(int temp) {
    /* 温度单位是 0.1°C */
    if (temp < svooc_thresholds.normal_low_temp) {
        return SVOOC_TEMP_NORMAL;
    } else if (temp < svooc_thresholds.normal_high_temp) {
        return SVOOC_TEMP_NORMAL_HIGH;
    } else if (temp < svooc_thresholds.high_temp) {
        return SVOOC_TEMP_HIGH;
    } else if (temp < svooc_thresholds.over_high_temp) {
        return SVOOC_TEMP_OVER_HIGH;
    } else {
        return SVOOC_TEMP_CRITICAL;
    }
}

/* 获取 SVOOC 充电策略 */
svooc_charge_strategy_t svooc_get_charge_strategy(int soc, int temp) {
    svooc_temp_strategy_t temp_strategy = svooc_get_temp_strategy(temp);
    
    /* 危险温度停止充电 */
    if (temp_strategy == SVOOC_TEMP_CRITICAL) {
        return SVOOC_CHARGE_STOP;
    }
    
    /* 过高温降低充电功率 */
    if (temp_strategy == SVOOC_TEMP_OVER_HIGH) {
        return SVOOC_CHARGE_TRICKLE;
    }
    
    /* 根据 SOC 确定充电策略 */
    if (soc < 20) {
        return SVOOC_CHARGE_FAST;
    } else if (soc < 35) {
        return SVOOC_CHARGE_FAST;
    } else if (soc < 55) {
        return SVOOC_CHARGE_NORMAL;
    } else if (soc < 75) {
        return SVOOC_CHARGE_NORMAL;
    } else if (soc < 85) {
        return SVOOC_CHARGE_CONSTANT_VOLTAGE;
    } else if (soc < 99) {
        return SVOOC_CHARGE_TRICKLE;
    } else {
        return SVOOC_CHARGE_FULL;
    }
}

/* 检查 SVOOC 是否可用 */
int svooc_is_available(bool *available) {
    if (!available) {
        LOG_ERROR("Invalid parameters: available=%p", available);
        return -1;
    }
    
    /* 检查 SVOOC enable 节点是否存在 */
    *available = sysfs_exists(SYSFS_OPLUS_SVOOC_ENABLE);
    return 0;
}
