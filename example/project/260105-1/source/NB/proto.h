/*
 * @Author: your name
 * @Date: 2022-04-23 17:03:48
 * @LastEditTime: 2024-04-18 16:13:34
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \source\App\NB\proto.h
 */
#ifndef	PROTO_H
#define	PROTO_H

#include	"config.h"

#if SET_NB||SET_CAT1
#define	MAX_NB_LEN 	1024

//#define	TIME_OUT_NET	300		//5分钟
#define	TIME_OUT_NET	180			//3分钟--正常弱信号，则为3分20秒左右休眠

extern u32  g_nbiot_last_read_time;


/*******************************
协议属性字段定义
*******************************/
//属性列表
enum 
{
	ID_IMSI = 1,
	ID_ICCID,
	ID_CSQ,
	ID_VER,
	ID_BAT_VAL,
	ID_BAT_LOW,
	ID_WATER_REAL,			//水压
	ID_TEMP_REAL,
	ID_WATER_HIS,
	ID_TEMP_HIS,
	ID_HEART_REPORT,
	ID_HEART_SAMPLE,
	ID_WATERHIGH_SET,
	ID_WATERLOW_SET,
};
#define	ID_MESSAGE		0xef
#define	ID_DIR				0x12
enum 
{
	DIR_SER_SET = 0,
	DIR_DEV_RE,
	DIR_DEV_SET,
	DIR_SER_RE
};

extern void NB_TxPro(void);
extern u8 p_prto_dispose_data(u8 *buf, u16 len);
#endif

#endif
