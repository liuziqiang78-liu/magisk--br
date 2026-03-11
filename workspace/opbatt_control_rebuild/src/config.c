/**
 * @file config.c
 * @brief 配置文件管理实现
 * 
 * 管理 batt_control 配置文件的读取、保存和验证
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "config.h"
#include "logger.h"

/* 设置默认配置 */
void config_set_default(batt_control_config_t *config) {
    if (!config) {
        return;
    }
    
    /* 基础配置 */
    config->enabled = 1;
    config->inc_step = 100;
    config->dec_step = 100;
    
    /* UFCS 配置 */
    config->max_ufcs_chg_reset_cc = 1;
    config->ufcs_reset_delay = 180;
    config->ufcs_max = 9100;
    config->ufcs_soc_mon[0] = 20;
    config->ufcs_soc_mon[1] = 60;
    config->ufcs_interval_ms[0] = 650;
    config->ufcs_interval_ms[1] = 400;
    
    /* PPS 配置 */
    config->pps_max = 5000;
    config->pps_soc_mon[0] = 20;
    config->pps_soc_mon[1] = 68;
    config->pps_interval_ms[0] = 650;
    config->pps_interval_ms[1] = 400;
    
    /* 其他配置 */
    config->cable_override = 0;
    config->loop_interval_ms = 2000;
    
    /* 电池电压配置 */
    config->batt_vol_thr[0] = 4559;
    config->batt_vol_thr[1] = 4559;
    config->batt_vol_soc[0] = 75;
    config->batt_vol_soc[1] = 85;
    config->batt_con_soc = 94;
    
    /* 升流配置 */
    config->rise_quickstep_thr_mv = 4250;
    config->rise_wait_thr_mv = 3800;
    
    /* 恒压配置 */
    config->cv_vol_mv = 4565;
    config->cv_max_ma = 5000;
    
    /* TC 模式配置 */
    config->tc_vol_thr_mv = 4500;
    config->tc_thr_soc = 98;
    config->tc_full_ma = 400;
    config->tc_vol_full_mv = 4485;
    
    /* 其他 */
    config->curr_inc_wait_cycles = 4;
    config->batt_full_thr_mv = 4570;
}

/* 解析整数数组 */
int config_parse_int_array(const char *str, int *array, size_t size) {
    char *token;
    char *copy;
    int i = 0;
    
    if (!str || !array || size == 0) {
        return -1;
    }
    
    copy = strdup(str);
    if (!copy) {
        return -1;
    }
    
    token = strtok(copy, ",");
    while (token != NULL && i < size) {
        array[i++] = atoi(token);
        token = strtok(NULL, ",");
    }
    
    free(copy);
    return 0;
}

/* 读取配置文件 */
int config_read(const char *path, batt_control_config_t *config) {
    FILE *fp;
    char line[256];
    char key[128], value[128];
    
    if (!path || !config) {
        LOG_ERROR("Invalid parameters: path=%p, config=%p", path, config);
        return -1;
    }
    
    /* 设置默认值 */
    config_set_default(config);
    
    fp = fopen(path, "r");
    if (!fp) {
        LOG_WARN("Failed to open config file %s, using defaults: %s", path, strerror(errno));
        return 0;  /* 使用默认值 */
    }
    
    while (fgets(line, sizeof(line), fp)) {
        /* 跳过注释和空行 */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        /* 去除换行符 */
        line[strcspn(line, "\r\n")] = '\0';
        
        /* 解析键值对 */
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            /* 基础配置 */
            if (strcmp(key, "enabled") == 0) {
                config->enabled = atoi(value);
            } else if (strcmp(key, "inc_step") == 0) {
                config->inc_step = atoi(value);
            } else if (strcmp(key, "dec_step") == 0) {
                config->dec_step = atoi(value);
            }
            /* UFCS 配置 */
            else if (strcmp(key, "max_ufcs_chg_reset_cc") == 0) {
                config->max_ufcs_chg_reset_cc = atoi(value);
            } else if (strcmp(key, "ufcs_reset_delay") == 0) {
                config->ufcs_reset_delay = atoi(value);
            } else if (strcmp(key, "ufcs_max") == 0) {
                config->ufcs_max = atoi(value);
            } else if (strcmp(key, "ufcs_soc_mon") == 0) {
                config_parse_int_array(value, config->ufcs_soc_mon, 2);
            } else if (strcmp(key, "ufcs_interval_ms") == 0) {
                config_parse_int_array(value, config->ufcs_interval_ms, 2);
            }
            /* PPS 配置 */
            else if (strcmp(key, "pps_max") == 0) {
                config->pps_max = atoi(value);
            } else if (strcmp(key, "pps_soc_mon") == 0) {
                config_parse_int_array(value, config->pps_soc_mon, 2);
            } else if (strcmp(key, "pps_interval_ms") == 0) {
                config_parse_int_array(value, config->pps_interval_ms, 2);
            }
            /* 其他配置 */
            else if (strcmp(key, "cable_override") == 0) {
                config->cable_override = atoi(value);
            } else if (strcmp(key, "loop_interval_ms") == 0) {
                config->loop_interval_ms = atoi(value);
            }
            /* 电池电压配置 */
            else if (strcmp(key, "batt_vol_thr") == 0) {
                config_parse_int_array(value, config->batt_vol_thr, 2);
            } else if (strcmp(key, "batt_vol_soc") == 0) {
                config_parse_int_array(value, config->batt_vol_soc, 2);
            } else if (strcmp(key, "batt_con_soc") == 0) {
                config->batt_con_soc = atoi(value);
            }
            /* 升流配置 */
            else if (strcmp(key, "rise_quickstep_thr_mv") == 0) {
                config->rise_quickstep_thr_mv = atoi(value);
            } else if (strcmp(key, "rise_wait_thr_mv") == 0) {
                config->rise_wait_thr_mv = atoi(value);
            }
            /* 恒压配置 */
            else if (strcmp(key, "cv_vol_mv") == 0) {
                config->cv_vol_mv = atoi(value);
            } else if (strcmp(key, "cv_max_ma") == 0) {
                config->cv_max_ma = atoi(value);
            }
            /* TC 模式配置 */
            else if (strcmp(key, "tc_vol_thr_mv") == 0) {
                config->tc_vol_thr_mv = atoi(value);
            } else if (strcmp(key, "tc_thr_soc") == 0) {
                config->tc_thr_soc = atoi(value);
            } else if (strcmp(key, "tc_full_ma") == 0) {
                config->tc_full_ma = atoi(value);
            } else if (strcmp(key, "tc_vol_full_mv") == 0) {
                config->tc_vol_full_mv = atoi(value);
            }
            /* 其他 */
            else if (strcmp(key, "curr_inc_wait_cycles") == 0) {
                config->curr_inc_wait_cycles = atoi(value);
            } else if (strcmp(key, "batt_full_thr_mv") == 0) {
                config->batt_full_thr_mv = atoi(value);
            }
        }
    }
    
    fclose(fp);
    LOG_INFO("Config loaded from %s", path);
    
    return 0;
}

/* 保存配置文件 */
int config_save(const char *path, batt_control_config_t *config) {
    FILE *fp;
    
    if (!path || !config) {
        LOG_ERROR("Invalid parameters: path=%p, config=%p", path, config);
        return -1;
    }
    
    fp = fopen(path, "w");
    if (!fp) {
        LOG_ERROR("Failed to open config file %s: %s", path, strerror(errno));
        return -1;
    }
    
    /* 写入配置 */
    fprintf(fp, "# opbatt_control configuration\n");
    fprintf(fp, "enabled=%d\n", config->enabled);
    fprintf(fp, "inc_step=%d\n", config->inc_step);
    fprintf(fp, "dec_step=%d\n", config->dec_step);
    fprintf(fp, "max_ufcs_chg_reset_cc=%d\n", config->max_ufcs_chg_reset_cc);
    fprintf(fp, "ufcs_reset_delay=%d\n", config->ufcs_reset_delay);
    fprintf(fp, "ufcs_max=%d\n", config->ufcs_max);
    fprintf(fp, "pps_max=%d\n", config->pps_max);
    fprintf(fp, "cable_override=%d\n", config->cable_override);
    fprintf(fp, "ufcs_soc_mon=%d,%d\n", config->ufcs_soc_mon[0], config->ufcs_soc_mon[1]);
    fprintf(fp, "ufcs_interval_ms=%d,%d\n", config->ufcs_interval_ms[0], config->ufcs_interval_ms[1]);
    fprintf(fp, "pps_soc_mon=%d,%d\n", config->pps_soc_mon[0], config->pps_soc_mon[1]);
    fprintf(fp, "pps_interval_ms=%d,%d\n", config->pps_interval_ms[0], config->pps_interval_ms[1]);
    fprintf(fp, "loop_interval_ms=%d\n", config->loop_interval_ms);
    fprintf(fp, "batt_vol_thr=%d,%d\n", config->batt_vol_thr[0], config->batt_vol_thr[1]);
    fprintf(fp, "batt_vol_soc=%d,%d\n", config->batt_vol_soc[0], config->batt_vol_soc[1]);
    fprintf(fp, "batt_con_soc=%d\n", config->batt_con_soc);
    fprintf(fp, "rise_quickstep_thr_mv=%d\n", config->rise_quickstep_thr_mv);
    fprintf(fp, "rise_wait_thr_mv=%d\n", config->rise_wait_thr_mv);
    fprintf(fp, "cv_vol_mv=%d\n", config->cv_vol_mv);
    fprintf(fp, "cv_max_ma=%d\n", config->cv_max_ma);
    fprintf(fp, "tc_vol_thr_mv=%d\n", config->tc_vol_thr_mv);
    fprintf(fp, "tc_thr_soc=%d\n", config->tc_thr_soc);
    fprintf(fp, "tc_full_ma=%d\n", config->tc_full_ma);
    fprintf(fp, "tc_vol_full_mv=%d\n", config->tc_vol_full_mv);
    fprintf(fp, "curr_inc_wait_cycles=%d\n", config->curr_inc_wait_cycles);
    fprintf(fp, "batt_full_thr_mv=%d\n", config->batt_full_thr_mv);
    
    fclose(fp);
    LOG_INFO("Config saved to %s", path);
    
    return 0;
}

/* 验证配置 */
int config_validate(batt_control_config_t *config) {
    if (!config) {
        LOG_ERROR("Invalid parameters: config=%p", config);
        return -1;
    }
    
    /* 验证基础配置 */
    if (config->enabled < 0 || config->enabled > 1) {
        LOG_ERROR("Invalid enabled value: %d", config->enabled);
        return -1;
    }
    
    if (config->inc_step < 50 || config->inc_step > 200) {
        LOG_ERROR("Invalid inc_step value: %d", config->inc_step);
        return -1;
    }
    
    if (config->dec_step < 50 || config->dec_step > 200) {
        LOG_ERROR("Invalid dec_step value: %d", config->dec_step);
        return -1;
    }
    
    /* 验证 UFCS 配置 */
    if (config->ufcs_max < 3000 || config->ufcs_max > 13700) {
        LOG_ERROR("Invalid ufcs_max value: %d", config->ufcs_max);
        return -1;
    }
    
    /* 验证 PPS 配置 */
    if (config->pps_max < 3000 || config->pps_max > 6500) {
        LOG_ERROR("Invalid pps_max value: %d", config->pps_max);
        return -1;
    }
    
    /* 验证循环间隔 */
    if (config->loop_interval_ms < 1500 || config->loop_interval_ms > 2500) {
        LOG_ERROR("Invalid loop_interval_ms value: %d", config->loop_interval_ms);
        return -1;
    }
    
    LOG_INFO("Config validation passed");
    
    return 0;
}
