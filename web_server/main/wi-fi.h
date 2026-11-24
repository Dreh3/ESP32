#pragma once

#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "esp_netif.h"

#include "freertos/FreeRTOS.h"

static const esp_err_t ERR_PASSWORD_EXCEED = 9999;

esp_err_t wifi_init(void);
esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_deinit(void);


