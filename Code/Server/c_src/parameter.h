/*
 * parameter.h — 硬件配置参数读取
 *
 * 对应Python: parameter.py (class ParameterManager)
 * 读取同目录下的 params.json，提取硬件版本号。
 *
 * params.json 格式:
 *   {
 *     "Connect_Version": 1,   // 1 或 2
 *     "Pcb_Version":     1,   // 1=3.3V系数, 2=5.2V系数
 *     "Pi_Version":      1    // 1=Pi4及以下, 2=Pi5
 *   }
 */

#ifndef PARAMETER_H
#define PARAMETER_H

#define PARAM_FILE "params.json"

/* 读取失败时的默认值（与Python defaults一致） */
#define DEFAULT_PCB_VERSION     1
#define DEFAULT_CONNECT_VERSION 2
#define DEFAULT_PI_VERSION      1

/**
 * param_get_pcb_version - 从params.json读取PCB版本号
 * 返回: 1 或 2，读取失败时返回 DEFAULT_PCB_VERSION
 * 对应Python: ParameterManager.get_pcb_version()
 */
int param_get_pcb_version(void);

/**
 * param_get_connect_version - 从params.json读取连接板版本号
 * 返回: 1 或 2，读取失败时返回 DEFAULT_CONNECT_VERSION
 */
int param_get_connect_version(void);

/**
 * param_get_pi_version - 从params.json读取树莓派版本号
 * 返回: 1 或 2，读取失败时返回 DEFAULT_PI_VERSION
 */
int param_get_pi_version(void);

#endif /* PARAMETER_H */
