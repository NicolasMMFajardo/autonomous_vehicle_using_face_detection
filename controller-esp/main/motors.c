#include "motors.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAGMOTORS = "MOTORS";

static void set_motor(int value, ledc_channel_t ch_in1, ledc_channel_t ch_in2)
{
    int speed = abs(value);  // duty positivo
    if (speed > 255) speed = 255;

    if (value > 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in1, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in2, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in2);
    }
    else if (value < 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in1, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in2, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in2);
    }
    else { 
        // freewheel
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in1, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch_in2, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch_in2);
    }
}

void motors_init(void)
{
    // Configura o TIMER
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RES,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    // Macro pra facilitar registros
    #define CONF_CH(pin, ch) {                 \
        ledc_channel_config_t cfg = {          \
            .gpio_num = pin,                   \
            .speed_mode = LEDC_LOW_SPEED_MODE, \
            .channel = ch,                     \
            .timer_sel = LEDC_TIMER_0,         \
            .duty = 0,                         \
            .hpoint = 0                        \
        };                                      \
        ledc_channel_config(&cfg);             \
    }

    CONF_CH(MOTOR_LEFT_IN1, CH_LEFT_IN1);
    CONF_CH(MOTOR_LEFT_IN2, CH_LEFT_IN2);
    CONF_CH(MOTOR_RIGHT_IN1, CH_RIGHT_IN1);
    CONF_CH(MOTOR_RIGHT_IN2, CH_RIGHT_IN2);

    ESP_LOGI(TAGMOTORS, "Motores inicializados.");
}

void motors_set(int left, int right)
{
    //ESP_LOGI(TAGMOTORS, "Controle dos motores: Esquerdo -> %.2f , Direito -> %.2f", left, right);

    set_motor(left,  CH_LEFT_IN1,  CH_LEFT_IN2);
    set_motor(right, CH_RIGHT_IN1, CH_RIGHT_IN2);
}
