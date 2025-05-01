
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include <stdint.h>
#include "estado.h"

#define MATRIX_PIN 7

PIO pio = pio0;
uint offset;
uint sm;

extern volatile EstadoSemaforo estadoAtual;

void init_matrix() {
    offset = pio_add_program(pio, &ws2812_program);
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);
}

uint32_t aplicar_brilho(uint32_t color, float brilho) {
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t r = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    g = (uint8_t)(g * brilho);
    r = (uint8_t)(r * brilho);
    b = (uint8_t)(b * brilho);
    return (g << 16) | (r << 8) | b;
}

void ws2812_put_pixel(uint32_t pixel_grb) {
    while (pio_sm_is_tx_fifo_full(pio, sm));
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

void mostrar_faixas_rgb(EstadoSemaforo estado) {
    uint32_t cores_resistor[] = {
        0xFF0000, // VERDE
        0xCCFF00, // AMARELO
        0x00FF00, // VERMELHO
    };

    float brilho = 0.2f;
    uint32_t cor;

    switch (estado) {
        case VERDE:
            cor = aplicar_brilho(cores_resistor[0], brilho);
            break;
        case AMARELO:
        case NOTURNO:
            cor = aplicar_brilho(cores_resistor[1], brilho);
            break;
        case VERMELHO:
        default:
            cor = aplicar_brilho(cores_resistor[2], brilho);
            break;
    }

    int leds_ativos[] = {6, 7, 8, 11, 12, 13, 16, 17, 18};

    for (int i = 0; i < NUM_LEDS; i++) {
        bool acender = false;
        for (int j = 0; j < sizeof(leds_ativos)/sizeof(int); j++) {
            if (i == leds_ativos[j]) {
                acender = true;
                break;
            }
        }
        ws2812_put_pixel(acender ? cor : 0x000000); // acende ou apaga
    }
}