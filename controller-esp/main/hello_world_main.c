#include <stdio.h>
#include <math.h> // Necessário para fminf e fmaxf
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Includes dos componentes periféricos
#include "wifi.h"
#include "pid.h"
#include "motors.h"
#include "servo.h"

// === DEFINIÇÃO DAS VARIÁVEIS GLOBAIS ===
// (Aqui é onde a memória é alocada de verdade)
SemaphoreHandle_t newDataSemaphore = NULL;
SemaphoreHandle_t posMutex = NULL;


float angulo_atual = 90;

float posX = 0.0f;
float posY = 0.0f;
float posZ = 0.0f;

// --- control_task (versão com logs e clamps) ---
void control_task(void *pvParameters)
{
    motors_init();
    servo_init();

    // Teste rápido no init (provar hardware) - com delays curtos
    ESP_LOGI("CONTROL", "Teste inicial: servo 90, motores 0");
    servo_set_angle(90.0f);
    motors_set(0, 0);
    vTaskDelay(pdMS_TO_TICKS(500));

    PID_t pid_x, pid_z;
    PID_Init(&pid_x, 1, 0.01, 0.10, 0.10);
    PID_Init(&pid_z, 0.60, 0.00, 0.20, 0.10);

    //PID_SetOutputLimits(&pid_x, -255, 255);
    PID_SetOutputLimits(&pid_x, -255, 255); 
    PID_SetOutputLimits(&pid_z, -255, 255);

    const float Z_target = 1.00f;

    while (1)
    {
        // aguarda dado novo do Wi-Fi (bloqueante). Se quiser ver logs periódicos,
        // substitua portMAX_DELAY por um timeout e faça log quando timeout.
        if (xSemaphoreTake(newDataSemaphore, portMAX_DELAY) == pdTRUE) {
            float x, y, z;

            // leitura segura das variáveis globais
            if (xSemaphoreTake(posMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                x = posX;
                y = posY;
                z = posZ;
                xSemaphoreGive(posMutex);
            } else {
                ESP_LOGW("CONTROL", "Falha ao tomar posMutex");
                continue;
            }

            // logs para debug — comente quando não precisar
            ESP_LOGI("CONTROL", "Pos recebida: x=%.3f y=%.3f z=%.3f", x, y, z);

            float err_x = x;
            float err_z = Z_target - z;

            //float out_x = PID_Update(&pid_x, err_x);
            float out_x = err_x; // só proporcional para teste
            float out_z = PID_Update(&pid_z, err_z);

            //float motor_esq = out_z - out_x;
            //float motor_dir = out_z + out_x;
            
            float motor_esq = -out_x;
            float motor_dir = out_x;
            
            // saturação
            motor_esq = fminf(fmaxf(motor_esq, -255.0f), 255.0f);
            motor_dir = fminf(fmaxf(motor_dir, -255.0f), 255.0f);

            // log antes de enviar aos motores
            ESP_LOGI("CONTROL", "Motors raw: L=%.2f R=%.2f", motor_esq, motor_dir);
            motors_set((int)motor_esq, (int)motor_dir);

            // servo: calcule ângulo e garanta limites adicionais
            float angulo = angulo_atual + (y * 0.03);
            if (angulo < 0.0f) angulo = 0.0f;
            if (angulo > 180.0f) angulo = 180.0f;

            angulo_atual = angulo;
            
            ESP_LOGI("CONTROL", "Servo angulo pedido: %.2f", angulo);
            servo_set_angle(angulo);
        } else {
            // raro se usar portMAX_DELAY, mas deixa para robustez
            ESP_LOGW("CONTROL", "xSemaphoreTake(newDataSemaphore) falhou/timeout");
        }
    }
}

// --- app_main (com checagens de criação e logs) ---
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    newDataSemaphore = xSemaphoreCreateBinary();
    if (newDataSemaphore == NULL) {
        ESP_LOGE("MAIN", "Falha ao criar newDataSemaphore");
        esp_restart(); // ou trate de outra forma
    }

    posMutex = xSemaphoreCreateMutex();
    if (posMutex == NULL) {
        ESP_LOGE("MAIN", "Falha ao criar posMutex");
        esp_restart();
    }

    ESP_LOGI("MAIN", "Inicializando Wi-Fi...");
    wifi_init_sta();

    BaseType_t ok;
    ok = xTaskCreate(tcp_server_task, "tcp_server", 8192, NULL, 5, NULL); // aumentei stack p/ tcp
    if (ok != pdPASS) {
        ESP_LOGE("MAIN", "Falha ao criar tcp_server_task");
    }

    ok = xTaskCreate(control_task, "control", 8192, NULL, 6, NULL); // aumentei stack p/ segurança
    if (ok != pdPASS) {
        ESP_LOGE("MAIN", "Falha ao criar control_task");
    }

    // loop principal de supervisão (opcional)
    while (1) {
        //ESP_LOGI("MAIN", "Sistema rodando...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
