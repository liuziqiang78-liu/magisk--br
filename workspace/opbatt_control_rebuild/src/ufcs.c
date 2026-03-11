/**
 * @file ufcs.c
 * @brief UFCS（Unified Fast Charging Specification）协议控制实现
 * 
 * 提供 UFCS 快充协议的控制功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ufcs.h"
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* UFCS 充电参数 */
static ufcs_charge_params_t ufcs_params = {
    .target_vbus_mv = 11000,
    .curr_max_ma = 9100,
    .full_cool_sw_vbat_mv = 4568,
    .full_normal_sw_vbat_mv = 4568,
    .full_warm_vbat_mv = 4130
};

/* UFCS 配置 */
static ufcs_config_t ufcs_config = {
    .enabled = false,
    .max_current_ma = 9100,
    .soc_monitor = {20, 60},
    .interval_ms = {650, 400},
    .reset_count = 1,
    .reset_delay_sec = 180
};

/* 初始化 UFCS 模块 */
int ufcs_init(void) {
    LOG_INFO("UFCS module initialized");
    return 0;
}

/* 设置 UFCS 协议开关 */
int ufcs_set_enable(bool enable) {
    /* 检查 UFCS enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_UFCS_ENABLE)) {
        if (sysfs_write_int(SYSFS_OPLUS_UFCS_ENABLE, enable ? 1 : 0) != 0) {
            LOG_ERROR("Failed to set UFCS enable to %d", enable);
            return -1;
        }
    }
    
    ufcs_config.enabled = enable;
    LOG_INFO("UFCS enable set to: %d", enable);
    return 0;
}

/* 获取 UFCS 协议开关状态 */
int ufcs_get_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    /* 检查 UFCS enable 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_UFCS_ENABLE)) {
        int value;
        if (sysfs_read_int(SYSFS_OPLUS_UFCS_ENABLE, &value) == 0) {
            *enabled = (value == 1);
            return 0;
        }
    }
    
    *enabled = ufcs_config.enabled;
    return 0;
}

/* 设置 UFCS 最大电流 */
int ufcs_set_max_current(int current_ma) {
    if (current_ma < 3000 || current_ma > 13700) {
        LOG_ERROR("Invalid UFCS max current: %d", current_ma);
        return -1;
    }
    
    ufcs_config.max_current_ma = current_ma;
    LOG_INFO("UFCS max current set to: %dmA", current_ma);
    return 0;
}

/* 获取 UFCS 最大电流 */
int ufcs_get_max_current(int *current_ma) {
    if (!current_ma) {
        LOG_ERROR("Invalid parameters: current_ma=%p", current_ma);
        return -1;
    }
    
    *current_ma = ufcs_config.max_current_ma;
    return 0;
}

/* 获取 UFCS 温度策略 */
ufcs_temp_strategy_t ufcs_get_temp_strategy(int temp) {
    /* 温度单位是 0.1°C */
    if (temp < 50) {
        return UFCS_TEMP_EXTREME_LOW;
    } else if (temp < 120) {
        return UFCS_TEMP_LOW;
    } else if (temp < 160) {
        return UFCS_TEMP_LITTLE_COLD;
    } else if (temp < 210) {
        return UFCS_TEMP_LITTLE_COLD_HIGH;
    } else if (temp < 400) {
        return UFCS_TEMP_NORMAL_LOW;
    } else if (temp < 480) {
        return UFCS_TEMP_NORMAL_HIGH;
    } else if (temp < 530) {
        return UFCS_TEMP_HIGH;
    } else {
        return UFCS_TEMP_OVER_HIGH;
    }
}

/* 获取 UFCS 充电策略 */
ufcs_charge_strategy_t ufcs_get_charge_strategy(int soc, int temp) {
    ufcs_temp_strategy_t temp_strategy = ufcs_get_temp_strategy(temp);
    
    /* 过高温停止充电 */
    if (temp_strategy == UFCS_TEMP_OVER_HIGH) {
        return UFCS_CHARGE_STOP;
    }
    
    /* 根据 SOC 确定充电策略 */
    if (soc < 20) {
        return UFCS_CHARGE_FAST;
    } else if (soc < 35) {
        return UFCS_CHARGE_FAST;
    } else if (soc < 55) {
        return UFCS_CHARGE_NORMAL;
    } else if (soc < 75) {
        return UFCS_CHARGE_NORMAL;
    } else if (soc < 85) {
        return UFCS_CHARGE_CONSTANT_VOLTAGE;
    } else if (soc < 99) {
        return UFCS_CHARGE_TRICKLE;
    } else {
        return UFCS_CHARGE_FULL;
    }
}

/* 根据 SOC 和温度获取 UFCS 充电电流 */
int ufcs_get_charge_current(int soc, int temp) {
    ufcs_charge_strategy_t strategy = ufcs_get_charge_strategy(soc, temp);
    ufcs_temp_strategy_t temp_strategy = ufcs_get_temp_strategy(temp);
    
    /* 根据策略和温度计算充电电流 */
    switch (strategy) {
        case UFCS_CHARGE_FAST:
            /* 快速充电：根据温度调整 */
            switch (temp_strategy) {
                case UFCS_TEMP_EXTREME_LOW:
                    return ufcs_config.max_current_ma * 0.3;
                case UFCS_TEMP_LOW:
                    return ufcs_config.max_current_ma * 0.5;
                case UFCS_TEMP_LITTLE_COLD:
                    return ufcs_config.max_current_ma * 0.7;
                case UFCS_TEMP_LITTLE_COLD_HIGH:
                    return ufcs_config.max_current_ma * 0.8;
                case UFCS_TEMP_NORMAL_LOW:
                case UFCS_TEMP_NORMAL_HIGH:
                    return ufcs_config.max_current_ma;
                case UFCS_TEMP_HIGH:
                    return ufcs_config.max_current_ma * 0.6;
                default:
                    return 0;
            }
            
        case UFCS_CHARGE_NORMAL:
            /* 正常充电 */
            return ufcs_config.max_current_ma * 0.7;
            
        case UFCS_CHARGE_CONSTANT_VOLTAGE:
            /* 恒压充电 */
            return ufcs_config.max_current_ma * 0.5;
            
        case UFCS_CHARGE_TRICKLE:
            /* 涓流充电 */
            return ufcs_config.max_current_ma * 0.2;
            
        case UFCS_CHARGE_FULL:
        case UFCS_CHARGE_STOP:
        default:
            return 0;
    }
}

/* 重置 UFCS 充电器 */
int ufcs_reset_charger(void) {
    LOG_INFO("UFCS charger reset");
    /* 这里可以实现充电器重置逻辑 */
    return 0;
}

/* 检查 UFCS 是否可用 */
int ufcs_is_available(bool *available) {
    if (!available) {
        LOG_ERROR("Invalid parameters: available=%p", available);
        return -1;
    }
    
    /* 检查 UFCS enable 节点是否存在 */
    *available = sysfs_exists(SYSFS_OPLUS_UFCS_ENABLE);
    return 0;
}
