/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-03-07 10:47:21
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-27 15:27:07
 * @FilePath: \source\App\my_io.H
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef	MY_IO
#define	MY_IO

#include	"config.h"

/*纭?浠舵帴鍙ｅ畾涔?*/ 

//LED
#define PIN_LED_G       GPIO_P23
#define PIN_LED_R       GPIO_P26


#define SET_LED_G(x)      			hal_gpio_write(PIN_LED_G,x)
#define SET_LED_R(x)            hal_gpio_write(PIN_LED_R,x) 

//KEY
#define PIN_KEY         GPIO_P00
#define PIN_FC       	GPIO_P01

//PIR
#define	PIR_IN		GPIO_P14
#define	PWR_IR		GPIO_P11
#define	SET_PWR_IR(x)			hal_gpio_write(PWR_IR,x)
#define	READ_PIR_IN()			hal_gpio_read(PIR_IN)

#define PIN_BEEP       GPIO_P25
#define SET_BEEP(x)    hal_gpio_write(PIN_BEEP,x) 
extern void app_gpio_set(gpio_pin_e pin,gpio_dir_t dir,gpio_pupd_e type);


extern void	GPIO_config(void);
extern void PrintTest1(u8 *Test);
extern u16 GETU16(u8 *value);
extern u32 GETU32_Big(u8 *value);

extern void pGetCpuId(void);

#if SET_BLE_SUB
extern void BLE_RFM98_IO(void);
#endif

extern void Touch_Wake_set(void);

#endif
