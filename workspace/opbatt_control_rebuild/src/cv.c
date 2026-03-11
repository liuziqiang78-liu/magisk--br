/**
 * @file cv.c
 * @brief 恒压控制模块实现
 * 
 * 提供恒压充电控制功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cv.h"
#include "config.h"
#include "logger.h"

/* 恒压控制配置 */
static cv_config_t cv_config = {
    .enabled = true,
    .inc_step = 100,
    .dec_step = 100,
    .cv_vol_mv = 4565,
    .cv_max_ma = 5000,
    .batt_vol_thr = {4559, 4559},
    .batt_vol_soc = {75, 85},
    .batt_con_soc = 94,
    .rise_quickstep_thr_mv = 4250,
    .rise_wait_thr_mv = 3800,
    .curr_inc_wait_cycles = 4,
    .batt_full_thr_mv = 4570
};

/* TC 模式配置 */
static tc_config_t tc_config = {
    .tc_vol_thr_mv = 4500,
    .tc_thr_soc = 98,
    .tc_full_ma = 400,
    .tc_vol_full_mv = 4485
};

/* 恒压控制状态 */
static cv_state_t cv_state = {
    .tc_mode_active = false,
    .is_full = false,
    .current_ma = 0,
    .voltage_mv = 0,
    .soc_percent = 0
};

/* 初始化恒压控制模块 */
int cv_init(void) {
    LOG_INFO("Constant voltage control initialized");
    return 0;
}

/* 设置恒压开关 */
int cv_set_enable(bool enable) {
    cv_config.enabled = enable;
    LOG_INFO("CV enable set to: %d", enable);
    return 0;
}

/* 获取恒压开关状态 */
int cv_get_enable(bool *enabled) {
    if (!enabled) {
        LOG_ERROR("Invalid parameters: enabled=%p", enabled);
        return -1;
    }
    
    *enabled = cv_config.enabled;
    return 0;
}

/* 检查是否进入TC模式 */
bool cv_should_enter_tc_mode(int soc, int voltage_mv) {
    return (soc >= cv_config.batt_con_soc && voltage_mv >= tc_config.tc_vol_thr_mv);
}

/* 检查是否充满 */
bool cv_is_full(int voltage_mv, int current_ma) {
    return (voltage_mv >= tc_config.tc_vol_full_mv && current_ma <= tc_config.tc_full_ma);
}

/* 升流控制 */
int cv_increase_current(int current_ma, int voltage_mv) {
    if (voltage_mv <= cv_config.rise_wait_thr_mv) {
        /* 可以升流 */
        if (current_ma < cv_config.cv_max_ma) {
            int new_current = current_ma + cv_config.inc_step;
            if (new_current > cv_config.cv_max_ma) {
                new_current = cv_config.cv_max_ma;
            }
            LOG_DEBUG("Increase current: %dmA -> %dmA", current_ma, new_current);
            return new_current;
        }
    }
    return current_ma;
}

/* 降流控制 */
int cv_decrease_current(int current_ma, int voltage_mv) {
    /* 检查电压撞墙 */
    if (voltage_mv >= cv_config.batt_vol_thr[0]) {
        int new_current = current_ma - cv_config.dec_step;
        if (new_current < 0) {
            new_current = 0;
        }
        LOG_DEBUG("Decrease current: %dmA -> %dmA", current_ma, new_current);
        return new_current;
    }
    return current_ma;
}

/* 获取当前充电电流 */
int cv_get_current(int soc, int voltage_mv, int current_ma) {
    if (!cv_config.enabled) {
        return current_ma;
    }
    
    /* 更新状态 */
    cv_state.soc_percent = soc;
    cv_state.voltage_mv = voltage_mv;
    cv_state.current_ma = current_ma;
    
    /* 检查是否进入TC模式 */
    if (cv_should_enter_tc_mode(soc, voltage_mv)) {
        cv_state.tc_mode_active = true;
        LOG_INFO("Enter TC mode: SOC=%d%%, Voltage=%dmV", soc, voltage_mv);
        
        /* TC模式：降低充电电流 */
        if (current_ma > tc_config.tc_full_ma) {
            int new_current = current_ma - cv_config.dec_step;
            if (new_current < tc_config.tc_full_ma) {
                new_current = tc_config.tc_full_ma;
            }
            LOG_DEBUG("TC mode decrease current: %dmA -> %dmA", current_ma, new_current);
            return new_current;
        }
        
        /* 检查是否充满 */
        if (cv_is_full(voltage_mv, current_ma)) {
            cv_state.is_full = true;
            LOG_INFO("Battery full: Voltage=%dmV, Current=%dmA", voltage_mv, current_ma);
            return 0;  /* 停止充电 */
        }
    } else {
        cv_state.tc_mode_active = false;
    }
    
    /* 检查电压撞墙 */
    if (voltage_mv >= cv_config.batt_vol_thr[0]) {
        return cv_decrease_current(current_ma, voltage_mv);
    }
    
    /* 检查是否可以升流 */
    if (voltage_mv <= cv_config.rise_wait_thr_mv) {
        return cv_increase_current(current_ma, voltage_mv);
    }
    
    return current_ma;
}

/* 恒压控制主函数 */
int cv_control(int soc, int voltage_mv, int current_ma) {
    if (!cv_config.enabled) {
        return current_ma;
    }
    
    int new_current = cv_get_current(soc, voltage_mv, current_ma);
    
    /* 记录日志 */
    if (new_current != current_ma) {
        LOG_INFO("CV control: %dmA -> %dmA (SOC=%d%%, Voltage=%dmV)", 
                 current_ma, new_current, soc, voltage_mv);
    }
    
    return new_current;
}

/* 获取恒压控制状态 */
int cv_get_state(cv_state_t *state) {
    if (!state) {
        LOG_ERROR("Invalid parameters: state=%p", state);
        return -1;
    }
    
    memcpy(state, &cv_state, sizeof(cv_state_t));
    return 0;
}
