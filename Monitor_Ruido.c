#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "pico/bootrom.h"

// Definições do display OLED
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_ADDRESS 0x3C

// Instância do display OLED
ssd1306_t oled;

const uint LED_RED = 13;
const uint LED_BLUE = 12;
const uint LED_GREEN = 14; // LED Verde
const uint MIC_ADC = 28;
const uint NUM_AMOSTRAS = 100;

#define BUTTON_A 5
#define BUTTON_B 6

uint16_t ruido_base = 0; // Valor base do ruído ambiente

void debounce_delay()
{
    sleep_ms(50); // Pequeno atraso para debounce
}

// Função para calcular o nível médio do ruído ambiente
uint16_t calibrar_ruido()
{
    uint32_t soma = 0;
    for (int i = 0; i < NUM_AMOSTRAS; i++)
    {
        adc_select_input(2); // Seleciona ADC2 (GPIO28)
        soma += adc_read();
        sleep_ms(5); // Pequeno delay entre leituras
    }
    return soma / NUM_AMOSTRAS; // Retorna a média calculada
}

void init_ssd1306()
{
    // Inicializa o display OLED
    ssd1306_init(&oled, SSD1306_WIDTH, SSD1306_HEIGHT, false, SSD1306_ADDRESS, i2c0);
    ssd1306_clear(&oled); // Limpa o display
}

void display_ruido(float ruido)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Ruido: %.2f", ruido);
    ssd1306_clear(&oled); // Limpa o display
    ssd1306_draw_text(&oled, 0, 0, buffer);
    ssd1306_show(&oled);
}

int main()
{
    stdio_init_all();
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    // Configuração do botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B); // Ativa o pull-up para o botão B

    // Inicializa os LEDs
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(LED_GREEN); // Inicializa o LED verde
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Inicializa o ADC
    adc_init();
    adc_gpio_init(MIC_ADC);

    // Inicializa o display OLED
    init_ssd1306();

    printf("Calibrando o ruído ambiente...\n");
    ruido_base = calibrar_ruido();
    printf("Ruído base calibrado: %d\n", ruido_base);

    // Define os limiares com base na calibração
    uint16_t limiar_1 = ruido_base + 100; // Ativa LED azul
    uint16_t limiar_2 = ruido_base + 300; // Ativa LED vermelho

    bool led_blue_state = false; // Estado do LED azul
    uint32_t last_toggle_time = 0; // Tempo do último toggle do LED azul

    while (true)
    {
        // Verifica se o botão B foi pressionado para entrar em modo BOOTSEL
        if (!gpio_get(BUTTON_B))
        {
            debounce_delay();  // Chama o debounce para evitar múltiplos registros rápidos
            printf("[SISTEMA] Entrando em modo BOOTSEL\n");
            reset_usb_boot(0, 0); // Chama o comando para resetar e entrar no BOOTSEL
        }

        // Lê o valor do microfone
        adc_select_input(2);
        uint16_t mic_value = adc_read();
        float voltage = mic_value * 3.3f / (1 << 12);

        // Controle dos LEDs baseado no ruído
        if (mic_value > limiar_1 && mic_value < limiar_2)
        {
            gpio_put(LED_BLUE, true); // Liga LED azul
            gpio_put(LED_RED, false);
            gpio_put(LED_GREEN, false); // Apaga o LED verde
        }
        else if (mic_value >= limiar_2)
        {
            gpio_put(LED_BLUE, false);
            gpio_put(LED_RED, true); // Liga LED vermelho
            gpio_put(LED_GREEN, false); // Apaga o LED verde
        }
        else
        {
            gpio_put(LED_BLUE, false); // Apaga o LED azul
            gpio_put(LED_RED, false); // Apaga o LED vermelho
            gpio_put(LED_GREEN, true); // Liga o LED verde, indicando que está quieto
        }

        // Controle de piscagem do LED azul com base no ruído
        if (mic_value > ruido_base && mic_value < limiar_1) // Apenas pisca se houver algum ruído detectado
        {
            uint32_t current_time = time_us_32();
                
            // Mapeia o valor do ruído para um intervalo de piscagem de 0.5 a 1.5s
            uint32_t blink_interval = 1500000 - ((mic_value - ruido_base) * 1000000) / limiar_1;

            // Controla o piscar do LED azul
            if (current_time - last_toggle_time > blink_interval)
            {
                led_blue_state = !led_blue_state; // Alterna o estado do LED azul
                gpio_put(LED_BLUE, led_blue_state); // Aplica o novo estado
                last_toggle_time = current_time;
            }
        }

        // Exibe o ruído no display OLED
        display_ruido(voltage);

        printf("Ruído: %d | Limiar Azul: %d | Limiar Vermelho: %d\n", mic_value, limiar_1, limiar_2);
        sleep_ms(100); // Pequeno delay
    }
}
