#include "pid.h"

void PID_Init(PID_t *pid, float kp, float ki, float kd, float dt)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->integrator = 0.0f;
    pid->prevError = 0.0f;
    pid->output = 0.0f;

    pid->dt = dt;

    // limites padrão
    pid->outMin = -1000.0f;
    pid->outMax =  1000.0f;
}

void PID_SetOutputLimits(PID_t *pid, float min, float max)
{
    pid->outMin = min;
    pid->outMax = max;
}

void PID_Reset(PID_t *pid)
{
    pid->integrator = 0.0f;
    pid->prevError = 0.0f;
    pid->output = 0.0f;
}

float PID_Update(PID_t *pid, float error)
{
    // Proporcional
    float P = pid->Kp * error;

    // Integral com anti-windup
    pid->integrator += error * pid->dt;
    float I = pid->Ki * pid->integrator;

    // Derivativo
    float derivative = (error - pid->prevError) / pid->dt;
    float D = pid->Kd * derivative;

    float output = P + I + D;

    // Salva erro para próxima iteração
    pid->prevError = error;

    // Saturação + Anti-Windup
    if (output > pid->outMax) {
        output = pid->outMax;
        // opcional: impedir acumular mais integral quando saturado
        pid->integrator -= error * pid->dt;
    } else if (output < pid->outMin) {
        output = pid->outMin;
        pid->integrator -= error * pid->dt;
    }

    pid->output = output;
    return output;
}
