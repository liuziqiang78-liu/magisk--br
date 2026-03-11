/**
 * @file main.c
 * @brief opbatt_control 主程序
 * 
 * 电池控制程序主入口
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>

/* 包含所有模块头文件 */
#include "sysfs.h"
#include "sysfs_paths.h"
#include "config.h"
#include "logger.h"
#include "battery.h"
#include "charge.h"
#include "ufcs.h"
#include "pps.h"
#include "svooc.h"
#include "temp.h"
#include "cv.h"
#include "ddrc.h"
#include "soc.h"
#include "daemon.h"

/* 外部变量声明 */
extern temp_control_t temp_config;
extern ddrc_state_t ddrc_state;

/* 全局变量 */
static volatile sig_atomic_t running = 1;
static batt_control_config_t config;
static bool daemon_mode = false;

/* 信号处理函数 */
static void signal_handler(int signum) {
    switch (signum) {
        case SIGTERM:
        case SIGINT:
            LOG_INFO("Received signal %d, shutting down...", signum);
            running = 0;
            break;
        case SIGHUP:
            LOG_INFO("Received SIGHUP, reloading configuration...");
            config_read(CONFIG_FILE_PATH, &config);
            break;
        default:
            break;
    }
}

/* 设置信号处理 */
static int setup_signals(void) {
    struct sigaction sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        fprintf(stderr, "Failed to setup SIGTERM handler\n");
        return -1;
    }
    
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        fprintf(stderr, "Failed to setup SIGINT handler\n");
        return -1;
    }
    
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        fprintf(stderr, "Failed to setup SIGHUP handler\n");
        return -1;
    }
    
    signal(SIGPIPE, SIG_IGN);
    
    return 0;
}

/* 打印使用说明 */
static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("\nOptions:\n");
    printf("  -d, --daemon     Run as daemon\n");
    printf("  -f, --foreground Run in foreground (default)\n");
    printf("  -c, --config     Config file path (default: %s)\n", CONFIG_FILE_PATH);
    printf("  -l, --log        Log file path (default: %s)\n", LOG_FILE_PATH);
    printf("  -v, --version    Print version\n");
    printf("  -h, --help       Print this help\n");
    printf("\n");
}

/* 打印版本信息 */
static void print_version(void) {
    printf("opbatt_control v1.0.0\n");
    printf("Rebuilt version - Pure battery control module\n");
    printf("No network verification, no license check\n");
    printf("\n");
}

/* 主控制循环 */
static void main_loop(void) {
    battery_status_t status;
    int effective_temp;
    int target_current = 0;
    int loop_count = 0;
    
    LOG_INFO("Main loop started (interval=%dms)", config.loop_interval_ms);
    
    while (running) {
        /* 读取电池状态 */
        if (battery_read_status(&status) != 0) {
            LOG_WARN("Failed to read battery status");
            sleep(1);
            continue;
        }
        
        /* 获取有效温度（考虑温度伪装） */
        effective_temp = temp_get_effective_temp(status.temperature, 
                                                  temp_config.fake_temp_enabled, 
                                                  temp_config.fake_temp);
        
        /* 检查温度是否过高 */
        if (temp_is_over_temp(effective_temp)) {
            LOG_WARN("Temperature too high: %d°C", temp_to_celsius(effective_temp));
            charge_stop();
            sleep(5);
            continue;
        }
        
        /* 检查是否在充电 */
        if (!status.is_charging) {
            LOG_DEBUG("Not charging, skipping control loop");
            sleep(5);
            continue;
        }
        
        /* 恒压控制 */
        target_current = cv_control(status.soc_percent, status.voltage_mv, status.current_ma);
        
        /* 温度控制 */
        temp_strategy_t temp_strategy = temp_get_strategy(effective_temp);
        int temp_limit = temp_get_current_limit(temp_strategy);
        
        /* 应用温度限制 */
        if (target_current > temp_limit) {
            target_current = temp_limit;
            LOG_DEBUG("Temperature limit applied: %dmA", target_current);
        }
        
        /* UFCS 控制 */
        bool ufcs_available;
        if (ufcs_is_available(&ufcs_available) == 0 && ufcs_available) {
            int ufcs_current = ufcs_get_charge_current(status.soc_percent, effective_temp);
            if (ufcs_current > 0 && ufcs_current < target_current) {
                target_current = ufcs_current;
                LOG_DEBUG("UFCS current applied: %dmA", target_current);
            }
        }
        
        /* PPS 控制 */
        bool pps_available;
        if (pps_is_available(&pps_available) == 0 && pps_available) {
            int pps_current = pps_get_charge_current(status.soc_percent, effective_temp);
            if (pps_current > 0 && pps_current < target_current) {
                target_current = pps_current;
                LOG_DEBUG("PPS current applied: %dmA", target_current);
            }
        }
        
        /* SVOOC 控制 */
        bool svooc_available;
        if (svooc_is_available(&svooc_available) == 0 && svooc_available) {
            svooc_charge_strategy_t svooc_strategy = svooc_get_charge_strategy(status.soc_percent, effective_temp);
            if (svooc_strategy == SVOOC_CHARGE_STOP) {
                target_current = 0;
                LOG_DEBUG("SVOOC stop charging");
            }
        }
        
        /* DDRC 控制 */
        if (ddrc_should_trigger(status.voltage_mv, ddrc_state.deep_discharge_count)) {
            int ddrc_current = ddrc_get_limited_current(status.soc_percent, status.voltage_mv);
            if (ddrc_current < target_current) {
                target_current = ddrc_current;
                LOG_DEBUG("DDRC current applied: %dmA", target_current);
            }
        }
        
        /* 应用充电电流 */
        if (target_current != status.current_ma) {
            charge_set_current(target_current);
            LOG_INFO("Charge current: %dmA -> %dmA (SOC=%d%%, Voltage=%dmV, Temp=%d°C)",
                     status.current_ma, target_current, status.soc_percent, 
                     status.voltage_mv, temp_to_celsius(effective_temp));
        }
        
        /* SOC 平滑 */
        int smoothed_soc = soc_smooth(status.soc_percent, status.is_charging);
        
        /* 记录日志（每10个循环记录一次） */
        if (loop_count % 10 == 0) {
            logger_log_battery_status(smoothed_soc, status.voltage_mv, 
                                     target_current, effective_temp);
        }
        
        loop_count++;
        
        /* 等待下一个循环 */
        usleep(config.loop_interval_ms * 1000);
    }
    
    LOG_INFO("Main loop stopped");
}

/* 初始化所有模块 */
static int init_modules(void) {
    /* 初始化日志系统 */
    if (logger_init(LOG_FILE_PATH) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return -1;
    }
    
    /* 初始化配置 */
    if (config_read(CONFIG_FILE_PATH, &config) != 0) {
        LOG_WARN("Failed to read config, using defaults");
        config_set_default(&config);
    }
    
    /* 验证配置 */
    if (config_validate(&config) != 0) {
        LOG_ERROR("Invalid configuration");
        return -1;
    }
    
    /* 初始化 sysfs 接口层 */
    /* sysfs 接口层不需要初始化 */
    
    /* 初始化电池监控 */
    /* battery 模块不需要初始化 */
    
    /* 初始化充电控制 */
    if (charge_init() != 0) {
        LOG_ERROR("Failed to initialize charge control");
        return -1;
    }
    
    /* 初始化 UFCS */
    if (ufcs_init() != 0) {
        LOG_ERROR("Failed to initialize UFCS");
        return -1;
    }
    
    /* 初始化 PPS */
    if (pps_init() != 0) {
        LOG_ERROR("Failed to initialize PPS");
        return -1;
    }
    
    /* 初始化 SVOOC */
    if (svooc_init() != 0) {
        LOG_ERROR("Failed to initialize SVOOC");
        return -1;
    }
    
    /* 初始化温度控制 */
    if (temp_init() != 0) {
        LOG_ERROR("Failed to initialize temperature control");
        return -1;
    }
    
    /* 初始化恒压控制 */
    if (cv_init() != 0) {
        LOG_ERROR("Failed to initialize CV control");
        return -1;
    }
    
    /* 初始化 DDRC */
    if (ddrc_init() != 0) {
        LOG_ERROR("Failed to initialize DDRC");
        return -1;
    }
    
    /* 初始化 SOC 平滑 */
    if (soc_init() != 0) {
        LOG_ERROR("Failed to initialize SOC smooth");
        return -1;
    }
    
    /* 初始化守护进程 */
    if (daemon_init() != 0) {
        LOG_ERROR("Failed to initialize daemon");
        return -1;
    }
    
    LOG_INFO("All modules initialized successfully");
    
    return 0;
}

/* 清理所有模块 */
static void cleanup_modules(void) {
    /* 停止充电 */
    charge_stop();
    
    /* 关闭日志系统 */
    logger_close();
    
    /* 如果是守护进程模式，删除 PID 文件 */
    if (daemon_mode) {
        daemon_stop();
    }
}

/* 主函数 */
int main(int argc, char *argv[]) {
    int opt;
    const char *config_path = CONFIG_FILE_PATH;
    const char *log_path = LOG_FILE_PATH;
    
    static struct option long_options[] = {
        {"daemon",     no_argument,       0, 'd'},
        {"foreground", no_argument,       0, 'f'},
        {"config",     required_argument, 0, 'c'},
        {"log",        required_argument, 0, 'l'},
        {"version",    no_argument,       0, 'v'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    /* 解析命令行参数 */
    while ((opt = getopt_long(argc, argv, "dfc:l:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = true;
                break;
            case 'f':
                daemon_mode = false;
                break;
            case 'c':
                config_path = optarg;
                break;
            case 'l':
                log_path = optarg;
                break;
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    /* 打印版本信息 */
    print_version();
    
    /* 设置信号处理 */
    if (setup_signals() != 0) {
        fprintf(stderr, "Failed to setup signal handlers\n");
        return 1;
    }
    
    /* 如果是守护进程模式 */
    if (daemon_mode) {
        if (daemon_start() != 0) {
            fprintf(stderr, "Failed to start daemon mode\n");
            return 1;
        }
    }
    
    /* 初始化所有模块 */
    if (init_modules() != 0) {
        fprintf(stderr, "Failed to initialize modules\n");
        cleanup_modules();
        return 1;
    }
    
    /* 运行主循环 */
    main_loop();
    
    /* 清理所有模块 */
    cleanup_modules();
    
    LOG_INFO("opbatt_control exited");
    
    return 0;
}
