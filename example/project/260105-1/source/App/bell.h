/*
 * @Author: your name
 * @Date: 2021-02-18 16:42:14
 * @LastEditTime: 2021-02-24 09:16:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \¾ÆµêÃÅËø\BELL\bell.h
 */
#ifndef __BELL_H
#define __BELL_H

#include "..\APP\Config.h"

#define PLAY_SMOKE_ALARM()   				 Fun_beep(2, 500); 		//1´ÎÃù½Ğ
#define PLAY_KEY_ALARM()   				 	 Fun_beep(6, 500); 			//1´ÎÃù½Ğ
#define PLAY_KEY()             				Fun_beep(2, 75);    	//¶ÌÃù1Éù        
#define PLAY_SetSucceBell()    Fun_beep(6, 75);    				//¶ÌÃù3Éù         
#define PLAY_FailBell()        Fun_beep(4, 100);    			//¶ÌÃù2Éù

//extern u8 g_duty;

extern void Fun_beep(u16 iblinktimes, u16 iblinktime);
extern void Fun_beep_scan(void);
extern bool bell_is_bell(void);
extern void app_pwm_set(u16 pwm_freq);
extern void Fun_beep_stop(void);
#endif

