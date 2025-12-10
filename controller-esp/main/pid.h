#ifndef PID_H
#define PID_H

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float integrator;
    float prevError;

    float outMin;       // limite inferior
    float outMax;       // limite superior

    float dt;           // tempo entre updates (em segundos)

    float output;
} PID_t;

// Inicialização
void PID_Init(PID_t *pid, float kp, float ki, float kd, float dt);

// Seta limites da saída (anti-windup)
void PID_SetOutputLimits(PID_t *pid, float min, float max);

// Reseta integrador e erro anterior
void PID_Reset(PID_t *pid);

// Atualiza o PID e retorna a saída
float PID_Update(PID_t *pid, float error);

#endif
