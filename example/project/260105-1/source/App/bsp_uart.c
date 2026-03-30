#include <string.h>
#include "bsp_uart.h"


#if SET_NB||SET_CAT1
COMx_Define	  COM1;
u8	 TX1_Buffer[COM_TX1_Lenth];		//发送缓冲
u8 	 RX1_Buffer[COM_RX1_Lenth];		//接收缓冲

static void uart0_rx_callback(uart_Evt_t *pev)
{
    switch (pev->type)
    {
        case UART_EVT_TYPE_RX_DATA:
        case UART_EVT_TYPE_RX_DATA_TO:
			//if(COM1.B_RX_OK == 0)
			{
				if(COM1.RX_Cnt >= COM_RX1_Lenth)	COM1.RX_Cnt = 0;

				for (uint8_t i = 0; i < pev->len; i++)
				{
					RX1_Buffer[COM1.RX_Cnt++] = pev->data[i];
					
					#if SET_NB_LOG
					UART_putc(RX1_Buffer[COM1.RX_Cnt-1]);		//透传需要改成buff模式
					#endif
				}						
				
				COM1.RX_TimeOut = TimeOutSet1;
			}
			break;
    }
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
//#define	SIZE_LOG_BUF	660
//uint8 LOG_TX_BUF[SIZE_LOG_BUF];
#if SET_RX_TRAN
void uart_port_reconfig(void)
{
		uart_Cfg_t cfg_user =
    {
			
        .tx_pin = P9,

		#if SET_RX_TRAN
		.rx_pin = GPIO_DUMMY,
		#else
        .rx_pin = P10,
		#endif		
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
				.baudrate = 38400,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity     = FALSE,
        .evt_handler = uart0_rx_callback,
    };
		
    hal_uart_deinit(UART0);
	#if SET_RX_TRAN
	hal_gpio_fmux(P9,Bit_DISABLE);
    hal_gpio_pin_init(P9, OEN);
	#else	
    hal_gpio_fmux(P9,Bit_DISABLE);
    hal_gpio_fmux(P10,Bit_DISABLE);
    hal_gpio_pin_init(P9, OEN);
    hal_gpio_pin_init(P10,IE);
	#endif
		
    hal_uart_init(cfg_user,UART0);//uart init
		

    LOG("uart nb port...\n");		
}
#else

#if SET_NB_LOG
#define	SIZE_LOG_BUF	660
uint8 LOG_TX_BUF[SIZE_LOG_BUF];
void uart_nb_log(void)
{
	uart_Cfg_t cfg_user =
    {
        .tx_pin = P15,
				.rx_pin = GPIO_DUMMY,	
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
				.baudrate = 460800,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
      //  .use_tx_buf = FALSE,
			.use_tx_buf = TRUE,
        .parity     = FALSE,
        .evt_handler = NULL,
    };		
    hal_uart_deinit(UART0);	
    hal_gpio_fmux(P15,Bit_DISABLE);
    hal_gpio_pin_init(P15, OEN);
    hal_uart_init(cfg_user,UART0);//uart init
		
		hal_uart_set_tx_buf(UART0,LOG_TX_BUF,SIZE_LOG_BUF);
}
#endif

void uart_port_reconfig(void)
{
	#if  SET_NB_LOG
	uart_nb_log();
	#endif

	uart_Cfg_t cfg_user =
    {

        .tx_pin = P9,
        .rx_pin = P10,
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
				#if NB_MODULE==NB_N701
				.baudrate = 115200,
				#else
				.baudrate = 9600,
				#endif
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity     = FALSE,
        .evt_handler = uart0_rx_callback,
    };
		#if SET_NB_LOG
		hal_uart_deinit(UART1);
		#else
    	hal_uart_deinit(UART0);
		#endif
		

    hal_gpio_fmux(P9,Bit_DISABLE);
    hal_gpio_fmux(P10,Bit_DISABLE);
    hal_gpio_pin_init(P9, OEN);
    hal_gpio_pin_init(P10,IE);
    //m_uartCtx.enable = FALSE;
		
		#if SET_NB_LOG
		hal_uart_init(cfg_user,UART1);
		#else
    hal_uart_init(cfg_user,UART0);//uart init
		#endif
		
	//hal_uart_set_tx_buf(UART1,LOG_TX_BUF,SIZE_LOG_BUF);

    LOG("uart nb port...\n");		
}
#endif
void TX1_write2buff(u8 dat)	//写入发送缓冲，指针+1
{
	#if SET_NB_LOG
	hal_uart_send_byte(UART1,dat);

	//241108：加一下发送AT的日志
	hal_uart_send_byte(UART0,dat);
	#else
	hal_uart_send_byte(UART0,dat);
	#endif
}
#else
void uart_port_reconfig(void)
{	
    uart_Cfg_t cfg_user =
    {
		#if USE_SX1262
		.tx_pin = P15,
    .rx_pin = GPIO_DUMMY,
		#else
        .tx_pin = P9,
        .rx_pin = P10,
		#endif
        //.rx_pin = GPIO_DUMMY,
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
        //.baudrate = 460800,
		#if SET_RX_TRAN
		.baudrate = 38400,
		#else
		.baudrate = 115200,
        #endif
		.use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
				//.use_tx_buf = TRUE,
        .parity     = FALSE,
        .evt_handler = NULL,
    };

    hal_uart_deinit(UART0);
	#if USE_SX1262
	hal_gpio_fmux(P15,Bit_DISABLE);
    hal_gpio_pin_init(P15, OEN);
	#else
    hal_gpio_fmux(P9,Bit_DISABLE);
    hal_gpio_fmux(P10,Bit_DISABLE);
    hal_gpio_pin_init(P9, OEN);
    hal_gpio_pin_init(P10,IE);
	#endif
    //m_uartCtx.enable = FALSE;
    hal_uart_init(cfg_user,UART0);//uart init
		
	//hal_uart_set_tx_buf(UART0,LOG_TX_BUF,SIZE_LOG_BUF);
    LOG("uart new port...\n");		
}
#endif

/*********************************************驱动************************************************/
#if SET_SPI==0
UART_CTRL  g_uart_ctrl_2            = {0x00};


void TX2_write2buff(u8 dat)	//写入发送缓冲，指针+1
{
	//hal_uart_send_byte(UART1,dat);
	hal_uart_send_buff(UART1,&dat,1);
}

void TX2_send_data(u8 *payload_dat, u16 payload_len)
{
	// u16 i = 0;
	// for (i = 0; i < payload_len; i++)
	// {
	// 	TX2_write2buff(payload_dat[i]);
	// }
	hal_uart_send_buff(UART1,&payload_dat[0],payload_len);
}

void PrintString2(u8 *put)
{
	while((*put) != '\0')
	{
		TX2_write2buff(*put);
		put++;
	}
}

#if SET_NB_LOG==0
static void uart1_rx_callback(uart_Evt_t *pev)
{
    switch (pev->type)
    {
        case UART_EVT_TYPE_RX_DATA:
        case UART_EVT_TYPE_RX_DATA_TO:
			//if(COM2.B_RX_OK == 0)
			{				
				if(g_uart_ctrl_2.m_rx_write >= UART_RX_LEN)	g_uart_ctrl_2.m_rx_write = 0;

				for (uint8_t i = 0; i < pev->len; i++)
				{
					//需要判断接收头为0xaa
					if(g_uart_ctrl_2.m_rx_write==0)
					{
						if(pev->data[i]==0xaa)
						{
							g_uart_ctrl_2.m_rx_buf[g_uart_ctrl_2.m_rx_write++] = pev->data[i];
							//LOG("%x",(u16)pev->data[i]);	//透传	
						}
					}
					else
					{
						g_uart_ctrl_2.m_rx_buf[g_uart_ctrl_2.m_rx_write++] = pev->data[i];
						//LOG("%x",(u16)pev->data[i]);	//透传	
					}
					
					//测试透传显示
					//hal_uart_send_byte(UART0,g_uart_ctrl_2.m_rx_buf[g_uart_ctrl_2.m_rx_write-1]);//透传
					//hal_uart_send_byte(UART1,pev->data[i]);				
				}	

				g_uart_ctrl_2.m_last_rx_time = my_time();
				TouchSend.isSleep = 0;  //串口未休眠
			}
			break;
    }
}
#endif
#endif

/********************* 串口配置************************/
void	UART_config(void)
{
	uart_port_reconfig();
	#if SET_SPI
	initMySPI();
	#else
		#if SET_NB_LOG
		#else
		uart_Cfg_t cfg_user = {
				.tx_pin = GPIO_P03,
			.rx_pin = GPIO_P02,
			.rts_pin = GPIO_DUMMY,
			.cts_pin = GPIO_DUMMY,
			//.baudrate = 9600,
				.baudrate = 38400,
			.use_fifo = TRUE,
			.hw_fwctrl = FALSE,
			.use_tx_buf = FALSE,
				//.use_tx_buf = TRUE,
			.parity     = FALSE,
			.evt_handler = uart1_rx_callback,
		};
		
		hal_uart_deinit(UART1);	
		hal_gpio_pin_init(GPIO_P03,OEN);
		hal_gpio_pin_init(GPIO_P02,IE);
		hal_uart_init(cfg_user,UART1);
		memset(&g_uart_ctrl_2,0,sizeof(g_uart_ctrl_2));
		
		//hal_uart_set_tx_buf(UART1,g_uart_ctrl_2.m_tx_buf,UART_TX_LEN);	
		g_uart_ctrl_2.m_last_rx_time = my_time();
		#endif
	#endif	
}

/*********************************************应用************************************************/

SM_FG state_last;						//记忆状态
R_SM device_info;						//烟感状态
service_t  service;						//平台协议状态
QUEUE_OBJ_t 	TouchSend;				//串口发送队列
CbSta_InitTypeDef  g_cbSta;	

#if  SET_SPI==0
void TouchSndCmd(u8 cmd)
{
	#if SET_NB_LOG
	return;
	#else
	//烟雾浓雾值不一致时需要下发设置
	service.m_smoke_set_val = (u16)service.m_smoke_set*0xff/100;
	
	TouchSend.m_tx_buf[1] = cmd;
	TouchSend.m_tx_buf[2] =	service.m_smoke_set_val;
	TouchSend.m_tx_buf[3] = TouchSend.m_tx_buf[1]+TouchSend.m_tx_buf[2];
	TouchSend.m_tx_buf[4] = 0x55;
	
	TouchSend.len = 3;
	TX2_send_data(&TouchSend.m_tx_buf[1],TouchSend.len);
	SM_LOG("Tx,3:");
	SM_LOG_BYTE(&TouchSend.m_tx_buf[1],TouchSend.len);
	TouchSend.m_timeout10ms = 30;		//等待回复命令
	
	TouchSend.isSleep = 0;
	#endif
}
#endif

/**
* @brief  串口主任务
* @param  None
* @retval None
*/


//取值Rmin 只要大于>Rmin就是该温度
//还是取标准即可
//3380  10K
u16 const CODE_TEMP_R_REG[] = {
	//R(1Ω)		T(℃)
	27493,		//   0.0
	26324,		//   1
	25211,		//   2
	24152,		//   3
	23144,		//   4
	22183,		//   5
	21268,		//   6
	20395,		//   7
	19564,		//   8
	18771,		//   9
	18015,		//   10
	17293,		//   11
	16604,		//   12
	15947,		//   13
	15319,		//   14
	14720,		//   15
	14147,		//   16
	13600,		//   17
	13077,		//   18
	12577,		//   19
	12098,		//   20
	11641,		//   21
	11203,		//   22
	10784,		//   23
	10383,		//   24
	10000,		//   25
	9632,		//   26
	9280,		//   27
	8942,		//   28
	8619,		//   29
	8309,		//   30
	8012,		//   31
	7727,		//   32
	7454,		//   33
	7191,		//   34
	6940,		//   35
	6698,		//   36
	6466,		//   37
	6244,		//   38
	6030,		//   39
	5825,		//   40
	5627,		//   41
	5438,		//   42
	5255,		//   43
	5080,		//   44
	4911,		//   45
	4749,		//   46
	4593,		//   47
	4443,		//   48
	4299,	    //   49
	4160,	    //   50
	4026,		//   51
	3898,	    //   52
	3773,		//   53
	3654,		//   54
	3539,		//   55
	3428,		//   56
	3321,		//   57
	3218,		//   58
	3119,		//   59
	3023,		//   60
	2931,		//   61
	2841,		//   62
	2755,		//   63
	2672,		//   64
	2592,		//   65
	2515,		//   66
	2441,		//   67
	2369,		//   68
	2299,		//   69
	2232,		//   70
	2167,		//   71
	2104,		//   72
	2044,		//   73
	1985,		//   74
	1928,		//   75
	1874,		//   76
	1821,		//   77
	1769,		//   78
	1720,		//   79
	1672,		//   80
	1626,		//   81
	1581,		//   82
	1537,		//   83
	1495,		//   84
	1455,		//   85
	1415,		//   86
	1377,			//   87
	1340,			//   88
	1305,			//   89
	1270,			//   90
	1236,			//   91
	1204,			//   92
	1172,			//   93
	1142,			//   94
	1112,			//   95
	1083,			//   96
	1056,			//   97
	1029,			//   98
	1002,			//   99
	977,			//   100
	952,
	928,
	905,
	883,
	861,			//105
	839,
	819,
	799,
	779,
	760,
	742,
	724,
	707,
	690,
	674,
	658,
	642,
	627,
	613,
	599,		//120
	582,
	569,
	556,
	543,
	531,	//125

};

	
/* Function */
u8 Tool_GetTemByRes(	/* 通过电阻取得温度 */
	u16 valRes
)
{
	u8 i;
	
	for(i = 0; i < sizeof(CODE_TEMP_R_REG)/2; i++){	/* 查找表 */
		
		if(valRes >= CODE_TEMP_R_REG[i]){	/* 查到温度 */
			return i;		
		}
	}
	return 125;	//最多125℃
}

void pGetTemp(void)
{
	// static u8  isTempFlash;
	#if SET_SPI==0

	static u8 tempHCnt,tempHrCnt;
	#if SET_NB==0
	static u8 tempLCnt,tempLrCnt;
	#endif

	#endif
	//查表转换成温度值
	//ADC 转换成温度 NTC+10K

	// #define C_T_AD_DATA_L1		79		//;Low temperature secondary AD  <5℃   5
	// #define C_T_AD_DATA_L2		91		//;Low temperature AD			 <10℃  3   
	// #define	C_T_AD_DATA_L3		151		//;Normal temperature AD         >36℃
	// #define	C_T_AD_DATA_L4		177		//;High-temperature first-level AD, greater than C_T_AD_DATA_L4 judged as high-temperature second-level AD >49℃
	//240923:低温
	if(device_info.t_adc<50)
	{
		service.m_temp = 0;
	}
	else
	{
		service.m_temp = Tool_GetTemByRes(10000*(u32)(255-device_info.t_adc)/device_info.t_adc);
	}
	
	
	if(service.m_tempMax<service.m_temp)
	{
		service.m_tempMax = service.m_temp;
	}
	SM_LOG("service.m_temp = %d ℃\n",(u16)service.m_temp);
	
	//测试
	// if(isTempFlash)
	// {
	// 	isTempFlash = 0;
	// 	service.m_temp = 55;
	// }
	// else
	// {
	// 	isTempFlash = 1;
	// 	service.m_temp = 54;
	// }
	

	//高低温都关闭
	if((service.m_temp_almset==0xff)&&(service.m_temp_lowset==0xff))
	{
		service.m_temp_alm1s = 0;		
	}

	//关闭高温检测
	if(service.m_temp_almset==0xff)
	{
		service.m_temp_state &= ~0x1;
	}
	else
	{
		//高温报警判断--12s持续高温
		if(service.m_temp>=service.m_temp_almset)
		{
			#if SET_SPI
			service.m_temp_almRest1s = 0;
			if(service.m_temp_alm1s<0xff)
			{
				service.m_temp_alm1s++;
			}
			if(service.m_temp_alm1s>=TEMP_ALM_1S)
			{
				service.m_temp_state = 1;
			}
			#else
			if(tempHCnt<0xff)
				tempHCnt++;
			
			service.m_temp_almRest1s = 0;
			
			if(service.m_temp_alm1s==0)
			{
				service.m_temp_alm1s = 1;
			}

			//正常采样4s一次,但有时候可能采样速度比较快(休眠20s定时器意义不大)
			//if(tempHCnt>=2)
			//8秒采样一次
			if((tempHCnt>=2)&&(service.m_temp_alm1s>=TEMP_ALM_1S))
			{
				service.m_temp_state = 1;
			}
			#endif
		}
		else if(service.m_temp<=service.m_temp_rstset)
		{
			#if SET_SPI
			service.m_temp_alm1s = 0;

			if(service.m_temp_almRest1s==0)
			{
				service.m_temp_almRest1s = 1;
			}
			else
			{
				if(service.m_temp_almRest1s<0xff-SET_SPI_GAP)
				{
					if(WatchDogFlagFast)
					{
						service.m_temp_almRest1s++;
					}
					else
					{
						service.m_temp_almRest1s += SET_SPI_GAP;
					}
				}	
			}

			if(service.m_temp_almRest1s>=TEMP_ALMREST_1S)
			{
				service.m_temp_state &= ~0x1;
			}
			#else
			tempHCnt = 0;
			if(tempHrCnt<0xff)
				tempHrCnt++;

			service.m_temp_alm1s = 0;
			if(service.m_temp_almRest1s==0)
			{
				service.m_temp_almRest1s = 1;
			}	
			
			//if(tempHrCnt>=2)
			if((tempHrCnt>=2)&&(service.m_temp_almRest1s>=TEMP_ALMREST_1S))
			{
				service.m_temp_state &= ~0x1;
			}
			#endif
		}
		//中间态不改变
		else
		{
		}
	}

	//NB版本关闭低温报警
	#if SET_NB||SET_CAT1
	service.m_temp_state &= ~0x2;
	#else
	//关闭低温检测
	if(service.m_temp_lowset==0xff)
	{
		service.m_temp_state &= ~0x2;
	}
	else
	{
		//低温报警判断--10s持续低温
		if(service.m_temp<=service.m_temp_lowset)
		{
			#if SET_SPI
			service.m_temp_almRest1s = 0;
			if(service.m_temp_alm1s<0xff)
			{
				service.m_temp_alm1s++;
			}
			if(service.m_temp_alm1s>=TEMP_ALM_1S)
			{
				service.m_temp_state = 2;
			}
			#else
			tempLrCnt = 0;
			if(tempLCnt<0xff)
				tempLCnt++;

			service.m_temp_almRest1s = 0;
			if(service.m_temp_alm1s==0)
			{
				service.m_temp_alm1s = 1;
			}

			//if(tempLCnt>=2)
			if((tempLCnt>=2)&&(service.m_temp_alm1s>=TEMP_ALM_1S))
			{
				service.m_temp_state = 2;
			}
			#endif

		}
		else if(service.m_temp>=service.m_temp_lowrstset)
		{
			#if SET_SPI
			service.m_temp_alm1s = 0;

			if(service.m_temp_almRest1s==0)
			{
				service.m_temp_almRest1s = 1;
			}
			else
			{
				if(service.m_temp_almRest1s<0xff-SET_SPI_GAP)
				{
					if(WatchDogFlagFast)
					{
						service.m_temp_almRest1s++;
					}
					else
					{
						service.m_temp_almRest1s += SET_SPI_GAP;
					}
				}	
			}			
			#else
			tempLCnt = 0;
			if(tempLrCnt<0xff)
				tempLrCnt++;

			service.m_temp_alm1s = 0;
			if(service.m_temp_almRest1s==0)
			{
				service.m_temp_almRest1s = 1;
			}

			//if(tempLrCnt>=2)
			if((tempLrCnt>=2)&&(service.m_temp_almRest1s>=TEMP_ALMREST_1S))
			{
				service.m_temp_state &= ~0x2;
			}
			#endif
		}
		//中间态不改变
		else
		{
		}
	}
	#endif
	

	//高温报警
	if(service.m_temp_state==1)
	{
		if((g_cbSta.bAlmSta&BIT_TEMP_STOP)!=0)
		{
			//高温报警并不会主动1s推送，没有超时退出暂停报警状态
			//?主动查询
			#if 0
			if(g_cbSta.bAlmTempTimeOut1s==0)
			{
				g_cbSta.bAlmSta &= ~BIT_TEMP_STOP;		//退出暂停报警状态
				
				//报警间隔
				if(g_cbSta.bTempSndTimeOut1s==0)	
				{
					g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
					Event_AddEvent(EVENT_TEMP);
					LOG("高温报警\r\n");
				}
			}
			#endif
		}
		else
		{
			//报警间隔
			if(g_cbSta.bTempSndTimeOut1s==0)	
			{
				#if SET_NB||SET_CAT1
				if(g_cbSta.bTempSndlast==0)
				{
					//新警
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
				}
				g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
					
				#else
				//g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
				if(g_cbSta.bTempSndlast==0)
				{
					g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;

					//新警
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
						
					if(IS_PROTO_SHIKE)
					{
						g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
						Event_AddEvent(EVENT_TEMP);
						LOG("高温报警\r\n");
					}
				}
				else
				{
					//超过12小时，固定12小时1次
					if(g_cbSta.bTempSndlast>=32768UL)
					{
						g_cbSta.bTempSndTimeOut1s = 43200UL;
					}
					else
					{
						g_cbSta.bTempSndTimeOut1s = 2*g_cbSta.bTempSndlast;
					}
				}
				#endif


				//230914:NB版本取消反复报警
				#if SET_NB||SET_CAT1
				if(g_cbSta.bTempSndlast==0)
				{
					g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;

					Event_AddEvent(EVENT_TEMP);
					LOG("高温报警\r\n");
				}
				#else
				
				if(service.m_proto_set==PROTO_ZH202)
				{
					g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
					Event_AddEvent(EVENT_TEMP);
					LOG("高温报警\r\n");
				}
				#endif
			}
		}
		
		//g_cbSta.bAlmTempTimeOut1s = 11;		
		service.m_temp_state_last = 1;
	}
	//低温报警
	else if(service.m_temp_state==2)
	{
		if((g_cbSta.bAlmSta&BIT_TEMP_STOP)!=0)
		{
			#if 0
			if(g_cbSta.bAlmTempTimeOut1s==0)
			{
				g_cbSta.bAlmSta &= ~BIT_TEMP_STOP;		//退出暂停报警状态
				
				//报警间隔
				if(g_cbSta.bTempSndTimeOut1s==0)	
				{
					g_cbSta.bTempSndTimeOut1s = 60;
					Event_AddEvent(EVENT_TEMP_LOW);
					LOG("低温报警\r\n");
				}
			}
			#endif
		}
		else
		{
			//报警间隔
			if(g_cbSta.bTempSndTimeOut1s==0)	
			{
				#if SET_NB||SET_CAT1
				if(g_cbSta.bTempSndlast==0)
				{
					//新警
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
				}
				g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;

				#else
				//g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;
				if(g_cbSta.bTempSndlast==0)
				{
					g_cbSta.bTempSndTimeOut1s = GAP_ALM_1S;

					//新警
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
					
					if(IS_PROTO_SHIKE)
					{
						g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
				
						Event_AddEvent(EVENT_TEMP_LOW);
						LOG("低温报警\r\n");
					}
				}
				else
				{
					//超过12小时，固定12小时1次
					if(g_cbSta.bTempSndlast>=32768UL)
					{
						g_cbSta.bTempSndTimeOut1s = 43200UL;
					}
					else
					{
						g_cbSta.bTempSndTimeOut1s = 2*g_cbSta.bTempSndlast;
					}
				}
				#endif

				//230914:NB版本取消反复报警
				#if SET_NB||SET_CAT1
				if(g_cbSta.bTempSndlast==0)
				{
					g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
				
					Event_AddEvent(EVENT_TEMP_LOW);
					LOG("低温报警\r\n");
				}
				#else
				if(service.m_proto_set==PROTO_ZH202)
				{
					g_cbSta.bTempSndlast = g_cbSta.bTempSndTimeOut1s;
				
					Event_AddEvent(EVENT_TEMP_LOW);
					LOG("低温报警\r\n");
				}
				
				#endif
				
			}
		}
		
		//g_cbSta.bAlmTempTimeOut1s = 11;		
		service.m_temp_state_last = 2;
	}
	else
	{
		g_cbSta.bAlmSta &= ~BIT_TEMP_STOP;		//退出暂停报警状态
		
		if(service.m_temp_state_last!=0)
		{
			if((service.m_temp_state_last&0x01)==1)
			{
				Event_AddEvent(EVENT_TEMP_RST);
				LOG("高温恢复\r\n");
			}
			if((service.m_temp_state_last&0x02)==2)
			{
				Event_AddEvent(EVENT_TEMP_LOW_RST);
				LOG("低温恢复\r\n");
			}
		}
		service.m_temp_state_last = 0;
		//g_cbSta.bAlmTempTimeOut1s = 0;
		g_cbSta.bTempSndTimeOut1s = 0;
		g_cbSta.bTempSndlast = 0;
		g_cbSta.bTempAlmId = 0;
	}
}

void almRestTick(u8 *Tick)
{
	if(*Tick<0xff)
	{
		if(*Tick==0)
		{
			*Tick = 1;
		}
		else
		{
			if(*Tick<0xff-SET_SPI_GAP)
			{
				if(WatchDogFlagFast)
				{
					*Tick += 1;
				}
				else
				{
					*Tick += SET_SPI_GAP;
				}
			}	
		}
	}
}

void pGetSmoke(void)
{
	#if SET_SPI==0
	static u8 bSmokeAlmCnt = 0;
	static u8 bSmokeOkCnt = 0;
	static u8 s_isAlm;
	static u8 s_isSet;
	u8 compen;
	#endif


	//230427:阈值以本地为准，上报不一致则需要下发设置
	//烟雾浓度合法性检查
	if((service.m_smoke_set<10)||(service.m_smoke_set>100))
	{
		service.m_smoke_set = DEFAULT_SMOKE_SET;
	}
	// 250220:烟雾浓度可编程
	//service.m_smoke_set = DEFAULT_SMOKE_SET;		//231113:固定40烟雾浓度
	

	service.m_smoke_set_val = (u16)service.m_smoke_set*0xff/100;

	#if SET_SPI==0
	//230427:通过比对上报阈值来决定设置与否
	if(device_info.bd_alarm != service.m_smoke_set_val)
	{
		//不频繁设置
		if(s_isSet==0)
		{
			s_isSet = 1;
			TouchSend.m_flag_set = 1;
		}
		else
		{
			s_isSet = 0;
		}
	}
	else
	{
		s_isSet = 0;
	}
	#endif

	#if SET_SPI
	SM_LOG("烟腔故障 = %d \n",(u16)device_info.state.F_SM_ERR);
	SM_LOG("标定状态 = %d \n",(u16)device_info.state.F_SM_BD);
	SM_LOG("标定完成 = %d \n",(u16)device_info.state.F_SM_BD_OK);
	SM_LOG("零点值 = %d \n",(u16)device_info.zero_data);
	SM_LOG("发射值 = %d \n",(u16)device_info.ir_data);
	SM_LOG("发射差值 = %d \n",(u16)device_info.ir_ad);
	SM_LOG("标定值 = %d \n",(u16)device_info.bd_zero);
	#endif	
	//230609:烟腔故障，不能再判断烟雾告警，烟雾浓度99修正为100
	if(device_info.state.F_SM_ERR)
	{
		service.m_smoke_errRest1s = 0;
		if(service.m_smoke_err1s<600)
		{
			if(WatchDogFlagFast)
			{
				service.m_smoke_err1s++;
			}
			else
			{
				service.m_smoke_err1s += SET_SPI_GAP;
			}
		}
		//10分钟烟腔故障才上报
		if(service.m_smoke_err1s>=600)
		{
			if(device_info.isErr==0)
			{
				device_info.isErr = 1;
				Event_AddEvent(EVENT_PLUS);
			}
		}
		return;
	}
	else
	{
		service.m_smoke_err1s = 0;
		if(service.m_smoke_errRest1s<600)
		{
			if(WatchDogFlagFast)
			{
				service.m_smoke_errRest1s++;
			}
			else
			{
				service.m_smoke_errRest1s += SET_SPI_GAP;
			}
		}
		
		if(service.m_smoke_errRest1s>=600)
		{
			if(device_info.isErr)
			{
				device_info.isErr = 0;
				Event_AddEvent(EVENT_PLUS);
			}			
		}
	}


	//SM_LOG("device_info.alarm = %d\n",device_info.alarm);
	//SM_LOG("service.m_smoke_set = %d\n",service.m_smoke_set);
	
	#if SET_SPI
	//实际需要根据温度补偿
	
	#define C_T_AD_ERR_L 		2		//;If temperature AD is less than this value, no temperature compensation is done
	#define C_T_AD_ERR_H 		254		//;If the temperature AD is greater than this value, no temperature compensation is done
	
	#define C_T_AD_DATA_L1		79		//;Low temperature secondary AD
	#define C_T_AD_DATA_L2		91		//;Low temperature AD
	#define	C_T_AD_DATA_L3		151		//;Normal temperature AD
	#define	C_T_AD_DATA_L4		177		//;High-temperature first-level AD, greater than C_T_AD_DATA_L4 judged as high-temperature second-level AD
	
	#define	C_T_ADD_L2			5		//;Low temperature secondary R_SM_ALARM compensation
	#define	C_T_ADD_L1			3		//;Low temperature level R_SM_ALARM compensation
	#define	C_T_SUB_L1			5		//;High temperature first level R_SM_ALARM compensation
	#define C_T_SUB_L2			7		//;High temperature secondary R_SM_ALARM compensation
	//温补算法
	if(device_info.t_adc>=C_T_AD_ERR_L)
	{
		//低温补偿
		if(device_info.t_adc<C_T_AD_DATA_L1)
		{
//			if(device_info.ir_ad>C_T_ADD_L2)
//			{
//				device_info.ir_ad -= C_T_ADD_L2; 
//			}
//			else
//			{
//				device_info.ir_ad = 0;
//			}
			if(0xff-device_info.ir_ad>=C_T_ADD_L2)
			device_info.ir_ad += C_T_ADD_L2;
			
		}
		else if(device_info.t_adc<C_T_AD_DATA_L2)
		{
//			if(device_info.ir_ad>C_T_ADD_L1)
//			{
//				device_info.ir_ad -= C_T_ADD_L1; 
//			}
//			else
//			{
//				device_info.ir_ad = 0;
//			}
			if(0xff-device_info.ir_ad>=C_T_ADD_L1)
				device_info.ir_ad += C_T_ADD_L1;
		}
		//高温补偿
		else if((device_info.t_adc<C_T_AD_DATA_L4)&&(device_info.t_adc >= C_T_AD_DATA_L3))
		{
//			device_info.ir_ad += C_T_SUB_L1;
			if(device_info.ir_ad>C_T_SUB_L1)
			{
				device_info.ir_ad -= C_T_SUB_L1; 
			}
			else
			{
				device_info.ir_ad = 0;
			}			
		}
		else if((device_info.t_adc<C_T_AD_ERR_H)&&(device_info.t_adc >= C_T_AD_DATA_L3))
		{
//			device_info.ir_ad += C_T_SUB_L2; 
			if(device_info.ir_ad>C_T_SUB_L2)
			{
				device_info.ir_ad -= C_T_SUB_L2; 
			}
			else
			{
				device_info.ir_ad = 0;
			}
		}
	}
	#endif


	//烟雾百分比
	//发射差值-标定值？
	if(device_info.ir_ad>device_info.bd_zero)
	{
		#if SET_SPI
		if(((u16)(device_info.ir_ad-device_info.bd_zero)*100%0xff)==0)
		{
			service.m_smoke = (u16)(device_info.ir_ad-device_info.bd_zero)*100/0xff;
		}
		else
		{
			service.m_smoke = (u16)(device_info.ir_ad-device_info.bd_zero)*100/0xff + 1;
		}
		#else
		//230427:阈值正确的情况下，实际的烟雾浓度还要进行补偿,-7~+5之间
		if(service.m_smoke_set_val==device_info.bd_alarm)
		{
			//补偿-
			if((device_info.alarm<service.m_smoke_set_val)&&(service.m_smoke_set_val-device_info.alarm<=7))
			{
				compen = service.m_smoke_set_val-device_info.alarm;
				if(device_info.ir_ad>compen)
					device_info.ir_ad -= compen;
			}
			//补偿+
			else if((device_info.alarm>service.m_smoke_set_val)&&(device_info.alarm-service.m_smoke_set_val<=5))
			{
				compen = device_info.alarm-service.m_smoke_set_val;
				if(0xff-compen>device_info.ir_ad)
				{
					
				}
				else
				{
					device_info.ir_ad += compen;
				}				
			}
		}
		
		//230511：温度补偿之后有可能发射差值小于标定值
		if(device_info.ir_ad>device_info.bd_zero)
		{
			if(device_info.ir_ad-device_info.bd_zero<0xfe)
			{
				if(((u16)(device_info.ir_ad-device_info.bd_zero)*100%0xff)==0)
				{
					service.m_smoke = (u16)(device_info.ir_ad-device_info.bd_zero)*100/0xff;
				}
				else
				{
					service.m_smoke = (u16)(device_info.ir_ad-device_info.bd_zero)*100/0xff + 1;
				}
			}	
		}
		else
		{
			service.m_smoke = 0;
		}
		#endif
	}
	else
	{
		//230511：发射差值小于标定值，清0浓度，而不是维持之前浓度
		service.m_smoke = 0;
	}

	
	SM_LOG("service.m_smoke = %d\n",(u16)service.m_smoke);

	//SPI版本自己判断报警
	#if SET_SPI	
	//5s报警
	if(service.m_smoke>=service.m_smoke_set)
	{
		//device_info.state.F_SM_ALARM = 1;
		//开始报警计时
		
		if(service.m_smoke_alm1s<0xff)
		{
			service.m_smoke_alm1s++;
		}
		//service.m_smoke_almRest1s = 0;
		if(service.m_smoke_alm1s>=2)
		{
			service.m_smoke_almRest1s = 0;
			service.m_smoke_almRest6 = 0;
			service.m_smoke_almRest7 = 0;
			service.m_smoke_almRest8 = 0;
		}
		
		#if SET_NEW_TEST
		if((service.m_alm_fast>SMOKE_ALM_1S)&&(service.m_alm_fast<=12))
		{
			if(service.m_smoke_alm1s>=service.m_alm_fast)
			{
				service.m_smoke_state = 1;
			}
		}
		else
		{
			if(service.m_smoke_alm1s>=SMOKE_ALM_1S)
			{
				service.m_smoke_state = 1;
			}	
		}
		#else
		if(service.m_smoke_alm1s>=SMOKE_ALM_1S)
		{
			service.m_smoke_state = 1;
		}	
		#endif
		
		if(service.m_smoke_state==0)
			SM_LOG("烟雾告警计时 = %d\n",(u16)service.m_smoke_alm1s);
	}
	//231013：50% 60% 70% 80%阈值恢复
	//50%阈值1Minu恢复
	//else if(service.m_smoke<service.m_smoke_set/2)
	else if(service.m_smoke<=(u16)service.m_smoke_set*4/5)
	{
		//device_info.state.F_SM_ALARM = 0;
		
		//开始恢复计时
		service.m_smoke_alm1s = 0;	

	
		almRestTick(&service.m_smoke_almRest1s);
		almRestTick(&service.m_smoke_almRest6);
		almRestTick(&service.m_smoke_almRest7);
		almRestTick(&service.m_smoke_almRest8);

		if(service.m_smoke<=service.m_smoke_set/2)
		{

		}
		else if(service.m_smoke<=(u16)service.m_smoke_set*3/5)
		{
			service.m_smoke_almRest1s = 0;
		}
		else if(service.m_smoke<=(u16)service.m_smoke_set*7/10)
		{
			service.m_smoke_almRest1s = 0;
			service.m_smoke_almRest6 = 0;

		}
		else
		{
			service.m_smoke_almRest1s = 0;
			service.m_smoke_almRest6 = 0;
			service.m_smoke_almRest7 = 0;
		}

		#if  SET_NEW_TEST
		if(service.m_smoke_almRest1s>=3)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest6>=4)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest7>=5)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest8>=10)
		{
			service.m_smoke_state = 0;
		}
		#else
		if(service.m_smoke_almRest1s>=SMOKE_ALMREST_1S)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest6>=30)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest7>=60)
		{
			service.m_smoke_state = 0;
		}
		else if(service.m_smoke_almRest8>=120)
		{
			service.m_smoke_state = 0;
		}
		#endif

		if(service.m_smoke_state)
				SM_LOG("烟雾恢复计时 = %d,%d,%d,%d\n",(u16)service.m_smoke_almRest1s,(u16)service.m_smoke_almRest6,(u16)service.m_smoke_almRest7,(u16)service.m_smoke_almRest8);
		
	}
	//中间态不改变
	else
	{
		//230714:中间态报警和恢复时间都要清空
		service.m_smoke_alm1s = 0;

		//231013：中间态恢复时间不清空
		//service.m_smoke_almRest1s = 0;
	}

	#if SET_NB||SET_CAT1
	//230710:当前烟雾告警，新警事件(非反复报警)还未上报出去，检测到小于阈值，马上取消烟雾告警事件
	if(g_device_info.isWaitSnd)
	{
		if((!Event_IsEmpty()))
		{
			if((service.m_smoke_state)&&(service.m_smoke<service.m_smoke_set))
			{
				//当前待发送的为烟雾告警则去除
				if(g_queueEvent.queue[(g_queueEvent.front+1) % EVENT_MAX].bEvent == EVENT_ALM)
				{
					service.m_smoke_alm1s = 0;
					service.m_smoke_state = 0;					
					g_cbSta.bAlmSta &= ~BIT_SMOKE_STOP;		//退出暂停报警状态
					service.m_smoke_state_last = 0;
					g_cbSta.bAlmTimeOut1s = 0;
					g_cbSta.bAlmSndTimeOut1s = 0;
					g_cbSta.bAlmSndlast = 0;
					
					g_cbSta.bSmokeAlmId = 0;
					Event_PopEvent();
					LOG("取消烟雾\r\n");
				}	
			}
		}
		else
		{
			g_device_info.isWaitSnd = 0;
		}	
	}
	#endif
	
	#else
	//230427:报警设备自己通过阈值来判断
	//报警,但是烟雾浓度低于烟雾阈值
	//情况1：正常报警
	if(device_info.state.F_SM_ALARM  &&  (service.m_smoke_set<=service.m_smoke))
	{
		bSmokeOkCnt = 0;
		bSmokeAlmCnt = 0;
	}
	//情况2：正常恢复
	else if((device_info.state.F_SM_ALARM==0) && (service.m_smoke_set>service.m_smoke))
	{
		bSmokeOkCnt = 0;
		bSmokeAlmCnt = 0;
	}
	//情况3：异常报警
	if(device_info.state.F_SM_ALARM  &&  (service.m_smoke_set>service.m_smoke))
	{
		//之前处于报警，延时去抖恢复
		if(s_isAlm)
		{
			if(bSmokeOkCnt<0xff)
				bSmokeOkCnt++;
			if(bSmokeOkCnt>2)
			{
				device_info.state.F_SM_ALARM = 0;
			}
		}
		//之前未报警，则为异常
		else
		{
			bSmokeOkCnt = 0;
			device_info.state.F_SM_ALARM = 0;
		}
	}
	//情况4：异常恢复
	else if((device_info.state.F_SM_ALARM==0) && (service.m_smoke_set<service.m_smoke))
	{
		//之前处于恢复状态，延时去抖报警
		if(s_isAlm==0)
		{
			if(bSmokeAlmCnt<0xff)
				bSmokeAlmCnt++;
			if(bSmokeAlmCnt>2)
			{
				device_info.state.F_SM_ALARM = 1;
			}
		}
		else
		{
			bSmokeAlmCnt = 0;
			device_info.state.F_SM_ALARM = 1;
		}
	}
	
	s_isAlm = device_info.state.F_SM_ALARM;

	//220826: 5s报警,1Minu恢复，报警计时状态不能休眠(休眠只有20s计时器)
	if(device_info.state.F_SM_ALARM)
	{
		//开始报警计时
		service.m_smoke_almRest1s = 0;
		
		if(service.m_smoke_alm1s<0xff)
		{
			service.m_smoke_alm1s++;
		}
		
		if(service.m_smoke_alm1s>=SMOKE_ALM_1S)
		{
			service.m_smoke_almRest1s = 0;
			service.m_smoke_state = device_info.state.F_SM_ALARM;
		}
	}
	else
	{
		//开始恢复计时
		service.m_smoke_alm1s = 0;	
		if(service.m_smoke_almRest1s==0)
			service.m_smoke_almRest1s = 1;

		if(service.m_smoke_almRest1s>=SMOKE_ALMREST_1S)
		{
			service.m_smoke_state = device_info.state.F_SM_ALARM;
		}
	}
	#endif
	//service.m_smoke_state = device_info.state.F_SM_ALARM;
	SM_LOG("service.m_smoke_state = %d \n",(u16)service.m_smoke_state);
	
//	SM_LOG("烟腔故障 = %d \n",(u16)device_info.state.F_SM_ERR);
//	SM_LOG("烟雾预警 = %d \n",(u16)device_info.state.F_SM_PRE_ALARM);
//	SM_LOG("标定状态 = %d \n",(u16)device_info.state.F_SM_BD);
//	SM_LOG("标定完成 = %d \n",(u16)device_info.state.F_SM_BD_OK);
//	SM_LOG("零点值 = %d \n",(u16)device_info.zero_data);
//	SM_LOG("发射值 = %d \n",(u16)device_info.ir_data);
//	SM_LOG("标定值 = %d \n",(u16)device_info.bd_zero);
	
	//烟雾报警
	if(service.m_smoke_state)
	{
		//暂停报警
		if((g_cbSta.bAlmSta&BIT_SMOKE_STOP)!=0)
		{
			//如果是间隔10s以上才再次收到的报警，那么确定是新的警情，需要退出暂停报警状态
			#if SET_SPI==0
			if(g_cbSta.bAlmTimeOut1s==0)
			{	
				g_cbSta.bAlmSta &= ~BIT_SMOKE_STOP;		

				//需要反复报警
				if(g_cbSta.bAlmSndTimeOut1s==0)	
				{
					//g_cbSta.bAlmSndTimeOut1s = GAP_ALM_1S;
					if(g_cbSta.bAlmSndlast==0)
					{
						g_cbSta.bAlmSndTimeOut1s = GAP_ALM_1S;
						//新警
						g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
						SM_LOG("新警\r\n");
						
						if(service.m_muffling==1)
						{
							service.m_muffling = 0;
							LOG("取消消音");
						}	
					}
					else
					{
						//超过12小时，固定12小时1次
						if(g_cbSta.bAlmSndlast>=32768UL)
						{
							g_cbSta.bAlmSndTimeOut1s = 43200UL;
						}
						else
						{
							g_cbSta.bAlmSndTimeOut1s = 2*g_cbSta.bAlmSndlast;
						}
					}
					g_cbSta.bAlmSndlast = g_cbSta.bAlmSndTimeOut1s;
					Event_AddEvent(EVENT_ALM);
					SM_LOG("烟雾告警\r\n");
				}
			}
			#endif					
		}
		else
		{
			//需要反复报警
			if(g_cbSta.bAlmSndTimeOut1s==0)	
			{		
				//230710:NB版本反复报警修改成10分钟
				
				#if SET_NB||SET_CAT1
				if(g_cbSta.bAlmSndlast==0)
				{
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					//230710：新烟雾告警，等待发送，发送期间允许取消，取消之后没有恢复
					g_device_info.isWaitSnd = 1;	
					LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
				}
				g_cbSta.bAlmSndTimeOut1s =  GAP_ALM_1S;
					
				#else		
				if(g_cbSta.bAlmSndlast==0)
				{
					g_cbSta.bAlmSndTimeOut1s = GAP_ALM_1S;
					g_cbSta.bBellTimeOut1s = BELL_ALM_1S;
					SM_LOG("新警\r\n");
					
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}

					//230203：智能家居，只有新警才报
					if(IS_PROTO_SHIKE)
					
					{
						g_cbSta.bAlmSndlast = g_cbSta.bAlmSndTimeOut1s;
						Event_AddEvent(EVENT_ALM);
						SM_LOG("烟雾告警\r\n");
					}
				}
				else
				{
					//超过12小时，固定12小时1次
					if(g_cbSta.bAlmSndlast>=32768UL)
					{
						g_cbSta.bAlmSndTimeOut1s = 43200UL;
					}
					else
					{
						g_cbSta.bAlmSndTimeOut1s = 2*g_cbSta.bAlmSndlast;
					}
					
				}
				#endif
				
				//230914:NB版本取消反复报警
				#if SET_NB||SET_CAT1
				if(g_cbSta.bAlmSndlast ==0)
				{
					g_cbSta.bAlmSndlast = g_cbSta.bAlmSndTimeOut1s;
					Event_AddEvent(EVENT_ALM);
					LOG("烟雾告警\r\n");
				}
				#else
				
				if(service.m_proto_set==PROTO_ZH202)
				{
					g_cbSta.bAlmSndlast = g_cbSta.bAlmSndTimeOut1s;
					Event_AddEvent(EVENT_ALM);
					LOG("烟雾告警\r\n");
				}
				#endif
			}	
		}	
		
		#if SET_SPI==0
		g_cbSta.bAlmTimeOut1s = SMOKE_ALMREST_1S;	
		#endif
		service.m_smoke_state_last = 1;	
	}
	//烟雾告警恢复
	else
	{
		g_cbSta.bAlmSta &= ~BIT_SMOKE_STOP;		//退出暂停报警状态
		if(service.m_smoke_state_last!=0)
		{
			Event_AddEvent(EVENT_ALM_RST);
			LOG("烟雾恢复\r\n");
		}
		service.m_smoke_state_last = 0;
		g_cbSta.bAlmTimeOut1s = 0;
		g_cbSta.bAlmSndTimeOut1s = 0;
		g_cbSta.bAlmSndlast = 0;
		
		g_cbSta.bSmokeAlmId = 0;
	}

	
	if((device_info.state.F_SM_BD==0)&&(state_last.F_SM_BD==1)&&(device_info.state.F_SM_BD_OK))
	{
		//标定完成
		LOG("标定操作完成\r\n");		
		//PLAY_SetSucceBell();
		PLAY_KEY();
		g_device_info.Beep10ms = 50;
		g_device_info.BeepCnt = 1;
	}
	
	state_last = device_info.state;
}

void pGetBat(void)
{
	
	static u8 bBattLowCnt = 0;
	static u8 bBattOkCnt = 0;
	u16 temp;
	static u8 bJumpCnt = 0;
	#if  SET_NB||SET_CAT1
	static u16 bNbBatCnt = 0;
	#endif
	
	
	//采用3.0V锂锰电池CR123A:开路电压3.3V，负载电压2.8V，标称电压3.0V，截止电压2.0V
	//实际电压值 = 1.2 * 256  / ad
	temp = 307200UL/device_info.vdd_adc;

	
	#if SET_NB||SET_CAT1
	//230710:NB版本，模块开机状态电压跌落不更新，除非持续5分钟以上
	if((temp<service.m_battery) && (g_device_info.isNbSleep==0))
	{
		if(WatchDogFlagFast)
		{
			bNbBatCnt++;
		}
		else
		{
			bNbBatCnt += SET_SPI_GAP;
		}
	
		if(bNbBatCnt< 280)
		{
			return;
		}		
	}
	
	#endif
	//230425：电压突降超过0.05V需要过滤，至少持续20秒
	if((temp<service.m_battery)&&(service.m_battery-temp>50) && (bJumpCnt<20))
	{
		if(WatchDogFlagFast)
		{
			bJumpCnt++;
		}
		else
		{
			bJumpCnt += SET_SPI_GAP;
		}	
	}
	else
	{
		bJumpCnt =  0;

		service.m_battery = 307200UL/device_info.vdd_adc;

		#if SET_NB||SET_CAT1
		bNbBatCnt = 0;
		#endif

		SM_LOG("service.m_battery = %d mv\n",(u16)service.m_battery);
	}
	
	
	#if 0
	//报警状态时不判断电池低压(电压波动)
	if(((service.m_smoke_state==1)||(service.m_temp_state!=0)) && (service.m_muffling!=2))
	{
		return;
	}
	#endif
	if(fIsAlm())
	{
		return;
	}

	//电池低压事件自行判断,低压需要多次确认
	if(service.m_battery<=service.m_battery_set)
	{
		bBattOkCnt = 0;

		if(bBattLowCnt<=30)
		{
			if(WatchDogFlagFast)
			{
				bBattLowCnt++;
			}
			else
			{
				bBattLowCnt += SET_SPI_GAP;
			}
		}
		else
		{
			if(service.m_battery_state==0)
			{
				service.m_battery_state = 1;
				//user_fs_isNeedSave = 1;
			}
			
			if(g_cbSta.isBattLowSnd==0)
			{
				g_cbSta.isBattLowSnd = 1;
				Event_AddEvent(EVENT_BAT);
				LOG("低压报警\r\n");
			}
		}			
	}
	//一次即可确认低压恢复，200mV的阈值
	//else if(service.m_battery>service.m_battery_set+200)
	//300mV阈值
	else if(service.m_battery>service.m_battery_set+300)
	{
		bBattLowCnt = 0;
		if(bBattOkCnt<180)
		{
			if(WatchDogFlagFast)
			{
				bBattOkCnt++;
			}
			else
			{
				bBattOkCnt += SET_SPI_GAP;
			}
		}
		else
		{
			if(service.m_battery_state==1)
			{
				service.m_battery_state = 0;
				
				//user_fs_isNeedSave = 1;

				// if(bBattOkCnt==4)
				// {				
					g_cbSta.isBattLowSnd = 0;
				#if 0	
					Event_AddEvent(EVENT_BAT_RST);
					LOG("低压恢复\r\n");
				#endif
					// bBattOkCnt = 5;
				// }	
			}
		}
	}
	else
	{
		bBattLowCnt = 0;
		bBattOkCnt = 0;
	}	
}

u8 pIsAlmNull(void)
{
	if(service.m_smoke_state)
	{
		return 0;
	}
	
	if(service.m_temp_state)
	{
		return 0;
	}
	
	return 1;
}

void formatInfo(void)
{
	static u8 isTouch = 0;
	pGetSmoke();

	pGetTemp();

	//#if SET_CAT1==0
	pGetBat();
	//#endif

	#if SET_BLE_DEBUG
	if(spi_Test)
	{
		LOG("测试信息发送\n");
		note_set_ble_debug();
		spi_Test = 0;
	}
	#endif
	
	#if SET_RST_DEVICE
	//50%阈值期间波动？，保证只重启一次？
	if(service.m_smoke>service.m_smoke_set/2)
	{
		if(isTouch==0)
		{
			isTouch = 1;
			SM_LOG("触发重启 \n");
			g_cbSta.isNeedRstDevice = 1;	
		}	
	}
	else if(service.m_smoke<service.m_smoke_set/4)
	{
		if(isTouch)
		{
			SM_LOG("允许下次触发 \n");
			isTouch = 0;
		}
		
	}
	#endif

	//预警判断
	#if SET_SPI
	//预警判断
	if(fIsAlm())
	{
		device_info.isWarn = 1;
	}
	else
	{
		//烟雾浓度达到阈值的50%
		if(service.m_smoke>=service.m_smoke_set/2)
		{
			#if SET_NEW_TEST
			if(device_info.isWarn==0)
			{
				if(service.m_smoke>service.m_smoke_set/5*7)
				{
					service.m_alm_fast = 12;
				}
				else if(service.m_smoke>service.m_smoke_set/5*6)
				{
					service.m_alm_fast = 10;
				}
				else if(service.m_smoke>service.m_smoke_set/10*9)
				{
					service.m_alm_fast = 9;
				}
				else
				{
					service.m_alm_fast = 8;
				}
					
			}
			#endif
			device_info.isWarn = 1;
			
			#if SET_NEW_TEST
			SM_LOG("烟雾预警:%d \n",service.m_alm_fast);
			#else
			SM_LOG("烟雾预警 \n");
			#endif
		}
		//温度大于阈值-5℃
		else if((service.m_temp_almset!=0xff)&&(service.m_temp_almset>5)&&(service.m_temp>service.m_temp_almset-5))
		{
			device_info.isWarn = 1;
			SM_LOG("温度预警 \n");
		}
		else
		{
			device_info.isWarn = 0;
		}
	}
	
	#if SET_RST_DEVICE
	if(device_info.isWarn||(TouchSend.isGet==0)||((g_device_info.RunTime<SET_SPI_GAP+5)) || g_cbSta.isNeedRstDevice)
	#else
	if(device_info.isWarn||(TouchSend.isGet==0)||(g_device_info.RunTime<SET_SPI_GAP+5))
	#endif
	{
		if(WatchDogFlagFast==0)
		{
			WatchDogFlagFast = 1;
			osal_stop_timerEx(watchdog_TaskID, WATCHDOG_1000MS_EVENT);
			osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, 1000);
			LOG("切换1s唤醒\r\n");
		}		
	}
	else
	{
		if(WatchDogFlagFast==1)
		{
			WatchDogFlagFast = 0;
			osal_stop_timerEx(watchdog_TaskID, WATCHDOG_1000MS_EVENT);
			#if 0
			osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, 2*1000);
			#else
			osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, SET_SPI_GAP*1000);
			#endif
			LOG("切换10s唤醒\r\n");
		}			
	}
	#endif
}

#if SET_SPI==0
void vTaskUart(void)
{
//	if(g_device_info.is_sleep)
//	{
//		return ;
//	}
	
	/*
		烟雾超时恢复：报警状态HT芯片会1s间隔主动推送;超时未接收到推送报警状态，也视为报警恢复
	*/
	if(g_cbSta.bAlmTimeOut1s==1)
	{
		if(service.m_smoke_state!=0)
		{
			service.m_smoke_state = 0;
			Event_AddEvent(EVENT_ALM_RST);
		}
		g_cbSta.bAlmTimeOut1s = 0;
	}
	else if(g_cbSta.bAlmTimeOut1s==0)
	{
		service.m_smoke_state = 0;
	}
	
	if(pIsAlmNull())
	{
		service.m_muffling = 0;		//全部报警恢复,取消消音，取消暂停报警
	}
	//报警状态下不休眠
	else
	{
		//消音90s超时暂不恢复
		#if 0
		if((g_cbSta.muffTimeOut1s==0)&&(service.m_muffling!=2))
		{
			service.m_muffling = 0;
		}
		#endif
	}
	
	
	//发送处理
	if(TouchSend.m_timeout10ms==0)
	{
		//不需要重发
		#if 0
		//需要重发	
		if(TouchSend.m_wait_cmd!=0)
		{
			if(TouchSend.m_tx_cnt>10)
			{
				TouchSend.m_tx_cnt = 0; 
				TouchSend.m_wait_cmd = 0;
			}
			else
			{
				TX2_send_data(TouchSend.m_tx_buf,TouchSend.len);
				TouchSend.m_timeout10ms = 30;
				TouchSend.m_tx_cnt++;
			}	
		}
		else
		#endif
		{
			//状态改变，定时轮询
			if(TouchSend.m_flag_set/*||(TouchSend.m_heart10ms==0)*/ )
			{
				//TouchSend.m_flag_set = 0;
				TouchSend.m_heart10ms = 10;
				
				if((service.m_smoke_set<=100)&&(service.m_smoke_set>=10))
				{
					TouchSndCmd(TOUCH_SET);
					TouchSend.m_wait_cmd = TOUCH_SET;	
				}
				
				//230427: 只设置1次即可，下次上报对比不一致再设置
				// else
				// {
				// 	TouchSend.m_flag_set = 0;
				// }
				TouchSend.m_flag_set = 0;
			}
		}
	}
	
	//AA 15 52 28 00 78 6D 00 00 78 55
	//接收处理
	//if(((my_time()-g_uart_ctrl_2.m_last_rx_time>10)&&(g_uart_ctrl_2.m_rx_write>=1))||(g_uart_ctrl_2.m_rx_write>=11))
	if(((my_time()-g_uart_ctrl_2.m_last_rx_time>30)&&(g_uart_ctrl_2.m_rx_write>=1))||(g_uart_ctrl_2.m_rx_write>=11))
	{
		u8 RxBuf[50];
		u8 RxLen;
		u8 RxPos;
		//TouchSend.isGet = 1;

		TouchSend.isSleep = 1;		//接收完检测芯片数据
		
		//先拷贝接收数据
		RxLen = g_uart_ctrl_2.m_rx_write;
		memcpy(RxBuf,g_uart_ctrl_2.m_rx_buf,RxLen);
		g_uart_ctrl_2.m_rx_write = 0;
		SM_LOG("Rx,%d:",RxLen);
		//LOG_DUMP_BYTE(RxBuf,RxLen);
		//SM_LOG_BYTE(RxBuf,RxLen);
		
		//分包处理一下
		//校验一下
		RxPos = 0;
		while (RxLen>=11)
		{
			if((RxBuf[RxPos]!=0xaa)||(RxBuf[RxPos+10]!=0x55))
			{
				//LOG("Rx Fail\r\n");
				break;
			}
			else
			{		
				TouchSend.isGet = 1;
				
				//LOG("Rx ok\r\n");
				//拷贝数据
				memcpy(&device_info,&RxBuf[1],sizeof(device_info));
				//解析转变一下需要的格式
				formatInfo();
			}

			if(RxLen>11)
			{
				RxLen -= 11;
			}
			else
			{
				RxLen = 0;
			}
		}	
	}
	
	if(my_time()-g_uart_ctrl_2.m_last_rx_time>30)
	{
		TouchSend.isSleep = 1;
	}


//	if(TouchSend.isSleep)
//	{
//		sleepTimer = 0;
//	}
}
#endif


