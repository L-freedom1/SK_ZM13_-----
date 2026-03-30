#ifndef _CONFIG_H_
#define _CONFIG_H_

/*************兼容******************/
	#ifndef TRUE
	#define TRUE  1
	#endif

	#ifndef FALSE
	#define FALSE 0
	#endif

	#ifndef FLASE
	#define FLASE 0
	#endif
	
	#ifndef NULL
	#define NULL 0
	#endif
	
	//类型
	#define	 	U8		 uint8
	#define		U16 	 uint16
	#define	 	u8		 uint8
	#define		u16 	 uint16
	#define		U32		 uint32
	#define		u32		 uint32
	#define		U8C		 uint8	
	#define		bit		 uint8
	#define		INT8U	 uint8
	#define		U8X		 uint8	
	//软件延时
	#define	Delay_nus		WaitUs
	#define	Delay_nms		WaitMs
	#define	delay_us		WaitUs
	#define	delay_ms		WaitMs
	//串口
	#define 	printf(...)  		//dbg_printf(__VA_ARGS__)
	#define		PrintString1		LOG
	#define		UART_putc(x)		hal_uart_send_byte(UART0,x)
	#define 	UART_PutStr			LOG
	#define		UART_PutHex(x)	LOG("%02X ", x)
	#define		net_printf			LOG
	#define		printfHex				my_dump_byte
	#define 	GATE_LOG 				LOG
	#define		printt_hex			my_dump_byte
	
	//custom
	#define	 isdigit(c)  		(((unsigned)(c) - '0') < 10)
/*************头文件****************/
	#define		SET_TEST_WAKE

	//硬件版本
	#define		SET_CAT1								0
	#define		SET_NB									0			
	#define 	SET_SPI									1
	#define		SET_RX_TRAN							0	//厂测，协议转换
	#define		SET_TEST_SLEEP					0

	//协议版本
	
	#define		PROTO_VER_JSON			1
	#define		PROTO_VER_ZH202			2
	#if SET_CAT1||SET_NB
		#define		SET_PROTO_VER				PROTO_VER_JSON
	#else
		#define		SET_PROTO_VER			0
	#endif

	#define			SET_BLE_DEBUG			0

	//NB需要关掉LOG,DEBUG_INFO==0
	#if SET_CAT1
		#define		SET_BLE_LEARN					1
		#define   SET_IP
		#define		SET_4G
		#define		SET_GATEWAY						0		//电池供电设备，不做网关，只上报自身设备数据
		#define		SET_MQTT							0
		
		#define		SET_BLE_SUB						0
		#define		USE_SX1262						0
		
		#define		SET_NB_LOG						1	
		#define		DEBUG_NET							0

	#elif SET_NB
		#define		SET_BLE_LEARN					0
		
		#define		SET_BLE_SUB						0
		#define		USE_SX1262						0
		#define		SET_NB_LOG						1
		#define		SET_USE_PSM						0		
		#define		SET_CQ_AEP						0
	#else
		#define		SET_BLE_SUB						1
	
		#if SET_RX_TRAN
			#define		USE_SX1262						0
		#else
			#define		USE_SX1262						1		//lora版本
			#define		SET_LORA_RL						0
		#endif
		
		#define		MULTIHOP_EN_TX					1
		#define		SET_TEST_RXTX						0	
		
		#define   SET_SMART								0	//智能家居版本
		#define   SET_ALARM_VER						0	//安防版本，需要改LORA频率
		#if SET_SMART
			#define	SET_LGW_NATION					0		//立陶宛
		#else
			#define	SET_LGW_NATION					0		//立陶宛
		#endif

	#endif

	#define		SET_BAT_WORK					1
	
	#define   SET_BLE_FAIL					0		//默认不发2.4g
	
	#define 	SET_VER_TK						0	//拓科版本

	#define		DEFAULT_SMOKE_SET				40		//默认烟雾浓度40%
	
	#define		SET_SPI_GAP							10	//10秒轮询

	#define		CODE_SMOKE_ERR					101
	#define		SET_RST_DEVICE					1
	
	
	#define		SET_NEW_TEST						1
	#define   SET_TEST								0		//测试
	#if	SET_TEST
		#define 	SET_ALL_DATA					1
	#endif
	

	
	
//=====C库头文件====//
//#include    <intrins.h>
	#include    <stdio.h>                                               /* define I/O functions  */
	//#include    <absacc.h>
	#include    <string.h>
	#include 	<math.h> 
	#include	<stdlib.h>
	//#include "stdio.h"
	#include "string.h"

	#include "OSAL.h"
	#include "watchdog.h"
	
	#include "log.h"

	#include  "gpio.h"
	#include	"timer.h"
	#include "clock.h"
	#include  "pwm.h"

	#include "pwrmgr.h"
	#include "error.h"
	
	//APP		
	#include 	"user_fs.h"
	#include 	"my_timer.h"
	#include	"my_io.h"
	
	#include	"bsp_uart.h"
	#include 	"event.h"
	#include	"bell.h"
		
	#if SET_CAT1
		#include 	"AIR720.h"
	#elif SET_NB
		#include 	"bc95.h"
	#else
		#include	"MSPI.h"
	
		//LORA通讯
		#include 	"sx126x-board.h"
		#include	"PR_Pro.h"
		
		#if	SET_BLE_SUB
			#include	"note_profile.h"
			#include	"user_rf.h"
		#endif
	#endif

	#if SET_PROTO_VER==PROTO_VER_JSON
		#include	"proto.h"
		#include	"json.h"
	#elif SET_PROTO_VER==PROTO_VER_ZH202
		#include 	"proto_202.h"
		#include	"user_json.h"
		#include	"PR_Pro.h"
		#include	"datapool.h"
	#endif

	#if SET_SPI
		#include	"UserSPI.h"
	#endif
		
	#include 	"app_main.h"	
	
	

/*************系统配置******************/
#define SysDEBUG    0

#define EN_LORA_SLEEP 1

extern uint8 g_system_ota;

#define  	SET_TEST_MODE				0

#if SET_TEST
#define DEBUG_LOG(...)				dbg_printf(__VA_ARGS__)
#define	SM_LOG(...)						dbg_printf(__VA_ARGS__)
#define SM_LOG_BYTE(...)			printfHex(__VA_ARGS__)
#else
#define DEBUG_LOG(...)				//dbg_printf(__VA_ARGS__)
#define	SM_LOG(...)						//dbg_printf(__VA_ARGS__)
#define SM_LOG_BYTE(...)			//printfHex(__VA_ARGS__)
#endif



/*************IO口配置******************/											
#endif
