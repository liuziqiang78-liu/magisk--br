/**
 * @file ddrc.c
 * @brief DDRC（Deep Discharge Recovery）模块实现
 * 
 * 提供深度放电恢复功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ddrc.h"
#include "logger.h"

/* DDRC 配置 */
static ddrc_config_t ddrc_config = {
    .uv_thr = 2800,           /* 2800mV */
    .count_thr = 50,          /* 50次 */
    .vbat_soc = 20,           /* 20% */
    .soc_range = {20, 30, 50, 70, 90},
    .temp_range = {-500, 1000, 3500}  /* -50°C, 100°C, 350°C */
};

/* DDRC 状态 */
ddrc_state_t ddrc_state = {
    .ddrc_active = false,
    .deep_discharge_count = 0,
    .limited_current_ma = 0
};

/* 初始化 DDRC 模块 */
int ddrc_init(void) {
    LOG_INFO("DDRC module initialized");
    return 0;
}

/* 检查是否触发 DDRC */
bool ddrc_should_trigger(int voltage_mv, int count) {
    return (voltage_mv < ddrc_config.uv_thr && count > ddrc_config.count_thr);
}

/* 获取 DDRC 限制电流 */
int ddrc_get_limited_current(int soc, int voltage_mv) {
    /* 根据 SOC 计算限制电流 */
    if (soc < ddrc_config.soc_range[0]) {
        return 500;   /* 最低电流 */
    } else if (soc < ddrc_config.soc_range[1]) {
        return 1000;
    } else if (soc < ddrc_config.soc_range[2]) {
        return 2000;
    } else if (soc < ddrc_config.soc_range[3]) {
        return 3000;
    } else if (soc < ddrc_config.soc_range[4]) {
        return 4000;
    } else {
        return 5000;  /* 最高电流 */
    }
}

/* 获取深度放电计数 */
int ddrc_get_count(int *count) {
    if (!count) {
        LOG_ERROR("Invalid parameters: count=%p", count);
        return -1;
    }
    
    *count = ddrc_state.deep_discharge_count;
    return 0;
}

/* 重置深度放电计数 */
int ddrc_reset_count(void) {
    ddrc_state.deep_discharge_count = 0;
    LOG_INFO("DDRC count reset");
    return 0;
}

/* 检查 DDRC 是否激活 */
bool ddrc_is_active(void) {
    return ddrc_state.ddrc_active;
}

/* 获取 DDRC 状态 */
int ddrc_get_state(ddrc_state_t *state) {
    if (!state) {
        LOG_ERROR("Invalid parameters: state=%p", state);
        return -1;
    }
    
    memcpy(state, &ddrc_state, sizeof(ddrc_state_t));
    return 0;
}
