#include "wifi_config.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

#define BUTTON_A 5    // Botão A no GPIO5
#define BUTTON_B 6    // Botão B no GPIO6

// Variáveis globais para armazenar os valores do ADC
volatile uint16_t adc_value = 0;
volatile float amplitude = 0.0f;
volatile float db_spl = 0.0f;

// Buffer para respostas HTTP
#define HTTP_RESPONSE_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html><body>" \
                      "<h1>Controle dos Botoes</h1>" \
                      "<p><a href=\"/button/a\">Pressionar Botao A</a></p>" \
                      "<p><a href=\"/button/b\">Pressionar Botao B</a></p>" \
                      "<h2>Valores do ADC</h2>" \
                      "<p>ADC Bruto: %d</p>" \
                      "<p>Amplitude: %.1f</p>" \
                      "<p>dB SPL: %.1f</p>" \
                      "</body></html>\r\n"

// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Processa a requisição HTTP
    char *request = (char *)p->payload;

    if (strstr(request, "GET /button/a")) {
        // Simula a pressão do botão A
        gpio_put(BUTTON_A, 1);
        sleep_ms(100);
        gpio_put(BUTTON_A, 0);
    } else if (strstr(request, "GET /button/b")) {
        // Simula a pressão do botão B
        gpio_put(BUTTON_B, 1);
        sleep_ms(100);
        gpio_put(BUTTON_B, 0);
    }

    // Atualiza a resposta HTTP com os valores do ADC
    char http_response[512];
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE_TEMPLATE, adc_value, amplitude, db_spl);

    // Envia a resposta HTTP
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);

    // Libera o buffer recebido
    pbuf_free(p);

    return ERR_OK;
}

// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    return ERR_OK;
}

// Função de setup do servidor TCP
void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Liga o servidor na porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão

    printf("Servidor HTTP rodando na porta 80...\n");
}

void start_wifi() {
    stdio_init_all();  // Inicializa a saída padrão
    sleep_ms(10000);
    printf("Iniciando servidor HTTP\n");

    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return;
    } else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    printf("Wi-Fi conectado!\n");
    printf("Para pressionar os botões acesse o Endereço IP seguido de /button/a ou /button/b\n");

    // Configura os botões como saída
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_OUT);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_OUT);

    // Inicia o servidor HTTP
    start_http_server();
}