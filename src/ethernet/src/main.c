#include "mcu_init.h"
#include "config.h"
#include "device.h"
#include "spi2.h"
#include "socket.h"
#include "w5500.h"
#include "at24c16.h"
#include "util.h"
#include "dhcp.h"

#include <stdio.h>

extern uint8 txsize[];
extern uint8 rxsize[];

uint8 buffer[2048];/*����һ��2KB�Ļ���*/
  
void main()
{
  uint8 dhcpret=0;
  
  RCC_Configuration(); /* ���õ�Ƭ��ϵͳʱ��*/
  GPIO_Configuration();/* ����GPIO*/
  NVIC_Configuration();/* ����Ƕ���ж�����*/
  Timer_Configuration();
  Systick_Init(72);/* ��ʼ��Systick����ʱ��*/
  USART1_Init(); /*��ʼ������ͨ��:115200@8-n-1*/
  at24c16_init();/*��ʼ��eeprom*/
  printf("W5500 EVB initialization over.\r\n");
  
  Reset_W5500();/*Ӳ����W5500*/
  WIZ_SPI_Init();/*��ʼ��SPI�ӿ�*/
  printf("W5500 initialized!\r\n");  
  
  set_default(); 
  
  init_dhcp_client();
  while(1)
  {
    dhcpret = check_DHCP_state(SOCK_DHCP);
    switch(dhcpret)
    {
      case DHCP_RET_NONE:
        break;
      case DHCP_RET_TIMEOUT:
        break;
      case DHCP_RET_UPDATE:
        break;
      case DHCP_RET_CONFLICT:
        while(1);
      default:
        break;
    }
  }
}