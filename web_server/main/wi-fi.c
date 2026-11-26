#include "wi-fi.h"
#include "string.h"
#include "esp_event.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// temporário

// #include "freertos/task.h"
// #include <stdbool.h>
//  <string.h>

const char *TAGW = "WIFI";

static esp_netif_t *netif_instance = NULL;
static wifi_config_t conf;


//tratamento de eventos
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static esp_event_handler_instance_t c_ip_event_handler;
static esp_event_handler_instance_t c_wifi_event_handler;
EventGroupHandle_t c_wifi_event_group = NULL;
static const char SSID_SALVAS[3][32] = {CONFIG_WIFI_SSID1, CONFIG_WIFI_SSID2, CONFIG_WIFI_SSID3}; //colocar no kconfig depois
static const char SENHAS_SALVAS[3][64] = {CONFIG_WIFI_PASSWORD1, CONFIG_WIFI_PASSWORD2, CONFIG_WIFI_PASSWORD3};
static int8_t tentativas_con = 0;
static int8_t ssid_testadas = 0;

static void c_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect(); //tenta primeira conexão
    }else if(event_base==WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        if(tentativas_con< TENTATIVA_MAX){
            //tenta conectar novamente com as mesmas credenciais
            esp_wifi_connect();
            tentativas_con ++;
            ESP_LOGI(TAGW, "Nova tentativas de conexão ...");
        }else if (ssid_testadas+1<3){
            tentativas_con = 0;
            strncpy((char*)conf.sta.ssid, SSID_SALVAS[ssid_testadas], sizeof(conf.sta.ssid));
            strncpy((char*)conf.sta.password, SENHAS_SALVAS[ssid_testadas], sizeof(conf.sta.password));
             esp_wifi_set_config(WIFI_IF_STA, &conf);
            ssid_testadas++;
        }else{
            xEventGroupSetBits(c_wifi_event_group, WIFI_FAIL_BIT);  //para testar na hora do post se tem conexão ou não
        };
    }else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAGW, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        tentativas_con = 0;
        xEventGroupSetBits(c_wifi_event_group, WIFI_CONNECTED_BIT);
        ssid_testadas = 0;
    };

    //falta registrar os eventos e o bits também e testar
};

// esp_err_t wifi_scan(const char *target_ssid)
// {
//     wifi_ap_record_t ap_records[20];
//     uint16_t ap_count = 20;

//     // Inicia a varredura e aguarda sua conclusão
//     esp_err_t err = esp_wifi_scan_start(NULL, true);

//     if (err != ESP_OK) {
//         ESP_LOGE(TAGW, "Falha no scan: %s", esp_err_to_name(err));
//         return err;
//     };

//     // Recupera os resultados
//     err = esp_wifi_scan_get_ap_records(&ap_count, ap_records);

//     if (err != ESP_OK) {
//         ESP_LOGE(TAGW, "Falha ao obter APs: %s", esp_err_to_name(err));
//         return err;
//     };

//     ESP_LOGI(TAGW, "Redes encontradas: %u", ap_count);

//     bool found = false;

//     for (int i = 0; i < ap_count; i++) {

//         ESP_LOGI(TAGW, "SSID: %s | RSSI: %d dBm",
//                  (char *)ap_records[i].ssid,
//                  ap_records[i].rssi);

//         if (strcmp((char *)ap_records[i].ssid, target_ssid) == 0) {
//             found = true;
//             ESP_LOGI(TAGW, "Rede desejada encontrada!");
//         };
//     };

//     if (!found) {
//         ESP_LOGE(TAGW, "SSID desejado não encontrado!");
//         return ESP_ERR_WIFI_SSID;
//     };

//     return ESP_OK;
// };




esp_err_t wifi_init(void){

    //inicializando armazenamento do NVS - não preciso ao usar constantes, ATENÇÃO: pode ser utilizada em outras partes do driver wi-fi
    //wifi config default configura nvs como habilitado - testar mudar depois
    if(nvs_flash_init() == ESP_OK)
        ESP_LOGI(TAGW, "NVS inicializado com sucesso!");

    if(esp_netif_init() != ESP_OK){
        ESP_LOGE(TAGW, "Falha ao inicializar netif!");
    }else{
        ESP_LOGI(TAGW, "netif ok");
    }

    //criando event loop - para salvar eventos - pode deletar ao finalizar para evitar erro de já ter sido criado
    esp_err_t eventloop = esp_event_loop_create_default();
    if(eventloop == ESP_ERR_INVALID_STATE){ //event loop já criado (COMO reseolver?)
        ESP_LOGE(TAGW, "event loop já foi criado");
    }else if(eventloop != ESP_OK){
        ESP_LOGE(TAGW, "falha ao criar event loop");
    }else{
        ESP_LOGI(TAGW, "sucesso ao criar event loop");
    };

    //sempre criar após o event loop - cria objeto netif com configurações padrões
    netif_instance = esp_netif_create_default_wifi_sta();

    //configurações do driver wi-fi
    //inicializa - seta configuração padrão - determino o modo - inicio o driver
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&config)!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao inicializar wi-fi");
    }else{
        ESP_LOGI(TAGW, "init wi-fi ok");
    };

    esp_err_t err_retorno =esp_wifi_set_mode(WIFI_MODE_STA);
    //defino como station para essa aplicação 
    if(err_retorno!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao configurar modo");
    }else{
        ESP_LOGI(TAGW, "set mode ok");
    };

    c_wifi_event_group = xEventGroupCreate();

    if(c_wifi_event_group==NULL){
        ESP_LOGE(TAGW, "Falha ao criar event group");
        return ESP_FAIL;
    };

    //registro o handler de eventos 
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                  ESP_EVENT_ANY_ID,
                                                  &c_event_handler,
                                                  NULL,
                                                  &c_wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                  IP_EVENT_STA_GOT_IP,
                                                  &c_event_handler,
                                                  NULL,
                                                  &c_ip_event_handler));   

    return ESP_OK;
};

esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password){
    //cria wifi_config_t struct com dados para as configurações e modifica apenas as credenciais do wi-fi
    
    //testar se precisa definir ou mudar algum parâmetro
    conf = (wifi_config_t){
        .sta.threshold.authmode = WIFI_AUTH_WPA_PSK, //okk - pode ser declarado como constante
    };

    printf("tamanho do ssid %zu", strlen(wifi_ssid));
    if(((strlen(wifi_ssid)<=32) & (strlen(wifi_ssid)>0)) & ((strlen(wifi_password)<=64) & (strlen(wifi_password)>0) )){
        strncpy((char*)conf.sta.ssid, wifi_ssid, sizeof(conf.sta.ssid));
        strncpy((char*)conf.sta.password, wifi_password, sizeof(conf.sta.password));
    }else{
        ESP_LOGE(TAGW, "Credenciais excedem o tamanho!!");
        return ERR_PASSWORD_EXCEED;
    };

    //
    esp_err_t err_retorno = esp_wifi_set_config(WIFI_IF_STA, &conf);
    if(err_retorno==ESP_ERR_WIFI_PASSWORD){
        ESP_LOGE(TAGW, "Senha informada é inválida");
        return ESP_ERR_WIFI_PASSWORD;
    }else if(err_retorno == ESP_OK){
        ESP_LOGE(TAGW, "configurações ok");
    }else if (err_retorno == ESP_ERR_WIFI_STATE){
        ESP_LOGE(TAGW, "módulo ainda não preparado para configurações");
    }else{
        ESP_LOGE(TAGW, "Falha ao configurar wi-fi");
    };

    if(esp_wifi_start()!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao iniciar driver wi-fi");
    }else{
        ESP_LOGI(TAGW, "start wi-fi ok");
    };

    //temporário
    // wifi_scan("AndroidAP");

    // err_retorno=esp_wifi_connect(); 
    // if(err_retorno==ESP_ERR_WIFI_SSID){
    //     ESP_LOGE(TAGW, "SSID inválido");
    // }else if(err_retorno!=ESP_OK){
    //     ESP_LOGE(TAGW, "connect wi-fi nok");
    // }else{
    //     ESP_LOGI(TAGW, "connect wi-fi ok");
    // };

    xEventGroupClearBits(c_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    EventBits_t bits = xEventGroupWaitBits(c_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAGW, "Connected to Wi-Fi network: %s", conf.sta.ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAGW, "Failed to connect to Wi-Fi network: %s", conf.sta.ssid);
        return ESP_FAIL;
    };

    return err_retorno;
};


esp_err_t wifi_disconnect(void){
    // if(esp_wifi_disconnect()!=ESP_OK){
    //     ESP_LOGE(TAGW, "Falha ao desconectar wifi");
    // }else{
    //     ESP_LOGI(TAGW, "desconnect wi-fi ok");
    // };

    if (c_wifi_event_group) {
        vEventGroupDelete(c_wifi_event_group);
    };

    return esp_wifi_disconnect();

};

esp_err_t wifi_shutdown(void){
    if(esp_wifi_stop()!=ESP_OK){
        ESP_LOGE(TAGW,"wifi-fi nem foi inicializado");
        return ESP_OK;
    };

    esp_wifi_deinit();
    if(esp_wifi_clear_default_wifi_driver_and_handlers(netif_instance)!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao limpar drivers");
        return ESP_FAIL;
    };
    esp_netif_destroy(netif_instance);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, c_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, c_event_handler));


    return ESP_OK;

};

// temporário
// esp_err_t wifi_wait_connection(void)
// {
//     wifi_ap_record_t ap_info;

//     for (int i = 0; i < 20; i++) {

//         esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);

//         if (err == ESP_OK) {
//             ESP_LOGI(TAGW, "Conectado ao AP: %s", ap_info.ssid);
//             ESP_LOGI(TAGW, "RSSI: %d", ap_info.rssi);

//             return ESP_OK;
//         };

//         ESP_LOGI(TAGW, "Aguardando conexão... tentativa %d", i + 1);

//         vTaskDelay(pdMS_TO_TICKS(500));
//     };

//     ESP_LOGE(TAGW, "Não foi possível conectar ao AP");

//     return ESP_ERR_WIFI_NOT_CONNECT;
// };

