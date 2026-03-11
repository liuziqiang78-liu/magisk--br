# opbatt_control - 电池控制程序（重建版）

## 项目简介

这是基于原始 `opbatt_control` 二进制文件逆向分析后重新实现的电池控制程序。本版本移除了所有网络验证和许可证验证功能，仅保留核心的电池充电和控制功能。

**版本**: v1.0.0  
**目标平台**: ARM64 (aarch64-linux-gnu)  
**目标设备**: OnePlus 13 (PJZ110) 及其他支持的设备

## 主要特性

### 核心功能

- ✅ **电池状态监控**: 实时监控电池电压、电流、温度、电量等参数
- ✅ **充电控制**: 支持充放电控制、电流电压设置
- ✅ **UFCS 快充协议**: 支持 UFCS（Unified Fast Charging Specification）协议
- ✅ **PPS 快充协议**: 支持 PPS（Programmable Power Supply）协议
- ✅ **SVOOC 快充协议**: 支持 SVOOC（Super VOOC）协议
- ✅ **温度控制**: 支持温度监控和温度伪装功能
- ✅ **恒压控制**: 支持恒压充电控制（CV 模式）
- ✅ **DDRC**: 支持深度放电恢复（Deep Discharge Recovery）
- ✅ **SOC 平滑**: 支持 SOC 显示平滑处理
- ✅ **配置管理**: 支持配置文件读取和保存
- ✅ **日志系统**: 支持日志记录和日志轮转
- ✅ **守护进程**: 支持后台守护进程模式

### 移除的功能

- ❌ 网络通信功能
- ❌ SSL/TLS 加密
- ❌ 许可证验证
- ❌ 在线验证
- ❌ 服务器/客户端模式

## 项目结构

```
opbatt_control_rebuild/
├── include/               # 头文件目录
│   ├── sysfs_paths.h     # sysfs 文件路径定义
│   ├── sysfs.h           # sysfs 接口层
│   ├── config.h          # 配置文件管理
│   ├── logger.h          # 日志系统
│   ├── battery.h         # 电池状态监控
│   ├── charge.h          # 充电控制
│   ├── ufcs.h            # UFCS 协议控制
│   ├── pps.h             # PPS 协议控制
│   ├── svooc.h           # SVOOC 协议控制
│   ├── temp.h            # 温度控制
│   ├── cv.h              # 恒压控制
│   ├── ddrc.h            # DDRC 模块
│   ├── soc.h             # SOC 平滑
│   └── daemon.h          # 守护进程
├── src/                  # 源文件目录
│   ├── sysfs.c
│   ├── logger.c
│   ├── config.c
│   ├── battery.c
│   ├── charge.c
│   ├── ufcs.c
│   ├── pps.c
│   ├── svooc.c
│   ├── temp.c
│   ├── cv.c
│   ├── ddrc.c
│   ├── soc.c
│   ├── daemon.c
│   └── main.c
├── Makefile              # 编译脚本
└── README.md             # 本文件
```

## 编译方法

### 本地编译

```bash
make
```

这将生成 `opbatt_control` 可执行文件。

### ARM64 交叉编译

```bash
make arm64
```

这将生成 `opbatt_control_arm64` 可执行文件，适用于 ARM64 设备。

**注意**: 交叉编译需要安装 `aarch64-linux-gnu-gcc` 工具链。

### 安装工具链（Ubuntu/Debian）

```bash
sudo apt-get install gcc-aarch64-linux-gnu
```

### 清理

```bash
make clean
```

## 使用方法

### 命令行参数

```bash
./opbatt_control [OPTIONS]
```

**选项**:

- `-d, --daemon`: 以守护进程模式运行
- `-f, --foreground`: 在前台运行（默认）
- `-c, --config`: 指定配置文件路径（默认: `/data/opbatt/batt_control`）
- `-l, --log`: 指定日志文件路径（默认: `/data/opbatt/battchg.log`）
- `-v, --version`: 打印版本信息
- `-h, --help`: 打印帮助信息

### 示例

**前台运行**:
```bash
./opbatt_control
```

**守护进程模式**:
```bash
./opbatt_control --daemon
```

**指定配置文件**:
```bash
./opbatt_control --config /path/to/config
```

## 配置文件

配置文件路径: `/data/opbatt/batt_control`

### 配置参数

| 参数名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| enabled | 整数 | 1 | 恒压开关（0=关闭, 1=开启） |
| inc_step | 整数 | 100 | 电流上升步长（mA） |
| dec_step | 整数 | 100 | 电流下降步长（mA） |
| ufcs_max | 整数 | 9100 | UFCS最大电流（mA） |
| pps_max | 整数 | 5000 | PPS最大电流（mA） |
| loop_interval_ms | 整数 | 2000 | 主循环间隔（ms） |
| batt_con_soc | 整数 | 94 | 开启TC模式的SOC（%） |
| cv_vol_mv | 整数 | 4565 | 恒压电压（mV） |
| cv_max_ma | 整数 | 5000 | 恒压最大电流（mA） |
| ... | ... | ... | 更多参数请参考源代码 |

### 配置文件示例

```ini
# opbatt_control configuration
enabled=1
inc_step=100
dec_step=100
ufcs_max=9100
pps_max=5000
loop_interval_ms=2000
batt_con_soc=94
cv_vol_mv=4565
cv_max_ma=5000
...
```

## 日志文件

日志文件路径: `/data/opbatt/battchg.log`

日志文件最大大小: 5MB（超过后自动备份）

### 日志级别

- DEBUG: 调试信息
- INFO: 一般信息
- WARN: 警告信息
- ERROR: 错误信息

## 系统要求

### 硬件要求

- ARM64 架构设备
- 支持的设备型号: OnePlus 13 (PJZ110) 及其他兼容设备

### 软件要求

- Android 系统
- Root 权限
- KernelSU 或 Magisk 模块支持

### sysfs 节点

程序需要访问以下 sysfs 节点：

- `/sys/class/power_supply/battery/*`
- `/sys/class/oplus_chg/battery/*`
- `/sys/class/oplus_chg/common/*`

## 技术架构

### 模块化设计

程序采用模块化设计，各模块职责清晰：

1. **sysfs 接口层**: 统一的 sysfs 文件访问接口
2. **电池监控模块**: 电池状态读取和监控
3. **充电控制模块**: 充放电控制
4. **快充协议模块**: UFCS/PPS/SVOOC 协议控制
5. **温度控制模块**: 温度监控和温度伪装
6. **恒压控制模块**: 恒压充电控制
7. **DDRC 模块**: 深度放电恢复
8. **SOC 平滑模块**: SOC 显示平滑处理
9. **配置管理模块**: 配置文件管理
10. **日志系统**: 日志记录和管理
11. **守护进程**: 后台运行支持

### 主控制循环

主控制循环按照以下流程运行：

1. 读取电池状态
2. 获取有效温度（考虑温度伪装）
3. 检查温度是否过高
4. 检查是否在充电
5. 恒压控制
6. 温度控制
7. UFCS/PPS/SVOOC 控制
8. DDRC 控制
9. 应用充电电流
10. SOC 平滑
11. 记录日志
12. 等待下一个循环

## 安全性

### 温度保护

程序内置了温度保护机制：

- 温度超过 53°C 时停止充电
- 温度超过 48°C 时降低充电功率
- 温度超过 42°C 时进入高温充电模式

### 电压保护

程序内置了电压保护机制：

- 电压超过阈值时自动降低充电电流
- 恒压模式下自动控制充电电压

### 电流保护

程序内置了电流保护机制：

- 最大充电电流限制（UFCS: 9100mA, PPS: 5000mA）
- 根据温度和 SOC 动态调整充电电流

## 风险提示

⚠️ **使用本程序可能导致以下风险**:

- 设备保修失效
- 电池损坏
- 设备无法启动
- 数据丢失

⚠️ **注意事项**:

- 需要root权限
- 仅支持指定机型
- DTBO修改有风险
- 不要禁用电池保护功能
- 使用前请备份重要数据

## 故障排除

### 程序无法启动

1. 检查是否有 root 权限
2. 检查 sysfs 节点是否存在
3. 检查日志文件获取详细错误信息

### 充电控制不生效

1. 检查配置文件是否正确
2. 检查充电器是否连接
3. 检查 sysfs 节点是否可写
4. 检查日志文件获取详细错误信息

### 温度过高

1. 检查温度伪装是否启用
2. 检查充电电流设置是否过高
3. 检查设备散热是否良好

## 开发者信息

**原始程序**: opbatt_control (4.5MB)  
**重建版本**: opbatt_control v1.0.0  
**重建方法**: 基于逆向分析和代码重建  
**代码语言**: C11  
**编译器**: aarch64-linux-gnu-gcc

## 许可证

本项目仅供学习和研究使用。使用本程序产生的任何后果由使用者自行承担。

## 致谢

感谢原始 opbatt_control 程序的开发者。  
感谢 OnePlus 提供的优秀设备。  
感谢 Android 开源社区的支持。

## 更新日志

### v1.0.0 (2026-03-11)

- ✅ 完成所有核心功能模块的实现
- ✅ 移除所有网络验证和许可证验证功能
- ✅ 实现完整的电池监控和充电控制功能
- ✅ 支持 UFCS/PPS/SVOOC 快充协议
- ✅ 实现温度控制和温度伪装功能
- ✅ 实现恒压控制和 DDRC 功能
- ✅ 实现配置管理和日志系统
- ✅ 支持守护进程模式

## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址: [GitHub](https://github.com/yourusername/opbatt_control_rebuild)
- 问题反馈: [Issues](https://github.com/yourusername/opbatt_control_rebuild/issues)

---

**免责声明**: 本程序仅供学习和研究使用，使用本程序产生的任何后果由使用者自行承担。作者不对因使用本程序而导致的任何损失负责。
