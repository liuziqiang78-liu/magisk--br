/**
 * opbatt_control - 电池控制程序
 * 重新实现版本
 * 
 * 功能：
 * - 电池状态监控
 * - 充放电控制
 * - 网络通信（SSL/TLS）
 * - 许可证验证
 */

#ifndef OPBATT_CONTROL_H
#define OPBATT_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

/* 版本信息 */
#define OPBATT_VERSION "1.0.0"
#define OPBATT_BUILD_DATE "2026-03-10"

/* 配置常量 */
#define MAX_BUFFER_SIZE 4096
#define DEFAULT_PORT 8888
#define MAX_CONNECTIONS 10
#define LICENSE_PATH "/root/opbattlic/openssl/../openssl-out/arm64-v8a/ssl"

/* 错误码 */
typedef enum {
    OPBATT_SUCCESS = 0,
    OPBATT_ERROR = -1,
    OPBATT_ERROR_INIT = -2,
    OPBATT_ERROR_NETWORK = -3,
    OPBATT_ERROR_LICENSE = -4,
    OPBATT_ERROR_BATTERY = -5,
    OPBATT_ERROR_SSL = -6
} opbatt_error_t;

/* 电池状态 */
typedef struct {
    uint32_t voltage_mv;      /* 电压 (mV) */
    uint32_t current_ma;      /* 电流 (mA) */
    uint32_t capacity_mah;    /* 容量 (mAh) */
    uint32_t level_percent;   /* 电量百分比 */
    uint8_t  status;          /* 状态：0=空闲, 1=充电, 2=放电 */
    uint8_t  health;          /* 健康度 */
    uint32_t temperature_c;   /* 温度 (°C) */
} battery_status_t;

/* 网络配置 */
typedef struct {
    char host[256];
    uint16_t port;
    bool use_ssl;
    bool is_server;
} network_config_t;

/* 许可证信息 */
typedef struct {
    bool valid;
    char license_key[256];
    char product_id[64];
    uint64_t expire_time;
} license_info_t;

/* 全局配置 */
typedef struct {
    battery_status_t battery;
    network_config_t network;
    license_info_t license;
    bool running;
    int server_fd;
} opbatt_context_t;

/* 函数声明 */

/* 初始化 */
int opbatt_init(opbatt_context_t *ctx);
int opbatt_cleanup(opbatt_context_t *ctx);

/* 电池控制 */
int opbatt_battery_init(opbatt_context_t *ctx);
int opbatt_battery_get_status(opbatt_context_t *ctx, battery_status_t *status);
int opbatt_battery_set_charge(opbatt_context_t *ctx, bool enable);
int opbatt_battery_set_current(opbatt_context_t *ctx, uint32_t current_ma);

/* 网络通信 */
int opbatt_network_init(opbatt_context_t *ctx);
int opbatt_network_start_server(opbatt_context_t *ctx);
int opbatt_network_connect(opbatt_context_t *ctx);
int opbatt_network_send(opbatt_context_t *ctx, const void *data, size_t len);
int opbatt_network_recv(opbatt_context_t *ctx, void *data, size_t len);
int opbatt_network_cleanup(opbatt_context_t *ctx);

/* SSL/TLS */
int opbatt_ssl_init(opbatt_context_t *ctx);
int opbatt_ssl_cleanup(opbatt_context_t *ctx);

/* 许可证验证 */
int opbatt_license_init(opbatt_context_t *ctx);
int opbatt_license_verify(opbatt_context_t *ctx);
bool opbatt_license_is_valid(opbatt_context_t *ctx);

/* 主循环 */
int opbatt_main_loop(opbatt_context_t *ctx);

/* 工具函数 */
void opbatt_log(const char *format, ...);
void opbatt_hexdump(const void *data, size_t len);

#endif /* OPBATT_CONTROL_H */
