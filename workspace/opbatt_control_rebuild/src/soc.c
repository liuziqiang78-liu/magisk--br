/**
 * @file soc.c
 * @brief SOC（State of Charge）平滑模块实现
 * 
 * 提供 SOC 显示平滑处理功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soc.h"
#include "logger.h"

/* SOC 平滑配置 */
static soc_smooth_config_t soc_config = {
    .reserve_chg_soc = 2,
    .reserve_dis_soc = 2,
    .enabled = true
};

/* 初始化 SOC 平滑模块 */
int soc_init(void) {
    LOG_INFO("SOC smooth module initialized");
    return 0;
}

/* 设置 SOC 平滑开关 */
int soc_set_enable(bool enable) {
    soc_config.enabled = enable;
    LOG_INFO("SOC smooth enable set to: %d", enable);
    return 0;
}

/* 获取 SOC 平滑开关状态 */
int soc_get_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    *enabled = soc_config.enabled;
    return 0;
}

/* SOC 平滑处理 */
int soc_smooth(int raw_soc, bool is_charging) {
    if (!soc_config.enabled) {
        return raw_soc;
    }
    
    int smoothed_soc;
    int reserve_soc = is_charging ? soc_config.reserve_chg_soc : soc_config.reserve_dis_soc;
    
    smoothed_soc = raw_soc - reserve_soc;
    
    /* 确保 SOC 不小于 0 */
    if (smoothed_soc < 0) {
        smoothed_soc = 0;
    }
    
    /* 确保 SOC 不大于 100 */
    if (smoothed_soc > 100) {
        smoothed_soc = 100;
    }
    
    LOG_DEBUG("SOC smooth: raw=%d%%, smoothed=%d%%, charging=%d", 
              raw_soc, smoothed_soc, is_charging);
    
    return smoothed_soc;
}

/* 设置充电保留 SOC */
int soc_set_reserve_chg_soc(int soc) {
    if (soc < 0 || soc > 10) {
        LOG_ERROR("Invalid reserve_chg_soc: %d", soc);
        return -1;
    }
    
    soc_config.reserve_chg_soc = soc;
    LOG_INFO("Reserve charge SOC set to: %d%%", soc);
    return 0;
}

/* 获取充电保留 SOC */
int soc_get_reserve_chg_soc(int *soc) {
    if (!soc) {
        LOG_ERROR("Invalid parameters: soc=%p", soc);
        return -1;
    }
    
    *soc = soc_config.reserve_chg_soc;
    return 0;
}

/* 设置放电保留 SOC */
int soc_set_reserve_dis_soc(int soc) {
    if (soc < 0 || soc > 10) {
        LOG_ERROR("Invalid reserve_dis_soc: %d", soc);
        return -1;
    }
    
    soc_config.reserve_dis_soc = soc;
    LOG_INFO("Reserve discharge SOC set to: %d%%", soc);
    return 0;
}

/* 获取放电保留 SOC */
int soc_get_reserve_dis_soc(int *soc) {
    if (!soc) {
        LOG_ERROR("Invalid parameters: soc=%p", soc);
        return -1;
    }
    
    *soc = soc_config.reserve_dis_soc;
    return 0;
}
