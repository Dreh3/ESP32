#include <stdio.h>
#include "esp_log.h"
// #include "aht20.h"
// #include "wi-fi.h"

#include "wi-fi.h"
#include "http_client.h"
// #define I2C_SCL_AHT20 18
// #define I2C_SDA_AHT20 19
// #define I2C_MNUM_AHT20 I2C_NUM_1
// #define I2C_MFREQ 100000 // 100kHz
// #define ATH20_ADDR 0x38

const char *TAG = "MAIN";

//configurações wi-fi
//definir credenciais, estabelecer conexão (tratar perda de conexão na task depois)
//obter ip da placa
//parte que depende das demais: rota para o post e porta para tentar conexão com servidor


void app_main(void)
{

    // const i2c_config_t i2c_config = {
    //     .mode = I2C_MODE_MASTER,
    //     .sda_io_num = I2C_SDA_AHT20,
    //     .sda_pullup_en = GPIO_PULLUP_ENABLE,
    //     .scl_io_num = I2C_SCL_AHT20,
    //     .scl_pullup_en = GPIO_PULLUP_ENABLE,
    //     .master.clk_speed = I2C_MFREQ
    // };

    // i2c_bus_handle_t i2c_bus = i2c_bus_create(I2C_MNUM_AHT20, &i2c_config);

    // aht20_i2c_config_t i2c_aht20 = {
    //     .bus_inst = i2c_bus,
    //     .i2c_addr = 0x39,
    // };
    
    // aht20_dev_handle_t aht20_handler;

    // aht20_new_sensor(&i2c_aht20, &aht20_handler);

    // uint32_t raw_temperatura, raw_umidade;
    // float temperatura, umidade;

    // while(true){
    //     aht20_read_temperature_humidity(aht20_handler,&raw_temperatura, &temperatura, &raw_umidade, &umidade);

    //     ESP_LOGI(TAG, "%-20s: %2.2f %%", "umidade e", umidade);
    //     ESP_LOGI(TAG, "%-20s: %2.2f degC", "temperatura e", temperatura);
    //     esp_rom_delay_us(8000);

    // };

    // aht20_del_sensor(aht20_handler);
    // i2c_bus_delete(&i2c_bus);
    // esp_rom_delay_us(80);
    ESP_LOGI(TAG, "rodando a main");
    // esp_log_level_set("wifi", ESP_LOG_NONE);

    // esp_rom_delay_us(80);

    if(wifi_init()==ESP_OK){
        ESP_LOGI(TAG, "wi-fi iniciado");
    }

    if (wifi_connect("CLARO_JANISSON-2G", "Cordeiro33")==ESP_OK){
        ESP_LOGI(TAG, "solicitação de conexão realizada");
    }

    // // Aguarda associação ao AP temporário
    // if (wifi_wait_connection() != ESP_OK) {
    //     ESP_LOGE(TAG, "Falha na conexão Wi-Fi");
    //     return;
    // }

    vTaskDelay(pdMS_TO_TICKS(1000));

    while(true){
        
        // esp_rom_delay_us(800);

        // wifi_ap_record_t ap_info;
        // esp_err_t retorno = esp_wifi_sta_get_ap_info(&ap_info); //verifica saidas e dados da rede
        // // //colocar os ifs
        // // ESP_LOGI(TAG, "SSID: %s", ap_info.ssid);

        // if(retorno == ESP_OK){
        //     ESP_LOGI(TAG, "Conectado!");
        //     ESP_LOGI(TAG, "SSID: %s", ap_info.ssid);

            //tentando http_post

            // EventBits_t bits = xEventGroupWaitBits(
            //     c_wifi_event_group,
            //     WIFI_CONNECTED_BIT,   //testar o bit de conexão antes de fazer request
            //     pdFALSE,
            //     pdFALSE,
            //     portMAX_DELAY
            // );

            // if (bits & WIFI_CONNECTED_BIT) {
            //     http_get();
            // }
            cliente = esp_http_client_init(&config);
            http_get_with_url("http://httpbin.org/get");
            esp_rom_delay_us(800);
            esp_http_client_cleanup(cliente);

            vTaskDelay(pdMS_TO_TICKS(10000));
        // }else{
        //     ESP_LOGE(TAG, "Estação desconectada ou inacessível");
        // };
        
        // esp_rom_delay_us(100000);
    
    };

    if(wifi_disconnect()==ESP_OK){
        ESP_LOGI(TAG, "wi-fi desconectado");
    };
    
};
