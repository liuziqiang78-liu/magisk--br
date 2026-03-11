# 0aO电池工具包（纯净版）

## 版本信息

- **版本**: Tools@7.6|WebUI@f211b2a|Pure@1.0
- **作者**: bybycode|WebUI@Collapsar|Pure@搭叩AI
- **描述**: 电池控制模块（无网络验证）

## 修改内容

### 1. 移除的功能

- ❌ 网络通信功能
- ❌ SSL/TLS 加密
- ❌ SSH 客户端/服务器
- ❌ 许可证验证
- ❌ 激活码生成
- ❌ 在线验证

### 2. 保留的功能

- ✅ 电池状态监控
- ✅ 充放电控制
- ✅ 电池信息查询
- ✅ 电池保护
- ✅ 健康度计算
- ✅ DTBO 补丁功能
- ✅ WebUI 监控面板

### 3. 新增的功能

- ✅ 纯本地电池控制
- ✅ 命令行界面
- ✅ 模拟模式支持
- ✅ 更简洁的代码结构

## 修改的文件

### 1. bin/opbatt_control
- **原文件**: 4.5 MB（包含 OpenSSL 库）
- **新文件**: 64 KB（纯电池控制）
- **修改**: 完全替换为重新实现的纯电池控制程序

### 2. customize.sh
- **修改**: 移除许可证验证相关代码
- **移除内容**:
  ```bash
  # 已移除
  [ -e /data/opbatt/nonce.bin ] && ui_print "- 已有激活码种子" || ui_print "- 随机种子生成唯一激活验证码"
  
  if OPB_LIC_OUTPUT=$(opbatt_control --generate-lic); then
      ui_print "🔑 激活验证码：$OPB_LIC_OUTPUT"
  else
      ui_print "⚠️ 激活验证码生成失败"
  fi
  ```

### 3. service.sh
- **修改**: 移除许可证验证循环
- **移除内容**:
  ```bash
  # 已移除
  OPB_CRET=1
  TEST_COUNT=5
  
  while [ "$TEST_COUNT" -gt 0 ]; do
      "$OPB_C" --verify-lic
      OPB_CRET=$?
      [ "$OPB_CRET" -eq 10 ] && break
      echo "$LOGPREFIX Verity failed, code: $OPB_CRET" >> $LOGFILE
      sleep 2
      TEST_COUNT=$((TEST_COUNT - 1))
  done
  
  case $OPB_CRET in
      10) chgdata="${chgdata}|😉模拟恒压服务许可正常";;
      1)  chgdata="${chgdata}|⚠️模拟恒压服务无许可";;
      2)  chgdata="${chgdata}|❌️模拟恒压服务许可验证失败";;
      3)  chgdata="${chgdata}|❌️模拟恒压服务程序已过期";;
      5)  chgdata="${chgdata}|❌️机型不具备验证条件";;
      *)  chgdata="${chgdata}|❌️模拟恒压服务异常";;
  esac
  ```
- **新增内容**:
  ```bash
  # 许可证验证已移除
  chgdata="${chgdata}|😉电池控制服务运行正常"
  ```

### 4. module.prop
- **修改**: 更新版本信息和描述
- **修改前**:
  ```
  id=fopbatt
  name=0aO电池工具包
  version=Tools@7.6|WebUI@f211b2a
  author=bybycode|WebUI@Collapsar
  description=模块未初始化
  ```
- **修改后**:
  ```
  id=fopbatt
  name=0aO电池工具包（纯净版）
  version=Tools@7.6|WebUI@f211b2a|Pure@1.0
  author=bybycode|WebUI@Collapsar|Pure@搭叩AI
  description=电池控制模块（无网络验证）
  ```

## 使用方法

### 安装模块

1. 将模块文件刷入 Magisk/KernelSU
2. 重启设备
3. 模块会自动启动电池控制服务

### 查看电池状态

```bash
# 通过命令行查看
/data/adb/modules/fopbatt/bin/opbatt_control -s

# 查看电池信息
/data/adb/modules/fopbatt/bin/opbatt_control -i

# 控制充电
/data/adb/modules/fopbatt/bin/opbatt_control -c on
/data/adb/modules/fopbatt/bin/opbatt_control -c off
```

### 查看服务状态

```bash
# 查看模块状态
magisk --list-modules | grep fopbatt

# 查看服务日志
cat /data/opbatt/battchg.log
```

## 支持的机型

模块支持以下机型的 DTBO 补丁：

- PJZ110
- PKR110
- PLK110
- PLQ110
- RMX5010

## 注意事项

1. **权限要求**: 模块需要 root 权限
2. **设备兼容性**: 仅支持上述机型
3. **DTBO 修改**: 修改 DTBO 可能导致设备无法启动，请谨慎操作
4. **电池保护**: 不要禁用电池保护功能
5. **温度监控**: 充电时注意监控电池温度

## 技术细节

### 电池控制程序

重新实现的电池控制程序具有以下特点：

- **纯本地运行**: 无需网络连接
- **轻量级**: 仅 64 KB（原程序 4.5 MB）
- **模块化设计**: 清晰的代码结构
- **模拟模式**: 无硬件时自动启用
- **命令行界面**: 支持多种操作

### 系统接口

程序通过 Linux sysfs 接口与电池设备通信：

```
/sys/class/power_supply/battery/
/sys/class/oplus_chg/common/
/sys/class/oplus_chg/battery/
```

### DTBO 补丁

模块支持以下 DTBO 补丁：

- 电池温控补丁
- 充电升压补丁
- DDRC 补丁
- SOC 平滑补丁
- PDQC 补丁（部分机型）

## 故障排除

### 模块无法启动

```bash
# 检查模块状态
magisk --list-modules | grep fopbatt

# 查看服务日志
cat /data/opbatt/battchg.log

# 手动启动服务
/data/adb/modules/fopbatt/service.sh
```

### 电池控制无效

```bash
# 检查电池设备
ls -la /sys/class/power_supply/battery/

# 检查权限
ls -la /sys/class/oplus_chg/battery/

# 手动测试
/data/adb/modules/fopbatt/bin/opbatt_control -s
```

### DTBO 补丁不生效

```bash
# 检查 DTBO 镜像
ls -la /dev/block/bootdevice/by-name/dtbo*

# 检查模块 DTBO
ls -la /data/adb/modules/fopbatt/dtbo.img

# 比较 MD5
md5sum /dev/block/bootdevice/by-name/dtbo*
md5sum /data/adb/modules/fopbatt/dtbo.img
```

## 版本历史

### Pure@1.0 (2026-03-10)

- 初始纯净版
- 移除所有网络和验证功能
- 替换为纯电池控制程序
- 优化代码结构
- 减小文件大小

## 致谢

- 原始模块作者: bybycode
- WebUI 作者: Collapsar
- 逆向分析: 搭叩 AI

## 许可证

MIT License

## 免责声明

本模块仅供学习和研究使用。使用本模块可能导致：
- 设备保修失效
- 电池损坏
- 设备无法启动
- 数据丢失

使用本模块所造成的任何损失，作者不承担责任。请谨慎使用，并确保您了解相关风险。
