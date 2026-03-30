#include	"proto.h"

#if SET_PROTO_VER==PROTO_VER_JSON
u32  g_nbiot_last_read_time = 0x00;


//不需处理数据
u8 p_prto_dispose_data(u8 *buf, u16 len)
{
	//u8 bPos = 0;
	//g_cbSta.bNetTimeOut1s = TIME_OUT_NET;
	g_cbSta.isNbNetOk = 1;	
	g_run_info.m_nbiot_err_cnt = 0;
	
	LOG("接收数据\n");
	//只要有数据则去除重发和正在发送状态
	g_run_info.m_flg_sending = FLASE;
	g_run_info.m_flg_retry = FLASE;
	g_run_info.m_last_nbiot_resend_count = 0;
	
	g_run_info.m_nbiot_create_sock_ok = TRUE;
	g_run_info.m_last_nbiot_connect_time = my_time();
	

	if(g_cbSta.bDevTimeOut1s<3)
		g_cbSta.bDevTimeOut1s = 3;	//等待平台下发数据
	MyJson_Unpack((char *)buf);
	
	return 1;
}

void pSndAll(void)
{
	QUEUE_OBJ_Event bEvent;
		
	//230608：不需要等回复，但也要有重发机制
	g_run_info.m_flg_sending = TRUE;
	
	//发送新的事件，填充发送;
	//否则按原数据重新发送
	if(TRUE==g_run_info.m_flg_retry)
	{
		g_run_info.m_flg_retry = FLASE;
	}
	else
	{
		//g_cbSta.bEventId++;
		
		//事件
		bEvent = Event_PopEvent();
		g_cbSta.bEventSndId = bEvent.bId;
		
		#if 0
		//自检和心跳需要上报信号
		//if((bEvent==EVENT_PLUS)||(bEvent==EVENT_TEST))
		//所有的都需要读取一下信号强度再发送（有遇到读取到注册了仍然不能上报）
		{
			g_bWaitTimeOut1s = 5;
			while(g_bWaitTimeOut1s!=0)
			{
				/* 核实模块注册上了没 */
				if (!((p_nblc_get_csq() == FALSE) || (g_run_info.m_csq == 99)))
				{
					break;
				}	
			}
		}
		#endif
		
		switch(bEvent.bEvent)
		{	
			case EVENT_PLUS:
				service.serviceID = 4;
				break;
			//监测状态，会不会状态已经改变？？
			case EVENT_ALM:
				//间隔时间加长
				#if SET_CAT1
				#else
				if(g_cbSta.bAlmSndTimeOut1s<GAP_ALM_1S)
					g_cbSta.bAlmSndTimeOut1s = GAP_ALM_1S;
				#endif
				service.serviceID = 1009;
				break;
			case EVENT_ALM_RST:
				service.serviceID = 1010;
				break;
			case EVENT_BOOT:
				service.serviceID = 1011;
				break;
			case EVENT_TEST:
				service.serviceID = 1004;
				break;
			case EVENT_TEMP:
				#if SET_CAT1
				#else
				if(g_cbSta.bTempSndTimeOut1s<GAP_ALM_1S)
					g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
				#endif
				service.serviceID = 1005;
				break;
			case EVENT_TEMP_RST:
				service.serviceID = 1008;
				break;
			case EVENT_BAT:
				//g_cbSta.bBatSndTimeOut1s = 60;
				service.serviceID = 1006;
				break;
			case EVENT_BAT_RST:
				service.serviceID = 1007;
				break;
			//220306：增加防拆事件和状态
			//#if (NB_MODULE==NB_N306) || (NB_MODULE==NB_N701) 
			case EVENT_FCALM:
				service.serviceID = 1012;
				break;
			case EVENT_FCRST:
				service.serviceID = 1013;
				break;
			//#endif
			default:
				service.serviceID = bEvent.bEvent-80+9000;
				break;
			
		}	
		
	
		memset(json_buf, 0, BUFFSIZE);
		MyJson_pack(json_buf, BUFFSIZE,bEvent.taskId); // 构建包

		#ifdef	SET_DISP
		/* 打印 */
		// printf(json_buf);
		// printf("\r\n");
			PrintString1(json_buf);
		PrintString1("\r\n");
		delay_ms(1000);
		#endif	
	}
	
	g_run_info.m_flg_retry = FLASE;
	
	if(FALSE == p_nblc_port_send_data((u8 *)json_buf,strlen(json_buf)))
	{
		delay_ms(2000);
		if(FALSE == p_nblc_port_send_data((u8 *)json_buf,strlen(json_buf)))
		{
			g_run_info.m_nbiot_create_sock_ok = FALSE;
			
			#if SET_NB
			pDelConnect();
			#endif
			
			#if SET_CAT1
			LOG("SND ERR\n");
			//g_run_info.m_flg_retry = TRUE;
			//g_run_info.m_flg_sending = FLASE;
			#endif
		}
		else
		{
			LOG("发送成功\n");

			#if SET_CAT1
			#else
			//230608：不需要等回复，但也要有重发机制
			g_run_info.m_flg_sending = FLASE;
			#endif
		}	
	}
	else
	{
		LOG("发送成功\n");
		#if SET_CAT1
		#else
		//230608：不需要等回复，但也要有重发机制
		g_run_info.m_flg_sending = FLASE;
		#endif
	}
	
	#if SET_TEST
//	if(fIsAlm()==0)
//	{
		g_cbSta.bDevTimeOut1s = 2;
//	}
//	else
//	{
//		g_cbSta.bDevTimeOut1s = 120;
//	}
	
	#else
	g_cbSta.bDevTimeOut1s = 2;	//230629:延时2秒等编程操作
	#endif
}

void NB_TxPro(void)
{
	u8 bTry;

	if(!Event_IsEmpty()  || (TRUE==g_run_info.m_flg_retry) || (TRUE==g_run_info.m_flg_reply))
	{
		g_run_info.m_last_nbiot_send_time = my_time();

		g_device_info.isWaitSnd = 0;
			
		//读到信号
		for(bTry=0;bTry<7;bTry++)
		{
			p_nblc_get_csq();
			if(g_run_info.m_csq!=0xff)
			{
				break;
			}
			else
			{
				Sleep_ms(500);
			}
		}
		#if SET_CAT1
		g_isSend = 1;
		#endif
		pSndAll();
		#if SET_CAT1
		g_isSend = 0;
		#endif
	}

	//如果是报警状态，需要利用定时上报维持心跳
	#if SET_CAT1
	else if((((my_time() - g_run_info.m_last_nbiot_send_time) > (50*1000))) && fIsAlm() && (g_system_ota==0))
	{
		g_isSend = 1;
		g_run_info.m_last_nbiot_send_time = my_time();
		PirCnt = 0;
		PirLast01ms = 0;
		Pir2Last01ms = 0;
		
		//Event_AddEvent(EVENT_PLUS);
		//if(FLASE==p_air720_port_send_data(0," ",1))
		if(FLASE==p_air720_port_send_data(0,&g_run_info.m_imei[0],sizeof(g_run_info.m_imei)-1))
		{
			LOG("4G SEND FAIL\r\n");
			Sleep_ms(100);

			//p_air720_set_ATE(0);
			if(FLASE==p_air720_port_send_data(0,&g_run_info.m_imei[0],sizeof(g_run_info.m_imei)-1))
			{
				g_run_info.m_nbiot_create_sock_ok = FLASE;
			}	
			else
			{
				g_run_info.m_last_nbiot_send_time = my_time();
			}
		}
		else
		{
			g_run_info.m_last_nbiot_send_time = my_time();
		}
		g_isSend = 0;
	}
	#endif
}
#endif
