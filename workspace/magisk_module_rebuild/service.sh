#!/system/bin/sh

MODDIR=${0%/*}

# 等待系统启动完成
while [ "$(getprop sys.boot_completed)" != "1" ]; do
    sleep 10
done

# 创建日志目录
mkdir -p /data/opbatt

# 日志文件管理
MAX_SIZE=$((5 * 1024 * 1024))
LOGFILE="/data/opbatt/battchg.log"
[ ! -e $LOGFILE ] && touch $LOGFILE
FILE_SIZE=$(stat -c %s "$LOGFILE" 2>/dev/null)
[ "$FILE_SIZE" -gt "$MAX_SIZE" ] && : > "$LOGFILE"

# 设置可执行权限
OPB_C=${MODDIR}/bin/opbatt_control
chmod +x $OPB_C

# 刷新深度放电计数
FOO_PATH="/sys/class/oplus_chg/common"
if [ -d "$FOO_PATH" ]; then
    origin_ddc=$(cat $FOO_PATH/deep_dischg_counts 2>/dev/null)
    echo 1 > $FOO_PATH/deep_dischg_counts 2>/dev/null
    echo 1 > $FOO_PATH/deep_dischg_count_cali 2>/dev/null
    echo ${origin_ddc:-0} > $FOO_PATH/deep_dischg_counts 2>/dev/null
    echo 0 > $FOO_PATH/deep_dischg_count_cali 2>/dev/null
fi

# 设置电池充电电流
BATT_PATH="/sys/class/oplus_chg/battery"
BCC_CUR=13700
BCC="$BATT_PATH/bcc_current"
NCD="$BATT_PATH/normal_cool_down"
CD="$BATT_PATH/cool_down"
if [ -f "$BCC" ]; then
    chmod 0644 $BCC
    chmod 0644 $NCD
    chmod 0644 $CD
    echo 0 >$CD 2>/dev/null
    echo 0 >$NCD 2>/dev/null
    echo $BCC_CUR >$BCC 2>/dev/null
    chmod 0400 $BCC
    chmod 0400 $NCD
    chmod 0400 $CD
fi

# 修复 hidl voter
chmod 0444 /sys/class/power_supply/battery/current_now 2>/dev/null

# 主服务循环
while true; do
    LOGPREFIX="[$(date +%Y-%m-%d-%H:%M:%S)][module]"
    
    echo "=====================MODULE start==========================" >> $LOGFILE
    echo "$LOGPREFIX [SERVICE] start" >> $LOGFILE
    
    # 运行 opbatt_control
    $OPB_C >> $LOGFILE 2>&1
    OPB_CRET=$?
    
    echo "$LOGPREFIX [SERVICE] REBOOTED exit code(${OPB_CRET})" >> $LOGFILE
    
    # 等待 2 秒后重启
    sleep 2
done
