/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-08-26 09:55:42
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-09-27 15:45:05
 * @FilePath: \source\App\my_timer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
    Filename:       adc_demo.h
    Revised:        $Date $
    Revision:       $Revision $


**************************************************************************************************/

#ifndef	MY_TIMER
#define	MY_TIMER

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "config.h"
/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    MACROS
*/
//KEY
#define  dKEY_ON   0    
#define	 dKEY_OFF  1
#define  dKey0CurSta  		hal_gpio_read(PIN_KEY)
typedef enum
{
	eKEYType0 ,
	eKEYType1 = 1,
	eKEYType2 = 2, 
}eTypedefEnum;

typedef union
{
	u8 B;
	struct
	{
		u8 ShortPress   : 1;
		u8 LongPress    : 1;
		u8 ReleasePress : 1;
		u8 Long2Press : 1;
	}b;
}uTypedefKeyFlag;

typedef struct 
{
	u16 TCount ;
	uTypedefKeyFlag  Flag;
}sKeyInfo;
extern sKeyInfo Key[1];

//防拆
#define  dFcCurSta  		hal_gpio_read(PIN_FC)
#define		FC_CLOSE			0
#define		FC_OPEN				1
#define		FC_TIMEOUT_1S		90		//90s

extern u16 bFcClose;
extern u16 bFcOpen;
extern u8 g_is10msTouch,g_is1sTouch;
/*********************************************************************
    FUNCTIONS
*/

extern void TIMER_config(void);
extern u16 bWatchDog1s;
extern u32 my_time(void);
extern void fKeyCheck(eTypedefEnum KeyType,u8 KeyCurSta);
extern void pDectFc(u8 isTouch);
extern void fKeyPro(void);

#if USE_SX1262
extern u8 g_is100msTouch;
#endif

#if SET_BLE_LEARN
extern u8     	g_bWaitTimeOut10ms;
#endif

#if SET_NB||SET_CAT1
extern u8 g_bWaitTimeOut1s;
extern void os_waitMs(u16 time_n);

extern u8 PirCnt;
extern u8 PirReg;
extern u16 PirLast01ms;
extern u16 Pir2Last01ms;
extern u8 g_isSend;
#endif

#if SET_BLE_DEBUG
extern void note_set_ble_debug(void);
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
