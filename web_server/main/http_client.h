#include "esp_http_client.h"

extern char resposta[256];
extern esp_http_client_config_t config;
extern esp_http_client_handle_t cliente;

esp_err_t http_get_with_url(char *url);
esp_err_t http_post_with_url(char *url, char *dados);


