
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/font.h"
#include "lib/ssd1306.h"
#include "lib/matriz.h"
#include "lib/estado.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

// Definições dos pinos utilizados
#define LED_VERDE 11
#define LED_VERMELHO 13
#define BOTAO_A 5
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ADDR 0x3C
#define BUZZER 21

// Objeto do display e variáveis globais de controle de estado
ssd1306_t ssd;
volatile bool modoNoturno = false;
volatile EstadoSemaforo estadoAtual = VERDE;

// Função para gerar beeps com o buzzer, com duração e repetição configuráveis
void buzzer_beep(int on_ms, int off_ms, int repeat) {
    for (int i = 0; i < repeat; i++) {
        gpio_put(BUZZER, true);
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        gpio_put(BUZZER, false);
        vTaskDelay(pdMS_TO_TICKS(off_ms));
    }
}

// Padrões de beep para cada estado
void buzzer_verde()   { buzzer_beep(100, 900, 1); }
void buzzer_amarelo() { buzzer_beep(100, 100, 5); }
void buzzer_vermelho() {
    gpio_put(BUZZER, true);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_put(BUZZER, false);
    vTaskDelay(pdMS_TO_TICKS(1500));
}
void buzzer_noturno() { buzzer_beep(100, 1900, 1); }

// Layout base do display OLED
void desenharLayoutBase() {
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);
    ssd1306_line(&ssd, 3, 25, 123, 25, true);
    ssd1306_draw_string(&ssd, "Semaforo", 8, 6);
    ssd1306_draw_string(&ssd, "Inteligente", 20, 16);
}

// Tarefa responsável por ler o botão A e alternar entre modos (normal <-> noturno)
void vBotaoTask(void *pvParameters) {
    const uint32_t DEBOUNCE_TIME_MS = 50;
    bool estado_anterior = true;

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    while (true) {
        bool estado_atual = gpio_get(BOTAO_A);

        if (!estado_atual && estado_anterior) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
            if (!gpio_get(BOTAO_A)) {
                modoNoturno = !modoNoturno;
                while (!gpio_get(BOTAO_A)) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }

        estado_anterior = estado_atual;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Tarefa principal do semáforo: controla LEDs e buzzer conforme o estado
void vSemaforoTask(void *pvParameters) {
    gpio_init(LED_VERDE);
    gpio_init(LED_VERMELHO);
    gpio_init(BUZZER);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);

    while (1) {
        if (!modoNoturno) {
            estadoAtual = VERDE;
            gpio_put(LED_VERDE, true);
            buzzer_verde();
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(LED_VERDE, false);

            estadoAtual = AMARELO;
            gpio_put(LED_VERDE, true);
            gpio_put(LED_VERMELHO, true);
            buzzer_amarelo();
            vTaskDelay(pdMS_TO_TICKS(2000));
            gpio_put(LED_VERDE, false);
            gpio_put(LED_VERMELHO, false);

            estadoAtual = VERMELHO;
            gpio_put(LED_VERMELHO, true);
            buzzer_vermelho();
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(LED_VERMELHO, false);
        } else {
            estadoAtual = NOTURNO;
            gpio_put(LED_VERDE, true);
            gpio_put(LED_VERMELHO, true);
            buzzer_noturno();
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_put(LED_VERDE, false);
            gpio_put(LED_VERMELHO, false);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

// Tarefa da matriz WS2812: exibe cor conforme estado ou pisca no modo noturno
void vMatrizTask(void *pvParameters) {
    bool ligado = true;
    while (1) {
        if (estadoAtual == NOTURNO) {
            uint32_t cor_amarelo = aplicar_brilho(0xCCFF00, 0.2f);
            int leds_ativos[] = {6, 7, 8, 11, 12, 13, 16, 17, 18};

            for (int i = 0; i < NUM_LEDS; i++) {
                bool acender = false;
                for (int j = 0; j < sizeof(leds_ativos) / sizeof(int); j++) {
                    if (i == leds_ativos[j]) {
                        acender = true;
                        break;
                    }
                }
                ws2812_put_pixel(acender ? (ligado ? cor_amarelo : 0x000000) : 0x000000);
            }

            ligado = !ligado;
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            mostrar_faixas_rgb(estadoAtual);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Tarefa do display OLED: atualiza as mensagens visuais com base no estado atual
void vDisplayTask(void *pvParameters) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, 128, 64, false, I2C_ADDR, I2C_PORT);
    ssd1306_config(&ssd);

    while (1) {
        desenharLayoutBase();
        switch (estadoAtual) {
            case VERDE:
                ssd1306_draw_string(&ssd, "Pode", 10, 30);
                ssd1306_draw_string(&ssd, "Atravessar", 10, 45);
                break;
            case AMARELO:
                ssd1306_draw_string(&ssd, "Atencao", 10, 35);
                break;
            case VERMELHO:
                ssd1306_draw_string(&ssd, "PARE", 10, 35);
                break;
            case NOTURNO:
                ssd1306_draw_string(&ssd, "Modo", 10, 30);
                ssd1306_draw_string(&ssd, "Noturno", 10, 45);
                break;
        }
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Função principal que inicializa tudo e inicia o escalonador de tarefas FreeRTOS
int main() {
    stdio_init_all();
    init_matrix();

    xTaskCreate(vBotaoTask, "Botao", 256, NULL, 1, NULL);
    xTaskCreate(vSemaforoTask, "Semaforo", 512, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "ssd", 512, NULL, 1, NULL);
    xTaskCreate(vMatrizTask, "Matriz", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}