/*
 * @Author: your name
 * @Date: 2021-12-09 15:24:33
 * @LastEditTime: 2024-04-22 15:22:53
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \ST17H66_SDK_3.1.1.0\example\project\211207-1\source\App\bsp_uart.h
 */

#ifndef    __BSP_UART__
#define    __BSP_UART__

#include	"config.h"

/*********************************************驱动************************************************/
#define UART_TX_LEN               8
#define UART_RX_LEN				  			50
#define UART_TIMEOUT              5
typedef struct
{
    //u8	m_tx_read;
    //u8	m_tx_write;
    //u8	m_tx_busy;

    u16	m_rx_read;
    u16	m_rx_write;
    u32	m_last_rx_time;

    u8	m_tx_buf[UART_TX_LEN];
    u8	m_rx_buf[UART_RX_LEN];
	
	u16 m_last_pos;
	
} UART_CTRL;

extern UART_CTRL  g_uart_ctrl_2;

extern void	UART_config(void);

/*********************************************应用************************************************/

/*****************************************************************
以下指令用于唤醒主控MCU
RS232_SEND(0x00);
RS232_SEND(0x00);
GCC_DELAY(2000);

有效数据从包头0XAA开始，
RS232_SEND(0xAA);					//包头
RS232_SEND(R_SM_ZERO_DATA);			//SM Zero AD   零点值
RS232_SEND(R_SM_IR_DATA);				//SM Emission AD 发射值
RS232_SEND(R_SM_DATA);					//SM AD         发射差值
RS232_SEND(R_SM_BD_ZERO);				//Calibration zero  标定值
RS232_SEND(R_SM_BD_ALARM);			//Calibration alarm 报警值
RS232_SEND(R_VDD_ADC[1]);				//VBG _H 8bit AD  电池AD值
RS232_SEND(R_T_ADC);				    //Temp AD   温度AD值
RS232_SEND(SM_State.Buf);       //烟雾状态
RS232_SEND(R_SM_ALARM);         //报警值
RS232_SEND(0x55);								//结束

R_SM_BD_ALARM与R_SM_ALARM 实际值相同，设置报警阈值时，两者均被操作，简单理解为两者都为报警值即可。

主控->烟感MCU
通讯协议如下：
Head + Ddata + CheckSum + Tail
CheckSum：Head + Ddata

设置报警阈值指令：0x01

eg:
设置阀值为50
CmdTab[]={0x01,0x32,0x33};



*********************************************************************************/
//命令
#define	TOUCH_SET			0x01

//位置
#define	TOUCH_POS_CMD		0x01
#define	TOUCH_POS_DATA		0x02

/************************
	接收数据
烟感MCU  -> 主控：
0x00+0x00+2000Delay+Head(0xaa)+Data[9]+Tail(0x55)
************************/
//#define	SMOKE_ALM_1S		5
#define	SMOKE_ALM_1S		8

//#define	SMOKE_ALMREST_1S	60
//230710:5秒则烟雾恢复
#define	SMOKE_ALMREST_1S	5
#define	TEMP_ALM_1S			10
#define TEMP_ALMREST_1S		60


#if SET_NB||SET_CAT1
	#define GAP_ALM_1S			600
#else
	#define GAP_ALM_1S			60
#endif

#define	BELL_ALM_1S			90

//烟雾状态值
typedef struct
{
    uint8   F_SM_ERR:1;           //烟腔故障
    uint8   F_SM_ALARM:1;         //烟雾报警
    uint8   F_SM_PRE_ALARM:1;     //烟雾预警
    uint8   F_SM_BD:1;            //标定状态
    uint8   F_SM_BD_OK:1;         //标定完成
    uint8   Reserved:3;
}SM_FG;

#if SET_SPI
typedef struct
{
	u8 ir_ad;			//发射差值
	u8 bd_zero;			//标定值
	u8 vdd_adc;			//电池AD值
	u8 t_adc;			//温度AD值
	SM_FG state;		//烟雾状态
	u8 zero_data;		//零点值
	u8 ir_data;			//发射值

	#if SET_RX_TRAN
	u8 bd_alarm;		//报警值
	u8 alarm;				//报警值
	#endif


	u8 isWarn;			//预警
	u8 isErr;
}R_SM;
#else
typedef struct
{
	u8 zero_data;		//零点值
	u8 ir_data;			//发射值
	u8 ir_ad;				//发射差值
	u8 bd_zero;			//标定值
	u8 bd_alarm;		//报警值
	u8 vdd_adc;			//电池AD值
	u8 t_adc;				//温度AD值
	SM_FG state;		//烟雾状态
	u8 alarm;				//报警值
}R_SM;
#endif

extern R_SM device_info;

/************************
	接收数据
主控->烟感MCU
Head(0xaa) + Ddata + CheckSum + Tail(0x55)
CheckSum：Head + Ddata
设置报警阈值指令：0x01
eg:
设置阀值为50
CmdTab[]={0x01,0x32,0x33};
************************/
#define SIZE_PARA 8


typedef struct    
{
	//需要保存的参数
	u16 m_battery_set;
	u8 m_temp_almset;
	u8 m_temp_rstset;
	u8 m_temp_lowset;
	u8 m_temp_lowrstset;	
	u8 m_heartbeat_time;

	//读取,不需要保存
	//230427:需要保存
	u8 m_smoke_set;

	#define		PROTO_ZH202			0xaa		//控电协议
	#define		PROTO_SHIKE			0x55		//时刻报警协议
	#define		PROTO_SHIKE_PLUS	0x66		//2300_plus
	u8 m_proto_set;						//协议：控电协议、时刻报警器

	u8 m_bak1;
	
	//服务器地址
	#if SET_CAT1
	u16 port;
	u8 Ip_buf[40];								//服务器IP与端口
	#endif
	
	/* 服务类型 */
	u16 serviceID;
	u16 taskId;
	
	/* 属性 */
	u8 m_smoke_state_last;
	u8 m_smoke_state;
	u8 m_smoke;
	u8 m_smoke_set_val;		//设置的原始值
	u8 m_smokeMax;
	
	u8 m_temp_state_last;
	u8 m_temp_state;
	u8 m_temp;
	u8 m_tempMax;					//取上报区间的最高温度
	
	u8 m_battery_state;	
	u16 m_battery;
	
	u8 m_muffling;	

	//220826:烟雾5秒报警，1分钟恢复
	u8 m_smoke_alm1s;
	u8 m_smoke_almRest1s;
	u8 m_smoke_almRest6;
	u8 m_smoke_almRest7;
	u8 m_smoke_almRest8;

	u8 m_temp_alm1s;
	u8 m_temp_almRest1s;
	
	
	#if SET_PROTO_VER==PROTO_VER_JSON
	float f_battery;
	float f_battery_set;
	#endif
	
	u16 m_smoke_err1s;
	u16 m_smoke_errRest1s;
	
	#if SET_NEW_TEST
	u8 m_alm_fast;
	#endif
}service_t;

extern service_t  service;

extern void vTaskUart(void);

typedef struct {                                                // object data type
  u8  m_tx_buf[UART_TX_LEN];		//缓冲
  u16 len;							        //长度

  u8 m_timeout10ms;
  u8 m_wait_cmd;                //等待回复命令

  u8 m_flag_set;                //触发立即发送
  u8 m_heart10ms;  
  u8 m_tx_cnt;

  u8 isSleep;

  u8 m_ErrCnt;
	
	u8 isGet;		//获取到数据
}QUEUE_OBJ_t;

extern QUEUE_OBJ_t 	TouchSend;		



typedef struct{			
	u8 bAlmTimeOut1s;			//超时恢复
	u8 bAlmTempTimeOut1s;
	u8 bAlmTestTimeOut1s;
	
	u16 bAlmSndTimeOut1s;
	u16 bTempSndTimeOut1s;

	u16 bAlmSndlast;
	u16 bTempSndlast;

	u8 muffTimeOut1s;			//消音90s后恢复(暂时不考虑)
	u8 bBellTimeOut1s;			//90s自动消音
	//使能读取值
//	u8 flagReadSmokeVal;
//	u8 flagReadBatVal;
//	u8 flagReadTempVal;
	
	//使能读取参数(开机时需要读取)
//	u8 flagReadSmoke;
//	u8 flagSetSmoke;
	#if SET_NB||SET_CAT1
	u16 bNetTimeOut1s;		//		
	u8 isNbNetOk;
	
	u16 bMsg;
	u16 bRxMsg;	
	
	u16 bBatVal;			//电池电压
	u8 isBattLow;
	u8 isBattRead;
	#endif
		
	u8 bTest;			//测试按键
	u8 bTempAlmSet;
	u8 bTempRstSet;
	
	u16 bheart;			//心跳时间
	
	u8 isBattLowSnd;
	u8 flagTestMode;	
	
	u8 bEventId;			
	u8 bSmokeAlmId;		//烟雾报警事件ID
	u8 bTempAlmId;		//温度报警事件ID
	
	u8 bAlmSta;			//报警状态，主要取暂停报警状态
	#define	BIT_SMOKE_ALM		0x01
	#define	BIT_TEMP_ALM		0x02
	#define	BIT_BAT_ALM			0x04
	#define	BIT_CHECK				0x08
	#define	BIT_SMOKE_STOP	0x10
	#define	BIT_TEMP_STOP		0x20
	
	u8 isFcOpen;	
	u8 bFcTimeOut1s;
	
	u8 bAlm1s;		//
	
	#if SET_NB||SET_CAT1
	u8 bEventSndId;
	u8 bDevTimeOut1s;
	#endif

	#if SET_NB||SET_CAT1
	u8 isBd;
	#endif
	
	u8 isNeedRstDevice;
	u8 isNeedRstMcu;
	
}CbSta_InitTypeDef;

extern CbSta_InitTypeDef  g_cbSta;

extern void TX2_write2buff(u8 dat);
extern void PrintString2(u8 *put);
extern void TouchSndCmd(u8 cmd);

#if SET_NB||SET_CAT1
typedef struct
{ 
	u16	TX_read;		//发送读指针
	u16	TX_write;		//发送写指针
	u8	B_TX_busy;		//忙标志
	
	u8  TX_TimeOut;		//发送超时

	u16 RX_Cnt;			//接收字节计数
	u8	RX_TimeOut;		//接收超时
	u8	B_RX_OK;		//接收块完成
} COMx_Define; 
#define COM_TX1_Lenth     1024
#define COM_RX1_Lenth	  	1024

//#define	TimeOutSet1			20
#define	TimeOutSet1			200

extern	COMx_Define  COM1;
extern	u8	 TX1_Buffer[COM_TX1_Lenth];	//发送缓冲
extern	u8 	 RX1_Buffer[COM_RX1_Lenth];	//接收缓冲
extern void TX1_write2buff(u8 dat);	//写入发送缓冲，指针+1
#endif

extern u8 pIsAlmNull(void);
extern void formatInfo(void);

#endif



