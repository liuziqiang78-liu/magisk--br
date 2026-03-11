/**
 * @file sysfs_paths.h
 * @brief sysfs 文件路径定义
 * 
 * 定义所有需要访问的 sysfs 文件路径
 * 一加8T (KB2000) 专用配置
 */

#ifndef SYSFS_PATHS_H
#define SYSFS_PATHS_H

/* 电池状态文件路径 */
#define SYSFS_BATTERY_STATUS        "/sys/class/power_supply/battery/status"
#define SYSFS_BATTERY_CAPACITY      "/sys/class/power_supply/battery/capacity"
#define SYSFS_BATTERY_VOLTAGE       "/sys/class/power_supply/battery/voltage_now"
#define SYSFS_BATTERY_CURRENT_NOW   "/sys/class/power_supply/battery/current_now"
#define SYSFS_BATTERY_TEMP          "/sys/class/power_supply/battery/temp"
#define SYSFS_BATTERY_CYCLE_COUNT   "/sys/class/power_supply/battery/cycle_count"
#define SYSFS_BATTERY_CHARGE_FULL   "/sys/class/power_supply/battery/charge_full"
#define SYSFS_BATTERY_TIME_TO_FULL  "/sys/class/power_supply/battery/time_to_full_avg"
#define SYSFS_USB_ONLINE            "/sys/class/power_supply/usb/online"
#define SYSFS_USB_PRESENT           "/sys/class/power_supply/usb/present"
#define SYSFS_USB_VOLTAGE           "/sys/class/power_supply/usb/voltage_now"
#define SYSFS_USB_CURRENT           "/sys/class/power_supply/usb/current_max"

/* 一加充电文件路径 - 一加8T专用 */
#define SYSFS_OPLUS_BATTERY_CC      "/sys/class/oplus_chg/battery/battery_cc"
#define SYSFS_OPLUS_BATTERY_FCC     "/sys/class/oplus_chg/battery/battery_fcc"
#define SYSFS_OPLUS_BATTERY_RM      "/sys/class/oplus_chg/battery/battery_rm"
#define SYSFS_OPLUS_BATTERY_SOH     "/sys/class/oplus_chg/battery/battery_soh"
#define SYSFS_OPLUS_BATTERY_MANU_DATE "/sys/class/oplus_chg/battery/battery_manu_date"
#define SYSFS_OPLUS_BATTERY_TYPE    "/sys/class/oplus_chg/battery/battery_type"
#define SYSFS_OPLUS_BATTERY_BCC_PARMS "/sys/class/oplus_chg/battery/bcc_parms"
#define SYSFS_OPLUS_BATTERY_CHIP_SOC "/sys/class/oplus_chg/battery/chip_soc"
#define SYSFS_OPLUS_BATTERY_VBAT_UV "/sys/class/oplus_chg/battery/vbat_uv"
#define SYSFS_OPLUS_BATTERY_LOG     "/sys/class/oplus_chg/battery/battery_log_content"

/* 一加8T 专用电池节点 */
#define SYSFS_OPLUS_BATTERY_QMAX    "/sys/class/oplus_chg/battery/qmax"
#define SYSFS_OPLUS_BATTERY_LIFE    "/sys/class/oplus_chg/battery/battery_life"
#define SYSFS_OPLUS_BATTERY_CELL_VOLTAGE "/sys/class/oplus_chg/battery/cell_voltage"
#define SYSFS_OPLUS_BATTERY_VOLTAGE_DIFF "/sys/class/oplus_chg/battery/voltage_diff"
#define SYSFS_OPLUS_BATTERY_SHELL_TEMP "/sys/class/oplus_chg/battery/shell_temp"
#define SYSFS_OPLUS_BATTERY_CYCLE_COUNT_OPLUS "/sys/class/oplus_chg/battery/cycle_count"
#define SYSFS_OPLUS_BATTERY_HEALTH  "/sys/class/oplus_chg/battery/battery_health"
#define SYSFS_OPLUS_BATTERY_SOH_ALT "/sys/class/oplus_chg/battery/soh"
#define SYSFS_OPLUS_BATTERY_MANUFACTURE_DATE "/sys/class/oplus_chg/battery/manufacture_date"
#define SYSFS_OPLUS_BATTERY_SHUTDOWN_VOLTAGE "/sys/class/oplus_chg/battery/shutdown_voltage"

/* 充电控制文件路径 - 一加8T专用 */
#define SYSFS_OPLUS_COMMON_CPA_POWER "/sys/class/oplus_chg/common/cpa_power"
#define SYSFS_OPLUS_INPUT_CURRENT_LIMIT "/sys/class/oplus_chg/common/input_current_limit"
#define SYSFS_OPLUS_FAST_CHARGE_CURRENT "/sys/class/oplus_chg/common/fast_charge_current"
#define SYSFS_OPLUS_CHARGE_VOLTAGE  "/sys/class/oplus_chg/common/charge_voltage"
#define SYSFS_OPLUS_THERMAL_MITIGATION "/sys/class/oplus_chg/common/thermal_mitigation"
#define SYSFS_OPLUS_CHARGING_ENABLED "/sys/class/power_supply/battery/charging_enabled"
#define SYSFS_OPLUS_CHARGE_STATUS   "/sys/class/oplus_chg/common/charge_status"
#define SYSFS_OPLUS_MAX_CHARGE_POWER "/sys/class/oplus_chg/common/max_charge_power"
#define SYSFS_OPLUS_CHARGE_POWER_RANGE "/sys/class/oplus_chg/common/charge_power_range"

/* 充电协议控制文件路径 - 一加8T专用 */
#define SYSFS_OPLUS_UFCS_ENABLE     "/sys/class/oplus_chg/common/ufcs_enable"
#define SYSFS_OPLUS_PPS_ENABLE      "/sys/class/oplus_chg/common/pps_enable"
#define SYSFS_OPLUS_SVOOC_ENABLE    "/sys/class/oplus_chg/common/svooc_enable"
#define SYSFS_OPLUS_VOOC_ENABLE     "/sys/class/oplus_chg/common/vooc_enable"
#define SYSFS_OPLUS_VOOC_CURRENT    "/sys/class/oplus_chg/common/vooc_current"
#define SYSFS_OPLUS_VOOC_VOLTAGE    "/sys/class/oplus_chg/common/vooc_voltage"

/* 恒压控制节点 - 一加8T专用 */
#define SYSFS_OPLUS_CV_ENABLE       "/sys/class/oplus_chg/common/cv_enable"
#define SYSFS_OPLUS_CV_VOLTAGE      "/sys/class/oplus_chg/common/cv_voltage"

/* DDRC 节点 - 一加8T专用 */
#define SYSFS_OPLUS_DDRC_ENABLE     "/sys/class/oplus_chg/common/ddrc_enable"
#define SYSFS_OPLUS_DDRC_THRESHOLD  "/sys/class/oplus_chg/common/ddrc_threshold"

/* SOC 平滑节点 - 一加8T专用 */
#define SYSFS_OPLUS_SOC_SMOOTH_ENABLE "/sys/class/oplus_chg/common/soc_smooth_enable"

/* 温度伪装节点 - 一加8T专用 */
#define SYSFS_OPLUS_FAKE_TEMP_ENABLE "/sys/class/oplus_chg/common/fake_temp_enable"
#define SYSFS_OPLUS_FAKE_TEMP_VALUE "/sys/class/oplus_chg/common/fake_temp_value"
#define SYSFS_OPLUS_BATTERY_FAKE_TEMP "/sys/class/oplus_chg/battery/fake_temp"

/* 温度控制节点 - 一加8T专用 */
#define SYSFS_BATT_THERM_PATH       "/sys/class/thermal/thermal_zone0/temp"
#define SYSFS_BATT_THERM_MONITOR    "/sys/class/oplus_chg/battery/temp"

/* 充电控制节点 - 一加8T专用 */
#define SYSFS_CHARGE_CONTROL_PATH   "/sys/class/power_supply/battery/charge_control_limit"
#define SYSFS_CHARGE_CONTROL_END_PATH "/sys/class/power_supply/battery/charge_control_end_threshold"

/* 目录路径 */
#define SYSFS_OPLUS_BATTERY_DIR     "/sys/class/oplus_chg/battery"
#define SYSFS_OPLUS_COMMON_DIR      "/sys/class/oplus_chg/common"
#define SYSFS_POWER_SUPPLY_DIR      "/sys/class/power_supply"

#endif /* SYSFS_PATHS_H */
