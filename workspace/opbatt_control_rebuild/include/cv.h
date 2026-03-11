/**
 * @file cv.h
 * @brief 恒压控制模块
 * 
 * 提供恒压充电控制功能
 */

#ifndef CV_H
#define CV_H

#include <stdint.h>
#include <stdbool.h>

/* 恒压控制配置 */
typedef struct {
    bool enabled;               /* 恒压开关 */
    int inc_step;               /* 电流上升步长 (mA) */
    int dec_step;               /* 电流下降步长 (mA) */
    int cv_vol_mv;              /* 恒压电压 (mV) */
    int cv_max_ma;              /* 恒压最大电流 (mA) */
    int batt_vol_thr[2];        /* 电池电压阈值 (mV) */
    int batt_vol_soc[2];        /* 电池电压SOC阈值 (%) */
    int batt_con_soc;           /* 开启TC模式的SOC (%) */
    int rise_quickstep_thr_mv;  /* 快速升流阈值 (mV) */
    int rise_wait_thr_mv;       /* 等待升流阈值 (mV) */
    int curr_inc_wait_cycles;   /* 升流等待周期 */
    int batt_full_thr_mv;       /* 电池充满阈值 (mV) */
} cv_config_t;

/* TC 模式配置 */
typedef struct {
    int tc_vol_thr_mv;          /* TC模式电压阈值 (mV) */
    int tc_thr_soc;             /* TC模式SOC阈值 (%) */
    int tc_full_ma;             /* 充满电流阈值 (mA) */
    int tc_vol_full_mv;         /* 充满电压 (mV) */
} tc_config_t;

/* 恒压控制状态 */
typedef struct {
    bool tc_mode_active;        /* TC模式是否激活 */
    bool is_full;               /* 是否充满 */
    int current_ma;             /* 当前充电电流 (mA) */
    int voltage_mv;             /* 当前电池电压 (mV) */
    int soc_percent;            /* 当前SOC (%) */
} cv_state_t;

/* 初始化恒压控制模块 */
int cv_init(void);

/* 设置恒压开关 */
int cv_set_enable(bool enable);

/* 获取恒压开关状态 */
int cv_get_enable(bool *enabled);

/* 恒压控制主函数 */
int cv_control(int soc, int voltage_mv, int current_ma);

/* 检查是否进入TC模式 */
bool cv_should_enter_tc_mode(int soc, int voltage_mv);

/* 检查是否充满 */
bool cv_is_full(int voltage_mv, int current_ma);

/* 获取当前充电电流 */
int cv_get_current(int soc, int voltage_mv, int current_ma);

/* 升流控制 */
int cv_increase_current(int current_ma, int voltage_mv);

/* 降流控制 */
int cv_decrease_current(int current_ma, int voltage_mv);

/* 获取恒压控制状态 */
int cv_get_state(cv_state_t *state);

#endif /* CV_H */
