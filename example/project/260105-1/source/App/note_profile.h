/*
 * @Author: your name
 * @Date: 2021-10-07 16:36:53
 * @LastEditTime: 2022-07-11 15:48:20
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @Description: 2.4G 设备接入协议，节点协议部分, 搜索 to modification 移植需要修改
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\note_profile.h
 */
#ifndef __NOTE_PROFILE_H__
#define __NOTE_PROFILE_H__

#include	"config.h"

#if	SET_BLE_SUB

#define PROFILE_MIN_LEN  19 //协议最短数据长度
/* 协议 字段位置 */
// 长度  通讯标号 本机uid  目标uid  预留  T  L  V      校验
// 1B      1B      6B      6B      2B  1B  1B  NB     1B
#define SITE_ALL_LEN    0
#define SITE_NUM        1
#define SITE_SRC_UID    2
#define SITE_TAR_UID    8
#define SITE_LOW_POWER  14//低功耗描述符
#define SITE_MULTIHOP   15//多跳描述字段
#define SITE_REP        16//重发字段
#define SITE_T          17
#define SITE_L          18
#define SITE_V          19

/* 重复包识别长度  包裹长度 通信标号、2个uuid */
#define REPEAT_PACK_SIZE    14

/* 标签域 T  */
#define T_SCAN_GATEWAY  0x04//扫描附近网关及应答
#define T_REG_NET       0x01//节点设备注网及心跳（上行）
#define T_GATEWAY_START_UP  0x02//网关设备上电
#define T_TRAN          0x03//LoRa 透传 指令

/* 最大扫描 数量 */
#define MAX_SCAN_NUM    20

/* 发送buf大小 */
#define SEND_BUF_SIZE   256

/* 节点通信， 心跳时间 */
#define HEARTBEAT_TIME   60
//#define HEARTBEAT_TIME   3

/* 通用重发间隔 */
#define COMM_RESEND_TIME    150

/* 低功耗 描述： 部分设备有低功耗功能，网关需要做一定处理，单位 50ms，表示节点设备
定时唤醒 N*50ms，无需低功耗设备填写 0，长低功耗设备填写 0xFF */
//#define LOW_POWER_FLGA  5//xx * 50 ms 
#define LOW_POWER_FLGA  0
#define MULTIHOP_FLGA  (4 << 4)//多跳次数
/* 通信标号 */
/* 是否上行包 */

#define profile_is_up(a) (a & (0x01 << 7))
/* 是否应答包 */
#define profile_is_ack(a) (a & (0x01 << 6))


/* 设置上行包 */
#define profile_set_up(a) (a |= (0x01 << 7))
/* 设置应答包 */
#define profile_set_ack(a) (a |= (0x01 << 6))
/* 是否下行包 */
#define profile_set_down(a) (a &= ~(0x01 << 7))
/* 是否应答包 */
#define profile_reset_ack(a) (a &= ~(0x01 << 6))

/* 接受数据类型 */
typedef enum
{
    NO_CAST = 0,//uid错误，类型不符合
    BROADCAST,//广播
    MULTICAST,//组播
    NOTECAST//节点通信
}RX_TYPE;

/* 接受数据类型 */
typedef enum
{
    PROFILE_NULL = 0,//协议通信 空
    PROFILE_UP,//协议通信上行
    PROFILE_DOWN//协议通信下行
}UP_DOWN;

/* 扫描热点 */
typedef struct
{
    unsigned char uid[6];
    unsigned char rssi;
    unsigned char is_cannot_reg;//是否不可以注册，部分可能无法注册
    unsigned char hop_times;//多跳次数 ，优先选跳的次数少的注网
}TYPE_SCAN_RESULT;


/* 节点设备 相关结构体 */
typedef struct
{
	unsigned char is_tx_busy;//是否当前正在发送 繁忙
    unsigned char gateway_uid[6];//当前注网，网关uid
    unsigned char device_uid[6];//本机uid
    unsigned char reg_status;//当前注网状态，1已注网，0为注网
    unsigned char reg_fail;
    unsigned char tx_num;//通信标号 0-63

    unsigned char rx_num;//通信标号 0-63
    
    unsigned char rssi;//网关信号强度
    RX_TYPE rx_type;//接收到的数据通信方式

    
    /* 扫描注册相关 */
    unsigned char is_scan_status;//是否在扫描状态
    TYPE_SCAN_RESULT scan_result[MAX_SCAN_NUM];//扫描结果
    unsigned char cur_scan_num;//当前扫描到的网关数量
    unsigned short reg_timer;//注册计时， 如果没有注册成功，30s需要扫描注册一下

    unsigned char is_need_report;
    unsigned short heartbeat_timer;//心跳计时

    void(*p_rx_cb)(unsigned char *,unsigned short, unsigned char);//接受数据回调
	
	//当前发送数据为应答包？
	 

}TYPE_NOTE_PROFILE;


typedef struct
{
    unsigned char resend_Times;//重发次数
    unsigned short wait_ack_timer;//等待回复计时
    unsigned char wait_ack_cmd;//等待重发命令
    unsigned char send_cmd;//当前发送命令
    unsigned char is_need_send;//当前是否需要发送
    unsigned short wait_ack_time;//xx*100ms秒内没回复重发
    unsigned short delay_timer;//单火动作时需要延时发送LoRa

    unsigned char send_fail_count;//发送失败计数, 超过次数上线，需要重新注网

    unsigned char send_test_ok;  //测试模式失败计数，用于统计通讯成功率

    unsigned char send_buf[SEND_BUF_SIZE];
    unsigned char send_len;
    unsigned char is_already_ack;//是否已经回复了
}TYPE_RF_SEND;

typedef struct
{
    unsigned char send_buf[SEND_BUF_SIZE];
    unsigned char send_len;
}TYPE_RF_SEND_BAK;


extern TYPE_NOTE_PROFILE g_note;
extern TYPE_RF_SEND g_rf_send;
extern TYPE_RF_SEND_BAK g_rf_ackbak;
/**
 * @description: 节点设备初始化
 * @param {unsigned char} *device_uid 本机uid
 * @param (void)p_rx_cb 接受数据回调
 * @return {*}
 */
void note_init(unsigned char *device_uid, void(*p_rx_cb)(unsigned char *,unsigned short, unsigned char));

/**
 * @description: 当前发送数据
 * @param {unsigned char} *dat_buf
 * @param {unsigned short} len
 * @return {*}
 */
unsigned char rf_tx(unsigned char *tx_dat, unsigned short len, unsigned short delay_time);


/**
 * @description: 设置发送 包含重发次数 重发间隔
 * @param {u8} cmd 发送命令
 * @param {u8} times 重发次数
 * @param {u16} resend_interval 重发间隔
 * @return {*}
 */
void note_set_send(unsigned char *tx_dat, unsigned short len, unsigned char times, unsigned short resend_interval);
/**
 * @description: 关闭重发
 * @param {*}
 * @return {*}
 */
void note_close_resend(void);

/**
 * @description: 节点流程处理 10ms 跑一次
 * @param {*}
 * @return {*}
 */
void note_process(void);

/**
 * @description: 通用透传命令上报 (之前的LoRa协议 普通设备上报)
 * @param {unsigned char} cmd1 
 * @param {unsigned char} cmd2
 * @param {unsigned char} times 重发次数
 * @param {unsigned short} resend_interval 重发间隔
 * @param {unsigned char} is_ack 是否应答包
 * @return {*}
 */
//void note_comm_tran_up(unsigned char cmd1, unsigned char cmd2, unsigned char times, unsigned short resend_interval,unsigned char is_ack);

extern void note_comm_tran_up(unsigned char *SendBuf, unsigned char times, unsigned short resend_interval,unsigned char is_ack);


/**
 * @description: 接受数据处理
 * @param {unsigned char} rx_dat 
 * @param {unsigned short} len
 * @param {unsigned char} rssi 信号强度， 需要取负值，例传入 40，实际为-40dbm
 * @return {*}
 */
void note_rx_deal(unsigned char *rx_dat, unsigned short len, unsigned char rssi, unsigned short phyFoff);

unsigned char note_cur_is_send(void);


extern unsigned char profile_check(unsigned char  *dat_buf, unsigned short len, UP_DOWN up_down);
#endif

#endif


