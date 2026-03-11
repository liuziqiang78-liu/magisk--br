/**
 * @file daemon.c
 * @brief 守护进程模式实现
 * 
 * 提供守护进程模式支持，包括信号处理、后台运行等
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "daemon.h"
#include "logger.h"

/* 守护进程配置 */
static daemon_config_t daemon_config = {
    .daemon_mode = false,
    .pid_file = "/data/opbatt/opbatt_control.pid",
    .log_file = LOG_FILE_PATH
};

/* 运行标志 */
static volatile sig_atomic_t running = 1;

/* 信号处理函数 */
void daemon_signal_handler(int signum) {
    switch (signum) {
        case SIGTERM:
        case SIGINT:
            LOG_INFO("Received signal %d, shutting down...", signum);
            running = 0;
            break;
        case SIGHUP:
            LOG_INFO("Received SIGHUP, reloading configuration...");
            /* 这里可以添加配置重载逻辑 */
            break;
        default:
            LOG_WARN("Received unexpected signal %d", signum);
            break;
    }
}

/* 设置信号处理函数 */
int daemon_setup_signals(void) {
    struct sigaction sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = daemon_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    /* 设置信号处理 */
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        LOG_ERROR("Failed to setup SIGTERM handler");
        return -1;
    }
    
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        LOG_ERROR("Failed to setup SIGINT handler");
        return -1;
    }
    
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        LOG_ERROR("Failed to setup SIGHUP handler");
        return -1;
    }
    
    /* 忽略 SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
    
    LOG_INFO("Signal handlers setup completed");
    return 0;
}

/* 创建 PID 文件 */
int daemon_create_pid_file(const char *path) {
    FILE *fp;
    pid_t pid;
    
    if (!path) {
        LOG_ERROR("Invalid parameters: path=%p", path);
        return -1;
    }
    
    /* 检查 PID 文件是否已存在 */
    if (access(path, F_OK) == 0) {
        LOG_ERROR("PID file already exists: %s", path);
        return -1;
    }
    
    fp = fopen(path, "w");
    if (!fp) {
        LOG_ERROR("Failed to create PID file %s: %s", path, strerror(errno));
        return -1;
    }
    
    pid = getpid();
    fprintf(fp, "%d\n", pid);
    fclose(fp);
    
    LOG_INFO("PID file created: %s (PID=%d)", path, pid);
    return 0;
}

/* 删除 PID 文件 */
int daemon_remove_pid_file(const char *path) {
    if (!path) {
        LOG_ERROR("Invalid parameters: path=%p", path);
        return -1;
    }
    
    if (unlink(path) == 0) {
        LOG_INFO("PID file removed: %s", path);
        return 0;
    }
    
    LOG_WARN("Failed to remove PID file %s: %s", path, strerror(errno));
    return -1;
}

/* 读取 PID 文件 */
int daemon_read_pid_file(const char *path, pid_t *pid) {
    FILE *fp;
    int pid_value;
    
    if (!path || !pid) {
        LOG_ERROR("Invalid parameters: path=%p, pid=%p", path, pid);
        return -1;
    }
    
    fp = fopen(path, "r");
    if (!fp) {
        LOG_ERROR("Failed to open PID file %s: %s", path, strerror(errno));
        return -1;
    }
    
    if (fscanf(fp, "%d", &pid_value) != 1) {
        LOG_ERROR("Failed to read PID from %s", path);
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    *pid = (pid_t)pid_value;
    
    return 0;
}

/* 初始化守护进程 */
int daemon_init(void) {
    LOG_INFO("Daemon module initialized");
    return 0;
}

/* 启动守护进程模式 */
int daemon_start(void) {
    pid_t pid, sid;
    int fd;
    
    LOG_INFO("Starting daemon mode...");
    
    /* 第一次 fork */
    pid = fork();
    if (pid < 0) {
        LOG_ERROR("First fork failed");
        return -1;
    }
    
    if (pid > 0) {
        /* 父进程退出 */
        exit(EXIT_SUCCESS);
    }
    
    /* 子进程继续 */
    /* 创建新会话 */
    sid = setsid();
    if (sid < 0) {
        LOG_ERROR("setsid failed");
        return -1;
    }
    
    /* 第二次 fork */
    pid = fork();
    if (pid < 0) {
        LOG_ERROR("Second fork failed");
        return -1;
    }
    
    if (pid > 0) {
        /* 父进程退出 */
        exit(EXIT_SUCCESS);
    }
    
    /* 设置文件权限掩码 */
    umask(0);
    
    /* 更改工作目录 */
    if (chdir("/") < 0) {
        LOG_ERROR("chdir failed");
        return -1;
    }
    
    /* 关闭所有打开的文件描述符 */
    for (fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }
    
    /* 重定向标准输入、输出、错误到 /dev/null */
    fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) {
            close(fd);
        }
    }
    
    /* 创建 PID 文件 */
    if (daemon_create_pid_file(daemon_config.pid_file) != 0) {
        return -1;
    }
    
    /* 设置信号处理 */
    if (daemon_setup_signals() != 0) {
        daemon_remove_pid_file(daemon_config.pid_file);
        return -1;
    }
    
    daemon_config.daemon_mode = true;
    LOG_INFO("Daemon mode started successfully");
    
    return 0;
}

/* 停止守护进程模式 */
int daemon_stop(void) {
    LOG_INFO("Stopping daemon mode...");
    
    /* 删除 PID 文件 */
    daemon_remove_pid_file(daemon_config.pid_file);
    
    daemon_config.daemon_mode = false;
    LOG_INFO("Daemon mode stopped");
    
    return 0;
}

/* 检查是否在守护进程模式 */
bool daemon_is_running(void) {
    return running;
}
