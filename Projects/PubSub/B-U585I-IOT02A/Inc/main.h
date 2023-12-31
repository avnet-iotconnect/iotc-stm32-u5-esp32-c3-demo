/**
  ******************************************************************************
  * @file           : main.h
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "stm32u5xx_ll_icache.h"
#include "stm32u5xx_ll_pwr.h"
#include "stm32u5xx_ll_crs.h"
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_bus.h"
#include "stm32u5xx_ll_system.h"
#include "stm32u5xx_ll_exti.h"
#include "stm32u5xx_ll_cortex.h"
#include "stm32u5xx_ll_utils.h"
#include "stm32u5xx_ll_dma.h"
#include "stm32u5xx_ll_gpio.h"
#include "stm32u5xx_ll_lpgpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ExpressLink_EVENT_Pin LL_GPIO_PIN_15
#define ExpressLink_EVENT_GPIO_Port GPIOD
#define LD2_Pin LL_GPIO_PIN_13
#define LD2_GPIO_Port GPIOE

#define ESP32_USART_HANDLER         huart3

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
