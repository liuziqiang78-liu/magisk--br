/**
 * battery_control - 主程序
 * 纯电池控制版本（无网络、无验证）
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

#include "battery_control.h"

/* 全局上下文 */
static battery_context_t g_ctx;

/* 信号处理 */
static volatile bool g_running = true;

static void signal_handler(int signum) {
    battery_log("接收到信号 %d，正在关闭...", signum);
    g_running = false;
}

/* 打印使用帮助 */
static void print_usage(const char *prog_name) {
    printf("battery_control - 电池控制程序 v%s\n", BATTERY_VERSION);
    printf("用法: %s [选项]\n\n", prog_name);
    printf("选项:\n");
    printf("  -s, --status           显示电池状态\n");
    printf("  -i, --info             显示电池信息\n");
    printf("  -c, --charge [on|off]  控制充电开关\n");
    printf("  -C, --current MA       设置充电电流 (mA)\n");
    printf("  -V, --voltage MV       设置充电电压 (mV)\n");
    printf("  -m, --monitor          持续监控电池状态\n");
    printf("  -d, --daemon           以守护进程模式运行\n");
    printf("  -v, --verbose          详细输出\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("\n示例:\n");
    printf("  %s -s                  # 显示电池状态\n", prog_name);
    printf("  %s -c on               # 开启充电\n", prog_name);
    printf("  %s -c off              # 关闭充电\n", prog_name);
    printf("  %s -C 2000             # 设置充电电流为 2000mA\n", prog_name);
    printf("  %s -m                  # 持续监控电池状态\n", prog_name);
}

/* 初始化默认配置 */
static void init_default_config(battery_context_t *ctx) {
    memset(ctx, 0, sizeof(battery_context_t));
    
    /* 充电配置 */
    ctx->charge_config.enable = false;
    ctx->charge_config.current_ma = 2000;
    ctx->charge_config.voltage_mv = 4200;
    ctx->charge_config.max_temp_c = 45;
    ctx->charge_config.min_temp_c = 0;
    
    /* 电池状态 */
    ctx->status.voltage_mv = 0;
    ctx->status.current_ma = 0;
    ctx->status.capacity_mah = 0;
    ctx->status.level_percent = 0;
    ctx->status.status = 0;
    ctx->status.health = 100;
    ctx->status.temperature_c = 25;
    ctx->status.cycle_count = 0;
    ctx->status.full_charge_capacity = 0;
    
    /* 运行状态 */
    ctx->initialized = false;
    ctx->simulation_mode = false;
}

/* 显示电池状态 */
static int show_status(battery_context_t *ctx) {
    battery_status_t status;
    int ret;
    
    ret = battery_get_status(ctx, &status);
    if (ret != BATTERY_SUCCESS) {
        fprintf(stderr, "获取电池状态失败\n");
        return ret;
    }
    
    battery_print_status(&status);
    return BATTERY_SUCCESS;
}

/* 显示电池信息 */
static int show_info(battery_context_t *ctx) {
    battery_info_t info;
    int ret;
    
    ret = battery_get_info(ctx, &info);
    if (ret != BATTERY_SUCCESS) {
        fprintf(stderr, "获取电池信息失败\n");
        return ret;
    }
    
    battery_print_info(&info);
    return BATTERY_SUCCESS;
}

/* 控制充电 */
static int control_charge(battery_context_t *ctx, const char *action) {
    bool enable;
    
    if (strcmp(action, "on") == 0 || strcmp(action, "1") == 0) {
        enable = true;
    } else if (strcmp(action, "off") == 0 || strcmp(action, "0") == 0) {
        enable = false;
    } else {
        fprintf(stderr, "无效的充电状态: %s\n", action);
        return BATTERY_ERROR;
    }
    
    int ret = battery_set_charge(ctx, enable);
    if (ret != BATTERY_SUCCESS) {
        fprintf(stderr, "设置充电状态失败\n");
        return ret;
    }
    
    printf("充电已%s\n", enable ? "开启" : "关闭");
    return BATTERY_SUCCESS;
}

/* 设置充电电流 */
static int set_charge_current(battery_context_t *ctx, uint32_t current_ma) {
    int ret = battery_set_charge_current(ctx, current_ma);
    if (ret != BATTERY_SUCCESS) {
        fprintf(stderr, "设置充电电流失败\n");
        return ret;
    }
    
    printf("充电电流已设置为 %u mA\n", current_ma);
    return BATTERY_SUCCESS;
}

/* 设置充电电压 */
static int set_charge_voltage(battery_context_t *ctx, uint32_t voltage_mv) {
    int ret = battery_set_charge_voltage(ctx, voltage_mv);
    if (ret != BATTERY_SUCCESS) {
        fprintf(stderr, "设置充电电压失败\n");
        return ret;
    }
    
    printf("充电电压已设置为 %u mV\n", voltage_mv);
    return BATTERY_SUCCESS;
}

/* 监控模式 */
static int monitor_mode(battery_context_t *ctx) {
    battery_log("进入监控模式...");
    
    while (g_running) {
        battery_status_t status;
        int ret;
        
        ret = battery_get_status(ctx, &status);
        if (ret == BATTERY_SUCCESS) {
            printf("\033[2J\033[H"); /* 清屏 */
            battery_print_status(&status);
            
            /* 检查保护 */
            ret = battery_check_protection(ctx);
            if (ret != BATTERY_SUCCESS) {
                battery_log("警告: 电池保护触发");
            }
        }
        
        sleep(1);
    }
    
    return BATTERY_SUCCESS;
}

/* 守护进程模式 */
static int daemon_run_mode(battery_context_t *ctx) {
    battery_log("进入守护进程模式...");
    battery_log("恒压服务启动中...");
    
    /* 设置充电参数 */
    battery_set_charge_current(ctx, ctx->charge_config.current_ma);
    battery_set_charge_voltage(ctx, ctx->charge_config.voltage_mv);
    
    /* 开启充电 */
    battery_set_charge(ctx, true);
    battery_log("充电已开启，电流: %u mA, 电压: %u mV", 
               ctx->charge_config.current_ma, ctx->charge_config.voltage_mv);
    
    while (g_running) {
        battery_status_t status;
        int ret;
        
        ret = battery_get_status(ctx, &status);
        if (ret == BATTERY_SUCCESS) {
            /* 检查保护 */
            ret = battery_check_protection(ctx);
            if (ret != BATTERY_SUCCESS) {
                battery_log("警告: 电池保护触发");
            }
            
            /* 如果充电中，检查是否需要调整 */
            if (status.status == 1) {
                /* 检查温度是否过高 */
                if (status.temperature_c > (int32_t)ctx->charge_config.max_temp_c) {
                    battery_log("温度过高 (%d°C)，停止充电", status.temperature_c);
                    battery_set_charge(ctx, false);
                }
                
                /* 检查是否充满 */
                if (status.level_percent >= 100) {
                    battery_log("电池已充满，停止充电");
                    battery_set_charge(ctx, false);
                }
            } else {
                /* 如果未充电，尝试开启充电 */
                if (status.level_percent < 100 && 
                    status.temperature_c >= (int32_t)ctx->charge_config.min_temp_c &&
                    status.temperature_c <= (int32_t)ctx->charge_config.max_temp_c) {
                    battery_log("尝试开启充电...");
                    battery_set_charge(ctx, true);
                }
            }
        } else {
            battery_log("获取电池状态失败，继续运行...");
        }
        
        sleep(2);
    }
    
    battery_log("守护进程模式退出");
    return BATTERY_SUCCESS;
}

/* 主函数 */
int main(int argc, char *argv[]) {
    int opt;
    int daemon_mode = 0;
    int verbose = 0;
    int action = 0; /* 0=无, 1=状态, 2=信息, 3=监控 */
    char charge_action[16] = {0};
    uint32_t current_ma = 0;
    uint32_t voltage_mv = 0;
    
    /* 解析命令行参数 */
    static struct option long_options[] = {
        {"status",   no_argument,       0, 's'},
        {"info",     no_argument,       0, 'i'},
        {"charge",   required_argument, 0, 'c'},
        {"current",  required_argument, 0, 'C'},
        {"voltage",  required_argument, 0, 'V'},
        {"monitor",  no_argument,       0, 'm'},
        {"daemon",   no_argument,       0, 'd'},
        {"verbose",  no_argument,       0, 'v'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    /* 初始化默认配置 */
    init_default_config(&g_ctx);
    
    /* 解析参数 */
    while ((opt = getopt_long(argc, argv, "sic:C:V:mdvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                action = 1;
                break;
            case 'i':
                action = 2;
                break;
            case 'c':
                action = 3;
                strncpy(charge_action, optarg, sizeof(charge_action) - 1);
                break;
            case 'C':
                current_ma = atoi(optarg);
                action = 4;
                break;
            case 'V':
                voltage_mv = atoi(optarg);
                action = 5;
                break;
            case 'm':
                action = 6;
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return BATTERY_SUCCESS;
            default:
                print_usage(argv[0]);
                return BATTERY_ERROR;
        }
    }
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    battery_log("battery_control v%s 启动", BATTERY_VERSION);
    battery_log("构建日期: %s", BATTERY_BUILD_DATE);
    
    /* 守护进程模式 */
    if (daemon_mode) {
        battery_log("切换到守护进程模式...");
        if (daemon(0, 0) < 0) {
            perror("daemon");
            return BATTERY_ERROR;
        }
    }
    
    /* 初始化程序 */
    if (battery_init(&g_ctx) != BATTERY_SUCCESS) {
        battery_log("初始化失败");
        return BATTERY_ERROR;
    }
    
    /* 执行操作 */
    int ret = BATTERY_SUCCESS;
    switch (action) {
        case 1: /* 显示状态 */
            ret = show_status(&g_ctx);
            break;
        case 2: /* 显示信息 */
            ret = show_info(&g_ctx);
            break;
        case 3: /* 控制充电 */
            ret = control_charge(&g_ctx, charge_action);
            break;
        case 4: /* 设置充电电流 */
            ret = set_charge_current(&g_ctx, current_ma);
            break;
        case 5: /* 设置充电电压 */
            ret = set_charge_voltage(&g_ctx, voltage_mv);
            break;
        case 6: /* 监控模式 */
            ret = monitor_mode(&g_ctx);
            break;
        default:
            /* 如果没有参数，进入守护进程模式 */
            if (daemon_mode || action == 0) {
                ret = daemon_run_mode(&g_ctx);
            } else {
                ret = show_status(&g_ctx);
            }
            break;
    }
    
    /* 清理资源 */
    battery_log("正在清理资源...");
    battery_cleanup(&g_ctx);
    
    battery_log("程序退出");
    return ret;
}
