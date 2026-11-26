#pragma once

#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

static const esp_err_t ERR_PASSWORD_EXCEED = 9999;
static const int TENTATIVA_MAX = 10;
extern EventGroupHandle_t c_wifi_event_group;


esp_err_t wifi_init(void);
esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_shutdown(void);

// temporário
// esp_err_t wifi_wait_connection(void);
// esp_err_t wifi_scan(const char *target_ssid);


