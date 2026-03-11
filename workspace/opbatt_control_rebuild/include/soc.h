/**
 * @file soc.h
 * @brief SOC（State of Charge）平滑模块
 * 
 * 提供 SOC 显示平滑处理功能
 */

#ifndef SOC_H
#define SOC_H

#include <stdint.h>
#include <stdbool.h>

/* SOC 平滑配置 */
typedef struct {
    int reserve_chg_soc;       /* 充电保留SOC (%) */
    int reserve_dis_soc;       /* 放电保留SOC (%) */
    bool enabled;              /* SOC平滑开关 */
} soc_smooth_config_t;

/* 初始化 SOC 平滑模块 */
int soc_init(void);

/* 设置 SOC 平滑开关 */
int soc_set_enable(bool enable);

/* 获取 SOC 平滑开关状态 */
int soc_get_enable(bool *enabled);

/* SOC 平滑处理 */
int soc_smooth(int raw_soc, bool is_charging);

/* 设置充电保留 SOC */
int soc_set_reserve_chg_soc(int soc);

/* 获取充电保留 SOC */
int soc_get_reserve_chg_soc(int *soc);

/* 设置放电保留 SOC */
int soc_set_reserve_dis_soc(int soc);

/* 获取放电保留 SOC */
int soc_get_reserve_dis_soc(int *soc);

#endif /* SOC_H */
