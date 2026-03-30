
/**********************************************************/
/*  公司名称：                    */
/*  开发人员：XTQ				          */
/*  实现功能：模拟SPI接口底层处理文件		          */
/**********************************************************/

/* Includes ------------------------------------------------------------------ */
#include "MSPI.h"

#if USE_SX1262
/*函数定义*/
void SPI_Init(void);						/* 模拟SPI接口初始化*/
uint8 SPI_RW_Data(uint8 iDat);	/* 模拟SPI同时读写一个字节*/

/*模拟SPI接口初始化*/
void SPI_Init(void)
{
	app_gpio_set(MSPI_SCK,GPIO_OUTPUT,GPIO_FLOATING);
	app_gpio_set(MSPI_MOSI,GPIO_OUTPUT,GPIO_FLOATING);
	app_gpio_set(MSPI_MISO,GPIO_INPUT,GPIO_PULL_UP);

	CLR_MSPI_SCK(); 
	CLR_MSPI_MOSI();
//	SET_MSPI_SS();
}

/* 模拟SPI同时读写一个字节*/
uint8 SPI_RW_Data(uint8 iDat)
{
	uint8 i,iResult = 0;
	
	SET_MSPI_MOSI();
	for(i=0;i<8;i++)
	{
		iResult = iResult << 1;

		if(iDat & 0x80)
		{
			SET_MSPI_MOSI();
		}else{
			CLR_MSPI_MOSI();
		}
		iDat = iDat << 1;	
		
		SET_MSPI_SCK();

		hal_gpio_pin_init(MSPI_MISO,GPIO_INPUT);
		if(READ_MSPI_MISO())
		{
			iResult |= 1;
		}
		CLR_MSPI_SCK();
	}

	return iResult;
}
#endif


