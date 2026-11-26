idf_component_register(
    SRCS "http_client.c" "wi-fi.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event esp_netif nvs_flash log esp_rom freertos esp_http_client
)