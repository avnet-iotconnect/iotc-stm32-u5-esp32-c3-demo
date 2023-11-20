/**
******************************************************************************
* @file           : esp32.h
* @version        : v 1.0.0
* @brief          : This file implements esp32 mini driver header
*/
#include "main.h"

#ifndef ESP32_H_
#define ESP32_H_

#define ESP32_BOOT_DELAY       3000
#define ESP32_COMMAND_TIMEOUT  3000


char*   esp32_reset               (void);
char*   esp32_mode                (void);
char*   esp32_setwifi             (char*, char*);
char*   esp32_sntp                (char* sntp_server);
char*   esp32_mqtt_cfg            (void);
char*   esp32_mqtt_username       (char* mqtt_username);
char*   esp32_mqtt_password       (void);
char*   esp32_mqtt_client_id      (char* mqtt_client_id);
char*   esp32_mqtt_connect        (char* mqtt_host);
char*   esp32_at                  (void);
char*   esp32_mqtt_clean          (void);
char*   esp32_mqtt_conn           (void);
char*   esp32_mqtt_pub            (char* topic, char* data);
char*   esp32_mqtt_pub_raw        (char* topic, uint8_t length);
char*   esp32_mqtt_send_raw_data  (char* data);
#endif /* ESP32_H_ */
