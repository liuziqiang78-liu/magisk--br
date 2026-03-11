/**
 * @file config.h
 * @brief 配置文件管理
 * 
 * 管理 batt_control 配置文件的读取、保存和验证
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stddef.h>

/* 配置文件路径 */
#define CONFIG_FILE_PATH "/data/opbatt/batt_control"

/* 配置结构体 */
typedef struct {
    /* 基础配置 */
    int enabled;                    /* 恒压开关 (0=关闭, 1=开启) */
    int inc_step;                   /* 电流上升步长 (mA) */
    int dec_step;                   /* 电流下降步长 (mA) */
    
    /* UFCS 配置 */
    int max_ufcs_chg_reset_cc;      /* UFCS降功率重置次数 */
    int ufcs_reset_delay;           /* UFCS重置延迟 (秒) */
    int ufcs_max;                   /* UFCS最大电流 (mA) */
    int ufcs_soc_mon[2];            /* UFCS SOC监控点 */
    int ufcs_interval_ms[2];        /* UFCS采样间隔 (ms) */
    
    /* PPS 配置 */
    int pps_max;                    /* PPS最大电流 (mA) */
    int pps_soc_mon[2];             /* PPS SOC监控点 */
    int pps_interval_ms[2];         /* PPS采样间隔 (ms) */
    
    /* 其他配置 */
    int cable_override;             /* 忽略线材检测 (0=不忽略, 1=忽略) */
    int loop_interval_ms;           /* 主循环间隔 (ms) */
    
    /* 电池电压配置 */
    int batt_vol_thr[2];            /* 电池电压阈值 (mV) */
    int batt_vol_soc[2];            /* 电池电压SOC阈值 (%) */
    int batt_con_soc;               /* 开启TC模式的SOC (%) */
    
    /* 升流配置 */
    int rise_quickstep_thr_mv;      /* 快速升流阈值 (mV) */
    int rise_wait_thr_mv;           /* 等待升流阈值 (mV) */
    
    /* 恒压配置 */
    int cv_vol_mv;                  /* 恒压电压 (mV) */
    int cv_max_ma;                  /* 恒压最大电流 (mA) */
    
    /* TC 模式配置 */
    int tc_vol_thr_mv;              /* TC模式电压阈值 (mV) */
    int tc_thr_soc;                 /* TC模式SOC阈值 (%) */
    int tc_full_ma;                 /* 充满电流阈值 (mA) */
    int tc_vol_full_mv;             /* 充满电压 (mV) */
    
    /* 其他 */
    int curr_inc_wait_cycles;       /* 升流等待周期 */
    int batt_full_thr_mv;           /* 电池充满阈值 (mV) */
} batt_control_config_t;

/* 读取配置文件 */
int config_read(const char *path, batt_control_config_t *config);

/* 保存配置文件 */
int config_save(const char *path, batt_control_config_t *config);

/* 设置默认配置 */
void config_set_default(batt_control_config_t *config);

/* 验证配置 */
int config_validate(batt_control_config_t *config);

/* 解析整数数组 */
int config_parse_int_array(const char *str, int *array, size_t size);

#endif /* CONFIG_H */
