/*
 * parameter.c — 硬件配置参数读取实现
 *
 * 对应Python: parameter.py (class ParameterManager)
 *
 * 不依赖JSON库，使用简单字符串查找解析以下格式:
 *   { "Pcb_Version": 1, "Connect_Version": 2, "Pi_Version": 1 }
 *
 * 查找逻辑：找到键名后提取后面第一个出现的数字。
 * 若文件不存在或解析失败，返回各自的默认值。
 */

#include "parameter.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* ── 内部实现 ────────────────────────────────────────── */

/*
 * _read_int_param - 从params.json中提取指定键的整数值
 * @key:          要查找的键名（如 "Pcb_Version"）
 * @default_val:  文件不存在或键不存在时的返回值
 * 返回: 键对应的整数值，或 default_val
 */
static int _read_int_param(const char *key, int default_val)
{
    FILE *fp = fopen(PARAM_FILE, "r");
    if (!fp) {
        /* params.json不存在，返回默认值（不报错，静默降级） */
        return default_val;
    }

    /* 读取整个文件内容（params.json通常很小）*/
    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);

    /* 在内容中查找键名 */
    const char *pos = strstr(buf, key);
    if (!pos) {
        return default_val;
    }

    /* 跳过键名，找到第一个数字字符 */
    pos += strlen(key);
    while (*pos && !isdigit((unsigned char)*pos)) {
        pos++;
    }

    if (!*pos) {
        return default_val;
    }

    /* 读取连续数字 */
    int value = 0;
    while (*pos && isdigit((unsigned char)*pos)) {
        value = value * 10 + (*pos - '0');
        pos++;
    }

    return value;
}

/* ── 公开API ─────────────────────────────────────────── */

int param_get_pcb_version(void)
{
    int v = _read_int_param("Pcb_Version", DEFAULT_PCB_VERSION);
    return (v == 1 || v == 2) ? v : DEFAULT_PCB_VERSION;
}

int param_get_connect_version(void)
{
    int v = _read_int_param("Connect_Version", DEFAULT_CONNECT_VERSION);
    return (v == 1 || v == 2) ? v : DEFAULT_CONNECT_VERSION;
}

int param_get_pi_version(void)
{
    int v = _read_int_param("Pi_Version", DEFAULT_PI_VERSION);
    return (v == 1 || v == 2) ? v : DEFAULT_PI_VERSION;
}
