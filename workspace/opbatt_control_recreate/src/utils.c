/**
 * opbatt_control - 工具函数模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "opbatt_control.h"

/* 日志级别 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3
} log_level_t;

static log_level_t g_log_level = LOG_LEVEL_INFO;

/* 获取当前时间戳 */
static const char *get_timestamp(void) {
    static char timestamp[64];
    struct timeval tv;
    struct tm *tm;
    
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000);
    
    return timestamp;
}

/* 日志输出 */
void opbatt_log(const char *format, ...) {
    va_list args;
    
    printf("[%s] ", get_timestamp());
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}

/* 十六进制转储 */
void opbatt_hexdump(const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    size_t i, j;
    
    for (i = 0; i < len; i += 16) {
        printf("%08zx: ", i);
        
        /* 十六进制 */
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02x ", bytes[i + j]);
            } else {
                printf("   ");
            }
        }
        
        printf(" ");
        
        /* ASCII */
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                uint8_t c = bytes[i + j];
                printf("%c", (c >= 32 && c < 127) ? c : '.');
            } else {
                printf(" ");
            }
        }
        
        printf("\n");
    }
}

/* 字符串转整数 */
int opbatt_strtoi(const char *str, int default_value) {
    if (!str || !*str) {
        return default_value;
    }
    return atoi(str);
}

/* 字符串转布尔值 */
bool opbatt_strtobool(const char *str, bool default_value) {
    if (!str || !*str) {
        return default_value;
    }
    
    if (strcasecmp(str, "true") == 0 || 
        strcasecmp(str, "yes") == 0 || 
        strcasecmp(str, "1") == 0) {
        return true;
    }
    
    if (strcasecmp(str, "false") == 0 || 
        strcasecmp(str, "no") == 0 || 
        strcasecmp(str, "0") == 0) {
        return false;
    }
    
    return default_value;
}

/* 内存分配 */
void *opbatt_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        opbatt_log("内存分配失败: %zu 字节", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/* 内存重新分配 */
void *opbatt_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        opbatt_log("内存重新分配失败: %zu 字节", size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

/* 内存释放 */
void opbatt_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

/* 字符串复制 */
char *opbatt_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char *new_str = (char *)opbatt_malloc(len);
    memcpy(new_str, str, len);
    
    return new_str;
}

/* 字符串拼接 */
char *opbatt_strjoin(const char *str1, const char *str2) {
    if (!str1) {
        return opbatt_strdup(str2);
    }
    if (!str2) {
        return opbatt_strdup(str1);
    }
    
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char *result = (char *)opbatt_malloc(len1 + len2 + 1);
    
    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2);
    result[len1 + len2] = '\0';
    
    return result;
}

/* 文件读取 */
char *opbatt_read_file(const char *path, size_t *size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        opbatt_log("无法打开文件: %s", path);
        return NULL;
    }
    
    /* 获取文件大小 */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size < 0) {
        opbatt_log("无法获取文件大小: %s", path);
        fclose(fp);
        return NULL;
    }
    
    /* 读取文件内容 */
    char *buffer = (char *)opbatt_malloc(file_size + 1);
    size_t read_size = fread(buffer, 1, file_size, fp);
    fclose(fp);
    
    buffer[read_size] = '\0';
    
    if (size) {
        *size = read_size;
    }
    
    return buffer;
}

/* 文件写入 */
int opbatt_write_file(const char *path, const void *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        opbatt_log("无法创建文件: %s", path);
        return OPBATT_ERROR;
    }
    
    size_t write_size = fwrite(data, 1, size, fp);
    fclose(fp);
    
    if (write_size != size) {
        opbatt_log("写入文件失败: %s", path);
        return OPBATT_ERROR;
    }
    
    return OPBATT_SUCCESS;
}

/* 检查文件是否存在 */
bool opbatt_file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/* 创建目录 */
int opbatt_mkdir_p(const char *path) {
    char tmp[512];
    char *p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                opbatt_log("创建目录失败: %s", tmp);
                return OPBATT_ERROR;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        opbatt_log("创建目录失败: %s", tmp);
        return OPBATT_ERROR;
    }
    
    return OPBATT_SUCCESS;
}

/* 获取文件大小 */
size_t opbatt_get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

/* 睡眠（毫秒） */
void opbatt_msleep(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}

/* 获取当前时间（毫秒） */
uint64_t opbatt_get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* CRC32 计算 */
uint32_t opbatt_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}
