#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/font.h"
#include "lib/ssd1306.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define LED_VERDE 11
#define LED_VERMELHO 13
#define BOTAO_A 5
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ADDR 0x3C
#define BUZZER 21

volatile bool modoNoturno = false;
ssd1306_t ssd;

enum EstadoSemaforo { VERDE, AMARELO, VERMELHO, NOTURNO };
volatile enum EstadoSemaforo estadoAtual = VERDE;

void buzzer_beep(int on_ms, int off_ms, int repeat) {
    for (int i = 0; i < repeat; i++) {
        gpio_put(BUZZER, true);
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        gpio_put(BUZZER, false);
        vTaskDelay(pdMS_TO_TICKS(off_ms));
    }
}

void buzzer_verde() {
    buzzer_beep(100, 900, 1);
}

void buzzer_amarelo() {
    buzzer_beep(100, 100, 5);
}

void buzzer_vermelho() {
    gpio_put(BUZZER, true);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_put(BUZZER, false);
    vTaskDelay(pdMS_TO_TICKS(1500));
}

void buzzer_noturno() {
    buzzer_beep(100, 1900, 1);
}

void desenharLayoutBase() {
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, true);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "Semaforo", 8, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, "Inteligente", 20, 16);  // Desenha uma string
}

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

int main() {
    stdio_init_all();

    // Cria as tarefas
    xTaskCreate(vBotaoTask, "Botao", 256, NULL, 1, NULL);
    xTaskCreate(vSemaforoTask, "Semaforo", 512, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "ssd", 512, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}