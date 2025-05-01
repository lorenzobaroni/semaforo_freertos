// matriz.h - Cabeçalho para controle da matriz WS2812

#ifndef MATRIZ_H
#define MATRIZ_H

#include <stdint.h>
#include "estado.h"

// Inicializa a matriz de LEDs
void init_matrix();

// Aplica brilho a uma cor RGB (formato 0xRRGGBB)
uint32_t aplicar_brilho(uint32_t color, float brilho);

// Envia um pixel no formato GRB para a matriz
void ws2812_put_pixel(uint32_t pixel_grb);

// Mostra as cores na matriz de leds
void mostrar_faixas_rgb(EstadoSemaforo estado);

#endif // MATRIZ_H
