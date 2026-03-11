/**
 * opbatt_control - 主程序
 * 电池控制程序重新实现版本
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "opbatt_control.h"

/* 全局上下文 */
static opbatt_context_t g_ctx;

/* 信号处理 */
static volatile bool g_running = true;

static void signal_handler(int signum) {
    opbatt_log("接收到信号 %d，正在关闭...", signum);
    g_running = false;
    g_ctx.running = false;
}

/* 打印使用帮助 */
static void print_usage(const char *prog_name) {
    printf("opbatt_control - 电池控制程序 v%s\n", OPBATT_VERSION);
    printf("用法: %s [选项]\n\n", prog_name);
    printf("选项:\n");
    printf("  -s, --server        以服务器模式运行\n");
    printf("  -c, --client HOST   以客户端模式连接到 HOST\n");
    printf("  -p, --port PORT     指定端口 (默认: %d)\n", DEFAULT_PORT);
    printf("  -l, --license PATH  指定许可证路径\n");
    printf("  -d, --daemon        以守护进程模式运行\n");
    printf("  -v, --verbose       详细输出\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("\n示例:\n");
    printf("  %s -s -p 8888              # 以服务器模式运行\n", prog_name);
    printf("  %s -c 192.168.1.100 -p 8888  # 连接到服务器\n", prog_name);
}

/* 初始化默认配置 */
static void init_default_config(opbatt_context_t *ctx) {
    memset(ctx, 0, sizeof(opbatt_context_t));
    
    /* 网络配置 */
    strcpy(ctx->network.host, "0.0.0.0");
    ctx->network.port = DEFAULT_PORT;
    ctx->network.use_ssl = true;
    ctx->network.is_server = true;
    
    /* 许可证配置 */
    ctx->license.valid = false;
    strcpy(ctx->license.license_key, "");
    strcpy(ctx->license.product_id, "OPBATT-001");
    ctx->license.expire_time = 0;
    
    /* 电池状态 */
    ctx->battery.voltage_mv = 0;
    ctx->battery.current_ma = 0;
    ctx->battery.capacity_mah = 0;
    ctx->battery.level_percent = 0;
    ctx->battery.status = 0;
    ctx->battery.health = 100;
    ctx->battery.temperature_c = 25;
    
    /* 运行状态 */
    ctx->running = true;
    ctx->server_fd = -1;
}

/* 主函数 */
int main(int argc, char *argv[]) {
    int opt;
    int daemon_mode = 0;
    int verbose = 0;
    char license_path[512] = {0};
    
    /* 解析命令行参数 */
    static struct option long_options[] = {
        {"server",   no_argument,       0, 's'},
        {"client",   required_argument, 0, 'c'},
        {"port",     required_argument, 0, 'p'},
        {"license",  required_argument, 0, 'l'},
        {"daemon",   no_argument,       0, 'd'},
        {"verbose",  no_argument,       0, 'v'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    /* 初始化默认配置 */
    init_default_config(&g_ctx);
    
    /* 解析参数 */
    while ((opt = getopt_long(argc, argv, "sc:p:l:dvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                g_ctx.network.is_server = true;
                strcpy(g_ctx.network.host, "0.0.0.0");
                break;
            case 'c':
                g_ctx.network.is_server = false;
                strncpy(g_ctx.network.host, optarg, sizeof(g_ctx.network.host) - 1);
                break;
            case 'p':
                g_ctx.network.port = atoi(optarg);
                if (g_ctx.network.port <= 0 || g_ctx.network.port > 65535) {
                    fprintf(stderr, "错误: 无效的端口号 %d\n", g_ctx.network.port);
                    return OPBATT_ERROR;
                }
                break;
            case 'l':
                strncpy(license_path, optarg, sizeof(license_path) - 1);
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return OPBATT_SUCCESS;
            default:
                print_usage(argv[0]);
                return OPBATT_ERROR;
        }
    }
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    opbatt_log("opbatt_control v%s 启动", OPBATT_VERSION);
    opbatt_log("构建日期: %s", OPBATT_BUILD_DATE);
    
    /* 守护进程模式 */
    if (daemon_mode) {
        opbatt_log("切换到守护进程模式...");
        if (daemon(0, 0) < 0) {
            perror("daemon");
            return OPBATT_ERROR;
        }
    }
    
    /* 初始化程序 */
    if (opbatt_init(&g_ctx) != OPBATT_SUCCESS) {
        opbatt_log("初始化失败");
        return OPBATT_ERROR;
    }
    
    /* 验证许可证 */
    if (opbatt_license_verify(&g_ctx) != OPBATT_SUCCESS) {
        opbatt_log("许可证验证失败");
        opbatt_cleanup(&g_ctx);
        return OPBATT_ERROR_LICENSE;
    }
    
    opbatt_log("许可证验证成功");
    
    /* 初始化网络 */
    if (opbatt_network_init(&g_ctx) != OPBATT_SUCCESS) {
        opbatt_log("网络初始化失败");
        opbatt_cleanup(&g_ctx);
        return OPBATT_ERROR_NETWORK;
    }
    
    /* 启动网络服务 */
    if (g_ctx.network.is_server) {
        if (opbatt_network_start_server(&g_ctx) != OPBATT_SUCCESS) {
            opbatt_log("服务器启动失败");
            opbatt_cleanup(&g_ctx);
            return OPBATT_ERROR_NETWORK;
        }
        opbatt_log("服务器启动成功，监听端口 %d", g_ctx.network.port);
    } else {
        if (opbatt_network_connect(&g_ctx) != OPBATT_SUCCESS) {
            opbatt_log("连接服务器失败");
            opbatt_cleanup(&g_ctx);
            return OPBATT_ERROR_NETWORK;
        }
        opbatt_log("已连接到服务器 %s:%d", g_ctx.network.host, g_ctx.network.port);
    }
    
    /* 主循环 */
    opbatt_log("进入主循环...");
    int ret = opbatt_main_loop(&g_ctx);
    
    /* 清理资源 */
    opbatt_log("正在清理资源...");
    opbatt_cleanup(&g_ctx);
    
    opbatt_log("程序退出");
    return ret;
}
