/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-11-14 11:08:29
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-09-26 14:43:53
 * @FilePath: \source\App\my_io.C
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include	"my_io.h"

//大端存储改成小端
u16 GETU16(u8 *value)
{
	u16 valueU16;
	valueU16 = ((*value)<<8) + *(value+1);
	return valueU16;
}

//????U32
u32 GETU32_Big(u8 *value)
{
	u32 valueU32;
	valueU32 = ((*value)<<24) + (*(value+1)<<16) + (*(value+2)<<8) + *(value+3);
	return valueU32;
}

void app_gpio_set(gpio_pin_e pin,gpio_dir_t dir,gpio_pupd_e type)
{
	hal_gpio_fmux(pin, Bit_DISABLE);
	hal_gpio_pin_init(pin, dir);
	hal_gpio_pull_set(pin, type);
	
	hal_gpioretention_register(pin);
	
	
}

/*
     P00~P03:default jtag,we can use it as wakeup pin when no debug.
     P04~P07,P11~P15,P18~P30:default gpio,use it easily.
     P08:mode select pin,cannot used as other usage.
     P09~P10,it is uart in burn mode which cannot config.it is configable when in debug mode.
     P16~P17:xtal pin,when use this pins,please use rc as system frequency.config hal_rtc_clock_config(CLK_32K_RCOSC) in hal_init first.
     P31~P34:default spif,we can use it as wakeup pin directly,we driver have completed its multiplex config.
*/
typedef struct gpioin_wakeup_t
{
    GPIO_Pin_e pin;
    gpioin_Hdl_t posedgeHdl;
    gpioin_Hdl_t negedgeHdl;
} gpioin_wakeup;

void gpio_callback_wakeup(GPIO_Pin_e pin,IO_Wakeup_Pol_e type)
{
	sleepTimer = 3;
	//DEBUG_LOG("KEY WAKE");
}




/******************** IO配置函数 **************************/
void	GPIO_config(void)
{	
	//LED
	app_gpio_set(PIN_LED_G,GPIO_OUTPUT,GPIO_FLOATING);
	app_gpio_set(PIN_LED_R,GPIO_OUTPUT,GPIO_FLOATING);
	SET_LED_G(1);
	SET_LED_R(1);

	
	//PIR
	app_gpio_set(PIR_IN,GPIO_INPUT,GPIO_FLOATING);
	app_gpio_set(PWR_IR,GPIO_OUTPUT,GPIO_FLOATING);
	SET_PWR_IR(0);
	
	//按键 防拆
	app_gpio_set(PIN_KEY,GPIO_INPUT,GPIO_PULL_UP_S);
	app_gpio_set(PIN_FC,GPIO_INPUT,GPIO_PULL_UP);
	//hal_gpioin_register(PIN_KEY,NULL,NULL);
	hal_gpioin_register(PIN_KEY,gpio_callback_wakeup,gpio_callback_wakeup);
	hal_gpioin_register(PIN_FC,NULL,NULL);
	
	
	
	#if USE_SX1262
	SPI_Init();		 // SPI接口初始化
	RFM98_IO_Init();  // 射频模块初始化
	#endif
	
	#if SET_NB
	//NB
	app_gpio_set(ME3616_POW_EN,GPIO_OUTPUT,GPIO_FLOATING);
	app_gpio_set(ME3616_NB_ON,GPIO_OUTPUT,GPIO_FLOATING);

	#if SET_USE_PSM
	app_gpio_set(ME3616_NB_ON,GPIO_OUTPUT,GPIO_FLOATING);
	SET_MN306_WAKE(OFF_WAKE_NB);
	#endif

	SET_ME3616_POW_EN(OFF_POW_NB);
	SET_ME3616_NB_ON(ON_RST_NB);
	#endif
	
	#if SET_CAT1
	//CAT1
	app_gpio_set(PIN_LTE_PWR_C,GPIO_OUTPUT,GPIO_FLOATING);
	app_gpio_set(PIN_LTE_RST_N,GPIO_OUTPUT,GPIO_FLOATING);
	SET_LTE_PWR_C(1);
	SET_LTE_RST_N(0);
	#endif

	#if SET_RX_TRAN
	app_gpio_set(GPIO_P10,GPIO_OUTPUT,GPIO_FLOATING);
	#endif
}
