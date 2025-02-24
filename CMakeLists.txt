# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define o tipo de placa (Pico ou Pico W)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

# Nome do projeto
project(Monitor_Ruido C CXX ASM)

# Inicializa o Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona o código-fonte do projeto
add_executable(Monitor_Ruido  
    Monitor_Ruido.c
    lib/ssd1306.c   # Certifique-se de que este arquivo exista no diretório 'lib'
    lib/wifi_config.c   # Adicione esta linha
)

# Configuração do nome e versão do programa
pico_set_program_name(Monitor_Ruido "Monitor_Ruido")
pico_set_program_version(Monitor_Ruido "0.1")

# Habilita saída UART e USB
pico_enable_stdio_uart(Monitor_Ruido 1)
pico_enable_stdio_usb(Monitor_Ruido 1)

# Adiciona bibliotecas necessárias para ADC, PWM e I2C
target_link_libraries(Monitor_Ruido 
    pico_stdlib 
    hardware_adc 
    hardware_pwm
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background  # Adicione esta linha
)

# Adiciona diretórios de inclusão para o projeto
target_include_directories(Monitor_Ruido PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    lib
)

# Gera arquivos extras para o Pico
pico_add_extra_outputs(Monitor_Ruido)
