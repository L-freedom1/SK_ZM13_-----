/*
 * @Author: dujianhuang
 * @Date: 2020-06-04 19:25:35
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-01 10:09:24
 */ 
/*!
 * \file      radio.h
 *
 * \brief     Radio driver API definition
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#ifndef __RADIO_H__
#define __RADIO_H__

#include "config.h"
#include "sx126x.h"
#if (USE_SX1262 > 0)

#define USR_434M    0
#define USE_444M    1

#define RF_SLEEP_CALL				440000000


#if (USR_434M > 0)
// #define RF_FREQUENCY                                433920000 // Hz
#define RF_FREQUENCY                                434000000 // Hz
#define LORA_BANDWIDTH                              LORA_BW_125         //带宽
#endif

#if (USE_444M > 0)
	#if SET_SMART
		#if SET_LGW_NATION
			#define RF_FREQUENCY_SHIKE                                434000000//500000000// // Hz
			#define LORA_BANDWIDTH_SHIKE                               LORA_BW_125        //带宽
			#define	LORA_SF_SHIKE										LORA_SF7
		#else
			#define RF_FREQUENCY_SHIKE                                444000000//500000000// // Hz
			#define LORA_BANDWIDTH_SHIKE                               LORA_BW_250        //带宽
			#define	LORA_SF_SHIKE										LORA_SF9
		#endif
	#else
		#define RF_FREQUENCY_SHIKE                                 434000000//500000000// // Hz
		
		#define LORA_BANDWIDTH_SHIKE                               LORA_BW_125        //带宽
		#define	LORA_SF_SHIKE																				 LORA_SF9
		
//		#define LORA_BANDWIDTH_SHIKE																 LORA_BW_010
//		#define	LORA_SF_SHIKE																				 LORA_SF10

//		#define LORA_BANDWIDTH_SHIKE																 LORA_BW_031
//		#define	LORA_SF_SHIKE																				 LORA_SF11

//			#define LORA_BANDWIDTH_SHIKE																 LORA_BW_015
//			#define	LORA_SF_SHIKE																				 LORA_SF12

//			#define LORA_BANDWIDTH_SHIKE																 LORA_BW_062
//			#define	LORA_SF_SHIKE																				 LORA_SF8
	#endif

#define RF_FREQUENCY                                444000000
#define LORA_BANDWIDTH                              LORA_BW_250         //带宽
#endif





/* 常量定义 */
// iStatus_RF状态定义
#define	RFM98_NONE					0x00		// 不处理
#define	RFM98_RX_TO_TX			0x01		// 转发送模式
#define	RFM98_DATA_TX				0x02		// 发送数据
#define	RFM98_WAIT_TX				0x03		// 等待发送成功
#define	RFM98_TX_TO_RX			0x04		// 转接收模式
#define	RFM98_DATA_RX				0x05		// 接收数据
#define	RFM98_TX_FAIL				0x06		// 发送数据错误
#define RFM98_TX_OK				  0x07		// 发送数据成功
#define RFM98_SLEEP				  0x08		// 休眠待机模式
#define RFM98_RX_TO_SLEEP		0x09		// 休眠待机模式
#define RFM98_QUITE_SLEEP   0x0A    // 退出休眠状态

#define RFM98_Process RF_Process
#define RFM96_Config() (RF_Init(0,21))
#define RFM96_LoRaEntryRx() (iStatus_RF = RFM98_TX_TO_RX)
#define RFM98_IO_Init() ;
/*全局变量声明*/
extern uint8  iStatus_RF;			
extern uint8  iRF_TX_Buf[64];		   		// RFM98射频发送数据缓存
extern uint8  iRF_RX_Buf[64];				// RFM98射频接收数据缓存
extern uint8  iRF_TX_Buf_bak[64];
extern uint32  fTimer_RF_TX;
//extern uint8  iRF_RX_Buf_Temp[64];			// RFM98射频接收数据缓存数据备份

extern void RF_Process(void);
extern void RF_Init(u8 mode, u8 len);
extern void RF_send_data(u8 *TxBuf,u8 len);

extern void rf_wait_tx_timeout(void);
extern void rf_100ms_timeout(void);

extern void RadioSleep( void );

extern u8 RF_cad_check_channel2(u32 LORA_FQ);
#endif

#endif
