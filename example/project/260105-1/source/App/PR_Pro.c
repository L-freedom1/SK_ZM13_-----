/*
 * @Author: dujianhuang
 * @Date: 2019-08-30 15:24:46
 * @LastEditors: leodjh1 33412386+leodjh1@users.noreply.github.com
 * @LastEditTime: 2026-03-30 14:02:37
 */
/**********************************************************/
/*  公司名称：泉州时刻有限公司     			              */
/*  开发人员：XTQ									      */
/*  实现功能：通讯协议用户处理文件				          */
/**********************************************************/

#include "..\APP\config.h"



#if  SET_BLE_SUB||(SET_PROTO_VER==PROTO_VER_ZH202)
/*函数定义*/
void App_Rec_Parser(uint8 *p);					   /* 用户接收数据解析 */
//void App_Rec_CMD_Process(uint8 *p, uint8 iLength); /* 接收命令处理*/
void App_Rec_CMD_Process(uint8 *p,uint8 iLength,uint8 iRssi);	
//uint8 App_Send_CMD_Process(uint8 iCMD);					/* 用户发送命令 */
uint8 App_Send_Para(uint8 iCMD, uint8 iLength, uint8 *iDat); /* 发送数据函数 */
//static void Get_Wireless_Data(uint8 *p,uint8 iW_Start,uint8 iW_Num);/* 获取无线/刷卡数据到发送缓存区 */
void SHIKE_Willess_Send(void);		  /* 时刻无线数据发送 */
u8 deal_lock_dowm(u8 *data1, u8 len); /* 处理指纹锁下行 */

uint8 IsExist(uint8 *uid);
/*测试显示定义*/

#if 1
/*全局变量定义*/
uint8  iMaster_Con_Num = 0; // 主机通讯标号
uint8  iSlave_Addr = 0;
uint8 iMaster_RepeNum = 0;
uint8  loraUid[6] = {0};

/*静态变量定义*/
#if SET_SPI
const sTypedefFirmwareVersion FirmwareVersion = {"260123.1"};
#else
const sTypedefFirmwareVersion FirmwareVersion = {"230522.1"};
#endif
/**
* @brief  添加loRa协议共用头部分
* @param  
* @retval None
*/
u8 creatLoRaHead(u8 *buf)
{
	u8 len = 0;
	u8 i = 0;

	buf[len++] = ++RF_Num;	//帧序号
	
	EEPROM_Ptr[RF_NUM_ADDR] = RF_Num;
	user_fs_updata_memory();
	
	for (i = 0; i < 6; i++) //设备uid
	{
		buf[len++] = g_device_info.s_cpu_id[i];
	}

	buf[len++] = 0xfa;
	return len;
}
/**
* @brief  LoRa上行协议校验值计算
* @param  
* @retval None
*/
u8 LoRaProtoUpCheck(u8 *buf)
{
	u8 bSum = 's' + 'k'; // 校验
	u8 i = 0;
	for (i = 0; i < buf[1]; i++)
	{
		bSum += buf[1 + i];
	}
	bSum = ~bSum;
	return bSum;
}

#if USE_SX1262
/* 用户接收数据解析 */
// //  *p    *(p+1)  *(p+2)~*(p+5) *(p+6)~*(p+7) *(p+8) *(p+9)  *(p+10)～
// //帧长度 通讯标号 设备类型地址	  主机编号   分机地址  命令	  数据/校验
void App_Rec_Parser(uint8 *p)
{
	uint8 i, iVer = 0, iLen = *p;
	static u8 Con_Repeat_Num = 0xff;

	//---------有效长度4
	if ((iLen < 10) || (iLen > 64))
	{
		return;
	}
	// 主机机地址－－ 不处理；

	//wMaster_Addr = (uint16)(*(p+6))*256+*(p+7);

	//----------校验判断5
	for (i = 0; i < iLen; i++)
	{ // 帧长度＋通讯标号到数据段所有数据求和，
		iVer += *(p + i);
	}
	//UART_PutStr("r0");
	// 主机命令7E/FF应答校验+'S'+'K'
	iVer += 'S';  // 加S
	iVer += 'K';  // 加K
	iVer = ~iVer; // 取反
	if (iVer != *(p + iLen))
	{
		//UART_PutStr("校验错误\r\n");
		return;
	}
	
	if(IS_PROTO_SHIKE)
	{
		if(IsExist(p+2))
		{
			#if USE_SX1262	
			g_rf_info.is_wait_ack_cmd = 0;
			g_rf_info.re_send_times = 0;	

			g_rf_info.re_plus_times = 0;
			g_rf_info.isLost = 0;
			#endif	
		}
		return;
	}
	
	//有效重复包判断
	if(IsExist(p+2))
	{
		if(Con_Repeat_Num == *(p+1))
		{
			iMaster_RepeNum = 1;
		}
		else
		{
			iMaster_RepeNum = 0;
		}
		
		Con_Repeat_Num = *(p+1);
	}
	else
	{
		iMaster_RepeNum = 0;
	}
	
	// 正常通讯包处理
	iMaster_Con_Num = *(p + 1);				   //====获取通讯标号
	memcpy(loraUid, (p + 2), 6);			   //====获取下行uid
	iSlave_Addr = *(p + 8);					   //====获取模块地址
	g_rf_info.is_lora_now = 1;
	if(iMaster_RepeNum)
	{
		//直接回复
		memset(iRF_TX_Buf,0,sizeof(iRF_TX_Buf));
		memcpy(iRF_TX_Buf,iRF_TX_Buf_bak,sizeof(iRF_TX_Buf));
		iStatus_RF = RFM98_RX_TO_TX;
	}
	else
	{
		App_Rec_CMD_Process((p + 9), (iLen - 10),0xff); //普通命令包处理
	}
}
#endif

u8 user_isnEqual(u8 *array1, u8 *array2, u16 awsize)
{
	u16 i = 0;
	for (i = 0; i < awsize; i++)
	{
		if (array1[i] != array2[i])
			return 0;
	}
	return 1;
}

uint8 IsExist(uint8 *uid)
{
	if (user_isnEqual(g_device_info.s_cpu_id, uid, 6))
	{
//		if(sleepTimer<3000)
//		{
//			sleepTimer = 3000;			//继续唤醒等3s
//		}
		return 1;
	}
	#if SET_CAT1==0
	else if(user_isnEqual(&g_device_info.s_cpu_id[4], &uid[4], 2)&&(IS_PROTO_SHIKE))
	{
		return 1;
	}
	#endif
	else
	{
		return 0;
	}
}

//----上报不带参数
void fAckHost(u8 cmd1)
{
	g_readySendBuf[SITE_CMD] = cmd1; //命令编码
	g_readySendBuf[SITE_LEN] = 0;	 //数据长度
}

void pAddSendBuf(u8 bId,u8 bEvent)
{
	u8 len = 0;
	u8 event_id = 0xFF;
	// unsigned char v_temp_len = 0;//V字段数据长度

	// 报警参数1字节
	switch (bEvent)
	{
	case EVENT_TEST:
		event_id = ZONE_EVT_ALARM;
		break;
	case EVENT_ALM:
	// case EVENT_TEMP:
		event_id = ZONE_EVT_ALARM;
		break;
	case EVENT_BAT:
		event_id = ZONE_EVT_LOW_POWER;
		break;
	case EVENT_FCALM:
		event_id = ZONE_EVT_ALARM;
		break;
	case EVENT_FCRST: // 防拆恢复
	case EVENT_PLUS:
	case EVENT_BOOT:
	case EVENT_PLUS_AG:
	default:
		event_id = ZONE_EVT_HEARTBEAT;
		break;
	}
	
	if(event_id != 0xFF)
	{
		g_readySendBuf[SITE_CMD] = T_ZONE_EVENT;
		g_readySendBuf[SITE_DATA_STAT + len++] = T_ZONE_EVENT;
		g_readySendBuf[SITE_DATA_STAT + len++] = 2;
		g_readySendBuf[SITE_DATA_STAT + len++] = DEVICE_ID; // 不关心设备类型
		g_readySendBuf[SITE_DATA_STAT + len++] = event_id;
		g_readySendBuf[SITE_LEN] = len; //数据长度	
	
	}
	else
	{
		g_readySendBuf[SITE_CMD] = 0;
	}

#if USE_SX1262
		g_rf_info.is_wait_ack_cmd = g_readySendBuf[SITE_CMD]; // 需要等待回复
		g_rf_info.re_send_times = 3;						  // 重发次数
#endif
}
#if 0
/*typedef struct
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
}R_SM;*/
u8 sum_check(u8 *buf,u8 len)
{
	u8 i;
	u8 check = 0;
	for(i=0;i<len;i++)
	{
		check += buf[i];
	}
	return check;
}
void spi_tran_ble(void)
{
    unsigned char tx_buf[50];
    unsigned char len = 0;
	u8 chk = 0;
	len = 1;
	tx_buf[len++] = device_info.zero_data;
	tx_buf[len++] = device_info.ir_data;
	tx_buf[len++] = device_info.ir_ad;
	tx_buf[len++] = device_info.bd_zero;
	tx_buf[len++] = service.m_smoke_set_val;
	tx_buf[len++] = device_info.vdd_adc;
	tx_buf[len++] = device_info.t_adc;
	tx_buf[len++] = service.m_smoke_set_val;
	
    tx_buf[SITE_ALL_LEN] = len - 1;
	//tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);
    tx_buf[len - 1] = sum_check(tx_buf, len - 1);
    rf_tx(tx_buf,  len,0);
}
#endif

void fDealEventQueuePro(void)
{
	QUEUE_OBJ_Event  eventbuf;
	if ((!Event_IsEmpty())	//有事件在队列
		&& ((note_cur_is_send()==0) 
	&& (g_readySendBuf[SITE_CMD] == 0) && (g_rf_info.re_send_times==0))							 //没有正在发送数据
	)
	{ 
		eventbuf = Event_PopEvent();
		
		// #if SET_SPI
		// if(eventbuf.bEvent==EVENT_SPI_TEST)
		// {
		// 	LOG("透传测试\r\n");
		// 	spi_tran_ble();
		// }
		// else
		// {
		// 	LOG("Deal Event\r\n");
		// 	pAddSendBuf(eventbuf.bId,eventbuf.bEvent);
		// }
		// #else
		LOG("Deal Event\r\n");
		pAddSendBuf(eventbuf.bId,eventbuf.bEvent);
		//#endif
	}
}

/* 接收命令处理 */
//  *p    *(p+1)～
//  命令   数据
// iLength：数据长度
//void App_Rec_CMD_Process(uint8 *p, uint8 iLength)

#define POS_DOWN_DATA	4
void App_Rec_CMD_Process(uint8 *p,uint8 iLength,uint8 iRssi)
{
	//uint8 i;
	// uint8 xdata iCMD_Temp = *p;	  	// 命令编码缓存
	u8 FirstCmd = *p;
	//u8 DeviceType = *(p + 1);		
	//u8 SecondCmd = *(p + 2);
	u8 SecondCmd = *(p + 3);
	//	if(DeviceType == DEVICE_ID)
	{
		switch (FirstCmd)
		{						 //第一层命令
								 //--------------不管设备是否学习--------------//
		case SEND_MAIN_TO_SMART: //0x30   学习专用
		{
			if (iLength != 3)
				break;
			if (IsExist(loraUid) == 0)
				break; //未学习并且不在学习状态
			{
				u8 *data1 = p + 1;
				if (iSlave_Addr != 0xFF)
				{
					//如果没学习且不在学习状态则推出
					if (0xfa != iSlave_Addr && g_device_info.learn_status != LEARN_STATUS_LEARN)
						break; //return 0;
					if (data1[0] != DEVICE_ID)
						break; //return 0;
				}
				if (data1[1] == CMD_SMART_LEARN_ACK) //case CMD_SMART_LEARN_ACK:
				{
					if(iSlave_Addr == 0xFF)
						break;
					g_device_info.learn_status = LEARN_STATUS_COM;

					g_readySendBuf[SITE_CMD] = SEND_SMART_TO_MAIN;		//命令编码
					g_readySendBuf[SITE_LEN] = 3;						//数据长度
					g_readySendBuf[SITE_DATA_STAT] = DEVICE_ID;			//设备类型
					g_readySendBuf[SITE_DATA_STAT + 1] = CMD_SMART_ACK; //回复
					g_readySendBuf[SITE_DATA_STAT + 2] = 0x00;
									
					UART_PutStr("主机应答学习\r\n");	
					
					if(g_rf_info.is_lora_now)
					{
						g_rf_info.is_lora_ack = 1;
					}
					else
					{
						g_rf_info.is_lora_ack = 0;
					}
					#if SET_BAT_WORK
					#else
					g_device_info.DeviceStatus.b.is_need_heart_report = 1;
					#endif
				}				
			}
		}
		break;
			//--------------需要设备学习--------------//

			//			if(IsExist(loraUid) == 1)
			//			{
		case Down_Gateway_Response_WithDate: //0x70应答都是统一的可能
		{									 //应答下行探头，带有数据需要解析//---开机/心跳/电子锁事件上报
			if (IsExist(loraUid) == 0)
				break;

			#if USE_SX1262	
			g_rf_info.is_wait_ack_cmd = 0;
			g_rf_info.re_send_times = 0;	
			#endif	

			if(iLength<4)
			{
				//有数据需要接收，需要延长唤醒一定时间	
				UART_PutStr("主机应答有数据需下发\r\n");
				g_rf_info.is_wait_ack_cmd = 0;
				g_rf_info.re_send_times = 0;
				
				sleepTimer = 5000;					//5s
			}
			else
			{
				{													//同步网络时间
					//memcpy(&g_device_info.UnixTimestamp, p + 1, 4); //p_proc_cclk(p+1);
					//同步网络时间，需要改成小端模式
					g_device_info.UnixTimestamp = GETU32_Big(p + 1);
				}
				UART_PutStr("主机应答\r\n");
				UART_PutStr("syn_sys_unix:");
				user_printf_hex((u8 *)(&g_device_info.UnixTimestamp), 4);
			}
			
		}
		break;

		case Down_Gateway_Response_NoDate: 		//0x71
		{								   		//应答下行探头，没有数据需要解析
			if (IsExist(loraUid) == 0)
				break;
			//有数据需要接收，需要延长唤醒一定时间	
			UART_PutStr("主机应答有数据需下发\r\n");
			g_rf_info.is_wait_ack_cmd = 0;
			g_rf_info.re_send_times = 0;
			
			sleepTimer = 5000;					//5s 
		}
		break;

		case Down_Gateway_Request_DeviceDate: 	//0x73
		{									  	//主动下行需要探头应答带数据
			if(IS_PROTO_SHIKE)
				break;
			
			if (IsExist(loraUid) == 0)
				break;
			
			sleepTimer = 5000;
			
			switch (SecondCmd)
			{
				case DOWN_SET_MODE:
					if(p[POS_DOWN_DATA]<=2)
					{
						
						
						//暂停报警操作
						if(p[POS_DOWN_DATA] ==2)
						{
							service.m_muffling = p[POS_DOWN_DATA];
							
							if(service.m_smoke_state)
							{
								g_cbSta.bAlmSta |= BIT_SMOKE_STOP;
							}
							
							if(service.m_temp_state)
							{
								g_cbSta.bAlmSta |= BIT_TEMP_STOP;
							}
							LOG("暂停报警\r\n");
						}
						else if(p[POS_DOWN_DATA]==1)
						{	
							
							service.m_muffling = p[POS_DOWN_DATA];
							g_device_info.DeviceStatus.b.is_mute = 1;

							g_cbSta.muffTimeOut1s = 0;	
							LOG("远程消音\r\n");
						}
						#if SET_RST_DEVICE
						else if(p[POS_DOWN_DATA]==3)
						{
							//重启传感器
							g_cbSta.isNeedRstDevice = 1;
							if(WatchDogFlagFast==0)
							{
								WatchDogFlagFast = 1;
								osal_stop_timerEx(watchdog_TaskID, WATCHDOG_1000MS_EVENT);
								osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, 1000);
								LOG("切换1s唤醒\r\n");
							}
							
							//重启芯片
							g_cbSta.isNeedRstMcu = 1;
						}
						#endif
					}
					break;
				case DOWN_SET_SMOKE:
					if((p[POS_DOWN_DATA]<=100)&&(p[POS_DOWN_DATA]>=10))
					{
						service.m_smoke_set = p[POS_DOWN_DATA];
						//user_fs_isNeedSave = 1;
						//230427:通过比对上报阈值来决定设置与否
						//TouchSend.m_flag_set = 1;
						user_fs_isNeedSave = 1;
					}
					break;
				case DOWN_SET_BAT:
					//
					if((GETU16(p+POS_DOWN_DATA)>2200)&&(GETU16(p+POS_DOWN_DATA)<3000))
					{
						service.m_battery_set = GETU16(p+POS_DOWN_DATA);
						user_fs_isNeedSave = 1;
					}
					break;
				case DOWN_SET_HIGHTEMP:
					memcpy(&service.m_temp_almset,&p[POS_DOWN_DATA],2);
					user_fs_isNeedSave = 1;
					break;
				case DOWN_SET_LOWTEMP:
					memcpy(&service.m_temp_lowset,&p[POS_DOWN_DATA],2);
					user_fs_isNeedSave = 1;
					break;
				case DOWN_SET_HEART:
					if(p[POS_DOWN_DATA]!=0)
					{
						service.m_heartbeat_time = p[POS_DOWN_DATA];
						user_fs_isNeedSave = 1;
					}
					break;
				case DOWN_SET_ALL:
					//memcpy(&service.m_smoke_set,&p[POS_DOWN_DATA],SIZE_PARA);
					//结构体可能内存未对齐，包括大小端等，还是一个个拷贝
					if((p[POS_DOWN_DATA]<=100)&&(service.m_smoke_set>=10))
					{
						service.m_smoke_set = p[POS_DOWN_DATA];
						user_fs_isNeedSave = 1;
					}
					if((GETU16(p+POS_DOWN_DATA+1)>2200)&&(GETU16(p+POS_DOWN_DATA+1)<3000))
					{
						service.m_battery_set = GETU16(p+POS_DOWN_DATA+1);
					}				
					memcpy(&service.m_temp_almset,&p[POS_DOWN_DATA+3],2);
					memcpy(&service.m_temp_lowset,&p[POS_DOWN_DATA+5],2);
					service.m_heartbeat_time = p[POS_DOWN_DATA+7];		
					
					//230427:通过比对上报阈值来决定设置与否
					//TouchSend.m_flag_set = 1;
					user_fs_isNeedSave = 1;
					break;
				default:
					break;
			}
			
			//下发指令应答
			//Event_AddEvent(SecondCmd|0x40);	
			g_cbSta.bEventId++;
			if(g_rf_info.is_lora_now)
			{
				g_rf_info.is_lora_ack = 1;
			}
			else
			{
				g_rf_info.is_lora_ack = 0;
			}
			pAddSendBuf(g_cbSta.bEventId,SecondCmd|0x40);	
			UART_PutStr("下发指令应答\r\n");
		}
		break;
			//			}
			//			else
			//			{
			//					UART_PutStr("类型不对\r\n");
			//			}
			//--------------设备其他状态或者其他未知命令--------------//
		default:
		{
		}
		break;
		}
	}
}


#if USE_SX1262
/*
* @brief:   LoRa准备发射，数据编辑
* @paramp:  
* @return:
*/
uint8 LoRa_Ready_send(void)
{
	if (iStatus_RF == RFM98_DATA_RX || iStatus_RF == RFM98_SLEEP)
	{
		//			{
		//				UART_PutStr("LORA准备发送\r\n");
		//				 if(iStatus_RF == RFM98_DATA_RX)
		//						UART_PutStr("RF接收\r\n");
		//				 else
		//					  UART_PutStr("RF休眠\r\n");
		//			}
		memset(iRF_TX_Buf, 0x00, sizeof(iRF_TX_Buf));

		iRF_TX_Buf[1] = g_readySendBuf[SITE_LEN] + LEN_TYPE_LORA; //帧长度
		iRF_TX_Buf[0] = iRF_TX_Buf[1] + 1;
		
		//221118:lora和2.4g采用同一个包序
		#if SET_BLE_SUB
		iRF_TX_Buf[2] = (g_note.tx_num%64)|0x80;
		#else
		if(g_isRfRetry)
		{
			g_isRfRetry = 0;
			iRF_TX_Buf[2] = RF_Num;//通信表标识
		}
		else
		{
			iRF_TX_Buf[2] = ++RF_Num;//通信表标识
			
			EEPROM_Ptr[RF_NUM_ADDR] = RF_Num;
			user_fs_updata_memory();
		}
		#endif

		memcpy(&iRF_TX_Buf[3], g_device_info.s_cpu_id, 6);
		// for(i = 0; i < 6; i++)//设备uid
		// {
		//     iRF_TX_Buf[3+i] = g_device_info.s_cpu_id[i];
		// }

		iRF_TX_Buf[9] = 0xfa;
		iRF_TX_Buf[10] = g_readySendBuf[SITE_CMD];		 //命令编码

		//数据
		memcpy(&iRF_TX_Buf[11], &g_readySendBuf[SITE_DATA_STAT], g_readySendBuf[SITE_LEN]);

		// bSum = 's' + 'k'; // 校验
		// for (i = 0; i < iRF_TX_Buf[1]; i++)
		// {
		// 	bSum += iRF_TX_Buf[1 + i];
		// }
		// iRF_TX_Buf[iRF_TX_Buf[1] + 1] = ~bSum;
		iRF_TX_Buf[iRF_TX_Buf[1] + 1] = LoRaProtoUpCheck(iRF_TX_Buf);
		
		//211129:备份一份回复包数据
		if((iRF_TX_Buf[10]==0xf3)||(iRF_TX_Buf[10]==0xf2))
		{
			memset(iRF_TX_Buf_bak, 0x00, sizeof(iRF_TX_Buf_bak));
			memcpy(iRF_TX_Buf_bak,iRF_TX_Buf,sizeof(iRF_TX_Buf));
		}
		//学习回复包也需要备份
		else if((iRF_TX_Buf[10]==0xb0)&&(iRF_TX_Buf[12]==0x01))
		{
			memset(iRF_TX_Buf_bak, 0x00, sizeof(iRF_TX_Buf_bak));
			memcpy(iRF_TX_Buf_bak,iRF_TX_Buf,sizeof(iRF_TX_Buf));
		}

		iStatus_RF = RFM98_RX_TO_TX; // 发送数据

		//221115:统一清空
		//g_readySendBuf[SITE_CMD] = 0x00;

		//心跳等待唤醒时间统一1.5秒即可
		g_rf_info.wait_ack_time = 15;

		

		// UART_PutStr("TX: ");
		// for (i = 0; i < iRF_TX_Buf[1] + 1; i++)
		// {
		// 	UART_PutHex(iRF_TX_Buf[1 + i]);
		// 	UART_PutStr(" ");
		// }
		// UART_PutStr("\n\r");
		
	}
	else
		return 0;
	return 1;
}
#endif


#endif

#endif
