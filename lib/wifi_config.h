#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

#define WIFI_SSID "Lucas 2.4"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "369258147" // Substitua pela senha da sua rede Wi-Fi

void start_wifi();
void start_http_server();

#endif // WIFI_CONFIG_H