#include "config.h"

/*需要改和弦音*/
//#if SET_CAT1
#if 0
void Fun_beep_scan(void)
{
	return;
}
void Fun_beep(u16 iblinktimes,u16 iblinktime)
{
	return;
}

void Fun_beep_stop(void)
{
	return;
}

bool bell_is_bell(void)
{
	return 0;
}
#else
/*全局变量定义*/
u8  g_EX_switch = 0;   // 外接喇叭开关
u16 g_EX_long = 0;     // 外接喇叭提示音长度
u8  g_EX_times = 0;	 		// 外接喇叭提示音几声
static u16 count_EX  = 0;  // 外接喇叭提示计数

#define PWM_IO_DA		GPIO_DUMMY
//#if SET_VER_TK
//		#define PWM_FREQ 				800

//		#define PWM_FREQ_LOUD1	2000
//		
//    #define PWM_FREQ_LOUD   3200
//#else
//    #define PWM_FREQ 		3200
//#endif

#define		PWM_FREQ		3200

#if	SET_CAT1
	#define PWM_DUTY 		20
#else
	#define PWM_DUTY 		50/// 0~100
#endif

//u8 g_duty;
void app_pwm_set(u16 pwm_freq)
{	
	u8 g_duty;
	#if SET_VER_TK
	if(g_cbSta.bBellTimeOut1s>0)
	{
		//5%-->50%
		g_duty = (BELL_ALM_1S-g_cbSta.bBellTimeOut1s)/2+5;
	}
	else
	{
		g_duty = 10;
	}
	#else
		g_duty = PWM_DUTY;
	#endif
	
	hal_pwm_init(PWM_CH0, PWM_CLK_DIV_4, PWM_CNT_UP, PWM_POLARITY_FALLING);
	hal_pwm_open_channel(PWM_CH0, PWM_IO_DA);

	//hal_pwm_set_count_val(PWM_CH0, (((16000000/(1<<PWM_CLK_DIV_4))/pwm_freq)*PWM_DUTY/100), ((16000000/(1<<PWM_CLK_DIV_4))/pwm_freq));
	
  	hal_pwm_set_count_val(PWM_CH0, (((16000000/(1<<PWM_CLK_DIV_4))/pwm_freq)*g_duty/100), ((16000000/(1<<PWM_CLK_DIV_4))/pwm_freq));
	
}

void Fun_beep_stop(void)
{
	g_EX_switch = 0;
	g_EX_long = 0;     
  g_EX_times = 0;	
	count_EX = 0;
	hal_pwm_stop();		
	SET_BEEP(0);

}
/**
 * @description: 蜂鸣器响 扫描
 * @param {type} 
 * @return: 
 */
void Fun_beep_scan(void)
{
    if (g_EX_switch == 1)
    {
        count_EX++;
        if (count_EX == 1)
        {
            /* 打开时间 */
            if (g_EX_times)
                g_EX_times--;
        }
        else if (count_EX > (g_EX_long / 10 - 1))
        {
            count_EX = 0;
            if (g_EX_times == 0)
                g_EX_switch = 0;
        }
    }
    
	if (count_EX == 1 && g_EX_times % 2 == 1)
    {				
			//LOG("pwm en\r\n");
			SET_BEEP(1);
			app_pwm_set(PWM_FREQ);
			hal_pwm_start();
    }
    if (g_EX_times % 2 == 0)
    {		
			hal_pwm_stop();	
			SET_BEEP(0);
    }
}
/**
 * @description: l蜂鸣器响方式
 * @param {type} iblinktimes，响次数*2; iblinktime:响一次时间
 * @return: 
 */
void Fun_beep(u16 iblinktimes,u16 iblinktime)
{	
    g_EX_switch = 1;   					// 外接喇叭开关
    g_EX_long = iblinktime;     // 外接喇叭提示音长度
    g_EX_times = iblinktimes;	 	// 外接喇叭提示音几声
    count_EX = 0;//计数清0

		LOG("Beep:%d,%d\r\n",iblinktime,iblinktimes/2);
}

/**
 * @description: 是否正在响bell
 * @param {*}
 * @return {*}
 */
bool bell_is_bell(void)
{
    bool ret = false;
    if(g_EX_switch)
    {
        ret = true;
    }
    else if(g_device_info.Beep10ms>0)
		{
				ret = true;
		}
		else
    {
        ret = false;
    }

    return ret;
}
#endif
