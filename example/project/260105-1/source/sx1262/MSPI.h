/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-10-23 11:58:07
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-11-15 16:58:05
 * @FilePath: \source\sx1262\MSPI.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _MSPI_H_
#define _MSPI_H_

#include "config.h"

/*硬件接口定义*/   
//#define MSPI_SS		P17
#define MSPI_MOSI       GPIO_P32
#define MSPI_MISO       GPIO_P33
#define MSPI_SCK		GPIO_P34

//#define SET_MSPI_SS()               (MSPI_SS = 1)        
//#define CLR_MSPI_SS()               (MSPI_SS = 0)   
#define SET_MSPI_SCK()              	hal_gpio_write(MSPI_SCK,1)    
#define CLR_MSPI_SCK()              	hal_gpio_write(MSPI_SCK,0)    
#define SET_MSPI_MOSI()            		hal_gpio_write(MSPI_MOSI,1)      
#define CLR_MSPI_MOSI()             	hal_gpio_write(MSPI_MOSI,0) 

#define READ_MSPI_MISO()              hal_gpio_read(MSPI_MISO)
/*常量定义*/
#define MSPI_WRITE		            		0  			//写
#define MSPI_READ											1  			//读

/*外部函数声明*/
extern void SPI_Init(void);								/* 模拟SPI接口初始化*/
extern uint8 SPI_RW_Data(uint8 iDat);			/* 模拟SPI同时读写一个字节*/


#endif


