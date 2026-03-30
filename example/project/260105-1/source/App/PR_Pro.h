/*
 * @Author: dujianhuang
 * @Date: 2019-08-30 15:24:46
 * @LastEditors: leodjh1 33412386+leodjh1@users.noreply.github.com
 * @LastEditTime: 2026-03-30 10:15:09
 */

#ifndef _PR_PRO_H_
#define _PR_PRO_H_

#include "config.h"

#define	LORA_SLEEP_TIME		15

/*通讯包常数定义 */

//主机命令编码
#define ACK_SLAVE_WORKHAND					0x00			// 模块握手命令应答
#define ACK_SLAVE_STATUS					0x01			// 模块状态改变应答
#define ACK_SLAVE_KEEP						0x02			// 模块心跳应答
#define ACK_SLAVE_WIRELESS					0x03			// 发送无线数据应答

//#define READ_SLAVE_STATUS					0x14			// 读从机状态
#define READ_SLAVE_LAST_nDATA2				0x16			// 读最后n条无线数据并清除新状态位
#define READ_SLAVE_VER						0x17			// 读取模块版本日期


//模块命令编码
#define SEND_SLAVE_WORKHAND					0x80			// 模块握手命令
#define SEND_SLAVE_STATUS					0x81			// 模块状态改变
#define SEND_SLAVE_KEEP						0x82			// 模块心跳
#define SEND_SLAVE_WIRELESS					0x83			// 发送无线数据
//#define SEND_SLAVE_485_DATA					0x84			// 发送485数据应答
#define SEND_SK5110L_WIRELESS				0x85			// SK5110L专用无线数据命令
//#define SEND_SLAVE_STATUS_TEMP				0x8f			// 模块状态改变
//#define WRITE_SLAVE_BROADCASS_ACK			0xFF			// 广播命令

#define SEND_MAIN_TO_SMART                 0x30    //时刻主机下发智能设备信息  9字节数据(6字节设备uid +1字节设备类型+1字节第二层命令+1字节状态)
#define CMD_INFRARED_DOWM                  0x31     //时刻红外控制器下行
#define CMD_INFRARED_DOWM_T                0x32     //时刻红外控制器主动上行应答
#define SEND_SMART_TO_MAIN                 0xB0    //时刻智能设备上报信息 3字节数据(一字节设备类型，一字节第二层命令，一字节状态)
#define CMD_INFRARED_UP                    0xB1     //时刻红外控制器上行
#define CMD_INFRARED_UP_T                  0xB2     //时刻红外控制器主动上行


/*常量定义*/


#define SMART_PACK_SIZE                           13//上报数据    10个基本长度+3数据(一字节设备类型，一字节第二层命令，一字节状态)
                                                            //设备类型 0x01智能插座，0x02智能开关
                                                            //命令 0x00状态上报，0x01应答
                                                            //状态 0x00开，0x01关
//0x30下第二层命令  
#define CMD_SMART_QUERY                     0x01    //主机主动查询
#define CMD_MAIN_ACK                        0x02    //主机应答
#define CMD_SMART_LEARN_ACK                 0x03    //学习反馈主机地址
#define CMD_SMART_CONTROL                   0x04    //主机控制
#define CMD_SMART_SYNC                      0x05    //主机发送同步命令
//0xB0下第二层命令  
#define CMD_SMART_ACK                       0x01    //应答
#define CMD_SMART_STATUS                    0x02    //上报状态
#define CMD_SMART_LEARN                     0x03    //学习状态

#define CMD_SMART_EDEE_FIX                  0x06    //随意贴/智能遥控 控制
#define CMD_SNART_START_UP                  0x07    //设备上电报告状态

#define CMD_SMART_QUERY_RSSI                      0x0B    //主机查询双方LoRa RSSI
#define CMD_SMART_QUERY_RSSI_ACK                      0x0B    //主机查询双方LoRa RSSI 回复

// 下行
#define SEND_SOCK_DATA			0x06
#define SEND_MASTER_DATA_ACK		0x07
#define SEND_SOCK_REGEDIT_ACK		0x08			
#define SEND_SOCK_DM_ACK			0x30			// 主机对码应答，分配主机UID和防区号

// 上行
#define SEND_SOCK_DATA_ACK		    0x86			// 应答
#define SEND_MASTER_DATA			0x87			// 指纹锁主板数据转发
#define SEND_SOCK_REGEDIT			0x88			// 指纹锁开机报告
#define SEND_SOCK_DM				0xB0			// 进入对码状态，定时2秒发送，连续发送15次；


//指纹锁命令
#define HEART_PACK				0x00		// 心跳事件
#define EVENT_UP				0x03		// 开门事件
#define LEN_TYPE_LORA   		10//LORA协议格式长度


//==============================下行	
//---------------第一层命令	
	#define Down_Gateway_Response_WithDate 	0x70
	#define Down_Gateway_Response_NoDate   	0x71
	#define Down_Gateway_Request_DeviceAck   0x72
	#define Down_Gateway_Request_DeviceDate  0x73	
//---------------第二层命令	
	
//=============================数据上行
//----------------第一层命令
	#define Up_Device_Initiative_NeedPara 	  0xF0
	#define Up_Device_Initiative_NoNeedPara   0xF1
	#define Up_Device_Response_NoNeedPara	  0xF2
	#define Up_Device_Response_NeedPara       0xF3

	#define	Up_SHIKE_PROTO					  0x83
	#define	Up_SHIKE_PROTO_NEW				0x84
//----------------第二层命令
	#define Up_Device_StartUp  				0x00
	#define Up_Device_Heartbeat				0x01
	#define Up_Device_Event					0x02
	#define Up_Device_Status				0x03
	#define Up_Device_AddResult				0x04
	#define Up_Device_Mute					0x05
	#define Up_Device_ForbiddentMode		0x06
	#define Up_Device_AddUserMode			0x07
	#define Up_Device_unlocking				0x08	
	#define Up_Device_NormalOpenMode		0x09	
	#define Up_Device_DeleteUser			0x0A	
	#define Up_Device_GetUserInfo			0x0B
	#define	UP_Device_LoraWake				0x0c
	#define	UP_Device_SetUser				0x0d
	#define	UP_Device_GetCard				0x0e
	#define	UP_Device_FactoryReset			0x0f	
//------下发指令
	#define	DOWN_SET_MODE					0x81
	#define	DOWN_SET_SMOKE				0x82
	#define	DOWN_SET_BAT					0x83
	#define	DOWN_SET_HIGHTEMP			0x84
	#define	DOWN_SET_LOWTEMP			0x85
	#define	DOWN_SET_HEART				0x86
	#define	DOWN_SET_ALL					0x8f
//------下发指令应答
	#define	UP_ACK_COMMAND	0x91

		/* 防区事件上报 */
#define ZONE_EVT_CLOSE   0x00//关门/拔卡
#define ZONE_EVT_OPEN   0x01//开门/插卡
#define ZONE_EVT_LOW_POWER   0x03//低压上报
#define ZONE_EVT_START_UP   0x04//开机
// #define ZONE_EVT_   0x05//防拆拆开
// #define ZONE_EVT_   0x06//防拆恢复
// #define ZONE_EVT_   0x07//防区恢复
// #define ZONE_EVT_   0x08//防区短路
// #define ZONE_EVT_   0x09//防区开路
#define ZONE_EVT_ALARM   0x0A//设备报警
// #define ZONE_EVT_   0x0B//设备消警
// #define ZONE_EVT_   0x0C//设备布防
// #define ZONE_EVT_   0x0D//设备撤防
#define ZONE_EVT_HEARTBEAT   0x30//心跳

/*全局变量声明*/
//-----固件版本
typedef struct
{
	u8 str[8]; 
}sTypedefFirmwareVersion;
extern const	sTypedefFirmwareVersion FirmwareVersion;
extern uint8  iMaster_Con_Num;		// 主机通讯标号 

void func_report_device_info(u8 cmd1,u8 cmd2,u8 itimes);

/*外部函数声明*/										  	
extern void App_Rec_Parser(uint8 *p);  								/* 用户接收数据解析 */
extern uint8 App_Send_Para(uint8 iCMD,uint8 iLength,uint8 *iDat);	/* 发送数据函数 */
//extern uint8 App_Send_CMD_Process(uint8 iCMD);					/* 用户发送命令 */

extern uint8 LoRa_Ready_send(void);

extern u8 creatLoRaHead(u8 *buf);//添加loRa协议共用头部分
extern u8 lock_event_up_ack(u8 pack_no);
extern uint8 iSlave_Addr;
extern uint8  loraUid[6];
extern void App_Rec_CMD_Process(uint8 *p,uint8 iLength,uint8 iRssi);

extern u8 LoRaProtoUpCheck(u8 *buf);
extern void fDealEventQueuePro(void);
#endif




