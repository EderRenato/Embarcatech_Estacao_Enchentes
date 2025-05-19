#ifndef LED_MATRIX_H
#define LED_MATRIX_H


#include "pico/stdlib.h"

#define LED_COUNT 25
#define MATRIX_PIN 7

// Definição da estrutura de pixel (visível externamente)
typedef struct {
    uint8_t G, R, B;  // Componentes de cor: Verde, Vermelho e Azul (ordem GRB comum em LEDs endereçáveis)
} pixel_t;

// Alias para facilitar o uso
typedef pixel_t npLED_t;

// Função para calcular o índice do LED na matriz
int getIndex(int x, int y);

// Função para inicializar o PIO para controle dos LEDs
void npInit(uint pin);

// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);

// Função para limpar (apagar) todos os LEDs
void npClear();

// Função para atualizar os LEDs no hardware
void npWrite();

#endif // LED_MATRIX_H