#include "servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAGSERVO = "SERVO";

// === ALTERE PARA O PINO DO SEU SERVO ===
#define SERVO_PIN       13

#define SERVO_MIN_US    500     // pulso mínimo (0º)
#define SERVO_MAX_US    2500    // pulso máximo (180º)

#define SERVO_FREQ      50      // 50 Hz
#define SERVO_TIMER     LEDC_TIMER_1
#define SERVO_CHANNEL   LEDC_CHANNEL_4

// Para LEDC 13 bits (0–8191)
#define SERVO_RESOLUTION LEDC_TIMER_13_BIT
#define SERVO_MAX_DUTY   ((1 << 13) - 1)

// Converte microssegundos → LEDC duty
static uint32_t us_to_duty(uint32_t us)
{
    uint32_t period_us = 1000000UL / SERVO_FREQ;   // 20ms
    return (us * SERVO_MAX_DUTY) / period_us;
}

void servo_init(void)
{
    // Timer
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num  = SERVO_TIMER,
        .freq_hz    = SERVO_FREQ,
        .duty_resolution = SERVO_RESOLUTION,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    // Canal
    ledc_channel_config_t ch_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = SERVO_CHANNEL,
        .gpio_num   = SERVO_PIN,
        .timer_sel  = SERVO_TIMER,
        .duty       = 0,
        .hpoint     = 0
        
    };
    ledc_channel_config(&ch_cfg);

    ESP_LOGI(TAGSERVO, "Servo inicializado.");
}

void servo_set_angle(float ang)
{
    //ESP_LOGI(TAGSERVO, "Angulo do servo %.2f.", ang);
    
    if (ang < 0) ang = 0;
    if (ang > 180) ang = 180;

    uint32_t pulse = SERVO_MIN_US +
                     (ang / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);

    uint32_t duty = us_to_duty(pulse);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL);
}
