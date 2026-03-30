/*!
 * \file      sx1262dvk1cas-board.c
 *
 * \brief     Target board SX1262DVK1CAS shield driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdlib.h>
#include "radio.h"
#include "sx126x-board.h"
#include "sx126x.h"
#include "MSPI.h"

#if (USE_SX1262 > 0)

/*!
 * Antenna switch GPIO pins objects
 */
uint8_t gPaOptSetting = 0;

extern void RadioOnDioIrq( void );
void SX126xIoInit( void )
{
    app_gpio_set(RF_RST,GPIO_OUTPUT,GPIO_FLOATING);
    app_gpio_set(RF_IRQ_PIN,GPIO_INPUT,GPIO_PULL_UP);
    app_gpio_set(RF_SEL_PIN,GPIO_OUTPUT,GPIO_FLOATING);
    app_gpio_set(RF_BUSY,GPIO_INPUT,GPIO_PULL_UP);
    
		app_gpio_set(RF_SW,GPIO_OUTPUT,GPIO_FLOATING);
}

void SX126xAntSwOn( void )
{
    // GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    //RF_SW = 1;
		hal_gpio_write(RF_SW,1);
		
}

void SX126xAntSwOff( void )
{
    // GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //RF_SW = 0;
   hal_gpio_write(RF_SW,0);
}


void SX126xReset( void )
{
    DelayMs( 20 );
    // GpioInit( &SX126x.Reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //RF_RST = 0;
    hal_gpio_write(RF_RST,0);
    DelayMs( 40 );
    // GpioInit( &SX126x.Reset, RADIO_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    //RF_RST = 1;
    hal_gpio_write(RF_RST,1);
    DelayMs( 20 );
}

void SX126xWaitOnBusy( void )
{
    static u8 isLoraDect = 0;

    // while( RF_BUSY != 0 );
    int i = 0;
    for(i = 0; i < 100; i ++)
    {
        p_RunPro();

        //if(RF_BUSY == 0)
        if(READ_RF_BUSY()==0)
        {
            break;
        }
        DelayMs(1);
    }
		
    //¼ì²âÊÇ·ñÓÐLORAÐ¾Æ¬
    if((i==100)&&(isLoraDect==0))
    {	
        LOG("CHK LORA FAIL\n");
        g_device_info.isok_get_Lora = 0;
    }
    else if(isLoraDect==0)
    {
        isLoraDect = 1;
        g_device_info.isok_get_Lora = 1;
        LOG("CHK LORA OK\n");
    }
}


void SX126xWakeup( void )
{
    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);
    // SPI_RW_Data(RADIO_GET_STATUS );
    // SPI_RW_Data(0x00 );
    SPI_RW_Data(RADIO_GET_STATUS);
    SPI_RW_Data(0x00);

    //RF_SEL_PIN = 1; 
    hal_gpio_write(RF_SEL_PIN,1);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);

    // SpiInOut( &SX126x.Spi, ( uint8_t )command );
    SPI_RW_Data(( uint8_t )command);

    for( i = 0; i < size; i++ )
    {
        SPI_RW_Data(buffer[i]);
    }

    //RF_SEL_PIN = 1; 
    hal_gpio_write(RF_SEL_PIN,1);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);

    SPI_RW_Data(( uint8_t )command);
    SPI_RW_Data(0x00);
    for( i = 0; i < size; i++ )
    {
        buffer[i] = SPI_RW_Data(0);
    }

    //RF_SEL_PIN = 1; 
    hal_gpio_write(RF_SEL_PIN,1);
    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);
    
    SPI_RW_Data(RADIO_WRITE_REGISTER );
    SPI_RW_Data(( address & 0xFF00 ) >> 8 );
    SPI_RW_Data(address & 0x00FF );
    
    for( i = 0; i < size; i++ )
    {
        SPI_RW_Data(buffer[i] );
    }

    //RF_SEL_PIN  = 1;
    hal_gpio_write(RF_SEL_PIN,1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);

    SPI_RW_Data(RADIO_READ_REGISTER );
    SPI_RW_Data(( address & 0xFF00 ) >> 8 );
    SPI_RW_Data(address & 0x00FF );
    SPI_RW_Data(0 );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = SPI_RW_Data(0 );
    }
    //RF_SEL_PIN  = 1;
    hal_gpio_write(RF_SEL_PIN,1);

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t dat;
    SX126xReadRegisters( address, &dat, 1 );
    return dat;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);

    SPI_RW_Data(RADIO_WRITE_BUFFER );
    SPI_RW_Data(offset );
    for( i = 0; i < size; i++ )
    {
        SPI_RW_Data(buffer[i] );
    }
    //RF_SEL_PIN  = 1;
    hal_gpio_write(RF_SEL_PIN,1);

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    uint16_t i;
    SX126xCheckDeviceReady( );

    //RF_SEL_PIN  = 0;
    hal_gpio_write(RF_SEL_PIN,0);

    SPI_RW_Data(RADIO_READ_BUFFER );
    SPI_RW_Data(offset );
    SPI_RW_Data(0 );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = SPI_RW_Data(0 );
    }
    //RF_SEL_PIN  = 1;
    hal_gpio_write(RF_SEL_PIN,1);

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetPaSelect( uint32_t channel )
{
	channel = channel;
    return SX1262;
}


#if 0
uint8_t SX126xGetPaOpt( )
{
    return gPaOptSetting;
}
#endif

void SX126xSetPaOpt( uint8_t opt )
{
    if(opt>3) return;
    
    gPaOptSetting = opt;
}


uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

#endif
