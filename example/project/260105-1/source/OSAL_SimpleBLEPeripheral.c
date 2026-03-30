/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
    Filename:       OSAL_SimpleBLEPeripheral.c
    Revised:
    Revision:

    Description:    This file contains function that allows user setup tasks



**************************************************************************************************/

/**************************************************************************************************
                                              INCLUDES
 **************************************************************************************************/
#if (APP_CFG == 0)
#include "OSAL.h"
#include "OSAL_Tasks.h"

/* LL */
#include "ll.h"

/* HCI */
#include "hci_tl.h"

#if defined ( OSAL_CBTIMER_NUM_TASKS )
    #include "osal_cbTimer.h"
#endif

/* L2CAP */
#include "l2cap.h"

/* gap */
#include "gap.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"

/* GATT */
#include "gatt.h"

#include "gattservapp.h"

/* Profiles */
#include "peripheral.h"

/* Application */
#include "config.h"
#include "simpleBLEPeripheral.h"
#include "watchdog.h"
#if SET_BLE_SUB
	#include	"phy_rf.h"
#endif
#include "app_main.h"

/*********************************************************************
    GLOBAL VARIABLES
*/

// The order in this table must be identical to the task initialization calls below in osalInitTask.
const pTaskEventHandlerFn tasksArr[] =
{
    LL_ProcessEvent,                                                  // task 0
    HCI_ProcessEvent,                                                 // task 1
    #if defined ( OSAL_CBTIMER_NUM_TASKS )
    OSAL_CBTIMER_PROCESS_EVENT( osal_CbTimerProcessEvent ),           // task 3
    #endif
    L2CAP_ProcessEvent,                                               // task 2
    SM_ProcessEvent,                                                  // task 3
    GAP_ProcessEvent,                                                 // task 4
    GATT_ProcessEvent,                                                // task 5
    GAPRole_ProcessEvent,                                             // task 6
    #if (DEF_GAPBOND_MGR_ENABLE==1)
    GAPBondMgr_ProcessEvent,                                          // task , add 2017-11-15
    #endif
    GATTServApp_ProcessEvent,                                         // task 7
		
    SimpleBLEPeripheral_ProcessEvent, 	// task 8
		#if SET_BLE_SUB
		PhyRf_ProcessEvent,
		#endif
		Watchdog_ProcessEvent,
		app_main_process_Event,
	
		
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16* tasksEvents;

/*********************************************************************
    FUNCTIONS
 *********************************************************************/
uint8 g_system_ota = 0;
uint8 p_RfInit_ota(void)
{
	uint8 taskID = 0;
	
	if(g_system_ota==1)
	{
		/* LL Task */
		LL_Init( taskID++ );
		/* HCI Task */
		HCI_Init( taskID++ );
		#if defined ( OSAL_CBTIMER_NUM_TASKS )
		/* Callback Timer Tasks */
		osal_CbTimerInit( taskID );
		taskID += OSAL_CBTIMER_NUM_TASKS;
		#endif
		/* L2CAP Task */
		L2CAP_Init( taskID++ );
		/* SM Task */
		SM_Init( taskID++ );
		/* GAP Task */
		GAP_Init( taskID++ );
		/* GATT Task */
		GATT_Init( taskID++ );
		/* Profiles */
		GAPRole_Init( taskID++ );
		GATTServApp_Init( taskID++ );
		
		SimpleBLEPeripheral_Init(taskID++);	
		
		#if	SET_BLE_SUB
		taskID++;
		#endif
	}
	else
	{
		taskID += 9;
		#if defined ( OSAL_CBTIMER_NUM_TASKS )
		taskID += OSAL_CBTIMER_NUM_TASKS;
		#endif		
		
		#if	SET_BLE_SUB
		PhyRf_Init( taskID++ );
		#endif
	}
	
	return taskID;
}
/*********************************************************************
    @fn      osalInitTasks

    @brief   This function invokes the initialization function for each task.

    @param   void

    @return  none
*/
void osalInitTasks( void )
{
    uint8 taskID = 0;
    tasksEvents = (uint16*)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
    osal_memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));
	
		//OTAÊ¹ÄÜ
		if(SUCCESS!=osal_snv_read(FILE_OTA_ID,1,&g_system_ota))
		{
			g_system_ota = 0;
		}
		LOG("g_system_ota = %d\r\n",g_system_ota);
		taskID += p_RfInit_ota();
		
		Watchdog_Init(taskID++);
		app_main_init(taskID);
		
}
#endif
/*********************************************************************
*********************************************************************/
