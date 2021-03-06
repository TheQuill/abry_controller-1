/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "w5500/w5500.h"
#include "w5500/socket.h"
#include "dhcp.h"
#include "E131_interface.h"
#include <string.h>
#include "abry_config.h"
#include "ws2812.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim1_ch2;
DMA_HandleTypeDef hdma_tim2_ch1;
DMA_HandleTypeDef hdma_tim3_ch1_trig;
DMA_HandleTypeDef hdma_tim4_ch1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart3_tx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define SOCK_DHCP   0
#define SOCKET_UDP    1

#define DMX_START_UNIVERSE  1
#define DMX_NR_OF_UNIVERSES 1
#define DMX_CHANNELS_PER_UNIVERSE 512
#define DATA_BUFFER_SIZE 512

#define BUFFERSIZE  4096

#define STM32_UUID ((uint32_t *)UID_BASE)

Ws2812DmaAdmin Ws2812DmaAdminDevice1;
Ws2812DmaAdmin Ws2812DmaAdminDevice2;
Ws2812DmaAdmin Ws2812DmaAdminDevice3;
Ws2812DmaAdmin Ws2812DmaAdminDevice4;

abry_hw_config hw_configuration;
abry_config configuration;


uint8 buffer[BUFFERSIZE] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void ws2812DmaIsrDev1(DMA_HandleTypeDef *hdma);
void ws2812DmaIsrDev2(DMA_HandleTypeDef *hdma);
void ws2812DmaIsrDev3(DMA_HandleTypeDef *hdma);
void ws2812DmaIsrDev4(DMA_HandleTypeDef *hdma);
uint32_t determineTimerPeriod(uint8_t timer);
void sendOutputs();

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void user_pwm_setvalue(uint16_t value, TIM_HandleTypeDef *htim, uint32_t channel )
{

    switch(channel)
    {
    case TIM_CHANNEL_1:
        htim->Instance->CCR1 = value;
        break;
    case TIM_CHANNEL_2:
        htim->Instance->CCR2 = value;
        break;
    case TIM_CHANNEL_3:
        htim->Instance->CCR3 = value;
        break;
    case TIM_CHANNEL_4:
        htim->Instance->CCR4 = value;
        break;
    default:
        //do nothing
        break;
    }
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8 dhcpret=0;
  uint16_t pwm_value;
  uint16_t step;

  uint32_t id = STM32_UUID[0];

  //create hw config
  hw_configuration.outputs[0].allowed_types = OT_PWM + OT_WS2811;
  hw_configuration.outputs[0].device = 12;
  hw_configuration.outputs[1].allowed_types = OT_PWM;
  hw_configuration.outputs[1].device = 11;
  hw_configuration.outputs[2].allowed_types = OT_PWM;
  hw_configuration.outputs[2].device = 13;
  hw_configuration.outputs[3].allowed_types = OT_PWM;
  hw_configuration.outputs[3].device = 14;
  hw_configuration.outputs[4].allowed_types = OT_PWM + OT_WS2811;
  hw_configuration.outputs[4].device = 21;
  hw_configuration.outputs[5].allowed_types = OT_PWM;
  hw_configuration.outputs[5].device = 22;
  hw_configuration.outputs[6].allowed_types = OT_PWM + OT_WS2811;
  hw_configuration.outputs[6].device = 31;
  hw_configuration.outputs[7].allowed_types = OT_PWM;
  hw_configuration.outputs[7].device = 32;
  hw_configuration.outputs[8].allowed_types = OT_PWM;
  hw_configuration.outputs[8].device = 33;
  hw_configuration.outputs[9].allowed_types = OT_PWM;
  hw_configuration.outputs[9].device = 34;
  hw_configuration.outputs[10].allowed_types = OT_PWM + OT_WS2811;
  hw_configuration.outputs[10].device = 41;
  hw_configuration.outputs[11].allowed_types = OT_PWM;
  hw_configuration.outputs[11].device = 42;
  hw_configuration.outputs[12].allowed_types = OT_PWM;
  hw_configuration.outputs[12].device = 43;
  hw_configuration.outputs[13].allowed_types = OT_PWM;
  hw_configuration.outputs[13].device = 44;
  hw_configuration.outputs[14].allowed_types = OT_UART;
  hw_configuration.outputs[14].device = 1;
  hw_configuration.outputs[15].allowed_types = OT_UART;
  hw_configuration.outputs[15].device = 2;
  hw_configuration.outputs[16].allowed_types = OT_UART;
  hw_configuration.outputs[16].device = 3;

  //config input
  configuration.input.type = DMX;
  configuration.input.start = 1;
  configuration.input.count = 1;

  //clear config
  for (int ii = 0; ii < NR_OF_OUTPUTS; ++ii)
  {
    configuration.outputs[ii].type = OT_OFF;
  }

  //config device 1
//  configuration.outputs[0].type = OT_WS2811;
//  configuration.outputs[0].channel_start = 0;
//  configuration.outputs[0].channel_count = 150; //(50 rgb pixels)
//
  //config device 1
  configuration.outputs[0].type = OT_WS2811;
  configuration.outputs[0].channel_start = 1;
  configuration.outputs[0].channel_count = 150; //(50 ledpixels)

//  //config device 2
  configuration.outputs[4].type = OT_WS2811;
  configuration.outputs[4].channel_start = 151;
  configuration.outputs[4].channel_count = 150; //(50 ledpixels output)





  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  /* USER CODE BEGIN 2 */


  uint8 pc_ip[4]={192,168,1,104};/*ÅäÖÃ·þÎñÆ÷µÄIPµØÖ·*/
  uint16 pc_port=6001;/*¶¨Òå¼ÆËã»úµÄÒ»¸ö¶Ë¿Ú²¢³õÊ¼»¯*/
  uint16 len=0;
  uint16_t datalen=0;
  uint16_t universe =0;
  uint8 rIP[4];
  uint16 rPort;
  uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};/*¶¨ÒåMac±äÁ¿*/
  uint8 lip[4]={192,168,1,114};/*¶¨Òålp±äÁ¿*/
  uint8 sub[4]={255,255,255,0};/*¶¨Òåsubnet±äÁ¿*/
  uint8 gw[4]={192,168,1,1};/*¶¨Òågateway±äÁ¿*/
  uint8 ip[4];
  uint8 test[10] = "test";
  uint32 counter=0;


  E131Handle E131Parser = create_E131();


  //clear receive buffer
  memset(buffer,0,BUFFERSIZE);
  //  RCC_Configuration(); /* ÅäÖÃµ¥Æ¬»úÏµÍ³Ê±ÖÓ*/
  //  GPIO_Configuration();/* ÅäÖÃGPIO*/
  //  NVIC_Configuration();/* ÅäÖÃÇ¶Ì×ÖÐ¶ÏÏòÁ¿*/
  //
  //   Systick_Init(72);/* ³õÊ¼»¯Systick¹¤×÷Ê±ÖÓ*/
  //   USART1_Init(); /*³õÊ¼»¯´®¿ÚÍ¨ÐÅ:115200@8-n-1*/
  //   at24c16_init();/*³õÊ¼»¯eeprom*/
  //   printf("W5500 EVB initialization over.\r\n");
  //
  //   Reset_W5500();/*Ó²ÖØÆôW5500*/
  //   WIZ_SPI_Init();/*³õÊ¼»¯SPI½Ó¿Ú*/
  iinchip_init(&hspi1);
  //   printf("W5500 initialized!\r\n");

  setSHAR(mac);/*ÅäÖÃMacµØÖ·*/
  setSUBR(sub);/*ÅäÖÃ×ÓÍøÑÚÂë*/
  setGAR(gw);/*ÅäÖÃÄ¬ÈÏÍø¹Ø*/
  setSIPR(lip);/*ÅäÖÃIpµØÖ·*/

  //Init. TX & RX Memory size of w5500
  uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
  uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};

  sysinit(txsize, rxsize); /*³õÊ¼»¯8¸ösocket*/

  setRTR(2000);/*ÉèÖÃÒç³öÊ±¼äÖµ*/
  setRCR(3);/*ÉèÖÃ×î´óÖØÐÂ·¢ËÍ´ÎÊý*/


  getSIPR (ip);
  printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getSUBR(ip);
  printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getGAR(ip);
  printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  printf("Network is ready.\r\n");



  //initialize  all outputs to off.
  //TODO: op basis van config maken
  ws2812Send(&(buffer[configuration.outputs[0].channel_start - 1]), (configuration.outputs[0].channel_count / 3), &Ws2812DmaAdminDevice1);
  ws2812Send(&(buffer[configuration.outputs[4].channel_start - 1]), (configuration.outputs[4].channel_count / 3), &Ws2812DmaAdminDevice2);

//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
//  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);



  while(1)
  {

    //Try to read from ethernet

    switch(getSn_SR(0))/*»ñÈ¡socket 0µÄ×´Ì¬*/
    {
    case SOCK_UDP:/*socket µÄÌ×½Ó×Ö³õÊ¼»¯Íê³É*/
      //setSn_IR(0, Sn_IR_RECV);
      counter++;
      //sprintf(test, "test:%03d\r\n", counter);
      //sendto(0, test,10, pc_ip, pc_port);
      //Delay_ms(100);
      if(getSn_IR(0) & Sn_IR_RECV)
      {
        setSn_IR(0, Sn_IR_RECV);/*Sn_IRµÄµÚ0Î»ÖÃ1*/
      }
      if((len=getSn_RX_RSR(0))>0)
      {
        //recvfrom(0, buffer, len, pc_ip,&pc_port);/*W5200½ÓÊÕ¼ÆËã»ú·¢ËÍÀ´µÄÊý¾Ý*/
        if (len <= E131_PACKET_SIZE)
        {
          //fill parser buffer
          recvfrom(0, E131_getBuffer(E131Parser), len, rIP,&rPort);

        }
        else
        {
          recvfrom(0, E131_getBuffer(E131Parser), E131_PACKET_SIZE, rIP,&rPort);
        }
        //parsepacket
        datalen = E131_parsePacket(E131Parser);
        universe = E131_getUniverse(E131Parser);

        //copy data to showbuffer
        if((universe >= configuration.input.start) && (universe < (configuration.input.start + configuration.input.count)))
        {
          memcpy(&(buffer[(universe - configuration.input.start) * 512]), E131_getData(E131Parser),datalen);
        }



        //all data received? -> update leds.
        if(universe == (configuration.input.start + configuration.input.count - 1))
        {
          sendOutputs();
        }
      }
      break;
    case SOCK_CLOSED:/*socket ¹Ø±Õ*/
      socket(0,Sn_MR_UDP,5568,0);/*³õÊ¼»¯socket 0µÄÌ×½Ó×Ö*/
      break;
    }
  }
}
//  uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
//  uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
//
//
//  //  Reset_W5500();
//  iinchip_init(&hspi1);
//  set_default();
//  init_dhcp_client();
//

  //init udp
  //FIXME: create function


  //Init. TX & RX Memory size of w5500
//  sysinit(txsize, rxsize); /*³õÊ¼»¯8¸ösocket*/
//  setRTR(2000);
//  setRCR(3);

//  /* USER CODE END 2 */
//
//  /* Infinite loop */
//  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//
//    uint16 len=0;
//    uint8 rIP[4];
//    uint16 rPort;
//    uint8 buffer[2048];
//
//    dhcpret = check_DHCP_state(SOCK_DHCP);
//    switch(dhcpret)
//    {
//      case DHCP_RET_NONE:
//        switch (getSn_SR(SOCKET_UDP))
//        /*»ñÈ¡socket 0µÄ×´Ì¬*/
//        {
//        case SOCK_UDP:/*socket µÄÌ×½Ó×Ö³õÊ¼»¯Íê³É*/
//          //setSn_IR(0, Sn_IR_RECV);
//    //      counter++;
//          //sprintf(test, "test:%03d\r\n", counter);
//          //sendto(0, test,10, pc_ip, pc_port);
//          //Delay_ms(100);
//          if (getSn_IR(SOCKET_UDP) & Sn_IR_RECV)
//          {
//            setSn_IR(SOCKET_UDP, Sn_IR_RECV);/*Sn_IRµÄµÚ0Î»ÖÃ1*/
//          }
//          if ((len = getSn_RX_RSR(SOCKET_UDP)) != 0)
//          {
//            //recvfrom(0, buffer, len, pc_ip,&pc_port);/*W5200½ÓÊÕ¼ÆËã»ú·¢ËÍÀ´µÄÊý¾Ý*/
//            recvfrom(SOCKET_UDP, buffer, len, rIP, &rPort);
//            printf("%d.%d.%d.%d:%d", rIP[0], rIP[1], rIP[2], rIP[3], rPort);
//            sendto(SOCKET_UDP, buffer, len, rIP, rPort);/*W5200°Ñ½ÓÊÕµ½µÄÊý¾Ý·¢ËÍ¸ø¼ÆËã»ú*/
//          }
//          break;
//        case SOCK_CLOSED:/*socket ¹Ø±Õ*/
//          socket(SOCKET_UDP, Sn_MR_UDP, 3000, 0);/*³õÊ¼»¯socket 0µÄÌ×½Ó×Ö*/
//          break;
//        }
//        break;
//      case DHCP_RET_TIMEOUT:
//        break;
//      case DHCP_RET_UPDATE:
//        break;
//      case DHCP_RET_CONFLICT:
//        while(1);
//      default:
//        break;
//    }
//
//
//  /* USER CODE END WHILE */
//
//  /* USER CODE BEGIN 3 */
//
//  }
//  /* USER CODE END 3 */
//
//}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = determineTimerPeriod(1);
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) !=HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  __TIM1_CLK_ENABLE();
  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = determineTimerPeriod(2);
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = determineTimerPeriod(3);
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = determineTimerPeriod(4);
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 1000000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 1000000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 1000000;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  Ws2812DmaAdminDevice1.DMAChannel = TIM_CHANNEL_2;
  Ws2812DmaAdminDevice1.TimerPtr = &htim1;
  Ws2812DmaAdminDevice1.DMACaptureCompare = TIM_DMA_CC2;
  hdma_tim1_ch2.XferCpltCallback = ws2812DmaIsrDev1;
  hdma_tim1_ch2.XferHalfCpltCallback = ws2812DmaIsrDev1;

  Ws2812DmaAdminDevice2.DMAChannel = TIM_CHANNEL_1;
  Ws2812DmaAdminDevice2.TimerPtr = &htim2;
  Ws2812DmaAdminDevice2.DMACaptureCompare = TIM_DMA_CC1;
  hdma_tim2_ch1.XferCpltCallback = ws2812DmaIsrDev2;
  hdma_tim2_ch1.XferHalfCpltCallback = ws2812DmaIsrDev2;

  Ws2812DmaAdminDevice3.DMAChannel = TIM_CHANNEL_1;
  Ws2812DmaAdminDevice3.TimerPtr = &htim3;
  Ws2812DmaAdminDevice3.DMACaptureCompare = TIM_DMA_CC1;
  hdma_tim3_ch1_trig.XferCpltCallback = ws2812DmaIsrDev3;
  hdma_tim3_ch1_trig.XferHalfCpltCallback = ws2812DmaIsrDev3;

  Ws2812DmaAdminDevice4.DMAChannel = TIM_CHANNEL_1;
  Ws2812DmaAdminDevice4.TimerPtr = &htim4;
  Ws2812DmaAdminDevice4.DMACaptureCompare = TIM_DMA_CC1;
  hdma_tim4_ch1.XferCpltCallback = ws2812DmaIsrDev4;
  hdma_tim4_ch1.XferHalfCpltCallback = ws2812DmaIsrDev4;


  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, uart2_txrx_select_Pin|uart3_txrx_select_Pin|uart1_txrx_select_Pin|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : uart2_txrx_select_Pin uart3_txrx_select_Pin uart1_txrx_select_Pin PA15 */
  GPIO_InitStruct.Pin = uart2_txrx_select_Pin|uart3_txrx_select_Pin|uart1_txrx_select_Pin|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */


void ws2812DmaIsrDev1(DMA_HandleTypeDef *hdma)
{
  ws2812DmaIsr(hdma, &Ws2812DmaAdminDevice1);
}

void ws2812DmaIsrDev2(DMA_HandleTypeDef *hdma)
{
  ws2812DmaIsr(hdma, &Ws2812DmaAdminDevice2);
}
void ws2812DmaIsrDev3(DMA_HandleTypeDef *hdma)
{
  ws2812DmaIsr(hdma, &Ws2812DmaAdminDevice3);
}

void ws2812DmaIsrDev4(DMA_HandleTypeDef *hdma)
{
  ws2812DmaIsr(hdma, &Ws2812DmaAdminDevice4);
}


/**
 * Determine if full or low speed is needed for the ws2811 output.
 * If the same timer is configured to use both a ws2811 output and a pwm output,
 * the low speed is chosen to make it possible to have a 255 range for pwm.
 */
uint32_t determineTimerPeriod(uint8_t timer)
{
  uint8_t ii;
  uint8_t start_range = timer * 10;
  uint8_t end_range = start_range + 9;
  uint32_t timer_period = 0x64; //asume high speed

  for (ii = 0; ii < NR_OF_OUTPUTS; ++ii)
  {
    if ((hw_configuration.outputs[ii].device >= start_range)
        && (hw_configuration.outputs[ii].device <= end_range))
    {
      if (configuration.outputs[ii].type == OT_PWM)
      {
        //at least one pwm, so set to low speed.
        timer_period = 0xFF;
      }
    }
  }
  return timer_period;
}

void sendOutputs()
{
  uint8_t outputcounter;
  TIM_HandleTypeDef *timerhandle;
  uint32_t channel;

  for (outputcounter = 0; outputcounter < NR_OF_OUTPUTS; ++outputcounter)
  {
    switch (configuration.outputs[outputcounter].type)
    {
    case OT_PWM:
      //determine timer and channel
      switch (hw_configuration.outputs[outputcounter].device % 10)
      {
      case 1:
        channel = TIM_CHANNEL_1;
        break;
      case 2:
        channel = TIM_CHANNEL_2;
        break;
      case 3:
        channel = TIM_CHANNEL_3;
        break;
      case 4:
        channel = TIM_CHANNEL_4;
        break;
      default:
        break;
      }
      if (hw_configuration.outputs[outputcounter].device < 20)
      {
        timerhandle = &htim1;
      }
      else if (hw_configuration.outputs[outputcounter].device < 30)
      {
        timerhandle = &htim2;
      }
      else if (hw_configuration.outputs[outputcounter].device < 40)
      {
        timerhandle = &htim3;
      }
      else
      {
        timerhandle = &htim4;
      }

      //set value
      uint16_t value =
          buffer[configuration.outputs[outputcounter].channel_start];
      user_pwm_setvalue(value, timerhandle, channel);
      break;
    case OT_WS2811:
    {
      Ws2812DmaAdmin * dmaAdmin;
      if (hw_configuration.outputs[outputcounter].device < 20)
      {
        dmaAdmin = &Ws2812DmaAdminDevice1;
      }
      else if (hw_configuration.outputs[outputcounter].device < 30)
      {
        dmaAdmin = &Ws2812DmaAdminDevice2;
      }
      else if (hw_configuration.outputs[outputcounter].device < 40)
      {
        dmaAdmin = &Ws2812DmaAdminDevice3;
      }
      else
      {
        dmaAdmin = &Ws2812DmaAdminDevice4;
      }

      ws2812Send(
          &(buffer[configuration.outputs[outputcounter].channel_start - 1]),
          (configuration.outputs[outputcounter].channel_count / 3), dmaAdmin);
      break;
    }
    case OT_UART:
      //send header (head mbv dma en interupt versturen?)
      //send data mbv dma
    case OT_OFF: /* fallthrough */
    default:
      break;
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
