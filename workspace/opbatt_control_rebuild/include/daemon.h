/**
 * @file daemon.h
 * @brief 守护进程模式
 * 
 * 提供守护进程模式支持，包括信号处理、后台运行等
 */

#ifndef DAEMON_H
#define DAEMON_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>  /* for pid_t */

/* 守护进程配置 */
typedef struct {
    bool daemon_mode;          /* 守护进程模式 */
    char pid_file[256];        /* PID 文件路径 */
    char log_file[256];        /* 日志文件路径 */
} daemon_config_t;

/* 初始化守护进程 */
int daemon_init(void);

/* 启动守护进程模式 */
int daemon_start(void);

/* 停止守护进程模式 */
int daemon_stop(void);

/* 检查是否在守护进程模式 */
bool daemon_is_running(void);

/* 信号处理 */
void daemon_signal_handler(int signum);

/* 设置信号处理函数 */
int daemon_setup_signals(void);

/* 创建 PID 文件 */
int daemon_create_pid_file(const char *path);

/* 删除 PID 文件 */
int daemon_remove_pid_file(const char *path);

/* 读取 PID 文件 */
int daemon_read_pid_file(const char *path, pid_t *pid);

#endif /* DAEMON_H */
