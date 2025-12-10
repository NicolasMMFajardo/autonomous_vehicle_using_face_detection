#pragma once

// === DEFINA AQUI OS SEUS PINOS ===
#define MOTOR_LEFT_IN2    14
#define MOTOR_LEFT_IN1    27
#define MOTOR_RIGHT_IN2   26
#define MOTOR_RIGHT_IN1   25

// PWM
#define PWM_FREQ        20000       // 20 kHz
#define PWM_RES         LEDC_TIMER_8_BIT

// Canais
#define CH_LEFT_IN1     LEDC_CHANNEL_0
#define CH_LEFT_IN2     LEDC_CHANNEL_1
#define CH_RIGHT_IN1    LEDC_CHANNEL_2
#define CH_RIGHT_IN2    LEDC_CHANNEL_3

void motors_init(void);
void motors_set(int left, int right);   // valores -255 a +255
