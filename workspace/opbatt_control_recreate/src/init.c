/**
 * opbatt_control - 初始化和清理模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opbatt_control.h"

/* 初始化程序 */
int opbatt_init(opbatt_context_t *ctx) {
    opbatt_log("初始化 opbatt_control...");
    
    /* 初始化电池模块 */
    if (opbatt_battery_init(ctx) != OPBATT_SUCCESS) {
        opbatt_log("电池模块初始化失败");
        return OPBATT_ERROR_INIT;
    }
    
    /* 初始化许可证模块 */
    if (opbatt_license_init(ctx) != OPBATT_SUCCESS) {
        opbatt_log("许可证模块初始化失败");
        return OPBATT_ERROR_INIT;
    }
    
    /* 初始化网络模块 */
    if (opbatt_network_init(ctx) != OPBATT_SUCCESS) {
        opbatt_log("网络模块初始化失败");
        return OPBATT_ERROR_INIT;
    }
    
    opbatt_log("初始化完成");
    return OPBATT_SUCCESS;
}

/* 清理程序 */
int opbatt_cleanup(opbatt_context_t *ctx) {
    opbatt_log("清理 opbatt_control...");
    
    /* 清理网络模块 */
    opbatt_network_cleanup(ctx);
    
    /* 清理许可证模块 */
    /* 许可证模块不需要特殊清理 */
    
    /* 清理电池模块 */
    /* 电池模块不需要特殊清理 */
    
    opbatt_log("清理完成");
    return OPBATT_SUCCESS;
}
