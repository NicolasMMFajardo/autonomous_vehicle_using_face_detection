#include "wifi.h"
#include "lwip/sockets.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" 
#include "freertos/semphr.h"   
#include <string.h>

static const char *TAGWIFI = "WiFi_TCP";

// === DECLARAÇÕES EXTERN ===
extern float posX;
extern float posY;
extern float posZ;

extern SemaphoreHandle_t newDataSemaphore;
extern SemaphoreHandle_t posMutex;

void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAGWIFI, "Conectando ao Wi-Fi...");
    esp_wifi_connect();
    
    // O ideal seria usar Event Groups para esperar a conexão, 
    // mas o delay fixo funciona para testes simples.
    vTaskDelay(pdMS_TO_TICKS(4000)); 
}

void tcp_server_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAGWIFI, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAGWIFI, "Servidor TCP escutando na porta %d", PORT);

    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);

        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAGWIFI, "Erro no accept: errno %d", errno);
            continue;
        }

        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr,
                    addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAGWIFI, "Cliente conectado: %s", addr_str);

        send(sock, "Conexao estabelecida!\n", 23, 0);

        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) break;

            rx_buffer[len] = 0;
            // ESP_LOGI(TAGWIFI, "Recebido: %s", rx_buffer); // Opcional: comentar para limpar o log

            float x, y, z;
            
            // Tenta ler com ou sem espaços
            if (sscanf(rx_buffer, "posX:%f, posY:%f, posZ:%f", &x, &y, &z) == 3 ||
                sscanf(rx_buffer, "posX:%f , posY:%f , posZ:%f", &x, &y, &z) == 3) {

                // Protege com mutex
                if(posMutex != NULL) {
                    xSemaphoreTake(posMutex, portMAX_DELAY);
                    posX = x;
                    posY = y;
                    posZ = z;
                    xSemaphoreGive(posMutex);
                }

                // Notifica a task de controle
                if(newDataSemaphore != NULL) {
                    xSemaphoreGive(newDataSemaphore);
                }

                ESP_LOGI(TAGWIFI, "POS -> X=%.2f Y=%.2f Z=%.2f", posX, posY, posZ);
                send(sock, "OK\n", 3, 0);
            }
            else {
                send(sock, "Formato invalido\n", 17, 0);
            }
        }

        ESP_LOGW(TAGWIFI, "Cliente desconectado");
        close(sock);
    }
    vTaskDelete(NULL);
}