/*
 * @Author: your name
 * @Date: 2021-10-11 10:27:16
 * @LastEditTime: 2021-10-11 19:08:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \SK_ZH101\project\peripheral\ZH101\Source\rf_profile\phy_rfc.h
 */
/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
    Filename:       phy_Rf.h
    Revised:
    Revision:

    Description:    This file contains the phy sample application
                  definitions and prototypes.


**************************************************************************************************/

#ifndef PHYRF_H
#define PHYRF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include	"config.h"

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

#include "simpleBLEPeripheral.h"


/*********************************************************************
    CONSTANTS
*/


// PHY Task Events
#define PPP_PERIODIC_TX_EVT         0x0001
#define PPP_PERIODIC_RX_EVT         0x0002
#define PPP_TX_DONE_EVT             0x0004
#define PPP_RX_DONE_EVT             0x0008
#define PPP_TRX_DONE_EVT            0x0010
#define PPP_RX_DATA_PROCESS_EVT     0x0020
#define PPP_RX_CHECK_TIMER          0x0040
#define PPP_PERIODIC_TX_FAST_EVT    0x0080


extern uint8 PhyRf_TaskID; // Task ID for internal task/event processing
/*********************************************************************
    MACROS
*/

/*********************************************************************
    FUNCTIONS
*/
/*
    Task Initialization for the PHY Application
*/
extern void PhyRf_Init( uint8 task_id );

/*
    Task Event Processor for the PHY Application
*/
extern uint16 PhyRf_ProcessEvent( uint8 task_id, uint16 events );

extern void phy_tx_buf_updata(uint8_t* txPayload,uint8_t dlen);

extern uint8 g_system_ota;
extern uint8 p_RfInit_ota(void);
extern void p_Rf_Rx(void);

extern uint8_t adv_buffer[32];
extern uint8_t adv_len;

#if SET_BLE_SUB==0
#if SET_BLE_LEARN
extern __align(4) uint8_t  phyBufTx[400];
extern __align(4) uint8_t  phyBufRx[256];
#endif
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* PHYRF_H */
