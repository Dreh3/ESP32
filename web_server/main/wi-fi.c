#include "wi-fi.h"
#include "string.h"

const char *TAGW = "WIFI";

static esp_netif_t *netif_instance = NULL;


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

    return ESP_OK;
};

esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password){
    //cria wifi_config_t struct com dados para as configurações e modifica apenas as credenciais do wi-fi
    
    //testar se precisa definir ou mudar algum parâmetro
    wifi_config_t conf = {
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
    }

    if(esp_wifi_start()!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao iniciar driver wi-fi");
    }else{
        ESP_LOGI(TAGW, "start wi-fi ok");
    };

    err_retorno=esp_wifi_connect(); 
    if(err_retorno==ESP_ERR_WIFI_SSID){
        ESP_LOGE(TAGW, "SSID inválido");
    }else if(err_retorno!=ESP_OK){
        ESP_LOGE(TAGW, "connect wi-fi nok");
    }else{
        ESP_LOGI(TAGW, "connect wi-fi ok");
    };

    return err_retorno;
};


esp_err_t wifi_disconnect(void){
    // if(esp_wifi_disconnect()!=ESP_OK){
    //     ESP_LOGE(TAGW, "Falha ao desconectar wifi");
    // }else{
    //     ESP_LOGI(TAGW, "desconnect wi-fi ok");
    // };

    return esp_wifi_disconnect();

};

esp_err_t wifi_deinit(){
    if(esp_wifi_stop()!=ESP_OK){
        ESP_LOGE(TAGW,"wifi-fi nem foi inicializado");
        return ESP_OK;
    };

    esp_wifi_deinit();
    if(esp_wifi_clear_default_wifi_driver_and_handlers(netif_instance)!=ESP_OK){
        ESP_LOGE(TAGW, "Falha ao limpar drivers");
        return ESP_FAIL;
    };

    return ESP_OK;

};



