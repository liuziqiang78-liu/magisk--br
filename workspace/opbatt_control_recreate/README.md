# opbatt_control - 电池控制程序

基于逆向分析重新实现的电池控制程序。

## 功能特性

- ✅ 电池状态监控（电压、电流、电量、温度）
- ✅ 充放电控制
- ✅ 网络通信（TCP/IP）
- ✅ SSL/TLS 加密通信（可选）
- ✅ 许可证验证机制
- ✅ 服务器/客户端模式
- ✅ 守护进程模式
- ✅ 命令行参数支持

## 编译

### 基础编译（不使用 OpenSSL）

```bash
cd /home/admin/workspace/opbatt_control_recreate
make clean
make
```

### 使用 OpenSSL 编译

```bash
cd /home/admin/workspace/opbatt_control_recreate
make clean
make CFLAGS="-DUSE_OPENSSL" LDFLAGS="-lssl -lcrypto"
```

## 使用方法

### 服务器模式

```bash
# 基础模式
./opbatt_control -s -p 8888

# 详细输出
./opbatt_control -s -p 8888 -v

# 守护进程模式
./opbatt_control -s -p 8888 -d
```

### 客户端模式

```bash
# 连接到服务器
./opbatt_control -c 192.168.1.100 -p 8888

# 详细输出
./opbatt_control -c 192.168.1.100 -p 8888 -v
```

### 命令行参数

```
选项:
  -s, --server        以服务器模式运行
  -c, --client HOST   以客户端模式连接到 HOST
  -p, --port PORT     指定端口 (默认: 8888)
  -l, --license PATH  指定许可证路径
  -d, --daemon        以守护进程模式运行
  -v, --verbose       详细输出
  -h, --help          显示帮助信息
```

## 协议说明

### 数据包格式

```c
typedef struct {
    uint8_t  cmd;      // 命令码
    uint8_t  resp;     // 响应码
    uint16_t length;   // 数据长度
    uint8_t  data[];   // 数据
} protocol_packet_t;
```

### 命令码

| 命令码 | 名称 | 说明 |
|--------|------|------|
| 0x01 | CMD_GET_STATUS | 获取电池状态 |
| 0x02 | CMD_SET_CHARGE | 设置充电状态 |
| 0x03 | CMD_SET_CURRENT | 设置充电电流 |
| 0x04 | CMD_GET_INFO | 获取程序信息 |
| 0x05 | CMD_PING | Ping 测试 |
| 0x06 | CMD_PONG | Pong 响应 |

### 响应码

| 响应码 | 名称 | 说明 |
|--------|------|------|
| 0x00 | RESP_SUCCESS | 成功 |
| 0x01 | RESP_ERROR | 错误 |
| 0x02 | RESP_INVALID_CMD | 无效命令 |
| 0x03 | RESP_LICENSE_ERR | 许可证错误 |

## 许可证

### 生成演示许可证

```c
#include "src/license.c"

int main() {
    opbatt_license_generate_demo("/tmp/license.dat");
    return 0;
}
```

### 许可证文件位置

默认路径：`/root/opbattlic/license.dat`

可以通过 `-l` 参数指定自定义路径。

## 配置文件

### 电池设备路径

默认路径：`/sys/class/power_supply/battery`

如果设备不存在，程序会自动切换到模拟模式。

### SSL/TLS 证书

证书路径：`/root/opbattlic/openssl/../openssl-out/arm64-v8a/ssl/cert.pem`
私钥路径：`/root/opbattlic/openssl/../openssl-out/arm64-v8a/ssl/private/key.pem`

## 开发

### 项目结构

```
opbatt_control_recreate/
├── include/           # 头文件
│   └── opbatt_control.h
├── src/              # 源文件
│   ├── main.c
│   ├── battery.c
│   ├── network.c
│   ├── license.c
│   ├── utils.c
│   └── main_loop.c
├── config/           # 配置文件
├── ssl/              # SSL 证书
└── Makefile
```

### 添加新功能

1. 在 `include/opbatt_control.h` 中添加函数声明
2. 在对应的 `.c` 文件中实现函数
3. 在 `Makefile` 中添加源文件（如果需要）

## 测试

### 单元测试

```bash
# 编译程序
make

# 运行服务器
./opbatt_control -s -p 8888 -v
```

### 网络测试

使用 telnet 或 nc 测试：

```bash
# 连接到服务器
telnet localhost 8888

# 发送命令（需要实现协议解析）
```

## 注意事项

1. **权限要求**：程序需要 root 权限才能访问电池设备
2. **设备兼容性**：不同设备的电池设备路径可能不同
3. **OpenSSL 依赖**：如果使用 SSL/TLS，需要安装 OpenSSL 开发库
4. **许可证验证**：演示模式下使用内置的演示许可证

## 故障排除

### 编译错误

```bash
# 检查编译器版本
gcc --version

# 检查 OpenSSL（如果使用）
pkg-config --modversion openssl
```

### 运行时错误

```bash
# 检查权限
sudo ./opbatt_control -s -p 8888

# 检查设备
ls -la /sys/class/power_supply/

# 查看日志
./opbatt_control -s -p 8888 -v
```

## 版本历史

### v1.0.0 (2026-03-10)

- 初始版本
- 基础电池控制功能
- 网络通信支持
- 许可证验证机制

## 作者

基于逆向分析重新实现

## 许可证

MIT License

## 致谢

原始程序：opbatt_control
逆向工具：radare2, QEMU
