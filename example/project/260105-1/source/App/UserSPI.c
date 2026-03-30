/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-04-28 16:06:57
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2025-07-18 17:00:59
 * @FilePath: \source\App\UserSPI.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "UserSPI.h"

#if SET_SPI
void initMySPI(void)
{
    #if SET_RX_TRAN
    hal_gpio_fmux(HT_MSPI_SCK, Bit_DISABLE);
    hal_gpio_pin_init(HT_MSPI_SCK, GPIO_INPUT);
    hal_gpio_pull_set(HT_MSPI_SCK, GPIO_PULL_UP);
    #else
    hal_gpio_fmux(HT_MSPI_SCK, Bit_DISABLE);
    hal_gpio_pin_init(HT_MSPI_SCK, GPIO_OUTPUT);
    hal_gpio_pull_set(HT_MSPI_SCK, GPIO_PULL_UP_S);
    hal_gpioretention_register(HT_MSPI_SCK);
	//---- CLK Output High ----//
    SET_CLK_OUTPUT(HIGH);
    #endif
	hal_gpio_fmux(HT_MSPI_MISO, Bit_DISABLE);
    hal_gpio_pin_init(HT_MSPI_MISO, GPIO_INPUT);
    hal_gpio_pull_set(HT_MSPI_MISO, GPIO_PULL_UP);
    //hal_gpioretention_register(HT_MSPI_MISO);
     //---- DIO Input Pull-High ----//
    
}

void MySPI_Delay(uint8 tm)
{
    while (tm--)
    {  
        WaitUs(5);
    }
}

#if SET_RX_TRAN
u8 MySPI_Start(void)
{
    u8 i = 0;
    while (1)
    {
        if(READ_CLK()==0)
        {
            MySPI_Delay(10);    //50us
            i++;
            //>500us,检测到
            if(i>10)
            {
                return 1;
            }
        }
        else
        {
            
            // if(i>10)
            // {
            //     return 1;     
            // }
            // else
            {
                LOG("SPI%d\r\n",i);
                i = 0;
                return 0; 
            }
        } 
    }
}
#else
void MySPI_Start(void)
{
   
    u8 i;
    SET_CLK_OUTPUT(LOW);

    //5ms超时
    for(i=0;i<200;i++)
    {
        #if SET_SPI_SLOW
        MySPI_Delay(10);
        #else
        MySPI_Delay(5);
        #endif
		if(READ_DIO()==0)
        {
            break;
        }
    }

    //5ms超时
    for(i=0;i<200;i++)
    {
        #if SET_SPI_SLOW
				MySPI_Delay(10);
				#else
        MySPI_Delay(5);
        #endif
        if(READ_DIO())
        {
            break;
        }
    }

    //MySPI_Delay(C_DLY_START);
   
}
#endif

#if SET_RX_TRAN
u8  MySPI_ReadByte(void)
{
    u8 i;
    u8 rdat = 0;

    for(i=0; i<8; i++)
    {
        rdat = rdat>>1;
        //上升沿时读取数据
        while (READ_CLK()==0);
        if(READ_DIO() )   rdat |= 0x80; 

        //输出个标志
        //SET_P10(1);
        while (READ_CLK());
        //SET_P10(0);
    }

    return rdat;
}
#else
void MySPI_SendByte(uint8 sdat)
{
    u8 i;

    SET_DIO_OUTPUT();

    for(i=0; i<8; i++)
    {
        if(sdat & 0x01)     SET_DIO_OUTPUT_H();
        else                SET_DIO_OUTPUT_L();
				#if SET_SPI_SLOW
				MySPI_Delay(4);
				#else
        MySPI_Delay(C_DLY_BIT);
				#endif
        SET_CLK_OUTPUT(HIGH);
        #if SET_SPI_SLOW
				MySPI_Delay(4);
				#else
        MySPI_Delay(C_DLY_BIT);
				#endif
        SET_CLK_OUTPUT(LOW);
        sdat = sdat>>1;
    }
}


u8  MySPI_ReadByte(void)
{
    u8 i;
    u8 rdat = 0;

    SET_DIO_INPUT();
    for(i=0; i<8; i++)
    {
        rdat = rdat>>1;
        SET_CLK_OUTPUT(HIGH);
				#if SET_SPI_SLOW
				MySPI_Delay(2);
				#else
        MySPI_Delay(C_DLY_HALF_BIT);
				#endif
        if(READ_DIO() )   rdat |= 0x80;  
				#if SET_SPI_SLOW
				MySPI_Delay(2);
				#else
        MySPI_Delay(C_DLY_HALF_BIT);
				#endif
        SET_CLK_OUTPUT(LOW);
				#if SET_SPI_SLOW
				MySPI_Delay(4);
				#else
        MySPI_Delay(C_DLY_BIT);
				#endif
    }

    return rdat;
}
#endif

void MySPI_Stop(void)
{
    SET_DIO_INPUT();
		#if SET_SPI_SLOW
		MySPI_Delay((20));
    SET_CLK_OUTPUT(HIGH);
    MySPI_Delay(20);
		#else
    MySPI_Delay((C_DLY_START));
    SET_CLK_OUTPUT(HIGH);
    MySPI_Delay(C_DLY_START);
		#endif
}

#if SET_BLE_DEBUG
u8 spi_Test = 0;
u8 spi_RcvBuf[11];
u8 spi_RcvLen;
#endif
void MySPI_pro(void)
{
    u8 i;
		u8 RcvBuf[11];
    u8 MaxLen;
		
	
    u8 chk,chk1;
    u8 isChkOk;
    //static u8 isTest;
    #if SET_RST_DEVICE
    static u8 isDisRst = 0;
    #endif
    //计时
    if(WatchDogFlagFast)
    {
        if(g_cbSta.bAlmSndTimeOut1s>0)
        {
            g_cbSta.bAlmSndTimeOut1s--;
        }
        
        if(g_cbSta.bTempSndTimeOut1s>0)
        {
            g_cbSta.bTempSndTimeOut1s--;
        }
    }
    else
    {
        if(g_cbSta.bAlmSndTimeOut1s>SET_SPI_GAP)
        {
            g_cbSta.bAlmSndTimeOut1s -= SET_SPI_GAP;
        }
        else
        {
            g_cbSta.bAlmSndTimeOut1s = 0;
        }
        
        if(g_cbSta.bTempSndTimeOut1s>SET_SPI_GAP)
        {
            g_cbSta.bTempSndTimeOut1s -= SET_SPI_GAP;
        }
        else
        {
            g_cbSta.bTempSndTimeOut1s = 0;
        }
    }
		
    
		
		
		#if SET_RX_TRAN
    //while (1)
    {
        //LOG("SPI\n");
        //memset(RcvBuf,0,10);
        //关中断
        drv_disable_irq();
        if(MySPI_Start())
        {
            MaxLen = 0;
            chk = MySPI_ReadByte();
            
            if(chk==0xa5)
            {
                MaxLen = 8;
            }
            else if(chk==0xaa)
            {
                MaxLen = 6;
            }
            //drv_enable_irq();
            if(MaxLen)
            {
                for(i=0;i<MaxLen;i++)
                {
                    RcvBuf[i] =  MySPI_ReadByte();
                }

                drv_enable_irq();

                LOG("Rcv:");
                LOG_DUMP_BYTE(&RcvBuf[0],MaxLen);
                chk = 0x53;
                for(i=0;i<MaxLen-1;i++)
                {
                    chk += RcvBuf[i];
                }
                LOG("Chk:%x",chk);

                //if(chk==RcvBuf[MaxLen-1])
								if((chk==RcvBuf[MaxLen-1])||(MaxLen==8))
                {
                    TouchSend.isGet = 1;
                    
                    memcpy(&device_info,&RcvBuf[0],MaxLen-1);   //一个字节是校验
                    /*
                    u8 zero_data;		//零点值
                    u8 ir_data;			//发射值
                    u8 ir_ad;				//发射差值
                    u8 bd_zero;			//标定值
                    u8 bd_alarm;		//报警值
                    u8 vdd_adc;			//电池AD值
                    u8 t_adc;				//温度AD值
                    SM_FG state;		//烟雾状态
                    u8 alarm;				//报警值
                    */
                    device_info.bd_alarm = 102;
                    device_info.alarm = 102;

                    if((device_info.ir_ad>device_info.bd_zero)&&(device_info.ir_ad-device_info.bd_zero>device_info.bd_alarm))
                    {
                        device_info.state.F_SM_ALARM = 1;
                    }
                    UART_putc(0x00);
                    UART_putc(0x00);
                    UART_putc(0xaa);
                    UART_putc(device_info.zero_data);
                    UART_putc(device_info.ir_data); 
                    UART_putc(device_info.ir_ad);    
                    UART_putc(device_info.bd_zero);  
                    UART_putc(device_info.bd_alarm); 
                    UART_putc(device_info.vdd_adc); 
                    UART_putc(device_info.t_adc); 
                    //UART_putc(device_info.state); 
										UART_putc(*(&device_info.t_adc +1));
                    UART_putc(device_info.alarm); 
                    UART_putc(0x55);
                }
            } 
            else
            {
                drv_enable_irq();
                LOG("fsb:%x",chk);
            }  
        }
        else
        {
            drv_enable_irq();
            LOG("FAIL");
        }
        
    }
    #else
		#if SET_TEST
		if(my_time()>10000)
		{
			isChkOk = 1;
			PLAY_KEY();
		}
		else
		#endif
		{
		
		
			//关中断
			drv_disable_irq();

			 //0xAA 0xA5两条命令
			MySPI_Start();

			#if SET_RST_DEVICE
			if(((g_cbSta.isNeedRstDevice%3)==1) && (isDisRst==0))
			{
					MySPI_SendByte(0xA6);			//
					MaxLen = 0;
					MySPI_Stop();

					drv_enable_irq();
					g_cbSta.isNeedRstDevice++;
					LOG("开始重启传感芯片\n");
					
					return;
			}
			#if SET_ALL_DATA
			else if(1)
			#else
							#if SET_BLE_DEBUG
							//else if((((WatchDogFlagFast)||(g_cbSta.isNeedRstDevice)) && (isDisRst==0))||(g_device_info.RunTime<=180))
							else if((((WatchDogFlagFast)||(g_cbSta.isNeedRstDevice)) && (isDisRst==0))||(spi_Test==1))
							#else
							else if(((WatchDogFlagFast)||(g_cbSta.isNeedRstDevice)) && (isDisRst==0))
							#endif
					#endif
			
			#else
			if(WatchDogFlagFast)
			#endif
			{
					#if SET_RST_DEVICE
					if(g_cbSta.isNeedRstDevice)
					{
							g_cbSta.isNeedRstDevice++;
							if(g_cbSta.isNeedRstDevice>6)
							{
									g_cbSta.isNeedRstDevice = 0;
									//LOG("重启传感芯片超时放弃\n");
									if(g_cbSta.isNeedRstMcu)
									{
											g_cbSta.isNeedRstMcu = 0;
											hal_system_soft_reset();
									}
							}
					}
					
					//重启多一个字节代表计数值
					if(isDisRst)
					{
							MaxLen = 8;
					}
					else
					{
							MaxLen = 9;
					}  	
					#else
					MaxLen = 8;
					#endif
					MySPI_SendByte(0xA5);
			}
			else
			{
					MaxLen = 6;
					MySPI_SendByte(0xAA);
			}
			
			for(i=0;i<MaxLen;i++)
			{
					//MySPI_Delay(20);
			MySPI_Delay(2);
					RcvBuf[i] =  MySPI_ReadByte();
			}
			MySPI_Stop();
			
			drv_enable_irq();

			/*
					R_SM_DATA
					R_SM_BD_ZERO
					R_VDD_ADC[1]
					R_T_ADC
					SM_State
					R_SM_ZERO_DATA
					R_SM_IR_DATA
			*/
			SM_LOG("Rcv%d:",MaxLen);
			SM_LOG_BYTE(&RcvBuf[0],MaxLen);
			
			#if SET_BLE_DEBUG
			memcpy(&spi_RcvBuf[0],&RcvBuf[0],MaxLen);
			spi_RcvLen = MaxLen;
			#endif
			chk = 0x53;
			 
			for(i=0;i<MaxLen-1;i++)
			{
					chk += RcvBuf[i];
					if(i<7)
					{
							chk1 = chk;
					}
			}
			SM_LOG("Chk:%x,%x,%x\n",chk,chk1,RcvBuf[MaxLen-1]);
			
			
			if(chk==RcvBuf[MaxLen-1])
			{
					isChkOk = 1;
			}
			else if((MaxLen==9)&&(chk1==RcvBuf[MaxLen-2]))
			{
					isChkOk = 1;
					isDisRst = 1;
					
				
			SM_LOG("旧协议传感\n");
			}
			else
			{
					isChkOk = 0;
					isDisRst = 0;
			}

			if(isDisRst)
			{
					g_cbSta.isNeedRstDevice = 0;
			}
			
			//if(chk==RcvBuf[MaxLen-1])
    }
		
		if(isChkOk)
    {
				//预警
				if(MaxLen-1>7)
				{
					memcpy(&device_info,&RcvBuf[0],7);
				}
				else
				{
					memcpy(&device_info,&RcvBuf[0],MaxLen-1);
				}
				
				//memcpy(&device_info,&RcvBuf[0],MaxLen-1);   //一个字节是校验
			
				#if SET_RST_DEVICE
				//重启是否成功
				if(g_cbSta.isNeedRstDevice  && (isDisRst==0) )
				{
						if(((RcvBuf[MaxLen-2]==1)||(RcvBuf[MaxLen-2]==0))&&(MaxLen==9))
						{
								SM_LOG("重启传感芯片成功\n");
								g_cbSta.isNeedRstDevice = 0;
								if(g_cbSta.isNeedRstMcu)
								{
										g_cbSta.isNeedRstMcu = 0;
										hal_system_soft_reset();
								}
						}
						else if((g_cbSta.isNeedRstDevice%3)!=1)
						{
								SM_LOG("重启传感芯片失败\n");
						}	
				}
				#endif
				
				
			 
				
				
				formatInfo();
			
        TouchSend.isGet = 1;
    
        TouchSend.isSleep = 1;			//串口休眠
			
    }
    	
		
    #endif

}
#endif

