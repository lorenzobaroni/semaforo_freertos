#include "hardware/pio.h"          
#include "ws2812.pio.h"           
#include <stdint.h>               
#include "estado.h"               

#define MATRIX_PIN 7              

// Variáveis globais para configuração do PIO
PIO pio = pio0;                   
uint offset;                      
uint sm;                          

extern volatile EstadoSemaforo estadoAtual;  // Estado atual do semáforo, vindo do semaforo_freertos.c

// Inicializa a matriz WS2812 usando PIO e a state machine
void init_matrix() {
    offset = pio_add_program(pio, &ws2812_program);             
    sm = pio_claim_unused_sm(pio, true);                       
    ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false); 
}

// Aplica um fator de brilho (0.0 a 1.0) à cor GRB
uint32_t aplicar_brilho(uint32_t color, float brilho) {
    uint8_t g = (color >> 16) & 0xFF;                           // Extrai o componente verde
    uint8_t r = (color >> 8) & 0xFF;                            // Extrai o componente vermelho
    uint8_t b = color & 0xFF;                                   // Extrai o componente azul

    g = (uint8_t)(g * brilho);                                  // Aplica brilho
    r = (uint8_t)(r * brilho);
    b = (uint8_t)(b * brilho);

    return (g << 16) | (r << 8) | b;                            // Retorna a cor formatada GRB
}

// Envia um pixel para a matriz WS2812 (espera até que possa enviar)
void ws2812_put_pixel(uint32_t pixel_grb) {
    while (pio_sm_is_tx_fifo_full(pio, sm));                   
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);             
}

// Exibe as faixas de cores nos LEDs da matriz conforme o estado atual do semáforo
void mostrar_faixas_rgb(EstadoSemaforo estado) {
    // Define as cores base no formato GRB:
    uint32_t cores_resistor[] = {
        0xFF0000, // VERDE   → GRB
        0xCCFF00, // AMARELO → GRB
        0x00FF00, // VERMELHO → GRB
    };

    float brilho = 0.2f;  // Fator de brilho reduzido para não ofuscar
    uint32_t cor;

    // Define a cor da matriz de acordo com o estado do semáforo
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

    // Define os índices dos LEDs que devem ser acesos (centro visual da matriz 5x5)
    int leds_ativos[] = {6, 7, 8, 11, 12, 13, 16, 17, 18};

    // Percorre todos os LEDs da matriz
    for (int i = 0; i < NUM_LEDS; i++) {
        bool acender = false;
        for (int j = 0; j < sizeof(leds_ativos)/sizeof(int); j++) {
            if (i == leds_ativos[j]) {
                acender = true;
                break;
            }
        }
        // Envia a cor correspondente: ligada ou apagada (preto)
        ws2812_put_pixel(acender ? cor : 0x000000);
    }
}