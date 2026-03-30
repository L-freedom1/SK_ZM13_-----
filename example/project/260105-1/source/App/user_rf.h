/*
 * @Author: your name
 * @Date: 2021-10-08 17:12:50
 * @LastEditTime: 2021-10-12 14:18:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\user_rf.H
 */

#ifndef __USER_RF_H__
#define __USER_RF_H__

/* 使用通用透传命令 */
#define USE_COM_TRAN    0



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
#define SEND_SMART_TO_MAIN                 0xB0    //时刻智能设备上报信息 3字节数据(一字节设备类型，一字节第二层命令，一字节状态)

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
//状态，开关
#define STATUS_OPEN                         0x01
#define STATUS_CLOSE                        0x00

void p_user_rx_cb(unsigned char *rx_buf,unsigned short len, unsigned char rssi);
void user_rf_learn_scan(void);
#endif

