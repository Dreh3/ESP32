#include <stdio.h>
#include "esp_log.h"
#include "aht20.h"

#define I2C_SCL_AHT20 18
#define I2C_SDA_AHT20 19
#define I2C_MNUM_AHT20 I2C_NUM_1
#define I2C_MFREQ 100000 // 100kHz
// #define ATH20_ADDR 0x38

const char *TAG = "aht20";




void app_main(void)
{

    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_AHT20,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_AHT20,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MFREQ
    };

    i2c_bus_handle_t i2c_bus = i2c_bus_create(I2C_MNUM_AHT20, &i2c_config);

    aht20_i2c_config_t i2c_aht20 = {
        .bus_inst = i2c_bus,
        .i2c_addr = 0x39,
    };
    
    aht20_dev_handle_t aht20_handler;

    aht20_new_sensor(&i2c_aht20, &aht20_handler);

    uint32_t raw_temperatura, raw_umidade;
    float temperatura, umidade;

    while(true){
        aht20_read_temperature_humidity(aht20_handler,&raw_temperatura, &temperatura, &raw_umidade, &umidade);

        ESP_LOGI(TAG, "%-20s: %2.2f %%", "umidade e", umidade);
        ESP_LOGI(TAG, "%-20s: %2.2f degC", "temperatura e", temperatura);
        esp_rom_delay_us(8000);

    };

    aht20_del_sensor(aht20_handler);
    i2c_bus_delete(&i2c_bus);
    
};
