/*
更新：
220826：
1、 烟感报警标志有效5秒才报警，60s恢复;温度报警10s报警(至少需要，并且需要接收到2次的数据)，60s恢复
2、 原1分钟间隔报警，改成1分钟-2分钟-4分钟-8分钟...间隔上报数据（最多间隔12小时）
3、 报警后90s自动静音接警
4、 去除低压恢复，心跳时如果还低压还会再报低压
5、 去除按键取消消音操作
220906：
其它新警或者自检测试取消消音

230427：
修改烟雾阈值以我为准，40%

230519：
类SPI通讯、主动查询方式、自己判断警情
预警： 1、烟雾浓度达到浓度20%以上阈值，且处于上升状态； 或者当前浓度已达到设置阈值的50%，则预警；50%以外且不处于上升趋势，或阈值的20%以下则退出预警
	  2、温度上升，且达到阈值-10℃以上；或者已达到阈值-5%以上，则预警；-5℃以外且不处于上升趋势，或者-10℃以下则退出预警
      3、预警是加快采集频率到1秒每次
报警：1、烟雾浓度达到阈值满5秒开始报警，浓度降低到阈值的50% 满1分钟才警情恢复，期间阈值不计时
     
*/

#include "app_main.h"
#include "user_1527.h"


extern void p_Rf_Rx(void);
uint8 App_main_TaskID;

extern u8 g_system_ota;

#if SET_SPI
u8 g_isSpiTouch;
#endif

u16 timer6h_down = 1; //开机马上发送心跳报告
u8 timer60s_down = 1;
u16  radomDight = 0;		 //随机数时间
/*软件版本信息*/

/*全局变量定义*/
volatile uint8 iEvent_Flag = 0;	 /* 系统标志位 */
volatile uint8 iEvent_Flag2 = 0; /* 系统标志位 */

volatile uint16 wTimer_UART_1ms = 0;

uint8  iTimer_RFM98_nRX = 0; // 非接收模式计时

uint8 RF_Num = 0x00;		 //lora通信标号
uint8  g_isRfRetry = 0;

u8  g_readySendBuf[ReadySendSize]; 			//准备发送buf
//u8  g_readySendBufbak[ReadySendSize];		//备份一下上一次回复的包数据
TYPE_DEVICE_INFO  g_device_info;	//设备信息
TYPE_RF_INFO g_rf_info;
u32 g_runTime_ms = 0; //运行时间 ms
u32 g_runTime_s = 0;  //运行时间 s
u16 sleepTimer = 0;	  //休眠倒计时
const u8 user_perpet_time[4] = {0xFF, 0xFF, 0xFF, 0xFF};

u8 s_LoRaYCount = 0;
u8 g_isLoraFree,g_isNeedChklora,g_isChkloraOk;

void user_printf_hex(u8 *dat, u16 len)
{
	int i = 0;
  for(i = 0; i < len; i++)
	{
		LOG("%02X ", dat[i]);
	}
	UART_PutStr("\r\n");
}

void user_printf(u8 *dat, u16 len)
{
    u16 i = 0;
    for(i = 0; i < len; i++)
    {
		UART_putc(dat[i]);
    }
	UART_PutStr("\r\n");
}

void pGetCpuId(void)
{
	//设备MAC地址
	u8 i;
	uint8 AT_mac_address[6];
	hal_flash_read(0x4004,AT_mac_address,2);   //读取mac地址
	hal_flash_read(0x4000,AT_mac_address+2,4);  
	LOG("MAC: %x %x %x %x %x %x",AT_mac_address[0],AT_mac_address[1],AT_mac_address[2],AT_mac_address[3],AT_mac_address[4],AT_mac_address[5]) ;
	memcpy(g_device_info.s_cpu_id,AT_mac_address,6);
	g_device_info.s_cpu_id[6] = g_device_info.s_cpu_id[0] + g_device_info.s_cpu_id[1] + g_device_info.s_cpu_id[2] + g_device_info.s_cpu_id[3] + g_device_info.s_cpu_id[4] + g_device_info.s_cpu_id[5];
	for(i=0;i<6;i++)
	{
		radomDight += g_device_info.s_cpu_id[i];
	}
}

/**
 * @description: 设备 初始化
 * @param {*}
 * @return {*}
 */
void user_info_init(void)
{
	memset(&g_device_info, 0x00, sizeof(TYPE_DEVICE_INFO));
	memset(&g_rf_info, 0x00, sizeof(TYPE_RF_INFO));
	g_rf_info.wait_ack_time = 25; //重发间隔 *100 MS 没回复重发
	pGetCpuId();
//	g_device_info.DeviceStatus.b.start_up = 1;		  //开机标识
	
//	R_SM device_info;						//烟感状态
//	service_t  service;					//平台协议状态
//	QUEUE_OBJ_t 	TouchSend;		//串口发送队列
//	CbSta_InitTypeDef  g_cbSta;	
	memset(&device_info,0,sizeof(device_info));
	memset(&service,0,sizeof(service));
	memset(&TouchSend,0,sizeof(TouchSend));
	memset(&g_cbSta,0,sizeof(g_cbSta));
}


/**
 * @description: 设备恢复出厂设置
 * @param {*}
 * @return {*}
 */
void device_reset(void)
{
	//擦除文件系统
	hal_fs_format(FS_OFFSET_ADDRESS,FS_SECTOR_NUM);

	LOG("fs_format OK\r\n");	
	
	//重启	
	hal_system_soft_reset();
}


void main_1s_timer(void);

bool Rf_is_Busy(void)
{
	bool ret = false;

	#if SET_NB||SET_CAT1
	if(g_device_info.isNbSleep)
	#elif USE_SX1262
	if(g_readySendBuf[SITE_CMD] == 0 && (note_cur_is_send()==0) && g_rf_info.re_send_times==0 )
	#else
	if(g_readySendBuf[SITE_CMD] == 0 && (note_cur_is_send()==0))
	#endif
    {
        ret = false;
    }
    else
    {
		ret = true;
    }
    return ret;
}

bool fIsAlm(void)
{
	//240807:烟腔故障，不报警
	//暂停报警也可休眠
	if((service.m_smoke_state && (device_info.state.F_SM_ERR==0)  && ((g_cbSta.bAlmSta&BIT_SMOKE_STOP)==0)) || (service.m_temp_state && ((g_cbSta.bAlmSta&BIT_TEMP_STOP)==0)) || (g_cbSta.bAlmSta&BIT_CHECK) || (g_cbSta.bFcTimeOut1s>0))
	{

		//240520:消音状态也不休眠--会重新再响警号
		if(g_cbSta.bAlm1s==0)
		{
			g_cbSta.bAlm1s = 1;
		}
		return 1;
		
		#if 0
		#if SET_NB||SET_CAT1
		return 1;
		#else
		//报警状态如果消音也允许休眠
		if(service.m_muffling==1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
		#endif
		#endif
	}
	else
	{
		g_cbSta.bAlm1s = 0;
		g_cbSta.bBellTimeOut1s = 0;		//
		return 0;
		
	}
}

//即将报警也不能休眠,需要中断计时
bool fIsWillAlm(void)
{
	//即将报警
	if(((service.m_temp_alm1s>0) && (service.m_temp_state==0)) ||((service.m_smoke_alm1s>0) && (service.m_smoke_state==0)))
	{
		return 1;
	}

	//即将恢复
	//if(((service.m_temp_almRest1s>0) && (service.m_temp_state==1)) ||((service.m_smoke_almRest1s>0) && (service.m_smoke_state==1)))
	if(((service.m_temp_almRest1s>0) && (service.m_temp_state==1)) ||(((service.m_smoke_almRest1s>0)||(service.m_smoke_almRest6>0)||(service.m_smoke_almRest7>0)||(service.m_smoke_almRest8>0)) && (service.m_smoke_state==1)))
	{
		return 1;
	}

	return 0;
}

//按键检测防拆检测需要时间，也需要检测完成 ==
//读参未读IO
bool fIsKeyOk(void)
{
	if((Key[0].TCount<=1000)&&(Key[0].TCount!=0))
	{
		return 0;
	}
	
	if(((g_cbSta.isFcOpen == FC_CLOSE)&& (bFcClose==0))||((g_cbSta.isFcOpen == FC_OPEN)&& (bFcOpen==0)))
	{
		return 0;
	}
	
	return 1;
}

bool fIsNeedSleep(void)
{
	if (sleepTimer == 0								//外部中断
		&& Rf_is_Busy() == 0						//RF发送完毕
			&& Event_IsEmpty()						//事件发完
			&& (user_1527_is_tx_busy()==0)			//433M 1527发送完毕(休眠会停掉发送定时器)
			#if SET_SPI==0
			&& TouchSend.isSleep==1				//串口休眠
			#endif
			&& g_device_info.learn_status != LEARN_STATUS_SET	//非对码模式
			&& fIsAlm()==0								//非报警
			&& fIsKeyOk()
			&& g_device_info.DeviceStatus.b.is_Run
			#if SET_SPI==0
			&& (TouchSend.m_flag_set==0)
			#endif
			
			&&(g_device_info.isBoot==0)		//开机发完

			#if SET_NB||SET_CAT1
			&& (g_isSpiTouch == 0)
			#endif
			&& (bell_is_bell()==0)
			&& (g_device_info.DeviceStatus.b.is_need_heart_report==0)
			&& fIsWillAlm()==0						
	) 
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
#if SET_NB||SET_CAT1
bool IsNeedRfSleep(void)
{
	if (Rf_is_Busy() == 0
			&& Event_IsEmpty()						//事件发完
	) 
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif

void fIsWakeFun(void)
{
	//先执行了唤醒才执行定时任务
	//中断任务先于唤醒任务
	//if (fIsNeedSleep()) 
//	{
//		
//	}
//	else	
	{
		sleepTimer = 3;
		#if SET_SPI==0
		TouchSend.isSleep = 0;
		#endif

		
		//??230517:唤醒后按键读取异常? 重新设置一下
		// #if SET_SPI
		// app_gpio_set(PIN_KEY,GPIO_INPUT,GPIO_PULL_UP);
		// #endif
		
		//唤醒后更新一下按键和防拆状态
		pDectFc(dFcCurSta==0);
		fKeyCheck(eKEYType0,dKey0CurSta);
		
		g_device_info.is_sleep = 0;
		g_device_info.is_wake = 1;
		hal_pwrmgr_lock(MOD_USR1);
		osal_start_reload_timer(App_main_TaskID, APP_MAIN_1MS_TIMER , 1);
	}
}

//定时唤醒后处理
static void hal_usr1_wakeup_handler(void)
{	

	DEBUG_LOG("usr1 wake\n");	
	//memset(&Key[0],0,sizeof(Key[0]));

	#if (SET_NB||SET_CAT1)==0
	//221115:不需要扫描注网
	g_note.reg_status = 1;
	#endif
	
	//先执行了唤醒才执行定时任务,所以定时任务检测到的唤醒需调用退出睡眠
	//中断任务先于唤醒任务，说以可以不调用检测退出休眠任务
	//执行了唤醒任务，说明一定是休眠了
	fIsWakeFun();
}

//休眠前处理
static void hal_usr1_sleep_handler(void)
{
	DEBUG_LOG("usr1 sleep\n");	

	#if SET_USE_PSM
	g_device_info.wakeSrc = 0;
	#endif	
}

void main_1s_timer(void)
{
	u8 _1s_count;
	
	//一般采用扫码添加方式
	//----发送LORA学习
	{
		_1s_count++;

		#if (SET_NB||SET_CAT1)==0
		if(g_device_info.proto_set_1s>0)
		{
			g_device_info.proto_set_1s--;
		}
		else
		{
			if(g_device_info.learn_status!=LEARN_STATUS_COM)
			{
				g_device_info.learn_status = LEARN_STATUS_COM;
				LOG("退出设置模式");
			}
		}
		#endif
		
		if((_1s_count%2) ==0) //2s
		{	
			#if 0
			if(g_device_info.learn_status == LEARN_STATUS_LEARN)
			{
				static u8 _60s_count = 0;

				g_readySendBuf[SITE_CMD] = SEND_SMART_TO_MAIN;		  //命令编码
				g_readySendBuf[SITE_LEN] = 3;						  //数据长度
				g_readySendBuf[SITE_DATA_STAT] = DEVICE_ID;			  //设备类型
				g_readySendBuf[SITE_DATA_STAT + 1] = CMD_SMART_LEARN; //学习码
				g_readySendBuf[SITE_DATA_STAT + 2] = 0x00;

				if (++_60s_count > 30)
				{ //退出学习
					_60s_count = 0;
					g_device_info.learn_status = LEARN_STATUS_COM;
				}
			}
			#endif
					
			if(g_device_info.DeviceStatus.b.is_need_heart_report)
			{
				g_device_info.DeviceStatus.b.is_need_heart_report = 0;
				#if (SET_NB||SET_CAT1)==0
				Event_AddEvent(EVENT_PLUS);
				#endif
			}
		}
		
		if(user_fs_isNeedSave)
		{
			user_fs_isNeedSave = 0;
			user_fs_updata_memory();
		}
		
		if(_1s_count>=100)
		{
			_1s_count = 0;
		}
	}
}


/**
 * @description: 主函数 1s 定时
 * @param {*}
 * @return {*}
 */
void alawys_10s_timer(void)
{
//static u8 s_ramdom;
	static u8 s_ramdomSet;
	static u16 s_Cnt10s;
	s_Cnt10s++;
	
	#if SET_USE_PSM
	//230619:
	if(g_device_info.bNbSleepTimeOut20s>0)
	{
		g_device_info.bNbSleepTimeOut20s--;
	}
	#endif
	
	if(s_ramdomSet>60)
	{
		s_ramdomSet = 0;
	}

	//50s
	if((s_Cnt10s%5)==0)
	{
		g_device_info.DeviceStatus.b.is_50s_touch	= 1;
		
	}
	if((s_Cnt10s%6)==0)
	{
		g_device_info.DeviceStatus.b.is_60s_touch	= 1;
		
	}
	
	DEBUG_LOG("10s_T\n");
	
	//测试
	#if SET_NB||SET_CAT1
	if(s_Cnt10s>=360)	
	#else
	//1hour
	if(g_device_info.DeviceStatus.b.is_60s_touch)
	#endif
	{	
		g_cbSta.bheart++;
		
		#if SET_NB||SET_CAT1
		#elif SET_SMART
		service.m_heartbeat_time = 7;
		#else
		if(IS_PROTO_SHIKE)
		{
			service.m_heartbeat_time = 7;
		}
		#endif
		service.m_heartbeat_time = 6;
	}

	if(s_Cnt10s>=360)
	{
		s_Cnt10s = 0;
	}
	
	#if SET_NB||SET_CAT1
	if((service.m_heartbeat_time==0)||(service.m_heartbeat_time>168))
	{
		service.m_heartbeat_time = 24;
	}
	#else
	if((service.m_heartbeat_time==0)||(service.m_heartbeat_time>24))
	{
		service.m_heartbeat_time = 12;
	}
	#endif
	
	#if SET_NB||SET_CAT1	
	//if((g_cbSta.bheart>= service.m_heartbeat_time) || (g_cbSta.bheart>=168) || (service.m_muffling==2))
	if(g_cbSta.bheart>= service.m_heartbeat_time) 
	#else	
		//安防模式需要再随机半小时
		//暂停模式下，心跳间隔变成1小时;
		//6:30-7:30随机
		if(((g_cbSta.bheart>= service.m_heartbeat_time*60-30+s_ramdomSet)&&(IS_PROTO_SHIKE)) ||((service.m_proto_set == PROTO_ZH202)&&(g_cbSta.bheart>= service.m_heartbeat_time*60)))
	#endif
	{
		#if SET_NB||SET_CAT1
		g_device_info.test_mode = 0;
		#endif
		
		g_cbSta.bheart = 0;	
		#if SET_RST_DEVICE
		g_cbSta.isNeedRstDevice = 1;
		if(WatchDogFlagFast==0)
		{
			WatchDogFlagFast = 1;
			osal_stop_timerEx(watchdog_TaskID, WATCHDOG_1000MS_EVENT);
			osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, 1000);
			LOG("切换1s唤醒\r\n");
		}	
		#endif
		
		LOG("心跳上报\n");
		
		Event_AddEvent(EVENT_PLUS);			
		
		s_ramdomSet = (AP_TIM3->CurrentCount)%60;
		LOG("ramdom:%d\n",s_ramdomSet);
		g_cbSta.isBattLowSnd = 0;		//上报心跳，可以重新再报电池低压事件	
	}
	

	#if SET_NB||SET_CAT1
	#else
	//240219：心跳随机间隔20秒-5分钟上报  ，测试4秒-1分钟
	if(g_rf_info.re_plus_times)
	{
		g_rf_info.wait_plus_time++;
		if(g_rf_info.wait_plus_time>g_rf_info.wait_plus_timer)
		{
			g_rf_info.re_plus_times--;
			g_rf_info.wait_plus_time = 0;
			g_isRfRetry = 1;		//重发
			Event_AddEvent(EVENT_PLUS_AG);
			
			if(g_rf_info.re_plus_times)
			{
				g_rf_info.wait_plus_timer = (AP_TIM3->CurrentCount)%29+1;
			
				LOG("WAIT_PLUS:%d\n",g_rf_info.wait_plus_timer);
			}	
			else
			{
				g_rf_info.isLost = 1;
				LOG("PLUS_OVER\n");
			}
		}
	}
	else
	{
		g_rf_info.wait_plus_time = 0;
	}
	#endif

	
	
	g_device_info.UnixTimestamp += 10;			//unix 计时
	
	if((g_device_info.DeviceStatus.b.is_50s_touch)&&(g_device_info.DeviceStatus.b.is_Run))
	{
		g_device_info.DeviceStatus.b.is_50s_touch = 0;
	
		//未标定,红灯常亮
		if(device_info.state.F_SM_BD_OK==0)
		{
			SET_LED_G(1);	
			SET_LED_R(0);
		}
		//探测器故障，红灯每60s快闪2次并发出2声响提示音
		else if(device_info.state.F_SM_ERR==1)
		{
			//240807:烟腔故障修改成每50秒闪2次黄灯，亮1秒灭0.5秒
			SET_LED_G(0);	
			SET_LED_R(0);
			g_device_info.LedFlashSleep = 3;
			osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 1000);
			//240807:恢复蜂鸣器低压提示音
			//230609：去除故障提示音
			if(g_device_info.is_sleep)
			{
				g_device_info.is_sleep = 0;
				g_device_info.is_wake = 1;
				hal_pwrmgr_lock(MOD_USR1);
				osal_start_reload_timer(App_main_TaskID, APP_MAIN_1MS_TIMER , 1);
			}
			LOG("故障蜂鸣\r\n");
			PLAY_KEY();
			// g_device_info.Beep10ms = 50;


		}	
		//探测器低电，黄灯每60s快闪1次并发出提示音
		else if(service.m_battery_state)
		{
			SET_LED_G(0);
			SET_LED_R(0);
			g_device_info.LedFlashSleep = 1;
			osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 25);

			//240807:恢复蜂鸣器低压提示音
//			230306:去除低压提示音
			if(g_device_info.is_sleep)
			{
				g_device_info.is_sleep = 0;
				g_device_info.is_wake = 1;
				hal_pwrmgr_lock(MOD_USR1);
				osal_start_reload_timer(App_main_TaskID, APP_MAIN_1MS_TIMER , 1);
			}
			LOG("低压蜂鸣\r\n");
			PLAY_KEY();
		}
		//报警,消音后,红灯每60s快闪1次
		else if((service.m_temp_state)||(service.m_smoke_state))
		{
			SET_LED_G(1);	
			SET_LED_R(0);
			g_device_info.LedFlashSleep = 1;
			osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 25);

		}
		//工作指示，绿灯每60s闪1次表示探测器工作正常
		else
		{
			//LOG("RUN\r\n");
			SET_LED_G(0);
			SET_LED_R(1);
			
			#if SET_NB||SET_CAT1
			g_device_info.LedFlashSleep = 1;
			#else
			//失联每60s闪2次绿灯
			if(g_note.reg_status==0)
			{
				g_device_info.LedFlashSleep = 3;
			}
			else
			{
				g_device_info.LedFlashSleep = 1;
			}
			#endif
			osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 25);
		}

	}
	
	TouchSend.isSleep = 1;		//20s唤醒不需要等串口数据
	
	TouchSend.isGet = 1;
	sleepTimer = 0;	

	g_device_info.DeviceStatus.b.is_60s_touch = 0;

		
}

#if (SET_NB||SET_CAT1)==0
void SendGUID(void)
{
    //FF AA 9B BH1 BH2 GRP 09 F0 Type XX XX XX XX XX XX Signal XOR
    u8 TxBuf[50];
    u8 TxLen = 0;
    u8  bXor = 0;
    TxBuf[TxLen++] = 0xFF;             //固定FF
    TxBuf[TxLen++] = 0xAA;             //固定AA
    TxBuf[TxLen++] = 0x9B;             //固定9B
    TxBuf[TxLen++] = 0;   //设备编号H
    TxBuf[TxLen++] = 0;  //设备编号L
    TxBuf[TxLen++] = 0;  //设备组号
    TxBuf[TxLen++] = 0x09;                          //固定09
    //TxBuf[TxLen++] = 0xF0;                          //固定F0
		TxBuf[TxLen++] = 0xF1;
    TxBuf[TxLen++] = DEVICE_ID;                     //二维码第一字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[0];                     //二维码第二字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[1];                     //二维码第三字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[2];                     //二维码第四字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[3];                     //二维码第五字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[4];                     //二维码第六字节
    TxBuf[TxLen++] = g_device_info.s_cpu_id[5];                     //二维码第七字节
    for(int i = 1; i < TxLen; i++)
    {
      bXor = bXor^TxBuf[i];
    }
    TxBuf[TxLen++] = bXor;                       //信号强度

    //HAL_UART_Transmit(&huart4, (u8 *)TxBuf, TxLen, 0xFFFF);
	hal_uart_send_buff(UART0,&TxBuf[0],TxLen);
}
#endif

void app_main_init(uint8 task_id)
{
	App_main_TaskID = task_id;

	//200-400ms的随机数
	WaitMs(((AP_TIM3->CurrentCount)*6)%200+200);
	
	user_info_init();			//初始化参数
	
	#if (SET_NB||SET_CAT1)==0
	SendGUID();
	#endif
	
	
	GPIO_config();
	
	TIMER_config();				//硬件初始化

	UART_config();
	
	user_fs_init();				//读取设置参数
	
	#if USE_SX1262
	g_device_info.isok_get_Lora = 1;
	//[停用LoRa]RFM96_Config();
	//[停用LoRa]RFM96_LoRaEntryRx();			//进入接收模式
	LOG("isok_get_Lora=%d",g_device_info.isok_get_Lora);
	
	//开机检测是否焊接lora芯片
	if(g_device_info.isok_get_Lora==0)
	{
		iStatus_RF = RFM98_NONE;
	}
	
	//强制使用2.4g
	#if 0
	#if SET_BLE_SUB
	//220406：检测LORA芯片，进入休眠
	LOG("RFM96 Init\r\n");
	RFM96_Config();		 		//setting base parameter	
	
	if(g_device_info.isok_get_Lora)
	{
		iStatus_RF = RFM98_RX_TO_SLEEP;
	}
	else
	{
		BLE_RFM98_IO();	
	}
	#endif
	#endif

	#endif
	
	
	#if SET_BLE_SUB
	//if(g_device_info.isok_get_Lora==0)
	note_init(&g_device_info.s_cpu_id[0],p_user_rx_cb);
	#endif
		
	#if SET_CAT1
	osal_start_reload_timer(App_main_TaskID, APP_MAIN_1MS_TIMER , 10);		//1ms
	#else
	osal_start_reload_timer(App_main_TaskID, APP_MAIN_1MS_TIMER , 1);		//1ms
	#endif
	
	hal_pwrmgr_register(MOD_USR1, hal_usr1_sleep_handler, hal_usr1_wakeup_handler);
	hal_pwrmgr_lock(MOD_USR1);
	#if SET_BLE_SUB
	//FirmwareVersion
	LOG((char *)&FirmwareVersion);
	//221115:不需要扫描注网
	g_note.reg_status = 1;
	#endif
	g_runTime_s = 0;
	
	#if SET_NB||SET_CAT1
	g_device_info.test_mode = 1;
	#endif
	
	#if 1
	g_device_info.isBoot = 1;
	#else
	Event_AddEvent(EVENT_BOOT);
	#endif

	//固定模式
	#if SET_SMART
	service.m_proto_set = PROTO_SHIKE;
	#endif


	#if SET_SPI
		#if SET_NB||SET_CAT1
		g_isSpiTouch = 1;
		#else
		osal_set_event(App_main_TaskID,APP_MAIN_SPI);
		#endif
	#endif

	#if SET_USE_PSM
	g_device_info.wakeSrc = 1;
	#endif
	
	#if SET_CAT1
	g_cbSta.bDevTimeOut1s = 3;
	#endif
	
}


#if USE_SX1262
/**
 * @description: LoRa 100ms 定时 处理重发 等
 * @param {*}
 * @return {*}
 */
void rf_100ms_timeout(void)
{
    // 数据发送模式超时计时
    if (iStatus_RF != RFM98_DATA_RX && iStatus_RF != RFM98_SLEEP)
    {
        iTimer_RFM98_nRX++;
        if (iTimer_RFM98_nRX >= 50)
        { // 超时5秒未进入接收状态，重新配置RMF98模块
            iTimer_RFM98_nRX = 0;
							
            //[停用LoRa]RFM96_Config();      // setting base parameter
            //[停用LoRa]RFM96_LoRaEntryRx(); // 进入接收模式				
		}
    }
	
	//231230：重发要等上一次发送完才可以计时
	if(g_readySendBuf[SITE_CMD]==0)
	{		
		if (g_rf_info.re_send_times)
			g_rf_info.wait_ack_timer++; //需要重发
		else
		{
			g_rf_info.wait_ack_timer = 0;
			g_rf_info.is_wait_ack_cmd = 0;
		}
		if(g_rf_info.wait_ack_timer >= g_rf_info.wait_ack_time) 
		{
			//240219：休眠不需重发
			if(IS_PROTO_SHIKE &&(g_readySendBuf[SITE_DATA_STAT+2]==0))
			{
				g_rf_info.wait_ack_timer = 0;
				g_rf_info.is_wait_ack_cmd = 0;
				g_rf_info.re_send_times = 0;
				LOG("LORA OK\n");
			}	
			else
			{
				g_readySendBuf[SITE_CMD] = g_rf_info.is_wait_ack_cmd;
				g_rf_info.wait_ack_timer = 0;
				g_rf_info.re_send_times--;
				g_isRfRetry = 1;	
				LOG("Retry\r\n");
				
				if(g_rf_info.re_send_times==0)
				{
					g_rf_info.isLost = 1;
				}
			}	
			
		}
	}
}
#endif


void pRfSendBuf(void)
{
	//2.4g一定发送
	#if	SET_BLE_SUB
	//如果是LORA回复的命令，直接ACK
	note_comm_tran_up(g_readySendBuf,6,COMM_RESEND_TIME,g_rf_send.is_already_ack);
	g_rf_send.is_already_ack = 0;
	//reSendTimes = 0;
	//g_readySendBuf[SITE_CMD] = 0;
	#endif
	
	
	#if	USE_SX1262
	//221115:2.4g失败才需要lora首次同时发送
	LOG("isok_get_Lora=%d,send_fail_count=%d\r\n",g_device_info.isok_get_Lora,g_rf_send.send_fail_count);
	if((g_device_info.isok_get_Lora==1) && ((g_rf_send.send_fail_count>0)|| g_rf_info.is_lora_ack ))
	{
		g_rf_info.wait_ack_timer = 0;	//重发计时需要清空
		//[停用LoRa]LoRa_Ready_send();				//需要发送
	}
	#endif

	g_readySendBuf[SITE_CMD] = 0;
}

void p_RunPro(void)
{
	//固定模式
	#if SET_SMART
	service.m_proto_set = PROTO_SHIKE;
	#endif
	
	#if	SET_BLE_SUB
		#if SET_BLE_FAIL
		g_rf_send.send_fail_count = 1;
		#endif
	#endif
	
	if(g_is10msTouch)
	{
		g_is10msTouch = 0;

		//230605:
		//Fun_beep_scan();
		#if USE_SX1262	
		if(s_LoRaYCount>0)
		{
			s_LoRaYCount--;
		}
		#endif
		

		#if	SET_BLE_SUB
		p_Rf_Rx();
		note_process();

		//需要等注册上网关再休眠
		if(g_readySendBuf[SITE_CMD] && (note_cur_is_send()==0))
		{
			#if USE_SX1262
			if((g_device_info.isok_get_Lora==0 || iStatus_RF == RFM98_DATA_RX || iStatus_RF == RFM98_SLEEP))
			#else
			if(1)
			#endif
			{
				u8 isNeedSend = 0;
				//240219：每次lora发送都需要经过CAD检测
				if(IS_PROTO_SHIKE &&(g_device_info.isok_get_Lora==1))
				{
					if(s_LoRaYCount==0)
					{
						static u8 s_Cnt;

						if(g_isNeedChklora==0)
						{
							g_isNeedChklora = 1;
						}
							
						if(g_isChkloraOk)
						{
							//检测信道空闲才发送
							if((g_isLoraFree)||(s_Cnt>2))
							{	
								LOG("RF FREE\r\n");	
								isNeedSend = 1;	
								s_Cnt = 0;		//连续3次还是忙，也还是发送
							}				
							else
							{
								s_Cnt++;
								LOG("RF BUSY\r\n");
								isNeedSend = 0;
								//随机延时0.1-1s
								s_LoRaYCount = 10+ ((AP_TIM3->CurrentCount)%90);
							}
							g_isChkloraOk = 0;
							g_isNeedChklora = 0;
						}	
					}
					else
					{
						//LOG("RF WAIT\r\n");
						isNeedSend = 0;
					}
				}
				else
				{
					isNeedSend = 1;
				}
				
				if(isNeedSend)
				{
					pRfSendBuf();

					#if SET_BLE_SUB
					g_rf_send.is_already_ack = 0;
					g_readySendBuf[SITE_CMD] = 0;
					//不需要清空
					// if((g_device_info.isok_get_Lora==0) || (g_rf_send.send_fail_count==0))
					// {
					// 	g_rf_info.re_send_times = 0;
					// }
					if(g_device_info.isok_get_Lora==0)
					{
						g_rf_info.re_send_times = 0;
						g_rf_info.wait_ack_timer = 0;
					}
					#endif
				}
				
			}

			
		}
		#endif

		#if USE_SX1262
		if(g_device_info.isok_get_Lora==1)
		{
			// RFM98模块发送数据等待：64个数据约1秒,发送数据越少,等待时间越短!!!
			if (iStatus_RF == RFM98_WAIT_TX)
			{
				//LOG("-");
				if (READ_RF_IRQ())
				{                             // Packet send over 发送完成了IRQ 变为H,平时L ----持续时间
					iStatus_RF = RFM98_TX_OK; // 数据发送成功处理
					LOG("LORA-TX-OK\n");
					
					//等待应答
					if(sleepTimer<500)
							sleepTimer = 500;
					
				}
				else
				{
					if (++fTimer_RF_TX >= 10000)
					{ 
						iStatus_RF = RFM98_TX_FAIL;
						LOG("LORA-TX-FAIL\n");
					}
				}
			}
		}
		#endif
	}
	//消音操作
	if(g_device_info.DeviceStatus.b.is_mute==1)
	{
		g_device_info.DeviceStatus.b.is_mute = 0;
		Fun_beep_stop();
		
		if((service.m_smoke_state && ((g_cbSta.bAlmSta&BIT_SMOKE_STOP)==0)) || (service.m_temp_state && ((g_cbSta.bAlmSta&BIT_TEMP_STOP)==0)))
		{
			service.m_muffling = 1;	
			LOG("报警消音");
		}
		//无需消音
		else
		{
			service.m_muffling = 0;
		}
		
		if((g_cbSta.bAlmSta&BIT_CHECK)||(g_cbSta.bFcTimeOut1s>0))
		{
			g_cbSta.bFcTimeOut1s = 0; //防拆暂时可以取消，最多蜂鸣1分钟,后续看需求

			//230607:自检不能取消
			//g_cbSta.bAlmSta &= ~BIT_CHECK;
			LOG("取消自检/防拆");
		}
	}

	fKeyPro();

	#if SET_SPI==0
	vTaskUart();
	#endif
	
	#if (SET_NB||SET_CAT1)==0
	if(TouchSend.isGet)
		fDealEventQueuePro();			//处理队列		
	#endif
	
	#if USE_SX1262
	if(g_device_info.isok_get_Lora==1)
	{
		//100ms
		if(g_is100msTouch)
		{
			g_is100msTouch = 0;
			//[停用LoRa]rf_100ms_timeout();
		}
	}
	#endif
	
	
	//1s
	if(g_is1sTouch)
	{
		g_is1sTouch = 0;

		main_1s_timer();
	}

	#if SET_NB||SET_CAT1
	if(g_isSpiTouch)
	{
		//NB模块开机时，采用定时器来产生查询
		g_isSpiTouch = 0;

		//避开CLK被拉低,有可能处于标定过程
		if(READ_DIO() ==0)
		{
			DEBUG_LOG("标定\r\n");
		}
		else
		{
			MySPI_pro();
			
			if(pIsAlmNull())
			{
				service.m_muffling = 0;		//全部报警恢复,取消消音，取消暂停报警
			}
		}
	}
	#endif
}

void fSleepPro(void)
{
	#if SET_RX_TRAN
	sleepTimer = 10;
	#else

	
	#if SET_NB||SET_CAT1
//		#if SET_TEST
//		if((g_device_info.isNbSleep==0) && Event_IsEmpty()  && (FLASE==g_run_info.m_flg_retry) && (FLASE==g_run_info.m_flg_sending) && (FLASE==g_run_info.m_flg_reply)&& (g_cbSta.bDevTimeOut1s==0))
//		#else
		//230130:报警情况下模块不休眠
		if((g_device_info.isNbSleep==0) && Event_IsEmpty()  && (FLASE==g_run_info.m_flg_retry) && (FLASE==g_run_info.m_flg_sending) && (FLASE==g_run_info.m_flg_reply) && (g_cbSta.bDevTimeOut1s==0) && (fIsAlm()==0))
//		#endif
	{
		#if SET_NB
			#if NB_MODULE==NB_N306
			//[停用NB]NB_setBand();
			#endif
		#endif

		#if NB_MODULE==NB_N701
		LOG("软件关机\r\n");
		//[停用NB]p_nblc_common_func(TRUE, "ECPOWD=0",	"DOWN",3000, NULL,FALSE);
		#endif

		//直接断电
		#if SET_NB
		//230617:开机标定不操作进入PSM，直接掉电处理
			#if SET_USE_PSM
			if(g_cbSta.isBd==0)
				//[停用NB]NB_PsmSleep();

			g_cbSta.isBd = 0;

			if(Event_IsEmpty())
			{
				if(TRUE==g_run_info.m_flg_psm)
				{
					LOG("NB sleep\r\n");
				}
				//PSM进入不了则断电
				else
				{
					g_device_info.TryPsmCnt = 0;
					//[停用NB]SET_ME3616_POW_EN(OFF_POW_NB);
					LOG("NB off\r\n");
				}
				//g_run_info.m_nbiot_canbe_init = TRUE;
				#if SET_BLE_LEARN
				g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
				#endif
				g_run_info.m_nbiot_init_ok = FALSE;
				g_run_info.m_flg_sending = FALSE;
				g_run_info.m_flg_retry = FALSE;	
				g_device_info.isNbSleep = 1;	
				//g_cbSta.isBattLowSnd = 0;		//下次开机可以再次发送低压报警				
				//5分钟后断电处理，防止
				
				

				//230619: NB休眠5分钟过后，不再有异常唤醒查询模块状态和控制模块休眠机制
				g_device_info.bNbSleepTimeOut20s = 15;	// 
			}
			else
			{
				//不管进没进PSM模式，唤醒操作一下
				//[停用NB]NB_PsmWake();
			}
			#else
			g_run_info.m_flg_psm = FLASE;
		
			//[停用NB]SET_ME3616_POW_EN(OFF_POW_NB);

			//g_run_info.m_nbiot_canbe_init = TRUE;
			g_run_info.m_nbiot_init_ok = FALSE;
			g_run_info.m_flg_sending = FALSE;
			g_run_info.m_flg_retry = FALSE;	
			g_device_info.isNbSleep = 1;
			LOG("NB off\r\n");
			
			#if SET_BLE_LEARN
			g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
			#endif
			
			
			#endif
		#endif

		#if SET_CAT1
		//[停用CAT1]SET_LTE_PWR_C(1);
		//[停用CAT1]SET_LTE_RST_N(0);
		g_device_info.isNbSleep = 1;
		g_run_info.m_nbiot_init_ok = FALSE;
		g_run_info.m_flg_sending = FALSE;
		g_run_info.m_flg_retry = FALSE;	
		
		g_run_info.m_nbiot_init_sta = STA_4G_INT_0;
		LOG("CAT1 sleep\r\n");
		#endif
			
		//221103:等低压恢复
		//g_cbSta.isBattLowSnd = 0;		//下次开机可以再次发送低压报警
	}
	#endif
	
	#endif

	#if SET_USE_PSM
	if(g_device_info.isNbSleep &&(TRUE==g_run_info.m_flg_psm)&&(sleepTimer==0) && (g_device_info.bNbSleepTimeOut20s>0))
	{
		//230614:如果不是定时唤醒，则
		if((g_u16_m26l_rindex!=0) || (g_device_info.wakeSrc!=1))
		{
			DEBUG_LOG("PSM WAKE\r\n");
			//操作AT 确认一下是否处于唤醒状态,如果唤醒则操作休眠一次
			//dbg_printf("\r\n");
			//WaitMs(30);
			if (0/*[停用NB]原: p_nblc_communic_test() == TRUE*/)
			{
				g_device_info.TryPsmCnt++;
				if(g_device_info.TryPsmCnt<2)
				{
					DEBUG_LOG("唤醒\r\n");
					g_device_info.isNbSleep = 0;
					//g_run_info.m_flg_psm = FLASE;
					g_u16_m26l_rindex = 0;
					//NB_PsmWake();
				}
				else
				{
					g_device_info.TryPsmCnt = 0;
					//[停用NB]SET_ME3616_POW_EN(OFF_POW_NB);
					g_run_info.m_flg_psm = FLASE;
				}
			}
			g_u16_m26l_rindex = 0;
			g_device_info.wakeSrc = 1;			
		}
	}
	#endif
	
	
	/*休眠判断*/
	#if USE_SX1262
	#if 1
	//[停用LoRa]if (fIsNeedSleep() && (g_device_info.isBoot==0) && ((iStatus_RF==RFM98_SLEEP)||(g_device_info.isok_get_Lora==0)))
	#else
	//[停用LoRa]if (fIsNeedSleep() && ((iStatus_RF==RFM98_SLEEP)||(g_device_info.isok_get_Lora==0)))
	#endif
	#else
	if (fIsNeedSleep()) //非事件等待发送&&开机发送不出，会导致事件存在队列中，而无法休眠
	#endif
	{ 
		#if SET_BLE_SUB
		//休眠前保存一下
		if(g_note.tx_num != RF_Num)
		{
			RF_Num = g_note.tx_num;
			
			user_fs_updata_memory();
		}
		#endif
		
		extern u8 PirReg;
		if(PirReg)
		{
			PirReg = 0;
			hal_gpioin_unregister(PIR_IN);
		}

		DEBUG_LOG("Sleep:%d",Key[0].TCount);
		//TouchSend.isGet = 0;
		//准备进入休眠
		osal_stop_timerEx(App_main_TaskID, APP_MAIN_1MS_TIMER);		
		g_device_info.is_sleep = 1;
		
		hal_pwrmgr_unlock(MOD_USR1);			
		Fun_beep_stop();
		//不需要知道唤醒源
		//Touch_Wake_set();
		if(g_device_info.LedFlashSleep==0)
		{
			SET_LED_G(1);
		}
			
		#if	SET_BLE_SUB
		g_note.reg_timer = 0;
		#endif

		#if SET_SPI==0
		g_uart_ctrl_2.m_last_rx_time = my_time();
		#endif

		TouchSend.m_timeout10ms = 0; 
		SET_PWR_IR(0);

		#if SET_TEST_SLEEP
		//230605:测试STANDBY功耗
		{
			pwroff_cfg_t	mySleep_cfg;
			mySleep_cfg.type = POL_FALLING;
			mySleep_cfg.pin		= GPIO_P10 | GPIO_P15;
			mySleep_cfg.on_time = 100;
			hal_pwrmgr_enter_standby(&mySleep_cfg,2);
			//hal_pwrmgr_poweroff(&mySleep_cfg,1);
		}
		#endif
	}
}

#if SET_RX_TRAN
u8 g_isTran = 0;
#endif
uint16 app_main_process_Event(uint8 task_id, uint16 events)
{
	if(task_id != App_main_TaskID){
		return 0;
	}

	#if (SET_NB||SET_CAT1)==0	
	#if SET_SPI
	if( events & APP_MAIN_SPI){
		#if SET_RX_TRAN
		g_isTran = 1;
		MySPI_pro();
		g_isTran = 0;
		#else
		//避开CLK被拉低,有可能处于标定过程
		if(READ_DIO() ==0)
		{
			DEBUG_LOG("标定\r\n");
		}
		
		else
		{
			MySPI_pro();
			
			if(pIsAlmNull())
			{
				service.m_muffling = 0;		//全部报警恢复,取消消音，取消暂停报警
			}
		}
		#endif
		
		return (events ^ APP_MAIN_SPI);
	}
	#endif
	#endif

	if( events & APP_MAIN_LED_TIMER){
	
		#if SET_USE_PSM
		g_device_info.wakeSrc = 1;
		#endif
		
		//dbg_printf("LED");

		//LOG("LED\r\n");
		if(g_device_info.LedFlashSleep>0)
		{
			g_device_info.LedFlashSleep--;
			
			if((g_device_info.LedFlashSleep%2) ==0)
			{
				SET_LED_R(1);	
				SET_LED_G(1);
				//灭0.5s
				if(device_info.state.F_SM_ERR==1)
				{
					if(g_device_info.LedFlashSleep)
					{
						osal_stop_timerEx(App_main_TaskID, APP_MAIN_LED_TIMER);
						osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 500);
					}
				}
			}
			else
			{
				//未标定,红灯常亮
				//探测器故障，红灯每60s快闪2次并发出2声响提示音
				//if((device_info.state.F_SM_BD_OK==0)||(device_info.state.F_SM_ERR==1))
				if(device_info.state.F_SM_BD_OK==0)
				{
					SET_LED_R(0);	
					SET_LED_G(1);
				}
				//240807:烟腔故障，闪2次黄灯，亮1秒,灭0.5秒
				else if(device_info.state.F_SM_ERR==1)
				{
					SET_LED_R(0);	
					SET_LED_G(0);
					if(g_device_info.LedFlashSleep)
					{
						osal_stop_timerEx(App_main_TaskID, APP_MAIN_LED_TIMER);
						osal_start_reload_timer(App_main_TaskID, APP_MAIN_LED_TIMER , 1000);
					}
					
				}
				//探测器低电，黄灯每60s快闪1次并发出提示音
				else if(service.m_battery_state)
				{
					SET_LED_G(0);
					SET_LED_R(0);
				}
				//报警,消音后,红灯每60s快闪1次
				else if((service.m_temp_state)||(service.m_smoke_state))
				{
					SET_LED_G(1);	
					SET_LED_R(0);
				}
				//工作指示，绿灯每60s闪1次表示探测器工作正常
				else
				{
					SET_LED_G(0);	
					SET_LED_R(1);
				}
			}		
		}
		
		if(g_device_info.LedFlashSleep==0)
		{
			osal_stop_timerEx(App_main_TaskID, APP_MAIN_LED_TIMER);
		}
		return (events ^ APP_MAIN_LED_TIMER);
	}
	
	
	//1ms
	if( events & APP_MAIN_1MS_TIMER){
		if(WatchDogFlag==0)
		{
			WatchDogFlag = 1;
			
			#if SET_NB||SET_CAT1
			bWatchDog1s = 480;
			#endif
		}	

		//230614:如果不是定时唤醒，则应该是串口唤醒，如果串口唤醒，应该操作重新进入PSM
		//dbg_printf("1ms");
		
		#if SET_NB||SET_CAT1
		if(g_device_info.is_sleep==0)
		{
			#if SET_RX_TRAN==0
			if(g_system_ota==0)
			{
				//有事件才需要启动模块
				if(IsNeedRfSleep()==0)
				{
					//[停用NB]NB_pro();
				}

				//4分钟无信号清除事件
				if(g_cbSta.bNetTimeOut1s==0)
				{			
					EVENT_Init();	//清除事件
					g_run_info.m_flg_sending = FALSE;	//清除正在发送等回复
					g_run_info.m_flg_retry = FALSE;		//清除重发标志
				}
				#if SET_NB||SET_CAT1
				else if(Event_IsEmpty()  && (FLASE==g_run_info.m_flg_retry) && (FLASE==g_run_info.m_flg_sending) && (FLASE==g_run_info.m_flg_reply))
				{
					g_cbSta.bNetTimeOut1s = 0;
				}
				#endif
			}
			#if SET_CAT1
			//OTA状态：模块开启
			//else
			//{
			//	NB_pro();
			//}
			#endif
			
			#endif

			p_RunPro();

			#if SET_TEST_MODE
				sleepTimer = 10;
			#else
			
				#if SET_CAT1
				if(g_system_ota==0)
				{
					fSleepPro();
				}
				#else
				fSleepPro();
				#endif
			
			#endif
			
		}
		#else

		#if USE_SX1262
		if(g_device_info.isok_get_Lora==1)
		{
			//[停用LoRa]RFM98_Process();

			if(g_isNeedChklora)
			{
				if(0/*[停用LoRa]原RF_cad_check_channel2*/==0)
				{
					g_isLoraFree =  1;
				}
				else
				{
					g_isLoraFree =  0;
				}	
				g_isChkloraOk = 1;
			}
			 
		}	
		#endif
		

		p_RunPro();
		fSleepPro();	
		#endif
		
		return (events ^ APP_MAIN_1MS_TIMER);
	}
	
	#if SET_CAT1
	//重启任务
	if(events & APP_RESET_EVT )
	{
		WaitMs(200);
		hal_system_soft_reset();
		return (events ^ APP_RESET_EVT);
	}
	#endif

  return 0;
}
