/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:35:08
 * @LastEditors: leodjh1 33412386+leodjh1@users.noreply.github.com
 * @LastEditTime: 2026-03-30 14:03:21
 * @FilePath: \source\App\my_timer.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
    Filename:       timer_demo.c
    Revised:        $Date $
    Revision:       $Revision $


**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include "my_timer.h"

u16 bWatchDog1s;

u8 g_is10msTouch,g_is1sTouch;

#if USE_SX1262
u8 g_is100msTouch;
#endif

#if SET_BLE_LEARN
u8     	g_bWaitTimeOut10ms;
#endif

#if SET_NB||SET_CAT1
u16 g_bWait1ms;
u8 g_bWaitTimeOut1s;

void os_waitMs(u16 time_n)
{
	g_bWait1ms = time_n;
	while(g_bWait1ms!=0)
	{
		p_RunPro();
	}
}
#endif

//KEY
sKeyInfo Key[1];

//判断按键动作
void fKeyCheck(eTypedefEnum KeyType,u8 KeyCurSta) //10ms进入一次
{		
	static u8 flagGetKeyOff = 0;
	static u8 flagGetKeyOn = 0;
	
	//按下按键开机，特殊功能
	if((KeyCurSta == dKEY_ON) && (flagGetKeyOff==0))
	{	
		if(flagGetKeyOn<0xff)
			flagGetKeyOn++;
		
		if(flagGetKeyOn==100)
		{		
			#if (SET_NB||SET_CAT1)==0 && SET_SMART==0
			if(g_system_ota==0)
			{
				//LOG("进入对码模式\r\n");
				//g_device_info.learn_status = LEARN_STATUS_LEARN;
				
		
				LOG("进入工作模式设置\r\n");
				g_device_info.learn_status =LEARN_STATUS_SET;
				g_device_info.proto_set_1s = 5;
				//播报当前工作模式: 控电工作模式响一声；时刻报警器模式响二声
				PLAY_KEY();
				if(service.m_proto_set == PROTO_SHIKE)
				{
					g_device_info.Beep10ms = 50;
				}
				else if(service.m_proto_set == PROTO_SHIKE_PLUS)
				{
					g_device_info.Beep10ms = 50;
					g_device_info.BeepCnt = 1;
				}
				else
				{
					g_device_info.Beep10ms = 0;
				}
			}
			#endif
		}
		else if(flagGetKeyOn<100)
		{
			#if (SET_NB||SET_CAT1)==0 && SET_SMART==0
			if(g_system_ota==0)
			{
				//未进入工作模式之前，不蜂鸣不闪烁	
				g_device_info.learn_status = LEARN_STATUS_LEARN;		
			}
			#endif
		}
	}
	//开机需要检测到按键松开才正常按键检测
	else if((KeyCurSta == dKEY_ON) && (flagGetKeyOff>=20))
	{	//按键按下计数	

		if(Key[KeyType].TCount==0)
		{
			//DEBUG_LOG("T0\n");	
		}
		if(Key[KeyType].TCount <= 65535) //超过则停止计数 
		{
			//DEBUG_LOG("T");
			Key[KeyType].TCount++;
		}

		
	}
	
	//短按0.2s
	if(Key[KeyType].TCount == 20)
	{	
		Key[KeyType].Flag.b.ShortPress = 1;	
	}
	#if 0
	else if((Key[KeyType].TCount == 100)&&(g_device_info.RunTime<40))
	{
		Key[KeyType].Flag.b.LongPress = 1;
	}
	#endif
	else if(Key[KeyType].TCount == 1000)	
	{
		Key[KeyType].Flag.b.Long2Press = 1;
	}
	#if 0
	else if((KeyCurSta == dKEY_OFF)&& (Key[KeyType].TCount >= 10)&&(Key[KeyType].TCount < 190)) //超过2秒为长按: 2000/10=200 ,否则为短按
	{//按键松开 && 按下时间 5 < 200  ->短按
		Key[KeyType].Flag.b.ShortPress = 1;  		
	}
	#endif

	if(KeyCurSta == dKEY_OFF)
	{//置位松开标志
		if(flagGetKeyOff<0xff)
		{
			flagGetKeyOff++;
		}
		
		Key[KeyType].Flag.b.ReleasePress = 1;

		if(Key[KeyType].TCount!=0)
		{
			//DEBUG_LOG("R:%d\n",Key[KeyType].TCount);
		}
		Key[KeyType].TCount = 0;		//松开清计时	
	}	
}
#if SET_BLE_DEBUG

#define T_TRAN_DEBUG    0x0f

void note_set_ble_debug(void)
{
		/*填充内容*/
    u8 tx_buf[256];
	
    u16 len = 0;
    unsigned char tx_num = (++g_note.tx_num)%64;
    unsigned char v_temp_len = 0;//V字段数据长度
    u8 index;

    memset(tx_buf,0,sizeof(tx_buf));
    profile_set_up(tx_num);//设置上行包
   
    len++;//空余一下长度字段
    tx_buf[len++] = tx_num;
    memcpy(&tx_buf[len],g_note.device_uid,6);//发送方uid
    len += 6;
    
    memcpy(&tx_buf[len],g_note.gateway_uid,6);//注册网关uid
    len += 6;

    tx_buf[len++] = LOW_POWER_FLGA;//低功耗描述符
    tx_buf[len++] = 0x00;//预留2字节
    tx_buf[len++] = 0x00;

    tx_buf[len++] = T_TRAN_DEBUG;//命令
    tx_buf[len++] = 0;//没有数据长度0

    /* V字段 */
    v_temp_len = len;

    index = 0;

    //填充数据
		/*
				零点值、发射值、发射差值、标定值、烟雾浓度值、电池电压、温度
		*/
		// SM_LOG("烟腔故障 = %d \n",(u16)device_info.state.F_SM_ERR);
		// SM_LOG("标定状态 = %d \n",(u16)device_info.state.F_SM_BD);
		// SM_LOG("标定完成 = %d \n",(u16)device_info.state.F_SM_BD_OK);
		// SM_LOG("零点值 = %d \n",(u16)device_info.zero_data);
		// SM_LOG("发射值 = %d \n",(u16)device_info.ir_data);
		// SM_LOG("发射差值 = %d \n",(u16)device_info.ir_ad);
		// SM_LOG("标定值 = %d \n",(u16)device_info.bd_zero);

		sprintf((char *)&tx_buf[v_temp_len+index],"零点值:%d\r\n",device_info.zero_data);
    index = strlen((const char *)&tx_buf[v_temp_len]);

		sprintf((char *)&tx_buf[v_temp_len+index],"发射值:%d\r\n",device_info.ir_data);
    index = strlen((const char *)&tx_buf[v_temp_len]);

		sprintf((char *)&tx_buf[v_temp_len+index],"发射差值:%d\r\n",device_info.ir_ad);
    index = strlen((const char *)&tx_buf[v_temp_len]);

		sprintf((char *)&tx_buf[v_temp_len+index],"标定值:%d\r\n",device_info.bd_zero);
    index = strlen((const char *)&tx_buf[v_temp_len]);

		sprintf((char *)&tx_buf[v_temp_len+index],"烟雾浓度值:%d\r\n",service.m_smoke);
    index = strlen((const char *)&tx_buf[v_temp_len]);

		sprintf((char *)&tx_buf[v_temp_len+index],"电池电压:%dmV\r\n",service.m_battery);
    index = strlen((const char *)&tx_buf[v_temp_len]);
			
    sprintf((char *)&tx_buf[v_temp_len+index],"温度:%d℃\r\n",service.m_temp);
    index = strlen((const char *)&tx_buf[v_temp_len]);
		
		if(device_info.state.F_SM_ERR)
		{
			sprintf((char *)&tx_buf[v_temp_len+index],"烟腔故障\r\n");
			index = strlen((const char *)&tx_buf[v_temp_len]);
		}
		else
		{
			sprintf((char *)&tx_buf[v_temp_len+index],"烟腔正常\r\n");
			index = strlen((const char *)&tx_buf[v_temp_len]);
		}
		
		
		sprintf((char *)&tx_buf[v_temp_len+index],"Rcv %d:",spi_RcvLen);
		
		index = strlen((const char *)&tx_buf[v_temp_len]);
		if((spi_RcvLen>0)&&(spi_RcvLen<11))
		{
			for(u8 i=0; i<spi_RcvLen; i++)
			{
				if(i==5)
				{
					sprintf((char *)&tx_buf[v_temp_len+index],"%02x\r\n",spi_RcvBuf[i]);
				}
				else
				{
					sprintf((char *)&tx_buf[v_temp_len+index],"%02x ",spi_RcvBuf[i]);
				}
				index = strlen((const char *)&tx_buf[v_temp_len]);
			}
		}
		spi_RcvLen = 0;
		sprintf((char *)&tx_buf[v_temp_len+index],"\r\n");
		index = strlen((const char *)&tx_buf[v_temp_len]);
		
		//UID
		sprintf((char *)&tx_buf[v_temp_len+index],"UID:%02X%02X%02X%02X%02X%02X\r\n",g_note.device_uid[0],g_note.device_uid[1],g_note.device_uid[2],g_note.device_uid[3],g_note.device_uid[4],g_note.device_uid[5]);
		index = strlen((const char *)&tx_buf[v_temp_len]);
		
    //设备型号
    sprintf((char *)&tx_buf[v_temp_len+index],",T&YG003,V&260123");
    index = strlen((const char *)&tx_buf[v_temp_len]);


    //sprintf((char *)&tx_buf[len], "温度:%.1f℃/%.1f℉\r\n湿度:%d\r\n%s\r\n电池电压:%dmV/%d%%\r\n %s\r\nIMEI %s\r\n信号 %s防区数目:%d\r\n433M:%02X%02X%02X,T&ZU-201TH,V&%s",temp_f,g_wReadFah,g_ReadHumi,ac,bBattVolt*10,g_device_info.BatPercent,card,&g_bSIMID[0],sig,devNums,rf_buf[0],rf_buf[1],rf_buf[2],PROGRAM_VERSION);
    //sprintf((char *)&tx_buf[len],"%s\r\n%s\r\n信号 %s",ac,card,sig);
    len += strlen((const char *)&tx_buf[len]);

    if(len>254)
    {
        len = 254;
    }

    tx_buf[SITE_L] = len - v_temp_len;

    len++;//空一下校验字段
		tx_buf[SITE_ALL_LEN] = len - 1;
		tx_buf[len - 1] = profile_check(tx_buf, len - 1, PROFILE_UP);

		g_rf_send.is_need_send = 1;
		g_rf_send.resend_Times = 5;
		g_rf_send.wait_ack_time = COMM_RESEND_TIME/10; //重发间隔

		memcpy(g_rf_send.send_buf, tx_buf, len);
		g_rf_send.send_len = len;  
}
#endif
//根据按键动作进行处理
//对码：开机的前
void fKeyPro(void)
{
	if(Key[0].Flag.B > 0)
	{		
		//长按处理		
		if(Key[0].Flag.b.Long2Press == 1)
		{
			if(g_system_ota==0)
			{
				LOG("OTA ENABLE\r\n");
				g_system_ota = 1;
			}		
			else
			{
				LOG("OTA DISABLE\r\n");
				g_system_ota = 0;
			}
			WaitMs(1000);
			osal_snv_write(FILE_OTA_ID,1,&g_system_ota);
		
			hal_system_soft_reset();				
		}
		#if 0
		//开机前40s,长按1S可以进入场测模式
		else if(Key[0].Flag.b.LongPress == 1)
		{
			//开机前40s,长按进入对码
			if(g_device_info.learn_status == LEARN_STATUS_COM)
			{
				g_device_info.learn_status = LEARN_STATUS_LEARN;
				LOG("进入厂测模式\r\n");
				service.m_muffling = 0;
				g_cbSta.bFcTimeOut1s = 0; //防拆暂时可以取消，最多蜂鸣1分钟
				g_cbSta.bAlmSta &= ~BIT_CHECK;
			}
			else
			{
				g_device_info.learn_status = LEARN_STATUS_COM;
				LOG("退出厂测模式\r\n");
			}
			
			#if 0
			//同样需要自检上报？
			if((g_cbSta.bAlmSta&BIT_CHECK) ==0)
			{
				//不一定要判断自检成功
				//如果是查询，需要等获取到参数
				g_cbSta.bAlmSta |= BIT_CHECK;
				Event_AddEvent(EVENT_TEST);
				LOG("自检上报");
			}	
			#endif
		}	
		#endif
		else if(Key[0].Flag.b.ShortPress == 1)
		{
			if(g_system_ota==1)
			{
				LOG("OTA DISABLE\r\n");
				g_system_ota = 0;
							
				osal_snv_write(FILE_OTA_ID,1,&g_system_ota);
				hal_system_soft_reset();
			}
			else
			{
				if(g_device_info.learn_status == LEARN_STATUS_LEARN)
				{
					g_device_info.learn_status = LEARN_STATUS_COM;
					//LOG("退出对码");
				}
				//去掉取消消音
				#if 0
				//消音时取消消音
				else if(service.m_muffling==1)
				{
					service.m_muffling = 0;
					LOG("取消消音");
				}
				#endif
				#if (SET_NB||SET_CAT1)==0
				else if(g_device_info.learn_status == LEARN_STATUS_SET)
				{
					if(service.m_proto_set==PROTO_SHIKE)
					{
						service.m_proto_set = PROTO_ZH202;
						service.m_heartbeat_time = 12;
					}
					else if(service.m_proto_set == PROTO_ZH202)
					{
						service.m_proto_set = PROTO_SHIKE_PLUS;
						service.m_heartbeat_time = 7;
					}
					else
					{
						service.m_proto_set = PROTO_SHIKE;
						service.m_heartbeat_time = 7;
					}
					user_fs_isNeedSave = 1;
					#if USE_SX1262
					iStatus_RF = RFM98_TX_FAIL;
					#endif
					LOG("切换工作模式:%x\n",service.m_proto_set);
					
					g_device_info.proto_set_1s = 5;
					PLAY_KEY();
					if(service.m_proto_set==PROTO_SHIKE)
					{
						g_device_info.Beep10ms = 50;
					}
					else if(service.m_proto_set==PROTO_SHIKE_PLUS)
					{
						g_device_info.Beep10ms = 50;
						g_device_info.BeepCnt = 1;
					}
					else
					{
						g_device_info.Beep10ms = 0;
					}
				}
				#endif
				else if(fIsAlm() && (service.m_muffling==0))
				{
					g_device_info.DeviceStatus.b.is_mute = 1;

					//230818:消音90秒限制
					g_cbSta.muffTimeOut1s = 90;		

					LOG("按键消音");
				}
				//否则自检上报
				else
				{
					#if 1
					if((g_cbSta.bAlmSta&BIT_CHECK) ==0)
					{
						//不一定要判断自检成功
						//如果是查询，需要等获取到参数
						Event_AddEvent(EVENT_TEST);

						LOG("自检上报");
						
						//取消效应
						if(service.m_muffling==1) 
						{
							service.m_muffling = 0;
							LOG("取消消音");
						}
						
						//处于报警开始报警声
						if((service.m_smoke_state && ((g_cbSta.bAlmSta&BIT_SMOKE_STOP)==0)) || (service.m_temp_state && ((g_cbSta.bAlmSta&BIT_TEMP_STOP)==0)) )
						{
							g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
							
							//反复上报时间重新清零
							if(g_cbSta.bAlmSndTimeOut1s>GAP_ALM_1S)
							{
								g_cbSta.bAlmSndTimeOut1s = GAP_ALM_1S;
								g_cbSta.bAlmSndlast = g_cbSta.bAlmSndTimeOut1s;
							}
							
							if(g_cbSta.bTempSndTimeOut1s>GAP_ALM_1S)
							{
								g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
								g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
							}	
						}
						else
						{
							g_cbSta.bAlmSta |= BIT_CHECK;
						}		
					}	
					#endif
				}
			
						
			}
			
			#if SET_BLE_DEBUG
			if(IS_PROTO_SHIKE)
			{
				spi_Test = 1;
				osal_set_event(App_main_TaskID,APP_MAIN_SPI);
			}
			//250709:上电3分钟内，安消模式增加调试信息透传
			// if((g_device_info.RunTime<180)&&(IS_PROTO_SHIKE))
			// {

			// 	LOG("测试信息发送\n");
			// 	note_set_ble_debug();
			// }
			#endif
		}

		//收尾处理
		Key[0].Flag.B = 0;  //清标志
	}
}	



//防拆
u16 bFcClose;
u16 bFcOpen;
void pDectFc(u8 isTouch)
{
	//static u8 bFcClose,bFcOpen;

	if(isTouch)
	{
		bFcOpen = 0;
		if(bFcClose<0xffff)
		{
			bFcClose++;
		}

		//短按发送一次数据
		// #if SET_SPI
		// if((bFcClose==20)&&(TouchSend.isGet)&& (g_device_info.RunTime<60))
		// {
		// 	Event_AddEvent(EVENT_SPI_TEST);
		// }
		// #endif
		
//		if(TouchSend.isGet==1)
		{
			//2s 防拆恢复
			//if((g_cbSta.isFcOpen!=FC_CLOSE)&(bFcClose>200))
			//240108：闭合7秒，断开1秒
			if((g_cbSta.isFcOpen!=FC_CLOSE)&(bFcClose>700))
			{
				g_cbSta.isFcOpen = FC_CLOSE;
				g_cbSta.bFcTimeOut1s = 0;
				//220306:NB也增加防拆
				//#if SET_NB==0
				#if 1
				Event_AddEvent(EVENT_FCRST);
				#endif
				PrintString1("防拆合上\r\n");
				
			}	
			#if 1
			if(bFcClose>700)
			{
				if(g_device_info.isBoot)
				{
					Event_AddEvent(EVENT_BOOT);
					g_device_info.isBoot = 0;
				}
			}
			#endif
		}
//		else
//		{
//			if(bFcClose>5)
//				g_cbSta.isFcOpen = FC_CLOSE;
//		}	
	}
	else
	{
		
		bFcClose = 0;
		if(bFcOpen<0xffff)
		{
			bFcOpen++;
		}
		
//		if(TouchSend.isGet==1)
		{
			//10s防拆报警		
			//if((g_cbSta.isFcOpen!=FC_OPEN)&&(bFcOpen>1000))
			//240108：闭合7秒，断开1秒
			if((g_cbSta.isFcOpen!=FC_OPEN)&&(bFcOpen>100))
			{
				g_cbSta.isFcOpen = FC_OPEN;
				
				//221207:去除防拆语音
				//g_cbSta.bFcTimeOut1s =FC_TIMEOUT_1S;
				
				//220306:NB也增加防拆
				//NB版本没有防拆上报
				//#if SET_NB==0
				#if 1
				Event_AddEvent(EVENT_FCALM);				
				#endif
				PrintString1("防拆打开\r\n");
				if(service.m_muffling==1)
					service.m_muffling = 0;	//退出静音
				
				#if 1
				if(g_device_info.isBoot)
				{
					Event_AddEvent(EVENT_BOOT);
					g_device_info.isBoot = 0;
				}
				#endif
			}
		}
//		else
//		{
//			if(bFcOpen>5)
//				g_cbSta.isFcOpen = FC_OPEN;
//		}
	}
}

//LED
u8 isFlash;
u8 isFlashSlow;
void Led_pro(void)
{
	static u8 s_CheckAlmCnt = 0;
	static u8 s_isPowOn = 0;
	//要等获取到初值
	if(TouchSend.isGet)
	{
		g_device_info.DeviceStatus.b.is_Run = 0;
		
		//间隔后蜂鸣
		if(g_device_info.Beep10ms>0)
		{
			g_device_info.Beep10ms--;
			if(g_device_info.Beep10ms==0)
			{
				LOG("间隔蜂鸣\r\n");
				PLAY_KEY();
				if(g_device_info.BeepCnt>0)
				{
					g_device_info.BeepCnt--;
					g_device_info.Beep10ms = 50;
				}
			}
		}
		
		//OTA模式：黄灯长亮
		if(g_system_ota)
		{
			SET_LED_R(0);
			SET_LED_G(0);
		}
		#if (SET_NB||SET_CAT1)==0
		else if(g_device_info.learn_status == LEARN_STATUS_LEARN)
		{
			SET_LED_R(1);
			SET_LED_G(1);
		}
		//模式设定状态：黄灯慢闪
		else if(g_device_info.learn_status == LEARN_STATUS_SET)
		{
			SET_LED_R(isFlashSlow);
			SET_LED_G(isFlashSlow);
		}
		#endif			
		//处于标定过程:红绿灯交替慢闪
		else if(device_info.state.F_SM_BD)
		{
			if(isFlashSlow)
			{
				SET_LED_R(1);
				SET_LED_G(0);
			}
			else
			{
				SET_LED_R(0);
				SET_LED_G(1);
			}			
		}
		//未标定：红灯常亮
		else if(device_info.state.F_SM_BD_OK==0)
		{
			SET_LED_G(1);	
			SET_LED_R(0);	
			//未标定也可进入休眠正常运行状态
			g_device_info.DeviceStatus.b.is_Run = 1;	//正常运行
		}

		//230609：烟腔故障不能影响正常自检
		#if 0
		//未标定或者烟腔故障: 红灯每60s快闪2次并发出2声响提示
		else if(device_info.state.F_SM_ERR==1)
		{
			//故障也可进入休眠正常运行状态
			g_device_info.DeviceStatus.b.is_Run = 1;	//正常运行
		}
		#endif
		else
		{
			//电池初装
			if(s_isPowOn==0)
			{
				s_isPowOn = 1;
				
				//指示灯闪烁1次、蜂鸣器短鸣1次
				#if SET_NB||SET_CAT1
					#if NB_MODULE==NB_N701
					if(service.m_battery>=2750)
					#else
					if(service.m_battery>=2700)
					#endif
				#else
					if(service.m_battery>=2700)
				#endif
				{
					isFlash = 1;
					g_device_info.LedFlash = 2;
					
					LOG("开机蜂鸣1声\r\n");
					PLAY_KEY();
				}
				//指示灯闪烁2次、蜂鸣器短鸣2次
				else
				{
					isFlash = 1;
					g_device_info.LedFlash = 4;			
					//500ms间歇后再蜂鸣一次
					//PLAY_FailBell();
					LOG("开机蜂鸣2声\r\n");
					PLAY_KEY();
					g_device_info.Beep10ms = 50;
				}
				//红灯灭
				SET_LED_R(1);		
			}
			//开机绿灯还没闪完
//			else if(g_device_info.LedFlash>0)
//			{
//				SET_LED_G(isFlash);
//			}
			//报警指示
			else if(fIsAlm())
			{
				//烟雾报警（高低温报警同），红灯每1秒内闪1次时并发出报警声，表示探测器报警状态
				if((service.m_smoke_state && ((g_cbSta.bAlmSta&BIT_SMOKE_STOP)==0))|| (service.m_temp_state && ((g_cbSta.bAlmSta&BIT_TEMP_STOP)==0)))
				{
					if(g_device_info.DeviceStatus.b.is_1s_touch)
					{
						g_device_info.DeviceStatus.b.is_1s_touch = 0;
					
						#if SET_NB||SET_CAT1
						
						if((IsNeedRfSleep()==0))
						{
				
						}
						else
						{
							isFlash = 1;
							g_device_info.LedRedFlash = 2;
						}
						#else
						isFlash = 1;
						g_device_info.LedRedFlash = 2;
						#endif

						//230607:烟雾告警，即使消音操作，也至少要响2声
						//if(service.m_muffling!=1)

						//230818:没有90秒最长蜂鸣时间限制
						#if SET_BLE_SUB
						if(service.m_muffling!=1)
						{
							PLAY_SMOKE_ALARM();
						}
						#else
						//230627:3s后才开始蜂鸣
						if(g_cbSta.bBellTimeOut1s<=BELL_ALM_1S-3)
						{
							//230818:没有90秒最长蜂鸣时间限制
							if((service.m_muffling!=1)||(g_cbSta.bBellTimeOut1s>=BELL_ALM_1S-2-3))
							{
								PLAY_SMOKE_ALARM();
							}
						}
						#endif
					}
				}
				//测试报警，按下手动测试键，红灯每1秒闪1次并发出报警声，提示3次
				else if(g_cbSta.bAlmSta&BIT_CHECK)
				{
					if(g_device_info.DeviceStatus.b.is_1s_touch)
					{
						g_device_info.DeviceStatus.b.is_1s_touch = 0;
						
						s_CheckAlmCnt++;
						
						isFlash = 1;
						g_device_info.LedRedFlash = 2;
						
						//if(service.m_muffling!=1)
//						if(s_CheckAlmCnt==1)
//						{
//							PLAY_KEY_ALARM();
//						}
						
						PLAY_SMOKE_ALARM();
						
						//230427: 手动测试  多响几声
						//手动测试只响3声自动取消
						if(s_CheckAlmCnt>2)
						//if(s_CheckAlmCnt>10)
						{
							//extern u8 g_duty;
							//g_duty = 5;
							s_CheckAlmCnt = 0;
							g_cbSta.bAlmSta &= ~BIT_CHECK;
						}
					}
				}


				//防拆报警，探测器与底座脱离10s,红灯每4s闪1次并发出提示音
				//目前防拆有最长时长
				else if(g_cbSta.bFcTimeOut1s)
				{
					if(g_device_info.DeviceStatus.b.is_4s_touch)
					{
						g_device_info.DeviceStatus.b.is_4s_touch = 0;
						#if SET_NB||SET_CAT1
						if((IsNeedRfSleep()==0))
						{
				
						}
						else
						{
							isFlash = 1;
							g_device_info.LedRedFlash = 2;
						}
						#else
						isFlash = 1;
						g_device_info.LedRedFlash = 2;
						#endif
						LOG("防拆蜂鸣\r\n");
						PLAY_KEY();
					}
				}


				#if SET_NB||SET_CAT1
				if((IsNeedRfSleep()==0) && ((g_cbSta.bAlmSta&BIT_CHECK)==0))
				{
					if((g_run_info.m_nbiot_init_ok==FLASE))
					{
						SET_LED_R(isFlash);
						SET_LED_G(isFlash);
					}
					else
					{
						SET_LED_R(isFlashSlow);
						SET_LED_G(isFlashSlow);
					}				
				}
				else
				{
					//绿灯灭
					SET_LED_G(1);
					//红灯
					if(g_device_info.LedRedFlash>0)
					{
						//SET_LED_R(isFlash);
					}
					else
					{
						SET_LED_R(1);
					}
				}
				#else
				//绿灯灭
				SET_LED_G(1);
				//红灯
				if(g_device_info.LedRedFlash>0)
				{
					//SET_LED_R(isFlash);
				}
				else
				{
					SET_LED_R(1);
				}
				#endif
			}
			else
			{
				s_CheckAlmCnt = 0;

				#if SET_NB||SET_CAT1
				if((IsNeedRfSleep()==0))
				{
					if((g_run_info.m_nbiot_init_ok==FLASE))
					{
						SET_LED_R(isFlash);
						SET_LED_G(isFlash);
					}
					else
					{
						SET_LED_R(isFlashSlow);
						SET_LED_G(isFlashSlow);
					}				
				}
				else
				{
					//红灯灭
					if(g_device_info.LedFlashSleep==0)
						SET_LED_R(1);	
					g_device_info.DeviceStatus.b.is_Run = 1;	//正常运行
				}
				#else
				if(g_device_info.LedFlashSleep==0)
				{
					SET_LED_G(1);	
					SET_LED_R(1);
				}	
				g_device_info.DeviceStatus.b.is_Run = 1;	//正常运行
				#endif
			}				
		}
	}

	g_device_info.DeviceStatus.b.is_1s_touch = 0;
}

u8 PirCnt = 0;
u8 PirReg = 0;
u16 PirLast01ms = 0;
u16 Pir2Last01ms = 0;
u8 g_isSend = 0;
void infr_callback_wakeup(GPIO_Pin_e pin,IO_Wakeup_Pol_e type)
{
	static u8 isSyn = 0;
	//sleepTimer = 3;
	#if SET_NB || SET_CAT1
	if(((my_time() - g_run_info.m_last_nbiot_send_time) < 1000)||g_isSend)
	{
		DEBUG_LOG("NO_PIR\n");
		PirCnt = 0;
		PirLast01ms = 0;
		Pir2Last01ms = 0;
		return;
	}
	#endif
	
	//同步码可能>200
	//脉宽超过50ms过滤
	if(PirLast01ms>500)
	{
		DEBUG_LOG("E-L:%d\n",PirLast01ms);
		PirCnt = 0;
		PirLast01ms = 0;
		Pir2Last01ms = 0;
		isSyn = 0;
	}
	//可能是同步码
	else if((PirLast01ms>100)&&(isSyn==1))
	{
		DEBUG_LOG("E-L:%d\n",PirLast01ms);
		PirCnt = 0;
		PirLast01ms = 0;
		Pir2Last01ms = 0;
		
		isSyn = 0;
	}
	//可能是同步码
	else if((PirLast01ms>100)&&(isSyn==0))
	{
		isSyn = 1;
		DEBUG_LOG("Sync:%d\n",PirLast01ms);
		PirLast01ms = Pir2Last01ms;
	}
	//脉宽过窄
	else if(PirLast01ms==0)
	{
		DEBUG_LOG("E-L:%d\n",PirLast01ms);
		PirCnt = 0;
		PirLast01ms = 0;
		Pir2Last01ms = 0;
		isSyn = 0;
	}
	else
	{
		isSyn = 0;
		if(type==POL_FALLING)
		{
			DEBUG_LOG("H:%d,\n",PirLast01ms);
		}
		else
		{
			DEBUG_LOG("L:%d,\n",PirLast01ms);
		}
		
	}
	
	#if 1
	//1倍宽度 或 3倍宽度
	if(Pir2Last01ms>0)
	{
		//0.8-1.2倍 ， 2.5倍 - 3.5倍
		if(PirLast01ms>Pir2Last01ms)
		{
			//if(((PirLast01ms>=Pir2Last01ms*7/2)|| ((PirLast01ms>=Pir2Last01ms*6/5)&&(PirLast01ms<=Pir2Last01ms*5/2))))
			//5倍
			if(PirLast01ms>=Pir2Last01ms*5)
			{
				DEBUG_LOG("E-R:%d,%d,%d\n",PirLast01ms,Pir2Last01ms);
				
					PirCnt = 0;
					PirLast01ms = 0;
					Pir2Last01ms = 0;
				
				
			}
		}
		else
		{
			//if(((Pir2Last01ms>=PirLast01ms*7/2)|| ((Pir2Last01ms>=PirLast01ms*6/5)&&(Pir2Last01ms<=PirLast01ms*5/2))))
			if(Pir2Last01ms>=PirLast01ms*5)
			{
					DEBUG_LOG("E-R:%d,%d,%d\n",PirLast01ms,Pir2Last01ms);

					PirCnt = 0;
					PirLast01ms = 0;
					Pir2Last01ms = 0;
				
			}
		}
	}
	#endif

	if(fIsAlm() && (service.m_muffling==0)&&(g_cbSta.bAlm1s>60))
	{
		PirCnt++;
	
		Pir2Last01ms = PirLast01ms;
		PirLast01ms = 0;
		DEBUG_LOG("R:%d\n",PirCnt);
		//if(PirCnt>=60)
		//if(PirCnt>=20)
		#if SET_CAT1
		if(PirCnt>=60)
		#else
		if(PirCnt>=40)
		#endif
		{
			g_device_info.DeviceStatus.b.is_mute = 1;
			LOG("红外消音\r\n");
			//230818:消音90秒限制
			g_cbSta.muffTimeOut1s = 90;	
			
		}
	}
}

//要注意中断和线程不要抢占资源
void timer_int_process(uint8_t evt)
{
		//static u8 PirCntL,PirCntH,PirOn;
		static u8 PirOn;
    switch(evt)
    {
			case HAL_EVT_TIMER_5:
					//LOG("t5\n");

				#if SET_RX_TRAN
				{
					extern u8 g_isTran;
					static u8 s_Cnt;
					if((READ_CLK()==0)&&(g_isTran==0))
					//if(READ_CLK()==0)
					{
						g_isTran = 1;
						osal_set_event(App_main_TaskID,APP_MAIN_SPI);
						//MySPI_pro();
					}
					
					s_Cnt++;
					if(s_Cnt<10)
					{
						return;
					}
					else
					{
						s_Cnt = 0;
					}
				}			
				#endif
				{
					static u8 s_Cnt;
					s_Cnt++;
					if(PirLast01ms<0xffff)
					{
						PirLast01ms++;
					}
					if(s_Cnt<10)
					{
						return;
					}
					else
					{
						s_Cnt = 0;
					}
				}
				
				if(WatchDogFlag==1)
				{
					WatchDogFlag = 2;
				}

				//唤醒超时
				if(sleepTimer)
				{
					sleepTimer--;
				}	
							
				g_runTime_ms++;	  //运行时间，不计休眠
				
				#if SET_NB||SET_CAT1
				if(g_bWait1ms>0)
				{
					g_bWait1ms--;
				}

				if(COM1.RX_TimeOut>0)
				{
					COM1.RX_TimeOut--;
				}
				#endif

				#if 1
				if(fIsAlm() && (service.m_muffling==0)&&(g_cbSta.bAlm1s>60))
				{
					SET_PWR_IR(1);
					if(PirOn<0xff)
						PirOn++;
					if(PirOn>10)
					{
						if(PirReg==0)
						{
							PirReg = 1;
							hal_gpioin_register(PIR_IN,infr_callback_wakeup,infr_callback_wakeup);
						}
					}
				}
				else
				{
					PirCnt = 0;
					PirOn = 0;
					SET_PWR_IR(0);
					if(PirReg)
					{
						PirReg = 0;
						hal_gpioin_unregister(PIR_IN);
					}
				}
				#else
				//240520:报警前3分钟不允许红外消音
				//增加对红外接收头支持
				if(fIsAlm() && (service.m_muffling==0)&&(g_cbSta.bAlm1s>60))
				{
					static u8 InfCnt = 0;
					SET_PWR_IR(1);
					if(PirOn<0xff)
						PirOn++;
					#if   0
					if(PirOn>10)
					{
						
						#if SET_CAT1
						if(my_time() - g_run_info.m_last_nbiot_send_time <1000)
						{
							PirCntH = 0;
							PirCntL = 0;
							InfCnt = 0;
						}
						else
						#endif
						{
							if(READ_PIR_IN()==0)
							{
								if(PirCntL<0xff)
								{
									PirCntL++;
								}
								else
								{
									PirCntH = 0;
									InfCnt = 0;
								}
								
								//if((PirCntL>1)&&(PirCntH>1))
								if((PirCntL>0)&&(PirCntH>0))
								{
									//PirCntL = 0;
									PirCntH = 0;
									
									InfCnt++;
									
									if(InfCnt>3)
									{
										InfCnt = 0;
										g_device_info.DeviceStatus.b.is_mute = 1;
										//LOG("红外消音\r\n");
										//230818:消音90秒限制
										g_cbSta.muffTimeOut1s = 90;	
									}
								}
							}
							else
							{
								if(PirCntH<0xff)
								{
									PirCntH++;
								}
								else
								{
									InfCnt = 0;
									//PirCntH = 0;
									PirCntL = 0;
								}	
							}
						}
					}
					else
					{
						PirCntH = 0;
						PirCntL = 0;
						InfCnt = 0;
					}	
					#else
					if(PirOn==10)
					{
						
					}
					#endif
				}
				else
				{
					PirOn = 0;
					PirCntL = 0;
					PirCntH = 0;
					SET_PWR_IR(0);
				}
				#endif
				
				//10ms
				if((g_runTime_ms%10)==0)
				{
					g_is10msTouch = 1;
					
					#if SET_BLE_LEARN
					if(g_bWaitTimeOut10ms>0)
					{
						g_bWaitTimeOut10ms--;
					}
					#endif

					pDectFc(dFcCurSta==0);
					fKeyCheck(eKEYType0,dKey0CurSta);
					Led_pro();
					
					//230605
					Fun_beep_scan();			

					if(TouchSend.m_timeout10ms>0)
					{
						TouchSend.m_timeout10ms--;
					}
					
					if(TouchSend.m_heart10ms>0)
					{
						TouchSend.m_heart10ms--;
					}	
				}
				#if USE_SX1262
				if((g_runTime_ms%100)==0)
					g_is100msTouch = 1;
				#endif
				if((g_runTime_ms%200)==0)
				{
					isFlash = !isFlash;
					
					if(g_device_info.LedFlash>0)
					{
						g_device_info.LedFlash--;
						if(g_device_info.LedFlash==0)
						{
							SET_LED_G(1);
						}	
						else
						{
							SET_LED_G(isFlash);
						}
					}
					
					if(g_device_info.LedRedFlash>0)
					{
						g_device_info.LedRedFlash--;
						if(g_device_info.LedRedFlash==0)
						{
							if(device_info.state.F_SM_BD_OK==0)
							{
								SET_LED_R(0);
							}
							else
							{
								SET_LED_R(1);
							}						
						}
						else
						{
							SET_LED_R(isFlash);
						}
					}
				}
				
				if((g_runTime_ms%500)==0)
				{
					isFlashSlow = !isFlashSlow;
				}
					
				if((g_runTime_ms%1000)==0)
				{
					g_runTime_s++;
					g_is1sTouch = 1;

					#if SET_SPI&&(SET_NB||SET_CAT1)
					if(g_device_info.isNbSleep==0)
					{
						if(WatchDogFlagFast)
						{
							g_isSpiTouch = 1;
						}
						else
						{
							static u8 s_spiCnt;
							s_spiCnt++;
							if(s_spiCnt>=SET_SPI_GAP)
							{
								s_spiCnt = 0;
								g_isSpiTouch = 1;
							}
						}	
					}						
					#endif
					
					#if SET_NB||SET_CAT1
					if(bWatchDog1s>0)
					{
						bWatchDog1s--;
						hal_watchdog_feed();
					}
					
					if(g_cbSta.bDevTimeOut1s>0)
					{
						g_cbSta.bDevTimeOut1s--;
					}
					#endif
					
					#if SET_PROTO_VER==PROTO_VER_ZH202
					g_cbNet.hreadbeat_timer1++;
					#endif
					
					
					g_device_info.DeviceStatus.b.is_1s_touch	= 1;
	
					if(g_cbSta.bFcTimeOut1s>0)
					{
						g_cbSta.bFcTimeOut1s--;
					}
						
					if(g_cbSta.bAlmTimeOut1s>1)
					{
						g_cbSta.bAlmTimeOut1s--;
					}

					if((g_cbSta.bAlm1s>0)&&(g_cbSta.bAlm1s<0xff))
					{
						g_cbSta.bAlm1s++;
					}

					//230818:没有90秒最长蜂鸣时间限制
					#if SET_BLE_SUB
					#else
					if(g_cbSta.bBellTimeOut1s>0)
					{
						g_cbSta.bBellTimeOut1s--;

						//230818:没有90秒最长蜂鸣时间限制	
//						if(fIsAlm()&&(g_cbSta.bBellTimeOut1s==0))
//						{
//							g_device_info.DeviceStatus.b.is_mute = 1;
//							LOG("超时自动消音");
//						}
					}
					#endif
					
					#if SET_NB||SET_CAT1
					if(g_bWaitTimeOut1s>0)
					{
						g_bWaitTimeOut1s--;
					}
					
					if(g_cbSta.bNetTimeOut1s>0)
					{
						g_cbSta.bNetTimeOut1s--;
					}
					#endif
					
					#if SET_CAT1
					if(init_Tmr>0)
					{
						init_Tmr--;
					}
					
					for(u8 i=0;i<2;i++)
					{
						if(sock_Tmr[i]>0)
						{
							sock_Tmr[i]--;
						}
					}
					
					#endif
				}	

				if((g_runTime_ms%4000)==0)
				{
					
					g_device_info.DeviceStatus.b.is_4s_touch	= 	1;
				}
						
        break;

    case HAL_EVT_TIMER_6:
        //LOG("t6\n");
        break;

    case HAL_EVT_WAKEUP:
			//LOG("timer will disable when sleep,so if you want it work please init it when wakeup");			
			//需要重新初始化定时器
			hal_timer_init(timer_int_process);
			#if 1
			hal_timer_set(AP_TIMER_ID_5,100);
			#else
			hal_timer_set(AP_TIMER_ID_5,1000);		
			#endif
			DEBUG_LOG("Timer Wake\r\n");
			
    break;

    case HAL_EVT_SLEEP:
			//LOG("timer sleep\n");
			//休眠：功耗考虑关闭LED
    break;

    default:
        //LOG("err ");
        break;
    }
}

/*********************************************************************
    @fn      AP_TIMER_Demo_Init

    @brief

    @param

    @return
*/
void TIMER_config(void)
{
    LOG("when test this case,you can uncomment comment which in timer int function\n");
		hal_timer_init(timer_int_process);

		//#if SET_RX_TRAN
		#if 1
		hal_timer_set(AP_TIMER_ID_5,100);	//100us
		#else
		hal_timer_set(AP_TIMER_ID_5,1000);	//1ms
		#endif
}

u32 my_time(void)
{
	return g_runTime_ms;
}
