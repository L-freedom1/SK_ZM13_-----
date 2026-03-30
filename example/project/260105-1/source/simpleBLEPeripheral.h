/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-04-15 10:44:25
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-22 11:39:59
 * @FilePath: \source\simpleBLEPeripheral.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
  Filename:       simpleBLEperipheral.h
  Revised:         
  Revision:        

  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.

 
**************************************************************************************************/

#ifndef SIMPLEBLEPERIPHERAL_H
#define SIMPLEBLEPERIPHERAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include  "config.h"

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT					0x0001
#define SBP_RESET_ADV_EVT						0x0002
#define	SBP_DEALDATA							0x0004

#define SBP_RELOAD_1S             0x0008
#define	SBP_OTA_EVT								0x0010
#define SBP_RELOAD_50MS           0x0020
/*********************************************************************
 * MACROS
 */
#define MAC_DATA_LEN							6

extern	uint8	dev_mac_data[MAC_DATA_LEN];
extern	uint8	simpleBLEPeripheral_TaskID;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void SimpleBLEPeripheral_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events );


#if SET_BLE_LEARN
typedef struct
{
    //u8 rx_buf[256];
    u8 rx_pos;
    u8 rx_cmd;
    u8 rx_ok;

    u8 tx_to_50ms;
    u16 tx_pos;
    u16 tx_len;
    //u8 tx_buf[512];
}TYPE_BLE_JSON;
extern __align(4) uint8_t  phyBufRx[256];
extern __align(4) uint8_t  phyBufTx[400];
#define	json_rx_buf	phyBufRx
#define	json_tx_buf	phyBufTx
extern TYPE_BLE_JSON g_ble_json;
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLEPERIPHERAL_H */
