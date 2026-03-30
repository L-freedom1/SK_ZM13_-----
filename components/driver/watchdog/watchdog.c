/**************************************************************************************************
*******
**************************************************************************************************/
#include "rom_sym_def.h"
#include "watchdog.h"
#include "error.h"
#include "pwrmgr.h"
#include "clock.h"
#include "OSAL_Timers.h"
#include "jump_function.h"

#include	"config.h"

uint8 WatchDogFlag;
uint8 WatchDogFlagFast;


uint8 watchdog_TaskID;

#if(HAL_WDG_CFG_MODE == WDG_USE_INT_MODE)
void __attribute__((used))  hal_WATCHDOG_IRQHandler(void)
{
    volatile uint32_t a;
    a = AP_WDT->EOI;
    AP_WDT->CRR = 0x76;
}
#endif
//static void hal_watchdog_feed(void)
void hal_watchdog_feed(void)
{
    AP_WDT->CRR = 0x76;
}

static bool watchdog_init(WDG_CYCLE_Type_e cycle)
{
    volatile uint32_t a;
    uint8_t delay;
    hal_clk_gate_enable(MOD_WDT);

    if((AP_PCR->SW_RESET0 & 0x04)==0)
    {
        AP_PCR->SW_RESET0 |= 0x04;
        delay = 20;

        while(delay-->0);
    }

    if((AP_PCR->SW_RESET2 & 0x04)==0)
    {
        AP_PCR->SW_RESET2 |= 0x04;
        delay=20;

        while(delay-->0);
    }

    AP_PCR->SW_RESET2 &= ~0x20;
    delay=20;

    while(delay-->0);

    AP_PCR->SW_RESET2 |= 0x20;
    delay=20;

    while(delay-->0);

    a = AP_WDT->EOI;
    AP_WDT->TORR = cycle;
    #if(HAL_WDG_CFG_MODE == WDG_USE_INT_MODE)
    JUMP_FUNCTION(WDT_IRQ_HANDLER)                  =   (uint32_t)&hal_WATCHDOG_IRQHandler;
    AP_WDT->CR = 0x1F;//use int
    NVIC_SetPriority((IRQn_Type)WDT_IRQn, IRQ_PRIO_HAL);
    NVIC_EnableIRQ((IRQn_Type)WDT_IRQn);
    #else
    JUMP_FUNCTION(WDT_IRQ_HANDLER)                  =   0;
    AP_WDT->CR = 0x1D;//no use int
    NVIC_DisableIRQ((IRQn_Type)WDT_IRQn);
    #endif
    AP_WDT->CRR = 0x76;
    //osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, WATCHDOG_1000MS_CYCLE);
    return PPlus_SUCCESS;
}

static void hal_watchdog_wakeup_handler(void)
{
   watchdog_init(HAL_WDG_CFG_CYCLE);
	
		DEBUG_LOG("WDT wake\r\n");
	
}

static void hal_watchdog_sleep_handler(void)
{
		DEBUG_LOG("WDT Sleep\r\n");
    //osal_stop_timerEx(watchdog_TaskID, WATCHDOG_1000MS_EVENT);
}

void Watchdog_Init(uint8 task_id)
{
    watchdog_TaskID = task_id;
		WatchDogFlag = 0;
	
//		pwroff_cfg_t	mySleep_cfg;
//		mySleep_cfg.type = POL_FALLING;
//		mySleep_cfg.pin		= GPIO_P14;
//		mySleep_cfg.on_time = 100;
		
    watchdog_init(HAL_WDG_CFG_CYCLE);
		#if SET_SPI
		osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, 1000);
		WatchDogFlagFast = 1;
		#else
		osal_start_reload_timer(watchdog_TaskID, WATCHDOG_1000MS_EVENT, WATCHDOG_1000MS_CYCLE);
		#endif
		
    hal_pwrmgr_register(MOD_WDT, hal_watchdog_sleep_handler, hal_watchdog_wakeup_handler);  
		LOG("WatchDog start\r\n");
	  //hal_pwrmgr_enter_standby(&mySleep_cfg,1);
		//hal_pwrmgr_poweroff(&mySleep_cfg,1);
}

//void my_watchdog_feed(void)
//{
//		hal_watchdog_feed();
//}

uint16 Watchdog_ProcessEvent(uint8 task_id, uint16 events)
{
    if(events & WATCHDOG_1000MS_EVENT)
    {	
			DEBUG_LOG("Dog\n");
			
			#if SET_SPI
			static u8 s_runCnt;
			s_runCnt++;
			#endif
			
			#if SET_USE_PSM
			if(g_device_info.wakeSrc==0)
			{
				g_device_info.wakeSrc = 1;
			}
			#endif
			
				
			if(WatchDogFlag==2)
			{
				WatchDogFlag = 0;
				#if (SET_NB||SET_CAT1)==0
				hal_watchdog_feed();
				#endif
			}	
			
			#if SET_SPI
			
			#if SET_RX_TRAN==0
			
				#if SET_NB||SET_CAT1
				if(g_device_info.isNbSleep)
				{
					g_isSpiTouch = 1;
					LOG("SPI TOUCH\r\n");
				}
				#else
				osal_set_event(App_main_TaskID,APP_MAIN_SPI);
				#endif
			
			
			#endif
			
			//230818:90秒消音限制
			if(g_cbSta.muffTimeOut1s>0)
			{
				if(WatchDogFlagFast)
				{
					g_cbSta.muffTimeOut1s--;
				}
				else
				{
					if(g_cbSta.muffTimeOut1s<SET_SPI_GAP)
					{
						g_cbSta.muffTimeOut1s = 0;
					}
					else
					{
						g_cbSta.muffTimeOut1s -= SET_SPI_GAP;
					}
				}
				
				//#if SET_BLE_SUB
				if(g_cbSta.muffTimeOut1s==0)
				{
					if(service.m_muffling==1)
					{
						service.m_muffling = 0;
						LOG("取消消音");
					}
				}
				//#endif
			}
			
			//预警时改成1s查询
			if(WatchDogFlagFast)
			{
				if(g_device_info.RunTime<0xff)
					g_device_info.RunTime++;									//运行时间
				
				
				if(s_runCnt%10==0)
				{
					alawys_10s_timer();
					s_runCnt = 0;
				}	
			}
			else
			{
				if(g_device_info.RunTime<0xff-SET_SPI_GAP)
					g_device_info.RunTime += SET_SPI_GAP;				//运行时间
				
				if(s_runCnt% (10/SET_SPI_GAP)==0)
				{
					alawys_10s_timer();
					s_runCnt = 0;
				}	
			}
			#else
			
			if(service.m_smoke_almRest1s>0)
			{
				if(service.m_smoke_almRest1s<0xff-20)
				{
					service.m_smoke_almRest1s += 20;
				}
			}
			
			if(service.m_temp_alm1s>0)
			{
				if(service.m_temp_alm1s<0xff-20)
				{
					service.m_temp_alm1s += 20;	
				}
			}
			if(service.m_temp_almRest1s>0)
			{
				if(service.m_temp_almRest1s<0xff-20)
				{
					service.m_temp_almRest1s += 20;
				}
			}
			
			alawys_20s_timer();
			#endif

			

      return (events ^ WATCHDOG_1000MS_EVENT);
    }
		
	return 0;
}



