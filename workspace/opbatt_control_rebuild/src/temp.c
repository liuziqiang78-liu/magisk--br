/**
 * @file temp.c
 * @brief 温度控制模块实现
 * 
 * 提供温度监控和温度伪装功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "temp.h"
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* 温度控制配置 */
temp_control_t temp_config = {
    .fake_temp_enabled = false,
    .fake_temp = 360,  /* 36°C */
    .over_temp_threshold = 530,  /* 53°C */
    .high_temp_threshold = 480,   /* 48°C */
    .normal_high_temp = 420,      /* 42°C */
    .normal_low_temp = 50         /* 5°C */
};

/* 初始化温度控制模块 */
int temp_init(void) {
    LOG_INFO("Temperature control initialized");
    return 0;
}

/* 设置温度伪装开关 */
int temp_set_fake_enable(bool enable) {
    /* 检查 fake_temp 节点是否存在 */
    if (sysfs_exists(SYSFS_OPLUS_BATTERY_FAKE_TEMP)) {
        if (sysfs_write_int(SYSFS_OPLUS_BATTERY_FAKE_TEMP, enable ? temp_config.fake_temp : 0) != 0) {
            LOG_ERROR("Failed to set fake temp enable to %d", enable);
            return -1;
        }
    }
    
    temp_config.fake_temp_enabled = enable;
    LOG_INFO("Fake temp enable set to: %d", enable);
    return 0;
}

/* 获取温度伪装开关状态 */
int temp_get_fake_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    *enabled = temp_config.fake_temp_enabled;
    return 0;
}

/* 设置伪装温度 */
int temp_set_fake_temp(int temp) {
    if (temp < 0 || temp > 600) {
        LOG_ERROR("Invalid fake temp: %d", temp);
        return -1;
    }
    
    temp_config.fake_temp = temp;
    
    /* 如果温度伪装已启用，立即应用 */
    if (temp_config.fake_temp_enabled) {
        if (sysfs_exists(SYSFS_OPLUS_BATTERY_FAKE_TEMP)) {
            sysfs_write_int(SYSFS_OPLUS_BATTERY_FAKE_TEMP, temp);
        }
    }
    
    LOG_INFO("Fake temp set to: %d°C", temp / 10);
    return 0;
}

/* 获取伪装温度 */
int temp_get_fake_temp(int *temp) {
    if (!temp) {
        LOG_ERROR("Invalid parameters: temp=%p", temp);
        return -1;
    }
    
    *temp = temp_config.fake_temp;
    return 0;
}

/* 获取有效温度（考虑温度伪装） */
int temp_get_effective_temp(int actual_temp, bool fake_enabled, int fake_temp) {
    return fake_enabled ? fake_temp : actual_temp;
}

/* 获取温度策略 */
temp_strategy_t temp_get_strategy(int temp) {
    /* 温度单位是 0.1°C */
    if (temp < temp_config.normal_low_temp) {
        return TEMP_STRATEGY_EXTREME_LOW;
    } else if (temp < 120) {
        return TEMP_STRATEGY_LOW;
    } else if (temp < 160) {
        return TEMP_STRATEGY_LITTLE_COLD;
    } else if (temp < 210) {
        return TEMP_STRATEGY_LITTLE_COLD;
    } else if (temp < 400) {
        return TEMP_STRATEGY_NORMAL;
    } else if (temp < temp_config.high_temp_threshold) {
        return TEMP_STRATEGY_NORMAL_HIGH;
    } else if (temp < temp_config.over_temp_threshold) {
        return TEMP_STRATEGY_HIGH;
    } else {
        return TEMP_STRATEGY_CRITICAL;
    }
}

/* 根据温度策略获取充电电流限制 */
int temp_get_current_limit(temp_strategy_t strategy) {
    switch (strategy) {
        case TEMP_STRATEGY_EXTREME_LOW:
            return 500;   /* 极低温：最低电流 */
        case TEMP_STRATEGY_LOW:
            return 1000;  /* 低温：低电流 */
        case TEMP_STRATEGY_LITTLE_COLD:
            return 2000;  /* 微冷：中低电流 */
        case TEMP_STRATEGY_NORMAL:
            return 5000;  /* 正常：正常电流 */
        case TEMP_STRATEGY_NORMAL_HIGH:
            return 4000;  /* 正常高温：中高电流 */
        case TEMP_STRATEGY_HIGH:
            return 2000;  /* 高温：低电流 */
        case TEMP_STRATEGY_CRITICAL:
            return 0;     /* 危险温度：停止充电 */
        default:
            return 0;
    }
}

/* 检查温度是否过高 */
bool temp_is_over_temp(int temp) {
    return temp >= temp_config.over_temp_threshold;
}

/* 检查温度是否在安全范围内 */
bool temp_is_safe(int temp) {
    return temp < temp_config.over_temp_threshold;
}
