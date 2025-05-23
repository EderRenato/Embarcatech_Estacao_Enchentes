#include "led_matrix.h"

#include "hardware/pio.h"
#include "ws2818b.pio.h"

static npLED_t leds[LED_COUNT];  // static para encapsulamento
static PIO np_pio;          // static para encapsulamento
static uint sm;             // static para encapsulamento
static uint data_pin;       // static para encapsulamento

int getIndex(int linha, int coluna) {
    // Transformação para corrigir a rotação de 90° anti-horário
    int nova_linha = coluna;
    int nova_coluna = 4 - linha;  // Inverter a coordenada da linha
    
    // Reutilizando a lógica original para lidar com o padrão em zigue-zague
    nova_linha = 4 - nova_linha;  // Inverter as linhas (0 -> 4, 1 -> 3, etc.)
    nova_coluna = 4 - nova_coluna; // Inverter as colunas (0 -> 4, 1 -> 3, etc.)
    
    if (nova_linha % 2 == 0) {
        return nova_linha * 5 + nova_coluna;       // Linha par (esquerda para direita)
    } else {
        return nova_linha * 5 + (4 - nova_coluna); // Linha ímpar (direita para esquerda)
    }
}

void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carregar o programa PIO
    np_pio = pio0;                                         // Usar o primeiro bloco PIO

    sm = pio_claim_unused_sm(np_pio, false);              // Tentar usar uma state machine do pio0
    if (sm < 0) {                                         // Se não houver disponível no pio0
        np_pio = pio1;                                    // Mudar para o pio1
        sm = pio_claim_unused_sm(np_pio, true);           // Usar uma state machine do pio1
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicializar state machine para LEDs

    for (uint i = 0; i < LED_COUNT; ++i) {                // Inicializar todos os LEDs como apagados
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;                                    // Definir componente vermelho
    leds[index].G = g;                                    // Definir componente verde
    leds[index].B = b;                                    // Definir componente azul
}

void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i) {                // Iterar sobre todos os LEDs
        npSetLED(i, 0, 0, 0);                             // Definir cor como preta (apagado)
    }
    npWrite();                                            // Atualizar LEDs no hardware
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {                // Iterar sobre todos os LEDs
        pio_sm_put_blocking(np_pio, sm, leds[i].G);       // Enviar componente verde
        pio_sm_put_blocking(np_pio, sm, leds[i].R);       // Enviar componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].B);       // Enviar componente azul
    }
}
