/**
 * opbatt_control - 许可证验证模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "opbatt_control.h"

#ifdef USE_OPENSSL
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#endif

/* 许可证文件路径 */
#define LICENSE_FILE "/root/opbattlic/license.dat"
#define LICENSE_PUBLIC_KEY "/root/opbattlic/public_key.pem"

/* 许可证数据结构 */
typedef struct {
    char product_id[64];
    char license_key[256];
    uint64_t issue_time;
    uint64_t expire_time;
    uint32_t features;
    uint8_t signature[256];
} license_data_t;

/* 计算许可证哈希 */
static int calculate_license_hash(const license_data_t *license, uint8_t *hash) {
#ifdef USE_OPENSSL
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    SHA256_Update(&sha256, license->product_id, strlen(license->product_id));
    SHA256_Update(&sha256, license->license_key, strlen(license->license_key));
    SHA256_Update(&sha256, &license->issue_time, sizeof(license->issue_time));
    SHA256_Update(&sha256, &license->expire_time, sizeof(license->expire_time));
    SHA256_Update(&sha256, &license->features, sizeof(license->features));
    
    SHA256_Final(hash, &sha256);
    return OPBATT_SUCCESS;
#else
    /* 简单的哈希实现 */
    uint32_t h = 0;
    const uint8_t *data = (const uint8_t *)license;
    size_t len = sizeof(license_data_t) - sizeof(license->signature);
    
    for (size_t i = 0; i < len; i++) {
        h = h * 31 + data[i];
    }
    
    memcpy(hash, &h, sizeof(h));
    return OPBATT_SUCCESS;
#endif
}

/* 验证许可证签名 */
static int verify_license_signature(const license_data_t *license) {
#ifdef USE_OPENSSL
    FILE *fp = fopen(LICENSE_PUBLIC_KEY, "r");
    if (!fp) {
        opbatt_log("无法打开公钥文件: %s", LICENSE_PUBLIC_KEY);
        return OPBATT_ERROR_LICENSE;
    }
    
    RSA *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);
    
    if (!rsa) {
        opbatt_log("读取公钥失败");
        return OPBATT_ERROR_LICENSE;
    }
    
    /* 计算哈希 */
    uint8_t hash[SHA256_DIGEST_LENGTH];
    calculate_license_hash(license, hash);
    
    /* 验证签名 */
    int result = RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH,
                           license->signature, 256, rsa);
    
    RSA_free(rsa);
    
    if (result != 1) {
        opbatt_log("许可证签名验证失败");
        return OPBATT_ERROR_LICENSE;
    }
    
    return OPBATT_SUCCESS;
#else
    /* 简单验证：检查许可证密钥格式 */
    if (strlen(license->license_key) < 16) {
        opbatt_log("许可证密钥格式无效");
        return OPBATT_ERROR_LICENSE;
    }
    return OPBATT_SUCCESS;
#endif
}

/* 检查许可证是否过期 */
static bool is_license_expired(const license_data_t *license) {
    time_t now = time(NULL);
    return (uint64_t)now > license->expire_time;
}

/* 加载许可证文件 */
static int load_license_file(license_data_t *license) {
    FILE *fp = fopen(LICENSE_FILE, "rb");
    if (!fp) {
        opbatt_log("无法打开许可证文件: %s", LICENSE_FILE);
        return OPBATT_ERROR_LICENSE;
    }
    
    size_t read_size = fread(license, 1, sizeof(license_data_t), fp);
    fclose(fp);
    
    if (read_size != sizeof(license_data_t)) {
        opbatt_log("许可证文件格式错误");
        return OPBATT_ERROR_LICENSE;
    }
    
    return OPBATT_SUCCESS;
}

/* 初始化许可证模块 */
int opbatt_license_init(opbatt_context_t *ctx) {
    opbatt_log("初始化许可证模块...");
    
    ctx->license.valid = false;
    strcpy(ctx->license.license_key, "");
    strcpy(ctx->license.product_id, "OPBATT-001");
    ctx->license.expire_time = 0;
    
    return OPBATT_SUCCESS;
}

/* 验证许可证 */
int opbatt_license_verify(opbatt_context_t *ctx) {
    license_data_t license;
    int ret;
    
    opbatt_log("验证许可证...");
    
    /* 加载许可证文件 */
    ret = load_license_file(&license);
    if (ret != OPBATT_SUCCESS) {
        opbatt_log("警告: 无法加载许可证文件，使用演示模式");
        /* 演示模式：设置默认许可证 */
        strcpy(license.product_id, "OPBATT-DEMO");
        strcpy(license.license_key, "DEMO-LICENSE-KEY-12345");
        license.issue_time = time(NULL);
        license.expire_time = time(NULL) + 365 * 24 * 3600; /* 1年有效期 */
        license.features = 0xFFFFFFFF;
    }
    
    /* 验证签名 */
    ret = verify_license_signature(&license);
    if (ret != OPBATT_SUCCESS) {
        opbatt_log("许可证签名验证失败");
        return ret;
    }
    
    /* 检查是否过期 */
    if (is_license_expired(&license)) {
        opbatt_log("许可证已过期");
        return OPBATT_ERROR_LICENSE;
    }
    
    /* 更新上下文 */
    ctx->license.valid = true;
    strcpy(ctx->license.license_key, license.license_key);
    strcpy(ctx->license.product_id, license.product_id);
    ctx->license.expire_time = license.expire_time;
    
    opbatt_log("许可证验证成功: 产品ID=%s, 过期时间=%lu",
               ctx->license.product_id, ctx->license.expire_time);
    
    return OPBATT_SUCCESS;
}

/* 检查许可证是否有效 */
bool opbatt_license_is_valid(opbatt_context_t *ctx) {
    if (!ctx->license.valid) {
        return false;
    }
    
    /* 检查是否过期 */
    time_t now = time(NULL);
    return (uint64_t)now < ctx->license.expire_time;
}

/* 生成演示许可证（用于测试） */
int opbatt_license_generate_demo(const char *output_path) {
    license_data_t license;
    
    memset(&license, 0, sizeof(license));
    
    strcpy(license.product_id, "OPBATT-DEMO");
    strcpy(license.license_key, "DEMO-LICENSE-KEY-12345");
    license.issue_time = time(NULL);
    license.expire_time = time(NULL) + 365 * 24 * 3600; /* 1年有效期 */
    license.features = 0xFFFFFFFF;
    
    /* 简单的签名（实际应该使用 RSA 签名） */
    uint8_t hash[32];
    calculate_license_hash(&license, hash);
    memcpy(license.signature, hash, sizeof(license.signature));
    
    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        opbatt_log("无法创建许可证文件: %s", output_path);
        return OPBATT_ERROR_LICENSE;
    }
    
    fwrite(&license, 1, sizeof(license_data_t), fp);
    fclose(fp);
    
    opbatt_log("演示许可证已生成: %s", output_path);
    return OPBATT_SUCCESS;
}
