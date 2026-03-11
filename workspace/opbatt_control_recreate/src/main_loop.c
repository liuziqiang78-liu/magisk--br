/**
 * opbatt_control - 主循环模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include "opbatt_control.h"

/* 协议命令定义 */
#define CMD_GET_STATUS    0x01
#define CMD_SET_CHARGE    0x02
#define CMD_SET_CURRENT   0x03
#define CMD_GET_INFO      0x04
#define CMD_PING         0x05
#define CMD_PONG         0x06

/* 协议响应码 */
#define RESP_SUCCESS      0x00
#define RESP_ERROR        0x01
#define RESP_INVALID_CMD  0x02
#define RESP_LICENSE_ERR  0x03

/* 协议数据包 */
typedef struct {
    uint8_t  cmd;
    uint8_t  resp;
    uint16_t length;
    uint8_t  data[MAX_BUFFER_SIZE];
} protocol_packet_t;

/* 处理客户端请求 */
static int handle_client_request(opbatt_context_t *ctx, int client_fd) {
    protocol_packet_t packet;
    int ret;
    
    /* 接收数据包 */
    ret = recv(client_fd, &packet, sizeof(packet), 0);
    if (ret <= 0) {
        return OPBATT_ERROR;
    }
    
    opbatt_log("收到命令: 0x%02x, 长度: %d", packet.cmd, packet.length);
    
    /* 处理命令 */
    packet.resp = RESP_SUCCESS;
    
    switch (packet.cmd) {
        case CMD_GET_STATUS: {
            /* 获取电池状态 */
            battery_status_t status;
            opbatt_battery_get_status(ctx, &status);
            
            memcpy(packet.data, &status, sizeof(status));
            packet.length = sizeof(status);
            break;
        }
        
        case CMD_SET_CHARGE: {
            /* 设置充电状态 */
            if (packet.length >= 1) {
                bool enable = packet.data[0] != 0;
                ret = opbatt_battery_set_charge(ctx, enable);
                if (ret != OPBATT_SUCCESS) {
                    packet.resp = RESP_ERROR;
                }
            } else {
                packet.resp = RESP_INVALID_CMD;
            }
            break;
        }
        
        case CMD_SET_CURRENT: {
            /* 设置充电电流 */
            if (packet.length >= 4) {
                uint32_t current_ma = *(uint32_t *)packet.data;
                ret = opbatt_battery_set_current(ctx, current_ma);
                if (ret != OPBATT_SUCCESS) {
                    packet.resp = RESP_ERROR;
                }
            } else {
                packet.resp = RESP_INVALID_CMD;
            }
            break;
        }
        
        case CMD_GET_INFO: {
            /* 获取程序信息 */
            char info[256];
            snprintf(info, sizeof(info), 
                    "opbatt_control v%s\nBuild: %s\nLicense: %s",
                    OPBATT_VERSION, OPBATT_BUILD_DATE,
                    ctx->license.valid ? "Valid" : "Invalid");
            
            memcpy(packet.data, info, strlen(info));
            packet.length = strlen(info);
            break;
        }
        
        case CMD_PING: {
            /* Ping 响应 */
            packet.cmd = CMD_PONG;
            strcpy((char *)packet.data, "PONG");
            packet.length = 4;
            break;
        }
        
        default: {
            packet.resp = RESP_INVALID_CMD;
            break;
        }
    }
    
    /* 发送响应 */
    ret = send(client_fd, &packet, sizeof(packet.cmd) + sizeof(packet.resp) + 
              sizeof(packet.length) + packet.length, 0);
    if (ret < 0) {
        opbatt_log("发送响应失败: %s", strerror(errno));
        return OPBATT_ERROR;
    }
    
    return OPBATT_SUCCESS;
}

/* 服务器主循环 */
static int server_main_loop(opbatt_context_t *ctx) {
    fd_set read_fds;
    int max_fd;
    struct timeval timeout;
    
    opbatt_log("服务器主循环启动");
    
    while (ctx->running) {
        FD_ZERO(&read_fds);
        FD_SET(ctx->server_fd, &read_fds);
        max_fd = ctx->server_fd;
        
        /* 设置超时 */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        /* 等待事件 */
        int ret = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            opbatt_log("select 失败: %s", strerror(errno));
            break;
        }
        
        /* 处理连接请求 */
        if (FD_ISSET(ctx->server_fd, &read_fds)) {
            int client_fd = opbatt_network_accept(ctx);
            if (client_fd >= 0) {
                /* 处理客户端请求 */
                handle_client_request(ctx, client_fd);
                close(client_fd);
            }
        }
        
        /* 更新电池状态 */
        opbatt_battery_get_status(ctx, &ctx->battery);
        
        /* 模拟电池状态变化（用于测试） */
        opbatt_battery_simulate(ctx);
    }
    
    return OPBATT_SUCCESS;
}

/* 客户端主循环 */
static int client_main_loop(opbatt_context_t *ctx) {
    protocol_packet_t packet;
    int ret;
    
    opbatt_log("客户端主循环启动");
    
    /* 发送 Ping */
    packet.cmd = CMD_PING;
    packet.resp = 0;
    packet.length = 0;
    
    ret = opbatt_network_send(ctx, &packet, sizeof(packet.cmd) + 
                             sizeof(packet.resp) + sizeof(packet.length));
    if (ret < 0) {
        opbatt_log("发送 Ping 失败");
        return OPBATT_ERROR;
    }
    
    /* 接收响应 */
    ret = opbatt_network_recv(ctx, &packet, sizeof(packet));
    if (ret < 0) {
        opbatt_log("接收响应失败");
        return OPBATT_ERROR;
    }
    
    opbatt_log("收到响应: 0x%02x", packet.cmd);
    
    /* 请求电池状态 */
    packet.cmd = CMD_GET_STATUS;
    packet.resp = 0;
    packet.length = 0;
    
    ret = opbatt_network_send(ctx, &packet, sizeof(packet.cmd) + 
                             sizeof(packet.resp) + sizeof(packet.length));
    if (ret < 0) {
        opbatt_log("发送状态请求失败");
        return OPBATT_ERROR;
    }
    
    /* 接收状态 */
    ret = opbatt_network_recv(ctx, &packet, sizeof(packet));
    if (ret < 0) {
        opbatt_log("接收状态失败");
        return OPBATT_ERROR;
    }
    
    if (packet.length >= sizeof(battery_status_t)) {
        battery_status_t *status = (battery_status_t *)packet.data;
        opbatt_log("电池状态: 电压=%dmV, 电流=%dmA, 电量=%d%%",
                   status->voltage_mv, status->current_ma, status->level_percent);
    }
    
    return OPBATT_SUCCESS;
}

/* 主循环 */
int opbatt_main_loop(opbatt_context_t *ctx) {
    if (ctx->network.is_server) {
        return server_main_loop(ctx);
    } else {
        return client_main_loop(ctx);
    }
}
