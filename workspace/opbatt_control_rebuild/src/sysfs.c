/**
 * @file sysfs.c
 * @brief sysfs 接口层实现
 * 
 * 提供统一的 sysfs 文件访问接口
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "sysfs.h"
#include "sysfs_paths.h"
#include "logger.h"

/* 读取 sysfs 整数值 */
int sysfs_read_int(const char *path, int *value) {
    FILE *fp;
    char buffer[64];
    int ret;
    
    if (!path || !value) {
        LOG_ERROR("Invalid parameters: path=%p, value=%p", path, value);
        return -1;
    }
    
    fp = fopen(path, "r");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", path, strerror(errno));
        return -1;
    }
    
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        LOG_ERROR("Failed to read %s: %s", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    /* 去除换行符 */
    buffer[strcspn(buffer, "\n")] = '\0';
    
    ret = sscanf(buffer, "%d", value);
    if (ret != 1) {
        LOG_ERROR("Failed to parse %s: %s", path, buffer);
        return -1;
    }
    
    return 0;
}

/* 读取 sysfs 字符串 */
int sysfs_read_string(const char *path, char *buffer, size_t size) {
    FILE *fp;
    
    if (!path || !buffer || size == 0) {
        LOG_ERROR("Invalid parameters: path=%p, buffer=%p, size=%zu", path, buffer, size);
        return -1;
    }
    
    fp = fopen(path, "r");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", path, strerror(errno));
        return -1;
    }
    
    if (fgets(buffer, size, fp) == NULL) {
        LOG_ERROR("Failed to read %s: %s", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    /* 去除换行符 */
    buffer[strcspn(buffer, "\n")] = '\0';
    
    return 0;
}

/* 写入 sysfs 整数值 */
int sysfs_write_int(const char *path, int value) {
    FILE *fp;
    int ret;
    
    if (!path) {
        LOG_ERROR("Invalid parameters: path=%p", path);
        return -1;
    }
    
    fp = fopen(path, "w");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", path, strerror(errno));
        return -1;
    }
    
    ret = fprintf(fp, "%d", value);
    if (ret < 0) {
        LOG_ERROR("Failed to write %s: %s", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    return 0;
}

/* 写入 sysfs 字符串 */
int sysfs_write_string(const char *path, const char *value) {
    FILE *fp;
    int ret;
    
    if (!path || !value) {
        LOG_ERROR("Invalid parameters: path=%p, value=%p", path, value);
        return -1;
    }
    
    fp = fopen(path, "w");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", path, strerror(errno));
        return -1;
    }
    
    ret = fprintf(fp, "%s", value);
    if (ret < 0) {
        LOG_ERROR("Failed to write %s: %s", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    return 0;
}

/* 检查文件是否存在 */
int sysfs_exists(const char *path) {
    if (!path) {
        LOG_ERROR("Invalid parameters: path=%p", path);
        return 0;
    }
    
    return (access(path, F_OK) == 0);
}
