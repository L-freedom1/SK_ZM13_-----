/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-02-04 16:38:06
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-02-19 16:49:05
 * @FilePath: \source\App\event.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _EVENT_H
#define _EVENT_H

#include	"config.h"

/* Define */
#define 	EVENT_MAX   	10  			

/* 事件代码 */
enum 
{
	EVENT_PLUS = 1,	//心跳
	EVENT_ALM,		//烟感报警
	EVENT_ALM_RST,	//烟感恢复
	EVENT_TEST,		//自检上报
	EVENT_TEMP,
	EVENT_TEMP_RST,
	EVENT_BAT,
	EVENT_BAT_RST,
	EVENT_BOOT,
	EVENT_TEMP_LOW,
	EVENT_TEMP_LOW_RST,
	EVENT_FCALM,
	EVENT_FCRST,
	EVENT_PLUS_AG,	//重复心跳
	
};

#define	EVENT_SPI_TEST	0xaa

typedef struct {                                                // object data type
  u8  bEvent;	//事件类型
	u8  bId;		//事件序号
//  u32 TimeVal;	//时间
//  u16 Sta;		//附加状态
	#if SET_PROTO_VER==PROTO_VER_JSON
	u16 taskId;		//
	#endif
	
}QUEUE_OBJ_Event;

/*
*   事件队列,循环队列实现.
*/
typedef struct
{
    QUEUE_OBJ_Event   queue[EVENT_MAX] ; 			//用数组作为队列的储存空间
    int   front,rear ;						//指示队头位置和队尾位置的变量
} Event_Queue; 						    		//顺序队列类型定义
    
/* External Variable */
extern Event_Queue  g_queueEvent;    /* 事件队列 */

/* Function */
extern void EVENT_Init(void);
extern bit Event_AddEvent(u8 bEvent);        
extern QUEUE_OBJ_Event Event_PopEvent(void);	   
extern bit Event_IsEmpty(void);                        

#endif
