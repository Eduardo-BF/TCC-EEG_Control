#ifndef MOTOR_H
#define MOTOR_H


#include <stdint.h>  // int16_t reconhecido

// Variáveis
extern int16_t direcao;
extern int16_t velA , velB ;

void set_velocidade_IA(int, int);
void set_velocidade_IB(int, int);
void movimento_motores(int);

#endif
