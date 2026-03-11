# 0aO电池工具包（重建版）- Magisk 模块

## 模块状态

⚠️ **当前状态**: 模块结构已准备完成，但缺少 ARM64 版本的可执行文件

## 模块信息

- **模块 ID**: fopbatt_rebuild
- **模块名称**: 0aO电池工具包（重建版）
- **版本**: Rebuild@1.0.0
- **作者**: 搭叩AI
- **描述**: 基于逆向分析重建的电池控制程序，移除所有网络验证和许可证验证功能

## 当前问题

当前 `bin/opbatt_control` 是 x86-64 版本（本地编译），不能在 Android 设备上运行。需要 ARM64 版本的可执行文件。

## 解决方案

### 方案 1: 交叉编译（推荐）

在有 ARM64 交叉编译环境的机器上执行：

```bash
cd /path/to/opbatt_control_rebuild
make arm64
```

这将生成 `opbatt_control_arm64` 可执行文件。

然后复制到模块目录：

```bash
cp opbatt_control_arm64 /path/to/magisk_module_rebuild/bin/opbatt_control
```

### 方案 2: 使用 Docker

使用 Docker 容器进行交叉编译：

```bash
docker run --rm -v $(pwd):/workspace -w /workspace/opbatt_control_rebuild \
  aarch64-linux-gnu-gcc make arm64
```

### 方案 3: 在 Android 设备上编译

如果设备上有编译环境，可以直接在设备上编译：

```bash
cd /data/opbatt/opbatt_control_rebuild
make
```

## 模块结构

```
magisk_module_rebuild/
├── bin/
│   └── opbatt_control          # 需要替换为 ARM64 版本
├── webroot/                    # WebUI 文件（可选）
├── META-INF/
│   └── com/google/android/
│       ├── update-binary       # Magisk 安装脚本
│       └── updater-script      # Magisk 更新脚本
├── customize.sh                # 模块自定义安装脚本
├── service.sh                  # 模块服务脚本
├── module.prop                 # 模块属性文件
└── README.md                   # 本文件
```

## 安装步骤

1. **准备 ARM64 版本的可执行文件**
   - 使用上述任一方案编译 ARM64 版本
   - 替换 `bin/opbatt_control` 为 ARM64 版本

2. **打包模块**
   ```bash
   cd /path/to/magisk_module_rebuild
   zip -r magisk_module_rebuild.zip .
   ```

3. **安装模块**
   - 将 `magisk_module_rebuild.zip` 传输到 Android 设备
   - 使用 Magisk Manager 安装模块
   - 重启设备

## 配置文件

模块安装后会创建默认配置文件：`/data/opbatt/batt_control`

配置参数说明：

```ini
# opbatt_control configuration
enabled=1                    # 恒压开关 (0=关闭, 1=开启)
inc_step=100                 # 电流上升步长 (mA)
dec_step=100                 # 电流下降步长 (mA)
ufcs_max=9100                # UFCS最大电流 (mA)
pps_max=5000                 # PPS最大电流 (mA)
loop_interval_ms=2000        # 主循环间隔 (ms)
batt_con_soc=94              # 开启TC模式的SOC (%)
cv_vol_mv=4565               # 恒压电压 (mV)
cv_max_ma=5000               # 恒压最大电流 (mA)
```

## 日志文件

- **日志路径**: `/data/opbatt/battchg.log`
- **最大大小**: 5MB（超过后自动备份）

## 支持的设备

- OnePlus 13 (PJZ110)
- 其他兼容设备（需要测试）

## 功能特性

### 已实现的功能

✅ 电池状态监控（电压、电流、温度、电量等）
✅ 充电控制（充放电、电流电压设置）
✅ UFCS 快充协议
✅ PPS 快充协议
✅ SVOOC 快充协议
✅ 温度控制和温度伪装
✅ 恒压控制（CV 模式）
✅ DDRC 深度放电恢复
✅ SOC 平滑
✅ 配置文件管理
✅ 日志系统（日志轮转）
✅ 守护进程模式

### 已移除的功能

❌ 网络通信
❌ SSL/TLS 加密
❌ 许可证验证
❌ 在线验证
❌ 服务器/客户端模式

## 注意事项

⚠️ **使用本模块可能导致以下风险**:

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

### 模块无法安装

1. 检查 Magisk 版本（需要 v20.4+）
2. 检查设备架构（需要 arm64-v8a）
3. 检查可执行文件是否为 ARM64 版本

### 程序无法启动

1. 检查是否有 root 权限
2. 检查 sysfs 节点是否存在
3. 检查日志文件获取详细错误信息

### 充电控制不生效

1. 检查配置文件是否正确
2. 检查充电器是否连接
3. 检查 sysfs 节点是否可写
4. 检查日志文件获取详细错误信息

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
- ⚠️ 等待 ARM64 交叉编译

## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址: [GitHub](https://github.com/yourusername/opbatt_control_rebuild)
- 问题反馈: [Issues](https://github.com/yourusername/opbatt_control_rebuild/issues)

---

**免责声明**: 本程序仅供学习和研究使用，使用本程序产生的任何后果由使用者自行承担。作者不对因使用本程序而导致的任何损失负责。
