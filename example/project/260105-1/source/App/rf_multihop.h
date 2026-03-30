/*
 * @Author: your name
 * @Date: 2021-11-27 11:54:48
 * @LastEditTime: 2021-11-30 17:37:33
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \SK_ZH101DS_吊扇智能面板,测试多跳\project\peripheral\sk_pro\Source\rf_profile\multihop.h
 */
#ifndef	RF_MULTIHOP
#define	RF_MULTIHOP

#include "note_profile.h"

#ifndef NULL
#define NULL 0
#endif

/* 多跳使能 */
//#define MULTIHOP_EN 1

/* 多跳头长度 */
#define MULTIHOP_HEAD_SIZE  5
/* 默认数据保持时间3s */
#define DEF_DATA_HOLD_TIME  4

/*  */
#define DELAY_HOP_TIME  5//延时多跳时间 *10ms
typedef union 
{
    unsigned char buf[MULTIHOP_HEAD_SIZE];
    struct
    {
        unsigned char is_valid;//是否还有效，收到回复数据则为无效，不需要再转发
        unsigned char countdown;//倒计时 时间到还没有收到回复数据则转发
        unsigned char data_hold_timer;//队列数据保持剩余时间 
    }info;
}TYPE_POOL_HEAD;

#define DAYA_START  MULTIHOP_HEAD_SIZE//数据起始位置

/* 多跳 限制次数 */
#define multihop_limit(a) ((a >> 4) & 0x0F)
/* 多跳 当前跳的次数 */
#define multihop_cur_hop(a) (a & 0x0F)

/**
 * @description: 多跳接收数据处理
 * @param {unsigned char} *rx_dat
 * @param {unsigned short} rx_len
 * @param {UP_DOWN} up_down 上下行
 * @return {*}
 */
extern void rf_mul_rx_dat(unsigned char *rx_dat, unsigned short rx_len, UP_DOWN up_down);
/**
 * @description: 多跳数据扫描处理 10ms 跑一次
 * @param {*}
 * @return {*}
 */
extern void rf_multihop_scan(void);

#endif


