#!/system/bin/sh

# Magisk 模块安装脚本

# 打印安装信息
ui_print "=========================================="
ui_print "  0aO电池工具包（重建版）"
ui_print "  版本: Rebuild@1.0.0"
ui_print "  作者: 搭叩AI"
ui_print "=========================================="
ui_print ""

# 检查设备
ui_print "正在检查设备..."
MODEL=$(getprop ro.product.model)
if [ -z "$MODEL" ]; then
    MODEL="未知设备"
fi
ui_print "设备型号: $MODEL"

# 检查 Android 版本
ANDROID_VERSION=$(getprop ro.build.version.release)
ui_print "Android 版本: $ANDROID_VERSION"

# 检查架构
ARCH=$(getprop ro.product.cpu.abi)
ui_print "CPU 架构: $ARCH"

# 检查是否为 ARM64
if [ "$ARCH" != "arm64-v8a" ]; then
    ui_print "警告: 本模块仅支持 ARM64 架构"
    ui_print "当前架构: $ARCH"
    ui_print "安装可能会失败"
    sleep 3
fi

# 创建必要的目录
ui_print ""
ui_print "正在创建必要的目录..."
mkdir -p /data/opbatt
mkdir -p $MODPATH/bin
mkdir -p $MODPATH/webroot

# 复制文件
ui_print "正在复制文件..."
cp -f $MODPATH/bin/* $MODPATH/bin/ 2>/dev/null
cp -f $MODPATH/webroot/* $MODPATH/webroot/ 2>/dev/null

# 设置权限
ui_print "正在设置权限..."
chmod 755 $MODPATH/service.sh
chmod 755 $MODPATH/bin/opbatt_control

# 创建配置文件
ui_print "正在创建配置文件..."
if [ ! -f /data/opbatt/batt_control ]; then
    cat > /data/opbatt/batt_control << 'EOF'
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
EOF
    ui_print "已创建默认配置文件"
fi

# 设置 SELinux 上下文
if [ -d /sys/fs/selinux ]; then
    restorecon -R $MODPATH/bin 2>/dev/null
    restorecon -R $MODPATH/webroot 2>/dev/null
fi

# 完成
ui_print ""
ui_print "=========================================="
ui_print "  安装完成！"
ui_print "=========================================="
ui_print ""
ui_print "注意事项:"
ui_print "1. 本模块需要 Root 权限"
ui_print "2. 本模块仅支持指定机型"
ui_print "3. 使用前请备份重要数据"
ui_print "4. 重启后模块将自动运行"
ui_print ""
ui_print "日志文件: /data/opbatt/battchg.log"
ui_print "配置文件: /data/opbatt/batt_control"
ui_print ""
ui_print "感谢使用！"
ui_print ""
