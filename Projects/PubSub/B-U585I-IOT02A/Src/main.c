/**
******************************************************************************
* @file           : main.c
* @brief          : Main program body
******************************************************************************

******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp32.h"
#include "iotconnect_telemetry.h"
#include "iotconnect_lib.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define MQTT_PUBLISH_TIME_BETWEEN_MS    ( 5000 )
#define NUMBER_OF_MSG                     200

#define APP_VERSION             "01.00.01"

#define WIFI_SSID               "your-wifi-ssid"
#define WIFI_PW                 "your-wifi-password"

#define SNTP_SERVER             "time.google.com"

#define MQTT_HOST               "poc-iotconnect-iothub-030-eu2.azure-devices.net"
#define MQTT_CLIENT_ID          ""
#define MQTT_USERNAME_FORMAT    "%s/%s/?api-version=2018-06-30"
#define MQTT_TOPIC_FORMAT       "devices/%s/messages/events/"


UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;


static char* response;
static char command_buffer [ 256 ] = {0};
static char payloadBuf[ 512 ]      = {0};
static IotclConfig config_esp32;
uint32_t nloops = 1 * NUMBER_OF_MSG;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */

static void appInit(void);
static void Process(void);

static void populate_iotcl_config(void) {
	config_esp32.device.env = "avnetpoc";
	config_esp32.device.cpid = "avtds";
	config_esp32.device.duid = MQTT_CLIENT_ID;
	config_esp32.device.duid = config_esp32.device.duid + 6;

    int ret = iotcl_init(&config_esp32);
    if (!ret) {
        printf("Failed to initialize the IoTConnect Lib\n");
    }
}

static int check_at_response(char* at_response) {
	int len = strlen(at_response);
	if (len > 255) {
		printf("AT response length is too long\r\n");
		return 0;
	}
	strcpy(command_buffer, at_response);

	if (strcmp(&command_buffer[len - 4], "OK\r\n") == 0) {
		if ((strncmp(command_buffer, "AT+MQTTCONN?", 12) == 0) || (strcmp(&command_buffer[26], "4") == 0)) {
			// printf("MQTT connection check is OK\r\n");
			return 2;
		}
		return 1;
	}
	printf("AT response is ERROR\r\n");
	return 0;
}

// publish telemetry data to iotc
static char* publish_telemetry() {
    IotclMessageHandle msg = iotcl_telemetry_create();

    // Optional. The first time you create a data point, the current timestamp will be automatically added
    // TelemetryAddWith* calls are only required if sending multiple data points in one packet.
    iotcl_telemetry_add_with_iso_time(msg, NULL);
    iotcl_telemetry_set_string(msg, "version", APP_VERSION);

    const char* str = iotcl_create_serialized_string(msg, false);
    iotcl_telemetry_destroy(msg);
    return (char* )str;

}

static void send_telemetry_data(void) {
	populate_iotcl_config();
	char* data = publish_telemetry();
	if (data == NULL) {
		printf("Publish telemetry is NULL...\n");
	  	return;
	}

	strcpy(payloadBuf, data);
	iotcl_destroy_serialized(data);

	int payload_len = strlen(payloadBuf);
	printf("Data is %s\r\nLength is %d\r\n", payloadBuf, payload_len);

	sprintf(command_buffer, MQTT_TOPIC_FORMAT, MQTT_CLIENT_ID);
	esp32_mqtt_pub_raw(command_buffer, payload_len);
	esp32_mqtt_send_raw_data(payloadBuf);
}

static int string_add_backslash(char* data, char* output_str) {
  bool add_backslash = false;
  int str_len = strlen(data);
  if (str_len > 256) {
	  return -1;
  }
  for (int i = 0; i < str_len + 1; i++) {  //strlen(data)+1 is for the last char \0
	  switch (*data) {
	    case '"':
		  add_backslash = true;
		  break;
	    case '\\':
		  add_backslash = true;
		  break;
	    case ',':
	      add_backslash = true;
	      break;
	    default:
	      add_backslash = false;
	      break;
	  }
	  if (add_backslash) {
		  *output_str = '\\';
		  output_str++;
	  }

	  *output_str = *data;
	  output_str++;
	  data++;
  }
  return 0;
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Enable PWR clock interface */

  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_PWR);

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the System Power */
  SystemPower_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_ICACHE_Init();
  
  appInit();

  while (1) {
	Process();
  }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_4)
  {
  }

  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_MSIS_Enable();

   /* Wait till MSIS is ready */
  while(LL_RCC_MSIS_IsReady() != 1)
  {
  }

  LL_RCC_MSI_EnableRangeSelection();
  LL_RCC_MSIS_SetRange(LL_RCC_MSISRANGE_0);
  LL_RCC_MSI_SetCalibTrimming(16, LL_RCC_MSI_OSCILLATOR_0);
  LL_RCC_PLL1_ConfigDomain_SYS(LL_RCC_PLL1SOURCE_MSIS, 3, 10, 1);
  LL_RCC_PLL1_EnableDomain_SYS();
  LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_4);
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
  LL_RCC_PLL1_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL1_IsReady() != 1)
  {
  }

   /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
  {
  }

  /* Insure 1us transition state at intermediate medium speed clock*/
  for (__IO uint32_t i = (160 >> 1); i !=0; i--);

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);
  LL_SetSystemCoreClock(160000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
   */
  LL_PWR_DisableUCPDDeadBattery();

  /*
   * Switch to SMPS regulator instead of LDO
   */
  LL_PWR_SetRegulatorSupply(LL_PWR_SMPS_SUPPLY);

  while(LL_PWR_IsActiveFlag_REGULATOR()!=1)
  {
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  LL_ICACHE_SetMode(LL_ICACHE_1WAY);
  LL_ICACHE_Enable();
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */
  
  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */
  
  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);

  /**/
  LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

  /**/
  GPIO_InitStruct.Pin = ExpressLink_EVENT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(ExpressLink_EVENT_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/**
* @brief ExpressLink module Configuration
* @retval None
*/
void appInit(void)
{
  printf("[INFO] Starting\r\n");
  HAL_Delay(2 * ESP32_BOOT_DELAY);
  
  /* Reset the ESP32 module is not required */
  // response = esp32_reset();

  response = esp32_mode();
  response = esp32_setwifi(WIFI_SSID, WIFI_PW);
  if (check_at_response(response) == 0) {   //wifi is not connected
	  printf("System reboot......\r\n");
	  NVIC_SystemReset();
  }

  response = esp32_sntp(SNTP_SERVER);
  HAL_Delay(ESP32_BOOT_DELAY);

  response = esp32_mqtt_clean();

  response = esp32_mqtt_cfg();
  sprintf(command_buffer, MQTT_USERNAME_FORMAT, MQTT_HOST, MQTT_CLIENT_ID);
  response = esp32_mqtt_username(command_buffer);
//  response = esp32_mqtt_password();
  response = esp32_mqtt_client_id(MQTT_CLIENT_ID);

  response = esp32_mqtt_connect(MQTT_HOST);
  if (check_at_response(response) == 0) {   //MQTT is not connected
	  printf("System reboot......\r\n");
	  NVIC_SystemReset();
  }
}

static void Process(void)
{
  response = esp32_mqtt_conn();
  if (check_at_response(response) != 2) {   //TLS mqtt connection is not connected
	  printf("System reboot......\r\n");
	  NVIC_SystemReset();
  }
  
  if (1)
  {
    send_telemetry_data();
	HAL_Delay(MQTT_PUBLISH_TIME_BETWEEN_MS);
    
    if (nloops-- == 0) {
      //ESP32_Disonnect();
      esp32_mqtt_clean();
      printf("The demo has ended\r\n");

      __disable_irq();

      while(1) {
          ;
      }
    }
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
