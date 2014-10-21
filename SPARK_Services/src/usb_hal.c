/**
 ******************************************************************************
 * @file    usb_hal.c
 * @author  Satish Nair
 * @version V1.0.0
 * @date    10-Sept-2014
 * @brief   USB HAL
 ******************************************************************************
  Copyright (c) 2013-14 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb_hal.h"
#include "usb_conf.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_prop.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
#ifdef USB_CDC_ENABLE
uint8_t  USART_Rx_Buffer[USART_RX_DATA_SIZE];
uint32_t USART_Rx_ptr_in = 0;
uint32_t USART_Rx_ptr_out = 0;
uint32_t USART_Rx_length  = 0;

uint8_t USB_Rx_Buffer[CDC_DATA_SIZE];
uint16_t USB_Rx_length = 0;
uint16_t USB_Rx_ptr = 0;

uint8_t  USB_Tx_State = 0;
uint8_t  USB_Rx_State = 0;

uint32_t USB_USART_BaudRate = 9600;

__IO uint8_t PrevXferComplete;
#endif

/* Extern variables ----------------------------------------------------------*/
extern LINE_CODING linecoding;

/* Private function prototypes -----------------------------------------------*/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);

#if defined (USB_CDC_ENABLE) || defined (USB_HID_ENABLE)
/*******************************************************************************
 * Function Name  : SPARK_USB_Setup
 * Description    : Spark USB Setup.
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void SPARK_USB_Setup(void)
{
  USB_Disconnect_Config();
  USB_Cable_Config(DISABLE);
  Delay_Microsecond(100000);
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
}

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(uint32_t*)ID1;
  Device_Serial1 = *(uint32_t*)ID2;
  Device_Serial2 = *(uint32_t*)ID3;

  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &USB_StringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &USB_StringSerial[18], 4);
  }
}
#endif

#ifdef USB_CDC_ENABLE
/*******************************************************************************
 * Function Name  : USB_USART_Init
 * Description    : Start USB-USART protocol.
 * Input          : baudRate.
 * Return         : None.
 *******************************************************************************/
void USB_USART_Init(uint32_t baudRate)
{
  linecoding.bitrate = baudRate;
  SPARK_USB_Setup();
}

/*******************************************************************************
 * Function Name  : USB_USART_Available_Data.
 * Description    : Return the length of available data received from USB.
 * Input          : None.
 * Return         : Length.
 *******************************************************************************/
uint8_t USB_USART_Available_Data(void)
{
  if(bDeviceState == CONFIGURED)
  {
    if(USB_Rx_State == 1)
    {
      return (USB_Rx_length - USB_Rx_ptr);
    }
  }

  return 0;
}

/*******************************************************************************
 * Function Name  : USB_USART_Receive_Data.
 * Description    : Return data sent by USB Host.
 * Input          : None
 * Return         : Data.
 *******************************************************************************/
int32_t USB_USART_Receive_Data(void)
{
  if(bDeviceState == CONFIGURED)
  {
    if(USB_Rx_State == 1)
    {
      if((USB_Rx_length - USB_Rx_ptr) == 1)
      {
        USB_Rx_State = 0;

        /* Enable the receive of data on EP3 */
        SetEPRxValid(ENDP3);
      }

      return USB_Rx_Buffer[USB_Rx_ptr++];
    }
  }

  return -1;
}

/*******************************************************************************
 * Function Name  : USB_USART_Send_Data.
 * Description    : Send Data from USB_USART to USB Host.
 * Input          : Data.
 * Return         : None.
 *******************************************************************************/
void USB_USART_Send_Data(uint8_t Data)
{
  if(bDeviceState == CONFIGURED)
  {
    USART_Rx_Buffer[USART_Rx_ptr_in] = Data;

    USART_Rx_ptr_in++;

    /* To avoid buffer overflow */
    if(USART_Rx_ptr_in == USART_RX_DATA_SIZE)
    {
      USART_Rx_ptr_in = 0;
    }

    if(CC3000_Read_Interrupt_Pin())
    {
      //Delay 100us to avoid losing the data
      Delay_Microsecond(100);
    }
  }
}
#endif

#ifdef USB_HID_ENABLE
/*******************************************************************************
 * Function Name : USB_HID_Send_Report.
 * Description   : Send HID Report Info to Host.
 * Input         : pHIDReport and reportSize.
 * Output        : None.
 * Return value  : None.
 *******************************************************************************/
void USB_HID_Send_Report(void *pHIDReport, size_t reportSize)
{
  if (bDeviceState == CONFIGURED)
  {
    /* Reset the control token to inform upper layer that a transfer is ongoing */
    PrevXferComplete = 0;

    /* Copy HID Report to ENDP1 Tx Packet Memory Area*/
    UserToPMABufferCopy((uint8_t *)pHIDReport, ENDP1_TXADDR, reportSize);
    SetEPTxCount(ENDP1, reportSize);

    /* Enable endpoint for transmission */
    SetEPTxValid(ENDP1);
  }
}
#endif

/*******************************************************************************
 * Function Name  : HexToChar.
 * Description    : Convert Hex 32Bits value into char.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;

  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[ 2* idx + 1] = 0;
  }
}
