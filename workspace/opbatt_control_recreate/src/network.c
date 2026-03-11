/**
 * opbatt_control - 网络通信模块
 * 支持 TCP 和 SSL/TLS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "opbatt_control.h"

/* SSL 上下文（如果使用 OpenSSL） */
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
static SSL_CTX *g_ssl_ctx = NULL;
static SSL *g_ssl = NULL;
#endif

/* 创建 TCP 套接字 */
static int create_socket(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        opbatt_log("创建套接字失败: %s", strerror(errno));
        return OPBATT_ERROR;
    }
    
    /* 设置套接字选项 */
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    
    return fd;
}

/* 初始化网络模块 */
int opbatt_network_init(opbatt_context_t *ctx) {
    opbatt_log("初始化网络模块...");
    
    ctx->server_fd = -1;
    
#ifdef USE_OPENSSL
    /* 初始化 OpenSSL */
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    /* 创建 SSL 上下文 */
    const SSL_METHOD *method = TLS_server_method();
    g_ssl_ctx = SSL_CTX_new(method);
    if (!g_ssl_ctx) {
        opbatt_log("创建 SSL 上下文失败");
        return OPBATT_ERROR_SSL;
    }
    
    /* 加载证书和私钥 */
    char cert_path[512];
    char key_path[512];
    snprintf(cert_path, sizeof(cert_path), "%s/cert.pem", LICENSE_PATH);
    snprintf(key_path, sizeof(key_path), "%s/private/key.pem", LICENSE_PATH);
    
    if (SSL_CTX_use_certificate_file(g_ssl_ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        opbatt_log("加载证书失败: %s", cert_path);
        ERR_print_errors_fp(stderr);
        return OPBATT_ERROR_SSL;
    }
    
    if (SSL_CTX_use_PrivateKey_file(g_ssl_ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        opbatt_log("加载私钥失败: %s", key_path);
        ERR_print_errors_fp(stderr);
        return OPBATT_ERROR_SSL;
    }
    
    if (!SSL_CTX_check_private_key(g_ssl_ctx)) {
        opbatt_log("私钥和证书不匹配");
        return OPBATT_ERROR_SSL;
    }
    
    opbatt_log("SSL/TLS 初始化成功");
#endif
    
    return OPBATT_SUCCESS;
}

/* 启动服务器 */
int opbatt_network_start_server(opbatt_context_t *ctx) {
    struct sockaddr_in addr;
    
    /* 创建套接字 */
    ctx->server_fd = create_socket();
    if (ctx->server_fd < 0) {
        return OPBATT_ERROR_NETWORK;
    }
    
    /* 绑定地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ctx->network.host);
    addr.sin_port = htons(ctx->network.port);
    
    if (bind(ctx->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        opbatt_log("绑定地址失败: %s", strerror(errno));
        close(ctx->server_fd);
        return OPBATT_ERROR_NETWORK;
    }
    
    /* 开始监听 */
    if (listen(ctx->server_fd, MAX_CONNECTIONS) < 0) {
        opbatt_log("监听失败: %s", strerror(errno));
        close(ctx->server_fd);
        return OPBATT_ERROR_NETWORK;
    }
    
    opbatt_log("服务器监听 %s:%d", ctx->network.host, ctx->network.port);
    return OPBATT_SUCCESS;
}

/* 连接到服务器 */
int opbatt_network_connect(opbatt_context_t *ctx) {
    struct sockaddr_in addr;
    struct hostent *host;
    
    /* 解析主机名 */
    host = gethostbyname(ctx->network.host);
    if (!host) {
        opbatt_log("无法解析主机名: %s", ctx->network.host);
        return OPBATT_ERROR_NETWORK;
    }
    
    /* 创建套接字 */
    ctx->server_fd = create_socket();
    if (ctx->server_fd < 0) {
        return OPBATT_ERROR_NETWORK;
    }
    
    /* 连接服务器 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ctx->network.port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
    
    if (connect(ctx->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        opbatt_log("连接失败: %s", strerror(errno));
        close(ctx->server_fd);
        return OPBATT_ERROR_NETWORK;
    }
    
#ifdef USE_OPENSSL
    /* 创建 SSL 连接 */
    g_ssl = SSL_new(g_ssl_ctx);
    SSL_set_fd(g_ssl, ctx->server_fd);
    
    if (SSL_connect(g_ssl) <= 0) {
        opbatt_log("SSL 连接失败");
        ERR_print_errors_fp(stderr);
        SSL_free(g_ssl);
        g_ssl = NULL;
        close(ctx->server_fd);
        return OPBATT_ERROR_SSL;
    }
    
    opbatt_log("SSL 连接建立成功");
#endif
    
    return OPBATT_SUCCESS;
}

/* 发送数据 */
int opbatt_network_send(opbatt_context_t *ctx, const void *data, size_t len) {
    int ret;
    
#ifdef USE_OPENSSL
    if (ctx->network.use_ssl && g_ssl) {
        ret = SSL_write(g_ssl, data, len);
    } else {
        ret = send(ctx->server_fd, data, len, 0);
    }
#else
    ret = send(ctx->server_fd, data, len, 0);
#endif
    
    if (ret < 0) {
        opbatt_log("发送数据失败: %s", strerror(errno));
        return OPBATT_ERROR_NETWORK;
    }
    
    return ret;
}

/* 接收数据 */
int opbatt_network_recv(opbatt_context_t *ctx, void *data, size_t len) {
    int ret;
    
#ifdef USE_OPENSSL
    if (ctx->network.use_ssl && g_ssl) {
        ret = SSL_read(g_ssl, data, len);
    } else {
        ret = recv(ctx->server_fd, data, len, 0);
    }
#else
    ret = recv(ctx->server_fd, data, len, 0);
#endif
    
    if (ret < 0) {
        opbatt_log("接收数据失败: %s", strerror(errno));
        return OPBATT_ERROR_NETWORK;
    }
    
    return ret;
}

/* 接受客户端连接 */
int opbatt_network_accept(opbatt_context_t *ctx) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_fd = accept(ctx->server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        opbatt_log("接受连接失败: %s", strerror(errno));
        return OPBATT_ERROR_NETWORK;
    }
    
    opbatt_log("接受来自 %s:%d 的连接",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
    
#ifdef USE_OPENSSL
    /* 创建 SSL 连接 */
    SSL *ssl = SSL_new(g_ssl_ctx);
    SSL_set_fd(ssl, client_fd);
    
    if (SSL_accept(ssl) <= 0) {
        opbatt_log("SSL 握手失败");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return OPBATT_ERROR_SSL;
    }
    
    /* 保存 SSL 对象到上下文 */
    g_ssl = ssl;
#endif
    
    return client_fd;
}

/* 清理网络模块 */
int opbatt_network_cleanup(opbatt_context_t *ctx) {
    opbatt_log("清理网络模块...");
    
#ifdef USE_OPENSSL
    if (g_ssl) {
        SSL_shutdown(g_ssl);
        SSL_free(g_ssl);
        g_ssl = NULL;
    }
    
    if (g_ssl_ctx) {
        SSL_CTX_free(g_ssl_ctx);
        g_ssl_ctx = NULL;
    }
#endif
    
    if (ctx->server_fd >= 0) {
        close(ctx->server_fd);
        ctx->server_fd = -1;
    }
    
    return OPBATT_SUCCESS;
}
