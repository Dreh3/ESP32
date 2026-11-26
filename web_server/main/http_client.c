#include "http_client.h"
#include "esp_log.h"
#include "sdkconfig.h"

char resposta[256] = {0};

esp_http_client_config_t config = {
    .query = "esp",
    .url = CONFIG_HTTP_SERVER_URL,
    .user_data = resposta,
    .disable_auto_redirect = true,
};

esp_http_client_handle_t cliente;

const char *TAGH = "HTTP-CLIENT";


esp_err_t http_get_with_url(char *url){
    
    // inciar na task
    // cliente = esp_http_client_init(&config);

    esp_http_client_set_url(cliente, url);

    esp_http_client_set_method(cliente, HTTP_METHOD_GET);

    esp_err_t err = esp_http_client_perform(cliente);

    if(err == ESP_OK){
        ESP_LOGI(TAGH, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(cliente),
                esp_http_client_get_content_length(cliente));
    }else{
        ESP_LOGE(TAGH, "Falha ao enviar get: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    ESP_LOG_BUFFER_HEX(TAGH, resposta, strlen(resposta));

    return ESP_OK;

};

esp_err_t http_post_with_url(char *url, char *dados){
    // const char *post_data = "{\"field1\":\"value1\"}";
    esp_http_client_set_url(cliente, url);
    esp_http_client_set_method(cliente, HTTP_METHOD_POST);
    esp_http_client_set_header(cliente, "Content-Type", "aplication/json");
    esp_http_client_set_post_field(cliente, dados, strlen(dados));
    esp_err_t err = esp_http_client_perform(cliente);

    if(err == ESP_OK){
        ESP_LOGI(TAGH, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(cliente),
                esp_http_client_get_content_length(cliente));
    }else{
        ESP_LOGE(TAGH, "Falha ao enviar post: %s", esp_err_to_name(err));
        return ESP_FAIL;
    };

    ESP_LOG_BUFFER_HEX(TAGH, resposta, strlen(resposta));

    // esp_http_client_set_post_field(cliente, NULL, 0);

    return ESP_OK;
};