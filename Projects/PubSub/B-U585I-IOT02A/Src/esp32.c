/**
 ******************************************************************************
 * @file           : esp32.c
 * @version        : v 1.0.0
 * @brief          : This file implements IoTConnect esp32 driver
 ******************************************************************************
 */
#include "main.h"
#include "esp32.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ESP32_TX_BUFFER_SIZE  256
#define ESP32_RX_BUFFER_SIZE  256

/*---------- ESP32_USART_HANDLER  -----------*/
extern UART_HandleTypeDef  ESP32_USART_HANDLER;

/*---------- ESP32_DEBUG  -----------*/
#define ESP32_DEBUG      1



static char command[ESP32_TX_BUFFER_SIZE] = { 0 };
static char response[ESP32_RX_BUFFER_SIZE] = { 0 };


/******************************************************************************/
/**
 * @brief Send a command to the module
 * @retval The ExpressLink Module response
 */
static char* esp32_execute_command(char *command, unsigned long timeout_ms)
{
  int i = 0;
  int command_flag = 0;
  int command_size = strlen(command);
  if (command_size > 255) {
	  printf("AT COMMAND is over size!\r\n");
	  return response;
  }

  if ((strcmp(command, "AT+RST\r\n") == 0)) {
	  command_flag = 1;  // RST command
  }
  else if ((strncmp(command, "AT+CIPSNTPCFG", 13) == 0)) {
	  command_flag = 2;  // SNTP command
  }
  else if ((strncmp(command, "AT+MQTTPUBRAW", 13) == 0)) {
	  command_flag = 3;
  }

  HAL_StatusTypeDef USART_STATUS = HAL_OK;

  //flush UART
  while (HAL_TIMEOUT != HAL_UART_Receive(&ESP32_USART_HANDLER, (uint8_t* )&response[0], ESP32_TX_BUFFER_SIZE, 50)) {
	  //do nothing
  }

  memset(response, 0, ESP32_RX_BUFFER_SIZE);

#if 0
  printf("%s", command);
#endif

  HAL_UART_Transmit(&ESP32_USART_HANDLER, (uint8_t* )command, command_size, timeout_ms);

if (!command_flag) { // command flag is 0
	  do
	  {
	    USART_STATUS = HAL_UART_Receive(&ESP32_USART_HANDLER, (uint8_t* )&response[i], 1, timeout_ms);
	    i++;

	    if (i > 3) {
	      if ((response[i - 1] == '\n') && (response[i - 2] == '\r') && (response[i - 3] == 'K') && (response[i - 4] == 'O')) {
	    	  break;  // \r\nOK\r\n
	      }
	      else if ((response[i - 1] == '\n') && (response[i - 2] == '\r') && (response[i - 3] == 'R') && (response[i - 4] == 'O')) {
	    	  break;  // \r\nERROR\r\n
	      }
	    }
	  } while ((USART_STATUS != HAL_TIMEOUT));
}
else {  // command_flag is 1 or 2 or 3
  do
  {
    USART_STATUS = HAL_UART_Receive(&ESP32_USART_HANDLER, (uint8_t* )&response[i], 1, timeout_ms);
    i++;

    if (i > 3) {
      if ((response[i - 1] == '\n') && (response[i - 2] == '\r') && (response[i - 3] == 'P') && (response[i - 4] == 'I')) {
      	if (command_flag == 1) {
      	  // printf("i is %d and  GOT IP\r\n", i);
          break;
      	}
      }
      else if ((response[i - 1] == '\n') && (response[i - 2] == '\r') && (response[i - 3] == 'D') && (response[i - 4] == 'E')) {
      	if (command_flag == 2) {
    	  // printf("i is %d and UPDATED\r\n", i);
      	  break;
      	}
      }
      else if ((response[i - 1] == '>') && (response[i - 2] == '\n') && (response[i - 3] == '\r')) {
      	if (command_flag == 3) {
    	  // printf("i is %d and >>>>>\r\n", i);
      	  break;
      	}
      }
    }
  } while ((USART_STATUS != HAL_TIMEOUT));
}
  if(USART_STATUS == HAL_TIMEOUT)
  {
    memset  (response, 0, ESP32_RX_BUFFER_SIZE);
    snprintf(response, ESP32_TX_BUFFER_SIZE, "ERROR\r\n");
  }

#if (ESP32_DEBUG)
  printf("%s", response);
#endif
  return response;
}


//----------ESP32--------------
char* esp32_at(void) {
  return esp32_execute_command("AT\r\n", ESP32_COMMAND_TIMEOUT);
}
char* esp32_reset(void) {
  return esp32_execute_command("AT+RST\r\n", 20000);
}

char* esp32_mode(void) {
  return esp32_execute_command("AT+CWMODE=1\r\n", ESP32_COMMAND_TIMEOUT);
}

char* esp32_setwifi(char* ssid, char* pw) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pw);
  return esp32_execute_command(command, 5 * ESP32_COMMAND_TIMEOUT);
}

char* esp32_sntp(char* sntp_server) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+CIPSNTPCFG=1,-6,\"%s\"\r\n", sntp_server);
  esp32_execute_command(command, 20000);
  return esp32_execute_command("AT+CIPSNTPTIME?\r\n", ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_cfg(void) {
  return esp32_execute_command("AT+MQTTUSERCFG=0,5,\"\",\"\",\"\",0,0,\"\"\r\n", ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_username(char* mqtt_username) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+MQTTUSERNAME=0,\"%s\"\r\n", mqtt_username);
  return esp32_execute_command(command, ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_password(void) {
  return esp32_execute_command("AT+MQTTPASSWORD=0,\"\"\r\n", ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_client_id(char* mqtt_client_id) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+MQTTCLIENTID=0,\"%s\"\r\n", mqtt_client_id);
  return esp32_execute_command(command, ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_connect(char* mqtt_host) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+MQTTCONN=0,\"%s\",8883,1\r\n", mqtt_host);
  return esp32_execute_command(command, 10 * ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_clean(void) {
	return esp32_execute_command("AT+MQTTCLEAN=0\r\n", 3 * ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_conn(void) {
	return esp32_execute_command("AT+MQTTCONN?\r\n", 3 * ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_pub(char* topic, char* data) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+MQTTPUB=0,\"%s\",\"%s\",1,0\r\n", topic, data);
  return esp32_execute_command(command, 5 * ESP32_COMMAND_TIMEOUT);
}

char* esp32_mqtt_pub_raw(char* topic, uint8_t length) {
  snprintf(command, ESP32_TX_BUFFER_SIZE, "AT+MQTTPUBRAW=0,\"%s\",%d,1,0\r\n", topic, length);
  return esp32_execute_command(command, 2 * ESP32_COMMAND_TIMEOUT);
}
char* esp32_mqtt_send_raw_data(char* data) {
  strcpy(command, data);
  return esp32_execute_command(command, 4 * ESP32_COMMAND_TIMEOUT);
}
