<div align="center">
  <img src="https://github.com/LucaScripts/ComunicaoSerial/blob/main/docs/Group%20658.png?raw=true" alt="Logo do Projeto" width="50%"/>
</div>

# Projeto: Sistema Embarcado para Monitoramento de Níveis de Ruído

## Autor
LUCAS DIAS DA SILVA

## Descrição
Este projeto monitora os níveis de ruído ambiente usando um microfone conectado a um ADC. O sistema aciona LEDs e um buzzer com base nos níveis de ruído detectados. Além disso, exibe informações em um display OLED.

## Funcionalidades
- **LED Verde**: Indica operação normal (níveis de ruído abaixo do limiar de alerta).
- **LED Azul**: Indica níveis de ruído médios.
- **LED Vermelho**: Indica níveis de ruído altos.
- **Buzzer**: Emite um alerta sonoro quando o nível de ruído atinge níveis extremos.
- **Display OLED**: Exibe informações sobre os níveis de ruído e os limiares configurados.

## Componentes Utilizados
- Microfone
- LEDs (Verde, Azul, Vermelho)
- Buzzer
- Display OLED
- Botões (A e B)
- Raspberry Pi Pico

## Configuração de Pinos
- **BUZZER_A**: GPIO21
- **BUZZER_B**: GPIO10
- **BUTTON_A**: GPIO5
- **BUTTON_B**: GPIO6
- **LED_RED**: GPIO13
- **LED_BLUE**: GPIO12
- **LED_GREEN**: GPIO11
- **MIC_ADC**: GPIO28 (ADC2)
- **I2C_SDA**: GPIO14
- **I2C_SCL**: GPIO15

## Instruções de Uso
1. **Configuração Inicial**:
    - Conecte os componentes aos pinos correspondentes no Raspberry Pi Pico.
    - Certifique-se de que o display OLED está conectado corretamente ao barramento I2C.

2. **Calibração do Ruído Ambiente**:
    - Ao iniciar o sistema, ele realizará uma calibração automática do ruído ambiente. O valor médio do ruído será utilizado como referência.

3. **Operação**:
    - O sistema monitorará continuamente os níveis de ruído.
    - Os LEDs e o buzzer serão acionados conforme os níveis de ruído detectados:
        - **LED Verde**: Níveis de ruído abaixo do limiar médio.
        - **LED Azul**: Níveis de ruído médios.
        - **LED Vermelho**: Níveis de ruído altos.
        - **Buzzer**: Níveis de ruído extremos.

4. **Botões**:
    - **Botão A**: Ativa/desativa manualmente o buzzer.
    - **Botão B**: Entra no modo BOOTSEL.

## Exibição no Display OLED
- **ADC**: Valor bruto do ADC.
- **dB SPL**: Nível de pressão sonora em dB SPL.
- **Medio**: Limiar médio configurado.
- **Alto**: Limiar alto configurado.
- **Extremo**: Limiar extremo configurado.

## Referência
Este projeto foi baseado no repositório [BitDogLab-C](https://github.com/BitDogLab/BitDogLab-C.git).


