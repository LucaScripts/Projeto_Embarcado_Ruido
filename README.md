<div align="center">
  <img src="https://github.com/LucaScripts/ComunicaoSerial/blob/main/docs/Group%20658.png?raw=true" alt="Logo do Projeto" width="50%"/>
</div>

# **Sistema Embarcado para Monitoramento de Níveis de Ruído**

📌 **Autor**: Lucas Dias da Silva  
📆 **Data**: 26/02/2025  

Este projeto monitora os níveis de ruído ambiente usando um microfone conectado a um ADC. O sistema aciona LEDs e um buzzer com base nos níveis de ruído detectados. Além disso, exibe informações em um display OLED.

---

## 📌 **Objetivos**
✅ Monitorar os níveis de ruído ambiente.  
✅ Acionar LEDs e buzzer com base nos níveis de ruído.  
✅ Exibir informações sobre os níveis de ruído em um display OLED.  

---

## 🛠 **Materiais Necessários**
🔹 1x **Microfone**  
🔹 3x **LEDs** (Verde, Azul, Vermelho)  
🔹 1x **Buzzer**  
🔹 1x **Display OLED**  
🔹 2x **Botões** (A e B)  
🔹 1x **Raspberry Pi Pico**  

---

## 🔧 **Configuração dos Componentes**
- **LED Verde**: Indica operação normal (níveis de ruído abaixo do limiar de alerta).  
- **LED Azul**: Indica níveis de ruído médios.  
- **LED Vermelho**: Indica níveis de ruído altos.  
- **Buzzer**: Emite um alerta sonoro quando o nível de ruído atinge níveis extremos.  
- **Display OLED**: Exibe informações sobre os níveis de ruído e os limiares configurados.  

---

## 🏗 **Esquema de Ligação**
| Componente  | Pino do Raspberry Pi Pico |
|-------------|---------------------------|
| BUZZER_A    | GPIO 21                   |
| BUZZER_B    | GPIO 10                   |
| BUTTON_A    | GPIO 5                    |
| BUTTON_B    | GPIO 6                    |
| LED_RED     | GPIO 13                   |
| LED_BLUE    | GPIO 12                   |
| LED_GREEN   | GPIO 11                   |
| MIC_ADC     | GPIO 28 (ADC2)            |
| I2C_SDA     | GPIO 14                   |
| I2C_SCL     | GPIO 15                   |

---

## 📜 **Implementação**
### 1️⃣ **Monitoramento de Ruído**
- Leitura dos valores do microfone.
- Aplicação de filtro móvel para suavizar a medição.
- Conversão da amplitude filtrada em dB SPL.

### 2️⃣ **Controle de LEDs e Buzzer**
- Acionamento dos LEDs com base nos níveis de ruído.
- Emissão de alerta sonoro pelo buzzer quando o ruído atinge níveis extremos.

### 3️⃣ **Exibição no Display OLED**
- Exibição dos valores do ADC e dB SPL.
- Exibição dos limiares configurados.

### 4️⃣ **Configuração do Wi-Fi e Servidor HTTP**
- Conexão à rede Wi-Fi.
- Configuração de um servidor HTTP para controle remoto dos botões e exibição dos valores do ADC.

---

## 📥 Clonando o Repositório e Compilando o Código

Para baixar o código e começar a trabalhar com ele, clone o repositório e carregue o código na placa seguindo os passos abaixo:

![Clonando o Repositório](https://github.com/LucaScripts/PWM/blob/main/docs/Bem-vindo%20-%20Visual%20Studio%20Code%202025-01-31%2018-49-32%20(1).gif?raw=true)

---

## 🚦 **Demonstração da Simulação Wokwi**

Abaixo está uma prévia da simulação serial no **Wokwi**:

![Wokwi](https://github.com/LucaScripts/ConversoresAD/blob/main/docs/diagram.json%20-%20ConversoresAD%20-%20Visual%20Studio%20Code%202025-02-14%2018-49-32.gif?raw=true)

🔗[Demonstração no Wokwi]()

---

## 🚦 **Demonstração do Projeto Video**

🔗 [Demonstração no Google Drive]()

---

## 🚦 **Demonstração do Projeto BitDogLab**

Abaixo está uma prévia do display na **BitDogLab**:

![display]()

Abaixo está uma prévia do RGB na **BitDogLab**:

![RGB PWM]()

---

## 📌 **Melhorias Futuras**
- **📡 Comunicação Serial:** Adição de monitoramento via UART.  
- **🔄 Otimização do PWM:** Melhor precisão no controle dos LEDs.  
- **🖥️ Interface Gráfica:** Exibição avançada no display OLED.  

---

## 📶 **Configuração do Wi-Fi e Servidor HTTP**

### 1️⃣ **Configuração do Wi-Fi**
Para configurar o Wi-Fi, edite o arquivo `wifi_config.h` e substitua `WIFI_SSID` e `WIFI_PASS` pelo nome e senha da sua rede Wi-Fi.

```c
// filepath: /c:/Users/lucas/Downloads/Projeto_Emcarcado_Ruido/lib/wifi_config.h
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

#define WIFI_SSID "NomeDaRedeWiFi"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "SenhaDaRedeWiFi" // Substitua pela senha da sua rede Wi-Fi

void start_wifi();
void start_http_server();

#endif // WIFI_CONFIG_H

---



## Referência
Este projeto foi baseado no repositório [BitDogLab-C](https://github.com/BitDogLab/BitDogLab-C.git).
