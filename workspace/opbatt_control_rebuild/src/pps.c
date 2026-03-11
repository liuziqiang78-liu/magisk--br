/**
 * @file pps.c
 * @brief PPS（Programmable Power Supply）协议控制实现
 * 
 * 提供 PPS 快充协议的控制功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pps.h"
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* PPS 充电参数 */
static pps_charge_params_t pps_params = {
    .target_vbus_mv = 20000,
    .curr_max_ma = 6500,
    .min_voltage_mv = 3000,
    .max_voltage_mv = 20000
};

/* PPS 配置 */
static pps_config_t pps_config = {
    .enabled = false,
    .max_current_ma = 5000,
    .soc_monitor = {20, 68},
    .interval_ms = {650, 400}
};

/* 初始化 PPS 模块 */
int pps_init(void) {
    LOG_INFO("PPS module initialized");
    return 0;
}

/* 设置 PPS 协议开关 */
int pps_set_enable(bool enable) {
    /* 检查 PPS enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_PPS_ENABLE)) {
        if (sysfs_write_int(SYSFS_OPLUS_PPS_ENABLE, enable ? 1 : 0) != 0) {
            LOG_ERROR("Failed to set PPS enable to %d", enable);
            return -1;
        }
    }
    
    pps_config.enabled = enable;
    LOG_INFO("PPS enable set to: %d", enable);
    return 0;
}

/* 获取 PPS 协议开关状态 */
int pps_get_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    /* 检查 PPS enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_PPS_ENABLE)) {
        int value;
        if (sysfs_read_int(SYSFS_OPLUS_PPS_ENABLE, &value) == 0) {
            *enabled = (value == 1);
            return 0;
        }
    }
    
    *enabled = pps_config.enabled;
    return 0;
}

/* 设置 PPS 最大电流 */
int pps_set_max_current(int current_ma) {
    if (current_ma < 3000 || current_ma > 6500) {
        LOG_ERROR("Invalid PPS max current: %d", current_ma);
        return -1;
    }
    
    pps_config.max_current_ma = current_ma;
    LOG_INFO("PPS max current set to: %dmA", current_ma);
    return 0;
}

/* 获取 PPS 最大电流 */
int pps_get_max_current(int *current_ma) {
    if (!current_ma) {
        LOG_ERROR("Invalid parameters: current_ma=%p", current_ma);
        return -1;
    }
    
    *current_ma = pps_config.max_current_ma;
    return 0;
}

/* 获取 PPS 温度策略 */
pps_temp_strategy_t pps_get_temp_strategy(int temp) {
    /* 温度单位是 0.1°C */
    if (temp < 50) {
        return PPS_TEMP_EXTREME_LOW;
    } else if (temp < 120) {
        return PPS_TEMP_LOW;
    } else if (temp < 160) {
        return PPS_TEMP_LITTLE_COLD;
    } else if (temp < 210) {
        return PPS_TEMP_LITTLE_COLD_HIGH;
    } else if (temp < 400) {
        return PPS_TEMP_NORMAL_LOW;
    } else if (temp < 480) {
        return PPS_TEMP_NORMAL_HIGH;
    } else if (temp < 530) {
        return PPS_TEMP_HIGH;
    } else {
        return PPS_TEMP_OVER_HIGH;
    }
}

/* 获取 PPS 充电策略 */
pps_charge_strategy_t pps_get_charge_strategy(int soc, int temp) {
    pps_temp_strategy_t temp_strategy = pps_get_temp_strategy(temp);
    
    /* 过高温停止充电 */
    if (temp_strategy == PPS_TEMP_OVER_HIGH) {
        return PPS_CHARGE_STOP;
    }
    
    /* 根据 SOC 确定充电策略 */
    if (soc < 20) {
        return PPS_CHARGE_FAST;
    } else if (soc < 35) {
        return PPS_CHARGE_FAST;
    } else if (soc < 55) {
        return PPS_CHARGE_NORMAL;
    } else if (soc < 75) {
        return PPS_CHARGE_NORMAL;
    } else if (soc < 85) {
        return PPS_CHARGE_CONSTANT_VOLTAGE;
    } else if (soc < 99) {
        return PPS_CHARGE_TRICKLE;
    } else {
        return PPS_CHARGE_FULL;
    }
}

/* 根据 SOC 和温度获取 PPS 充电电流 */
int pps_get_charge_current(int soc, int temp) {
    pps_charge_strategy_t strategy = pps_get_charge_strategy(soc, temp);
    pps_temp_strategy_t temp_strategy = pps_get_temp_strategy(temp);
    
    /* 根据策略和温度计算充电电流 */
    switch (strategy) {
        case PPS_CHARGE_FAST:
            /* 快速充电：根据温度调整 */
            switch (temp_strategy) {
                case PPS_TEMP_EXTREME_LOW:
                    return pps_config.max_current_ma * 0.3;
                case PPS_TEMP_LOW:
                    return pps_config.max_current_ma * 0.5;
                case PPS_TEMP_LITTLE_COLD:
                    return pps_config.max_current_ma * 0.7;
                case PPS_TEMP_LITTLE_COLD_HIGH:
                    return pps_config.max_current_ma * 0.8;
                case PPS_TEMP_NORMAL_LOW:
                case PPS_TEMP_NORMAL_HIGH:
                    return pps_config.max_current_ma;
                case PPS_TEMP_HIGH:
                    return pps_config.max_current_ma * 0.6;
                default:
                    return 0;
            }
            
        case PPS_CHARGE_NORMAL:
            /* 正常充电 */
            return pps_config.max_current_ma * 0.7;
            
        case PPS_CHARGE_CONSTANT_VOLTAGE:
            /* 恒压充电 */
            return pps_config.max_current_ma * 0.5;
            
        case PPS_CHARGE_TRICKLE:
            /* 涓流充电 */
            return pps_config.max_current_ma * 0.2;
            
        case PPS_CHARGE_FULL:
        case PPS_CHARGE_STOP:
        default:
            return 0;
    }
}

/* 检查 PPS 是否可用 */
int pps_is_available(bool *available) {
    if (!available) {
        LOG_ERROR("Invalid parameters: available=%p", available);
        return -1;
    }
    
    /* 检查 PPS enable 节点是否存在 */
    *available = sysfs_exists(SYSFS_OPLUS_PPS_ENABLE);
    return 0;
}
