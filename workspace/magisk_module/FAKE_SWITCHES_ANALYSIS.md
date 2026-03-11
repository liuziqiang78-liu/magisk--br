# Web 前端假开关分析报告

## 分析时间
2026-03-10 03:12:28

## 分析方法
对比 Web 前端定义的配置参数和 opbatt_control 脚本实现的功能

## Web 前端定义的所有配置参数

### 1. 开关类型（2 个）
| 参数名 | 说明 | 后端实现 |
|--------|------|----------|
| `enabled` | 模拟恒压开关，关闭后使用官方充电曲线 | ❌ **假开关** |
| `cable_override` | 忽略线材电流检测，注意线材承载能力 | ❌ **假开关** |

### 2. 滑块类型（18 个）
| 参数名 | 说明 | 范围 | 后端实现 |
|--------|------|------|----------|
| `ufcs_max` | UFCS最大充电电流 (mA) | 3000-13700 | ❌ **假开关** |
| `pps_max` | PPS最大充电电流 (mA) | 3000-6500 | ❌ **假开关** |
| `max_ufcs_chg_reset_cc` | UFCS降功率时重置充电器的最大次数 | 0-4 | ❌ **假开关** |
| `ufcs_reset_delay` | 重置UFCS充电头所需要的最短时间 (秒) | 0-300 | ❌ **假开关** |
| `curr_inc_wait_cycles` | 电压撞墙后的升流间隔圈数 | 4-8 | ❌ **假开关** |
| `batt_con_soc` | 开启TC模式充电的电量百分比 (%) | 90-97 | ❌ **假开关** |
| `tc_full_ma` | 充满电流阈值 (mA) | 200-800 | ❌ **假开关** |
| `inc_step` | 40-70%电流上升步长 (mA) | 50-200 | ❌ **假开关** |
| `dec_step` | 40-70%电流下降步长 (mA) | 50-200 | ❌ **假开关** |
| `loop_interval_ms` | 内部线程主循环时间 (ms) | 1500-2500 | ❌ **假开关** |
| `rise_quickstep_thr_mv` | 底层数据值，请勿修改 (mV) | 4000-4400 | ❌ **假开关** |
| `rise_wait_thr_mv` | 底层数据值，请勿修改 (mV) | 3800-4000 | ❌ **假开关** |
| `cv_vol_mv` | 底层数据值，请勿修改 (mV) | 4500-4600 | ❌ **假开关** |
| `cv_max_ma` | 底层数据值，请勿修改 (mA) | 2000-8000 | ❌ **假开关** |
| `tc_vol_thr_mv` | 底层数据值，请勿修改 (mV) | 4500-4600 | ❌ **假开关** |
| `tc_thr_soc` | 底层数据值，请勿修改 (%) | 80-99 | ❌ **假开关** |
| `tc_vol_full_mv` | 底层数据值，请勿修改 (mV) | 4200-4500 | ❌ **假开关** |
| `batt_full_thr_mv` | 底层数据值，请勿修改 (mV) | 4500-4600 | ❌ **假开关** |

### 3. 文本输入类型（8 个）
| 参数名 | 说明 | 后端实现 |
|--------|------|----------|
| `temp_range` | 对应过热温度 | ❌ **假开关** |
| `temp_curr_offset` | 温度超过阈值时对应减速量 (mA) | ❌ **假开关** |
| `batt_vol_thr` | 恒压TC模式电压阈值 (mV) | ❌ **假开关** |
| `batt_vol_soc` | 恒压TC模式电量阈值 (%) | ❌ **假开关** |
| `ufcs_soc_mon` | UFCS电流采样控制的电量监控点 (%) | ❌ **假开关** |
| `ufcs_interval_ms` | UFCS电流采样控制的间隔时间 (ms) | ❌ **假开关** |
| `pps_soc_mon` | PPS电流采样控制的电量监控点 (%) | ❌ **假开关** |
| `pps_interval_ms` | PPS电流采样控制的间隔时间 (ms) | ❌ **假开关** |

## opbatt_control 脚本实现的功能

### 已实现的功能（4 个）
1. `charging_enabled` - 充电开关（通过 sysfs 控制）
2. `PPS` - PPS 协议开关（仅保存到配置文件，未实际控制）
3. `UFCS` - UFCS 协议开关（仅保存到配置文件，未实际控制）
4. `SVOOC` - SVOOC 协议开关（仅保存到配置文件，未实际控制）

### 未实现的功能（28 个）
所有 Web 前端定义的 28 个配置参数都没有在后端实现。

## 假开关统计

| 类别 | 总数 | 假开关数 | 假开关比例 |
|------|------|----------|------------|
| 开关类型 | 2 | 2 | 100% |
| 滑块类型 | 18 | 18 | 100% |
| 文本输入类型 | 8 | 8 | 100% |
| **总计** | **28** | **28** | **100%** |

## 问题分析

### 为什么这些开关是假的？

1. **Web 前端逻辑**：
   - Web 前端读取 `/data/opbatt/batt_control` 文件
   - Web 前端将配置写入 `/data/opbatt/batt_control` 文件
   - Web 前端显示"保存成功！拔下充电器2秒后再插上生效"

2. **后端逻辑**：
   - opbatt_control 脚本完全不读取 `/data/opbatt/batt_control` 文件
   - opbatt_control 脚本使用硬编码的默认值：
     * `DEFAULT_CURRENT_MA=2000`
     * `DEFAULT_VOLTAGE_MV=4200`
     * `MAX_TEMP_C=45`
     * `MIN_TEMP_C=5`
   - opbatt_control 脚本没有实现任何高级充电算法

3. **结果**：
   - 用户在 Web 前端修改配置
   - 配置被保存到 `/data/opbatt/batt_control` 文件
   - 但 opbatt_control 脚本从不读取这个文件
   - 配置完全无效

## 原始程序的情况

原始 opbatt_control 二进制文件（4.4 MB）可能实现了这些功能，但：
1. 二进制文件被 stripped，无法直接分析
2. 这些功能可能需要特定的内核驱动支持
3. 这些功能可能需要特定的硬件支持

## 解决方案

### 方案 1：实现所有配置参数（推荐）

在 opbatt_control 脚本中实现所有 28 个配置参数：

```bash
# 读取配置文件
read_config() {
    local config_file="/data/opbatt/batt_control"
    if [ -f "$config_file" ]; then
        source "$config_file"
    fi
}

# 应用配置
apply_config() {
    # 应用 enabled
    if [ "$enabled" = "1" ]; then
        # 启用恒压模式
    else
        # 使用官方充电曲线
    fi
    
    # 应用 ufcs_max
    if [ -n "$ufcs_max" ]; then
        write_file "$CHG_PATH/ufcs_max_current" "$ufcs_max"
    fi
    
    # 应用 pps_max
    if [ -n "$pps_max" ]; then
        write_file "$CHG_PATH/pps_max_current" "$pps_max"
    fi
    
    # ... 应用其他参数
}
```

### 方案 2：移除假开关（不推荐）

从 Web 前端移除所有未实现的配置参数，只保留已实现的功能。

### 方案 3：实现核心功能（折中方案）

实现最重要的几个参数：
- `enabled` - 模拟恒压开关
- `ufcs_max` - UFCS最大充电电流
- `pps_max` - PPS最大充电电流
- `temp_range` - 过热温度
- `temp_curr_offset` - 温度补偿

## 建议

**推荐使用方案 1**，实现所有 28 个配置参数，因为：
1. 用户期望这些功能能够工作
2. 这些功能对于电池管理很重要
3. 实现这些功能可以提升用户体验

## 下一步行动

1. 修改 opbatt_control 脚本，添加配置文件读取功能
2. 实现所有 28 个配置参数的应用逻辑
3. 测试每个参数是否正常工作
4. 生成新的 Magisk 模块

## 附录：配置文件路径

- 配置文件：`/data/opbatt/batt_control`
- 协议配置：`/data/opbatt/chg_config`
- 日志文件：`/data/opbatt/battchg.log`
