#ifndef __USER_1527_H__
#define __USER_1527_H__

#include "config.h"

/*================= 硬件接口(按 433 改板实际接线修改) =================*/
/* 433M 1527 发射模块数据脚(原 LoRa 射频开关脚) */
#define PIN_RF1527_TX           GPIO_P31

/* 433M 模块供电使能脚(原 LoRa 复位脚): 0-开(发射)  1-关(省电) */
#define PIN_RF1527_EN           GPIO_P07

/* 模块若直接常供电(没有使能脚), 把该宏改为 0 */
#define RF1527_USE_PWR_EN       1

/* 数据脚: 发送过程在定时器中断里翻转, 用快速写 IO */
#define RF1527_TX(x)            hal_gpio_fast_write(PIN_RF1527_TX, (x))

#if RF1527_USE_PWR_EN
#define RF1527_EN(x)            hal_gpio_write(PIN_RF1527_EN, (x))
#else
#define RF1527_EN(x)
#endif

/* 1527 24bit 码值: 高16位为地址码, 低8位为状态/数据码 */
// #define RF1527_ADDR_BASE            0x4D8880UL
/* 心跳码: 低字节为心跳状态码(00/C3/C1/C4), 与按键码(02/07)低半字节不冲突 */
// #define RF1527_HEARTBEAT_CODE(code) (RF1527_ADDR_BASE | (uint32)(code))

void user_1527_init(void);
void user_1527_tx(uint32 tx_dat);   /* 发送 24bit 码值, 非阻塞, 由定时器中断逐位发出 */
void user_1527_tx_heartbeat(u8 hb_code);   /* 发送心跳码 */
void user_1527_tx_stop(void);       /* 中止发送 */
void user_1527_tick(void);          /* 发送节拍, 由 my_timer.c 的 HAL_EVT_TIMER_6 调用, 50us 一次 */
uint8 user_1527_is_tx_busy(void);   /* 1:正在发送 0:空闲, 休眠前需判断 */

#endif
