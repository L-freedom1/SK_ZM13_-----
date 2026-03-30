/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-04-28 16:06:57
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-08-07 16:50:00
 * @FilePath: \source\App\UserSPI.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _USER_SPI_H_
#define _USER_SPI_H_

#include    "config.h"

#define			SET_SPI_SLOW		1

#define     C_DLY_START     10

#define     C_DLY_BIT       2
#define     C_DLY_HALF_BIT  C_DLY_BIT/2

#define     HIGH    1
#define     LOW     0   
#define     HT_MSPI_SCK        GPIO_P03

#define     HT_MSPI_MISO       GPIO_P02

#define SET_CLK_OUTPUT(state)   hal_gpio_fast_write(HT_MSPI_SCK,state)

#define SET_DIO_OUTPUT()        hal_gpio_pin_init(HT_MSPI_MISO,GPIO_OUTPUT) 
#define SET_DIO_INPUT()         hal_gpio_pin_init(HT_MSPI_MISO,GPIO_INPUT) 
#define SET_DIO_OUTPUT_H()      hal_gpio_fast_write(HT_MSPI_MISO,1)
#define SET_DIO_OUTPUT_L()      hal_gpio_fast_write(HT_MSPI_MISO,0)  
#define READ_DIO()              hal_gpio_read(HT_MSPI_MISO)

#define READ_CLK()              hal_gpio_read(HT_MSPI_SCK)

#define SET_P10(state)               hal_gpio_fast_write(GPIO_P10,state)
//
//#define SET_CLK_INPUT()         hal_gpio_pin_init(HT_MSPI_SCK,GPIO_INPUT) 
//#define SET_CLK_OUTPUT()         hal_gpio_pin_init(HT_MSPI_SCK,GPIO_INPUT) 
//#define READ_CLK()              hal_gpio_read(HT_MSPI_SCK)

void initMySPI(void);
void MySPI_SendByte(u8 sdat);
u8  MySPI_ReadByte(void);
#if SET_RX_TRAN
#else
void MySPI_Start(void);
#endif
void MySPI_Stop(void);
void MySPI_Delay(u8 tm);

extern void MySPI_pro(void);

#if SET_BLE_DEBUG
extern u8 spi_Test;
extern u8 spi_RcvBuf[11];
extern u8 spi_RcvLen;
#endif
#endif
