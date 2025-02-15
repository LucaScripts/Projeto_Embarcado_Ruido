#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

#define BUZZER_A 21  // Buzzer A conectado no GPIO21
#define BUZZER_B 10  // Buzzer B conectado no GPIO10
#define BUTTON_A 5    // Botão A conectado no GPIO5
#define BUTTON_B 6    // Botão B conectado no GPIO6

const uint LED_RED = 13;
const uint LED_BLUE = 12;
const uint LED_GREEN = 14; // LED Verde
const uint MIC_ADC = 28;
const uint NUM_AMOSTRAS = 100;

uint16_t ruido_base = 0; // Valor base do ruído ambiente
bool buzzer_ligado = false; // Estado do buzzer

void debounce_delay()
{
    sleep_ms(50); // Pequeno atraso para debounce
}

void emitir_som_buzzer(uint buzzer_pin)
{
    // Configura o GPIO para PWM
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    
    // Obtém o número do slice de PWM
    uint slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    
    // Frequência do som: 2000 Hz
    pwm_set_clkdiv(slice_num, 125.0f);  // Ajuste para controle de frequência
    pwm_set_wrap(slice_num, 1000);      // Ajuste da contagem de ciclos
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 900);  // Duty cycle de 90% para maior intensidade

    pwm_set_enabled(slice_num, true);  // Ativa o PWM para gerar o som
}

void parar_som_buzzer(uint buzzer_pin)
{
    uint slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    pwm_set_enabled(slice_num, false);  // Desliga o PWM, apagando o som
}

// Função para calibrar o nível de ruído ambiente
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

int main()
{
    stdio_init_all();

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

    // Configuração do botão A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Ativa o pull-up para o botão A

    // Configuração do botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B); // Ativa o pull-up para o botão B

    printf("Calibrando o ruído ambiente...\n");
    ruido_base = calibrar_ruido();
    printf("Ruído base calibrado: %d\n", ruido_base);

    // Define os limiares com base na calibração
    uint16_t limiar_1 = ruido_base + 100; // Ativa LED azul
    uint16_t limiar_2 = ruido_base + 300; // Ativa LED vermelho

    while (true)
    {
        // Verifica se o botão A foi pressionado
        if (!gpio_get(BUTTON_A))
        {
            debounce_delay();  // Chama o debounce para evitar múltiplos registros rápidos
            if (!buzzer_ligado)
            {
                printf("[BOTÃO A] Pressionado! Emitindo som no buzzer.\n");
                emitir_som_buzzer(BUZZER_A); // Emite som no buzzer A
                emitir_som_buzzer(BUZZER_B); // Emite som no buzzer B
                buzzer_ligado = true; // Atualiza o estado do buzzer
            }
            else
            {
                printf("[BOTÃO A] Pressionado! Parando som no buzzer.\n");
                parar_som_buzzer(BUZZER_A);  // Para o som do buzzer A
                parar_som_buzzer(BUZZER_B);  // Para o som do buzzer B
                buzzer_ligado = false; // Atualiza o estado do buzzer
            }
            // Espera até que o botão seja liberado
            while (!gpio_get(BUTTON_A))
            {
                sleep_ms(10);
            }
        }

        // Verifica se o botão B foi pressionado para entrar em modo BOOTSEL
        if (!gpio_get(BUTTON_B))
        {
            debounce_delay();  // Chama o debounce para evitar múltiplos registros rápidos
            printf("[BOTÃO B] Pressionado! Entrando em modo BOOTSEL.\n");
            reset_usb_boot(0, 0); // Chama o comando para resetar e entrar no BOOTSEL
        }

        // Lê o valor do microfone
        adc_select_input(2);
        uint16_t mic_value = adc_read();

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

        printf("Ruído: %d | Limiar Azul: %d | Limiar Vermelho: %d\n", mic_value, limiar_1, limiar_2);
        sleep_ms(100); // Pequeno delay
    }
}
