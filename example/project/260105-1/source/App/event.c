/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-09-26 20:32:57
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-10-13 11:33:55
 * @FilePath: \source\App\event.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include	"event.h"

//由顺序表构成的循环队列的算法
Event_Queue  g_queueEvent; 					//定义顺序队列

void EVENT_Init(void) 												//初始化顺序循环队列
{
    g_queueEvent.front = g_queueEvent.rear = EVENT_MAX-1;				//初始化空队
}

bit Event_AddEvent(u8 bEvent)						//顺序循环队列的入队算法
{	
	u8 bEventId;
	//static u8 bEventLast;
	
	//230609:任意事件上报，定时时间清0
	g_cbSta.bheart = 0;		

	//230710:注网不上反复按不会增加时长
	#if SET_NB||SET_CAT1
//	if(g_cbSta.bNetTimeOut1s>0)
//	{
//		g_device_info.isNbSleep = 0;
//	}
//	else
//	{
		g_cbSta.bNetTimeOut1s = TIME_OUT_NET;
		g_device_info.isNbSleep = 0;
//	}
	#endif

	#if SET_USE_PSM
	g_device_info.TryPsmCnt = 0;
	#endif
	
	//事件ID判断：报警状态，重复上报报警事件，仍维持原来的事件Id;否则递增
	//230711:反复报警事件ID不需要一样
	#if 0
	if((bEvent==EVENT_TEMP)||(bEvent==EVENT_TEMP_LOW))
	{
		if(g_cbSta.bTempAlmId==0)
		{
			g_cbSta.bEventId++;
			if(g_cbSta.bEventId==0)
				g_cbSta.bEventId = 1;
			g_cbSta.bTempAlmId = g_cbSta.bEventId;
		}
		bEventId = g_cbSta.bTempAlmId;

	}
	else if(bEvent==EVENT_ALM)
	{
		if(g_cbSta.bSmokeAlmId==0)
		{
			g_cbSta.bEventId++;
			if(g_cbSta.bEventId==0)
				g_cbSta.bEventId = 1;
			g_cbSta.bSmokeAlmId = g_cbSta.bEventId;
		}
		bEventId = g_cbSta.bSmokeAlmId;
	}
	else
	#endif
	{
		g_cbSta.bEventId++;
		bEventId = g_cbSta.bEventId;
	}
	user_fs_updata_memory();
	LOG("EVENT = %d,Id = %d\r\n",bEvent,bEventId);
	//bEventLast = g_cbSta.bEventId;
	
    if ( g_queueEvent.front == (g_queueEvent.rear+1) % EVENT_MAX )		//满队，入队失败
        return 0;
    else {
        g_queueEvent.rear = (g_queueEvent.rear+1) % EVENT_MAX;			//调整队尾变量
        g_queueEvent.queue[g_queueEvent.rear].bEvent = bEvent ; 		//
		g_queueEvent.queue[g_queueEvent.rear].bId = bEventId ; 
	
		#if SET_NB||SET_CAT1
		g_queueEvent.queue[g_queueEvent.rear].taskId = service.taskId;
		#endif
		return 1 ;
    }										//入队成功

	
}

bit Event_IsEmpty(void)
{
	if (g_queueEvent.front == g_queueEvent.rear)	 				
        return 1;
	else
	{
		return 0;
	}
}
QUEUE_OBJ_Event Event_PopEvent(void)										//循环队列的出队算法
{
//    if (g_queueEvent.front == g_queueEvent.rear)	 				//空队，出队失败
//        return 0;
//    else 
		{
        g_queueEvent.front = ( g_queueEvent.front+1) % EVENT_MAX ; 	//调整队首变量
        return ( g_queueEvent.queue[g_queueEvent.front] ) ;
    } 															//返回队首元素
}
