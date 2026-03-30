#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"

extern uint8 App_main_TaskID;
#define	APP_MAIN_1MS_TIMER				0x0001
#define	APP_MAIN_LED_TIMER				0x0002
#define	APP_MAIN_10MS_TIMER				0x0004
#define	APP_RESET_EVT							0x0020
#define	APP_MAIN_SPI							0x0008

/*硬件定义*/
#define ID_ADDR_RAM             0XF1            //全球唯一ID

//智能设备类型
#define SMART_NO_LEARN   		0x00      					//未学习
#define SMART_SMART_ON_OFF      0x01                       //智能开关面板(零火3键)
#define SMART_SOCKET     		0x02                       //智能插座
#define SMART_ALARM             0x03                       //智能警号
#define SMART_SMART_LINKAGE     0x04                       //智能联动
#define SMART_INFRARED_CONTROL       	0x04               //红外控制器
#define SMART_SMART_CURTAIN     0x05                       //智能窗帘
#define SMART_GAS_CTRL       	0x06                       //燃气阀门
#define SMART_RKE_CONTROL     	0x07                       //智能门禁
#define SMART_DOOR_LOCK_CONTROL       	0x08               //智能门锁

#define	DEVICE_ID_202			0x3c
//#define DEVICE_ID                   0x20                       //中央空调
#define DEVICE_ID                  		0x81
#define	DEVICE_VER_HARD							0x01		//硬件版本
#define	DEVICE_VER_SOFT							0x01		//软件版本

#define	DEVICE_BLE_TYPE							1
#define	DEVICE_LORA_TYPE						2

#define ReadySendSize		100

//准备发送LoRa buf数据位置
#define SITE_CMD                    0//命令位置
#define SITE_LEN                    1//数据长度位置
#define SITE_DATA_STAT              2//数据开始位置


/*iApp_Status事件标志定义*/
#define	STATUS_NONE				0x00		// 空闲状态
//#define	APP_WAIT_ACK			0x01		// 发送等待应答状态
#define APP_RESEND_WAIT			0x02		// 重发机制
#define APP_RESEND				  	0x03		// 重发机制
#define APP_WAIT_WORKHAND			0x04		// 握手
#define APP_RELAY					0x05		// 中继
#define APP_WORKHAND				0x06		// 中继握手

/* 常量定义 */
#define APP_CMD_MAX				10		// 命令缓存长度  

//按键中断标识
//#define keyIrqFlagBit       	0x08
//#define keyIrqFlag_1        	(iEvent_Flag |= keyIrqFlagBit)
//#define keyIrqFlag_0        	(iEvent_Flag &=~keyIrqFlagBit)
//#define keyIrqFlag          	(iEvent_Flag &  keyIrqFlagBit)

#define LEARN_STATUS_COM		0x00	//普通工作状态
#define LEARN_STATUS_LEARN 		0x01	//学习状态
#define	LEARN_STATUS_SET		0x02	//设置模式状态
#define	LEARN_STATUS_TEST		0x03	//厂测模式

#define TIME_lORA_Y			40//LoRa 退让最大时间 xx*10ms
extern const u8 user_perpet_time[4];
/*结构定义*/

typedef union 
{
	u8 B;
	struct
	{
		u8 is_mute:1;				//触发一次消音
		u8 is_need_heart_report:1;
		u8 is_50s_touch:1;
		u8 learn_status:1;
		u8 is_Run:1;
		u8 is_1s_touch:1;
		u8 is_4s_touch:1;
		u8 is_60s_touch:1;
	}b;
}uTypedefDeviceStatusFlag;	

/* 设备信息 */
typedef struct 
{
	uTypedefDeviceStatusFlag DeviceStatus;  //设备状态--一些触发标志
	u8 learn_status;						//学习状态
	
	u32 UnixTimestamp;
	u8 	s_cpu_id[7];
	u8	is_sleep;
	u8  is_wake;

	u8	isok_get_Lora;	
	
	u8  LedFlash;							//LED闪烁次数，绿
	u8  LedRedFlash;						//LED闪烁次数，红



	#if SET_NB||SET_CAT1
	u8 test_mode;
	#else
	u8  proto_set_1s;
	#endif
	
	u8  Beep10ms;	//蜂鸣间隔
	u8 	BeepCnt;	//蜂鸣次数
	
	u8  LedFlashSleep;
		
	u8  RunTime;		//运行时间

	#if SET_NB||SET_CAT1
	u8 isNbSleep;
	u8 isNbIdle;

	u8 isWaitSnd;
	#if SET_USE_PSM
	u8 wakeSrc;
	u8 TryPsmCnt;

	u8 bNbSleepTimeOut20s;
	#endif



	#endif
	
	#if 1
	u8 isBoot;
	#endif
	
}TYPE_DEVICE_INFO;


/* 无线通信信息 */
typedef struct 
{
	u8 is_wait_ack_cmd;//是否需要等待应答,3s没应答重发cmd 0x00无需等待应答，其他为命令
	u8 re_send_times;//重发次数
	
	u8 wait_ack_time;//重发间隔4-5s没回复重发
	u8 wait_ack_timer;//等待应答计时xx*100ms，没恢复重发
	
	u8 learn_status_time_out;//学习超时时间
	
	u8 is_lora_now;		//当前处理的信号类型
	u8 is_lora_ack;

	u8 re_plus_times;	//心跳重发次数
	u8 wait_plus_time;
	u8 wait_plus_timer;
	u8 isLost;
}TYPE_RF_INFO;


/*全局变量声明*/
extern u16 sleepTimer;
extern u8 sleepCard;
/*全局变量声明*/
extern volatile uint8 iEvent_Flag;
extern volatile uint8 iEvent_Flag2;
extern volatile uint16 wTimer_Counter0;	  			// 100ms计时

extern volatile uint16 wTimer_UART_1ms;


extern uint8  iTimer_RFM98_nRX;	  			// 非接收模式计时  
	
extern uint8 RF_Num;   //lora通信标号   
extern uint8  g_isRfRetry;

extern u8  g_readySendBuf[ReadySendSize];//准备发送buf

extern TYPE_DEVICE_INFO  g_device_info;//设备信息
extern TYPE_RF_INFO g_rf_info;
extern u32 g_runTime_ms;
extern u32 g_runTime_s;
/*外部函数声明*/
extern void device_reset(void);
extern void fSetFronSleepTimer(u32 sleepTimer);

extern void user_printf_hex(u8 *dat, u16 len);
extern void user_printf(u8 *dat, u16 len);


extern void alawys_10s_timer(void);
extern void fRealTime(void);
extern void app_main_init(uint8 task_id);
extern uint16 app_main_process_Event(uint8 task_id, uint16 events);

extern bool Rf_is_Busy(void);
extern bool fIsNeedSleep(void);
extern bool fIsAlm(void);

extern void p_RunPro(void);
#if SET_NB||SET_CAT1
extern bool IsNeedRfSleep(void);
#endif

#if SET_SPI
extern u8 g_isSpiTouch;
#endif

#define	IS_PROTO_SHIKE	((service.m_proto_set==PROTO_SHIKE)||(service.m_proto_set==PROTO_SHIKE_PLUS))

#endif

