/*
 * Projeto: Monitor de Ruído
 * Autor: LUCAS DIAS DA SILVA
 * Descrição: Este projeto monitora os níveis de ruído ambiente usando um microfone conectado a um ADC.
 *            O sistema aciona LEDs e um buzzer com base nos níveis de ruído detectados. Além disso,
 *            exibe informações em um display OLED.
 */

#include <stdio.h>
#include <math.h>  // Para log10() e fabs()
#include <string.h> // Para strlen
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "wifi_config.h"  // Adicione esta linha

// Definições de pinos
#define BUZZER_A 21   // Buzzer A no GPIO21
#define BUZZER_B 10   // Buzzer B no GPIO10
#define BUTTON_A 5    // Botão A no GPIO5
#define BUTTON_B 6    // Botão B no GPIO6

const uint LED_RED   = 13;
const uint LED_BLUE  = 12;
const uint LED_GREEN = 11; 
const uint MIC_ADC   = 28; // ADC do microfone (GPIO28 -> ADC2)
const uint NUM_AMOSTRAS = 100;

#define I2C_PORT    i2c1
#define I2C_SDA     14
#define I2C_SCL     15
#define DISPLAY_ADDR 0x3C
#define WIDTH  128
#define HEIGHT 64

#define FILTER_SIZE 10

ssd1306_t ssd;

uint16_t ruido_base = 0;       // Valor médio (offset) do sinal, aproximado de 2048
bool buzzer_ligado = false;    // Estado do buzzer
bool led_vermelho_ligado = false; // Estado do LED vermelho

// Variáveis globais para armazenar os valores do ADC
extern volatile uint16_t adc_value;
extern volatile float amplitude;
extern volatile float db_spl;

// --- Funções auxiliares ---

// Pequeno atraso para debounce
void debounce_delay() {
    sleep_ms(50);
}

// Emite som via PWM no buzzer (configurado para aproximadamente 2000 Hz)
void emitir_som_buzzer(uint buzzer_pin) {
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    pwm_set_clkdiv(slice_num, 125.0f);
    pwm_set_wrap(slice_num, 1000);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 900); // 90% de duty cycle
    pwm_set_enabled(slice_num, true);
}

// Para o som do buzzer
void parar_som_buzzer(uint buzzer_pin) {
    uint slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    pwm_set_enabled(slice_num, false);
}

// Calibração do ruído ambiente: lê NUM_AMOSTRAS e calcula a média, que deverá ser próxima de 2048
uint16_t calibrar_ruido() {
    uint32_t soma = 0;
    for (int i = 0; i < NUM_AMOSTRAS; i++) {
        adc_select_input(2); // ADC2 (GPIO28)
        soma += adc_read();
    }
    return soma / NUM_AMOSTRAS;
}

// Filtro móvel simples (média dos últimos FILTER_SIZE valores)
// Agora usamos para filtrar a amplitude (valor absoluto) do sinal AC
float filterNoise(float newValue) {
    static float values[FILTER_SIZE];
    static int index = 0;
    static int count = 0;
    float sum = 0.0f;
    values[index] = newValue;
    index = (index + 1) % FILTER_SIZE;
    if (count < FILTER_SIZE) count++;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum / count;
}

// Função para converter a amplitude (em contagens do ADC) em dB SPL
// "adc_amplitude" deve ser o valor absoluto (em contagens) do sinal AC filtrado.
float convertToDBSPL(uint16_t adc_amplitude) {
    // Converte contagens para tensão (ADC de 12 bits, Vref = 3.3V)
    float voltage = adc_amplitude * (3.3f / 4095.0f);
    // Sensibilidade do microfone (em V/Pa). Ajuste conforme o seu microfone.
    const float MIC_SENSITIVITY = 0.007f; // Exemplo: 7 mV/Pa
    // Converte tensão para pressão sonora (Pa)
    float pressure = voltage / MIC_SENSITIVITY;
    // Pressão de referência para 0 dB SPL em ar: 20 µPa
    const float pref = 20e-6f;
    if (pressure <= 0) pressure = 1e-9f; // Evita log de zero
    float dbSPL = 20.0f * log10(pressure / pref);
    return dbSPL;
}

// Função auxiliar para centralizar texto no display com deslocamento opcional no eixo X
void draw_centered_string(ssd1306_t *ssd, const char *str, int y, int x_offset) {
    int len = strlen(str);
    int x = ((WIDTH - (len * 6)) / 2) + x_offset; // Cada caractere tem aproximadamente 6 pixels de largura
    ssd1306_draw_string(ssd, str, x, y);
}

int main() {
    stdio_init_all();

    // Inicializa o Wi-Fi e o servidor HTTP
    start_wifi();

    // Inicializa os LEDs
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Inicializa o ADC e o GPIO do microfone
    adc_init();
    adc_gpio_init(MIC_ADC);

    // Configura os botões com pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configura o I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa e configura o display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    printf("Calibrando o ruído ambiente...\n");
    ruido_base = calibrar_ruido();  // Este valor deve ser próximo de 2048
    printf("Ruído base calibrado: %d\n", ruido_base);

    // Defina limiares para controle dos LEDs e buzzer (esses valores podem ser ajustados)
    uint16_t limiar_1 = ruido_base + 100;   // Por exemplo, para acionar LED azul
    uint16_t limiar_2 = 3000; // Para acionar LED vermelho
    uint16_t limiar_3 = 4000;              // Para acionar LED vermelho e buzzer

    float noiseFiltered = 0.0f; // Valor filtrado (em contagens) da amplitude AC
    float noise_dBSPL = 0.0f;   // Nível em dB SPL

    while (true) {
        // Botão A: ativa/desativa o buzzer
        if (!gpio_get(BUTTON_A)) {
            debounce_delay();
            if (!buzzer_ligado) {
                printf("[BOTÃO A] Pressionado! Emitindo som no buzzer.\n");
                emitir_som_buzzer(BUZZER_A);
                emitir_som_buzzer(BUZZER_B);
                buzzer_ligado = true;
            } else {
                printf("[BOTÃO A] Pressionado! Parando som no buzzer.\n");
                parar_som_buzzer(BUZZER_A);
                parar_som_buzzer(BUZZER_B);
                buzzer_ligado = false;
            }
            while (!gpio_get(BUTTON_A)) {
                sleep_ms(10);
            }
        }
        
        // Botão B: entra no modo BOOTSEL
        if (!gpio_get(BUTTON_B)) {
            debounce_delay();
            printf("[BOTÃO B] Pressionado! Entrando em modo BOOTSEL.\n");
            reset_usb_boot(0, 0);
        }
        
        // Lê o valor do microfone (valor bruto do ADC: 0 a 4095)
        adc_select_input(2);
        uint16_t mic_value = 0;
        for (int i = 0; i < 10; i++) { // Média de 10 leituras para suavizar
            mic_value += adc_read();
            sleep_us(10); // Reduzido o atraso entre leituras
        }
        mic_value /= 10;
        
        // Verificação adicional para valores fora do esperado
        if (mic_value > 4095 || mic_value < 0 || mic_value < ruido_base) {
            printf("Valor do ADC fora do intervalo esperado ou abaixo do ruído base: %d\n", mic_value);
            continue; // Ignora esta leitura
        }

        // Calcula o sinal AC: subtrai o offset (ruído_base)
        int16_t sample = (int16_t)mic_value - (int16_t)ruido_base;
        // Calcula o valor absoluto (amplitude) do sinal AC
        float abs_sample = fabs((float)sample);
        // Aplica o filtro móvel para suavizar a medição (em contagens)
        noiseFiltered = filterNoise(abs_sample);
        // Converte a amplitude filtrada em dB SPL
        noise_dBSPL = convertToDBSPL((uint16_t)noiseFiltered);
        
        // Adiciona logs para verificar a consistência dos valores
        printf("ADC Bruto: %d | Amplitude: %.1f | dB SPL: %.1f\n", mic_value, noiseFiltered, noise_dBSPL);
        
        // Atualiza as variáveis globais
        adc_value = mic_value;
        amplitude = noiseFiltered;
        db_spl = noise_dBSPL;
        
        // Controle dos LEDs e buzzer com base no valor bruto (você pode também usar o dB SPL)
        if (mic_value > limiar_1 && mic_value < limiar_2) {
            gpio_put(LED_BLUE, true);
            gpio_put(LED_RED, false);
            gpio_put(LED_GREEN, false);
            if (!buzzer_ligado) {
                parar_som_buzzer(BUZZER_A);
                parar_som_buzzer(BUZZER_B);
            }
        }
        else if (mic_value >= limiar_2 && mic_value < limiar_3) {
            gpio_put(LED_BLUE, false);
            gpio_put(LED_RED, true);
            gpio_put(LED_GREEN, false);
            if (!buzzer_ligado) {
                parar_som_buzzer(BUZZER_A);
                parar_som_buzzer(BUZZER_B);
            }
        }
        else if (mic_value >= limiar_3) {
            gpio_put(LED_BLUE, false);
            gpio_put(LED_RED, true);
            gpio_put(LED_GREEN, false);
            if (!buzzer_ligado) {
                emitir_som_buzzer(BUZZER_A);
                emitir_som_buzzer(BUZZER_B);
            }
        }
        else { // mic_value <= limiar_1
            gpio_put(LED_BLUE, false);
            gpio_put(LED_RED, false);
            gpio_put(LED_GREEN, true);
            if (!buzzer_ligado) {
                parar_som_buzzer(BUZZER_A);
                parar_som_buzzer(BUZZER_B);
            }
        }
        
        // Atualiza o display OLED com informações
        ssd1306_draw_border(&ssd); // Desenha a borda ao redor do display
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "ADC:    %d", mic_value);
        draw_centered_string(&ssd, buffer, 5, -15);
        
        // Desenha o texto fixo "dB SPL:"
        ssd1306_draw_string(&ssd, "dB SPL:", 10, 15);
        // Desenha o valor numérico na mesma linha
        snprintf(buffer, sizeof(buffer), "  %.1f", noise_dBSPL);
        ssd1306_draw_string(&ssd, buffer, 60, 15); // Ajuste a posição x conforme necessário
        
        snprintf(buffer, sizeof(buffer), "Medio:  %d", limiar_1);
        draw_centered_string(&ssd, buffer, 25, -13); // Exibe o limiar 1
        snprintf(buffer, sizeof(buffer), "Alto:   %d", limiar_2);
        draw_centered_string(&ssd, buffer, 35, -13); // Exibe o limiar 2
        snprintf(buffer, sizeof(buffer), "Extremo:%d", limiar_3);
        draw_centered_string(&ssd, buffer, 45, -13); // Exibe o limiar 3
        
        // Desenha o texto fixo "Buzzer:"
        ssd1306_draw_string(&ssd, "Buzzer:", 15, 55); // Ajuste a posição x conforme necessário
        // Limpa a área onde o estado do buzzer será exibido
        ssd1306_fill_rect(&ssd, 90, 53, 30, 10, false); // Ajuste a altura para 16 pixels e a posição Y para 53
        // Desenha o estado do buzzer
        snprintf(buffer, sizeof(buffer), "%s", buzzer_ligado ? "ON" : "OFF");
        ssd1306_draw_string(&ssd, buffer, 88, 55); // Ajuste a posição x conforme necessário

        ssd1306_send_data(&ssd);
        
        sleep_ms(10); // Reduzido o tempo de espera no loop principal
    }
    
    return 0;
}
