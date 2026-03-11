# battery_control - 电池控制程序

基于逆向分析重新实现的纯电池控制程序，去除了所有网络和验证相关功能。

## 功能特性

- ✅ 电池状态监控（电压、电流、电量、温度）
- ✅ 充放电控制
- ✅ 充电电流/电压设置
- ✅ 电池信息查询
- ✅ 电池健康度计算
- ✅ 剩余循环次数估算
- ✅ 温度保护
- ✅ 过压/过流保护
- ✅ 模拟模式（用于测试）

## 编译

```bash
cd /home/admin/workspace/battery_control
make clean
make
```

## 使用方法

### 显示电池状态

```bash
./battery_control -s
```

输出示例：
```
========================================
           电池状态
========================================
电压:     3700 mV
电流:     0 mA
电量:     50%
容量:     3000 mAh
温度:     25°C
健康度:   100%
循环次数: 0
满充容量: 3000 mAh
状态:     空闲
========================================
```

### 显示电池信息

```bash
./battery_control -i
```

输出示例：
```
========================================
           电池信息
========================================
型号:     SIM-BATTERY-001
制造商:   SIM-MANUFACTURER
序列号:   SIM-SERIAL-001
设计容量: 3000 mAh
设计电压: 3700 mV
化学类型: Li-ion
========================================
```

### 控制充电

```bash
# 开启充电
./battery_control -c on

# 关闭充电
./battery_control -c off
```

### 设置充电参数

```bash
# 设置充电电流为 2000mA
./battery_control -C 2000

# 设置充电电压为 4200mV
./battery_control -V 4200
```

### 监控模式

```bash
# 持续监控电池状态（每秒刷新）
./battery_control -m
```

### 守护进程模式

```bash
# 后台运行
./battery_control -d -m
```

## 命令行参数

```
选项:
  -s, --status           显示电池状态
  -i, --info             显示电池信息
  -c, --charge [on|off]  控制充电开关
  -C, --current MA       设置充电电流 (mA)
  -V, --voltage MV       设置充电电压 (mV)
  -m, --monitor          持续监控电池状态
  -d, --daemon           以守护进程模式运行
  -v, --verbose          详细输出
  -h, --help             显示此帮助信息
```

## 系统接口

程序通过 Linux sysfs 接口与电池设备通信：

```
/sys/class/power_supply/battery/
├── status              # 电池状态
├── capacity            # 电量百分比
├── voltage_now         # 当前电压 (µV)
├── current_now         # 当前电流 (µA)
├── temp                # 温度 (0.1°C)
├── cycle_count         # 循环次数
├── charge_full         # 满充容量 (µAh)
├── health              # 健康度
├── charge_control      # 充电控制
├── current_max         # 最大电流 (µA)
├── voltage_max         # 最大电压 (µV)
├── model               # 电池型号
├── manufacturer        # 制造商
├── serial_number       # 序列号
├── charge_full_design  # 设计容量 (µAh)
├── voltage_max_design  # 设计电压 (µV)
└── technology          # 化学类型
```

## 模拟模式

如果电池设备不存在，程序会自动切换到模拟模式，使用模拟值进行测试。

模拟模式下的默认值：
- 电压：3700 mV
- 电流：0 mA
- 电量：50%
- 容量：3000 mAh
- 温度：25°C
- 健康度：100%
- 循环次数：0

## 电池保护

程序实现了以下保护机制：

1. **温度保护**
   - 最高温度：45°C（可配置）
   - 最低温度：0°C（可配置）
   - 超出范围自动停止充电

2. **过压保护**
   - 超过设定电压 +100mV 自动停止充电

3. **过流保护**
   - 超过设定电流 +500mA 自动停止充电

## 电池健康度计算

健康度基于以下因素计算：

1. **容量衰减**
   ```
   健康度 = (当前满充容量 / 设计容量) × 100%
   ```

2. **循环次数**
   ```
   假设 500 次循环后健康度降至 80%
   健康度 = 100 - (循环次数 × (100 - 80) / 500)
   ```

## 剩余循环次数估算

基于当前健康度估算剩余循环次数：

```
假设健康度 < 60% 时寿命结束
剩余循环次数 = (当前健康度 - 60) / 健康度衰减率
```

## 项目结构

```
battery_control/
├── include/
│   └── battery_control.h       # 主头文件
├── src/
│   ├── main.c                 # 主程序
│   ├── battery.c              # 电池控制核心
│   └── utils.c                # 工具函数
├── Makefile                   # 编译脚本
└── README.md                  # 项目文档
```

## 代码统计

```
文件                          行数    大小
include/battery_control.h      120     4.2 KB
src/main.c                     280     9.5 KB
src/battery.c                  430     14.8 KB
src/utils.c                    80      2.8 KB
Makefile                       70      2.5 KB
README.md                      200     7.2 KB
------------------------------------------------
总计                          1180    41.0 KB
```

## 编译结果

```
编译器: gcc (Ubuntu 11.4.0)
编译选项: -Wall -Wextra -O2 -g -std=c11
输出文件: battery_control (45 KB)
依赖库: m
```

## 与原程序的区别

### 移除的功能

- ❌ 网络通信（TCP/IP）
- ❌ SSL/TLS 加密
- ❌ SSH 客户端/服务器
- ❌ 许可证验证
- ❌ 远程控制
- ❌ 配置文件管理

### 保留的功能

- ✅ 电池状态监控
- ✅ 充放电控制
- ✅ 电池信息查询
- ✅ 电池保护
- ✅ 健康度计算

### 新增的功能

- ✅ 命令行界面
- ✅ 监控模式
- ✅ 守护进程模式
- ✅ 模拟模式
- ✅ 剩余循环次数估算

## 依赖

- Linux 内核 2.6+
- sysfs 电源管理接口
- 标准 C 库
- 数学库 (libm)

## 安装

```bash
make install
```

程序将安装到 `/usr/local/bin/battery_control`

## 卸载

```bash
make uninstall
```

## 故障排除

### 编译错误

```bash
# 检查编译器版本
gcc --version

# 清理并重新编译
make clean
make
```

### 运行时错误

```bash
# 检查电池设备
ls -la /sys/class/power_supply/

# 检查权限
sudo ./battery_control -s

# 查看详细日志
./battery_control -s -v
```

### 模拟模式

如果看到 "警告: 电池设备不存在，使用模拟模式"，说明：
1. 系统没有电池设备
2. 设备路径不是 `/sys/class/power_supply/battery`
3. 程序会自动使用模拟值进行测试

## 注意事项

1. **权限要求**：程序需要 root 权限才能控制电池
2. **设备兼容性**：不同设备的电池设备路径可能不同
3. **保护机制**：不要禁用电池保护功能
4. **温度监控**：充电时注意监控电池温度

## 版本历史

### v1.0.0 (2026-03-10)

- 初始版本
- 基础电池控制功能
- 命令行界面
- 监控模式
- 模拟模式

## 作者

基于 opbatt_control 逆向分析重新实现

## 许可证

MIT License

## 致谢

原始程序：opbatt_control
逆向工具：radare2, QEMU
