# 🌧️ Sistema de Monitoramento de Enchentes Inteligente

Um sistema embarcado baseado em **Raspberry Pi Pico** e **FreeRTOS** para monitoramento de **nível de água** e **volume de chuva**, com **alertas sonoros e visuais**, ideal para aplicações em reservatórios, rios e áreas de risco de alagamento.

---

## 🎯 Objetivo

Desenvolver um sistema capaz de:
- Medir o **nível da água** e o **volume de chuva** (simulados por potenciômetros).
- Exibir os dados em um **display OLED (SSD1306)**.
- Acionar **LEDs RGB** para indicar o status da situação.
- Emitir **alertas sonoros com buzzer** em situações críticas.
- Representar a intensidade da chuva com uma **matriz de LEDs 5x5**.
- Operar com **multitarefas em tempo real** usando **FreeRTOS**.

---

## 🧠 Funcionalidades

- 🔎 **Medição**
  - **Volume de chuva:** 0–80 mm³.
  - **Nível da água:** 0–10 metros.

- 🖥️ **Exibição**
  - OLED mostra:
    - Volume de chuva (mm³)
    - Nível da água (m)

- 🚨 **Alertas**
  - **Crítico:** Nível < 2m ou > 7m → buzzer rápido + LED vermelho.
  - **Alerta:** Entre 2–3m ou 6–7m → LED amarelo.
  - **Normal:** Entre 3–6m → LED verde.
  - **Chuva excessiva (>64mm):** buzzer lento + sprite de chuva forte.

- 💡 **LEDs**
  - RGB (GPIOs 11 e 13): Verde, Amarelo (Verde + Vermelho), Vermelho.

- 🌩️ **Matriz de LEDs 5x5**
  - Sprite de sol → sem chuva.
  - Pequena nuvem → chuva fraca.
  - Nuvem grande → chuva moderada.
  - Nuvem com água torrencial → chuva forte.

---

## ⚙️ Requisitos

- Raspberry Pi Pico
- VS Code com extensão do SDK da Pico
- CMake e compilador `arm-none-eabi`
- Biblioteca FreeRTOS compatível
- Biblioteca para display OLED (ex: `ssd1306` via I2C)
- Drivers WS2812B via PIO (para a matriz 5x5)
- Cabos e jumpers

---

## 🚀 Instruções de Clonagem

Clone este repositório com submódulos, se necessário:

```bash
git clone --recursive https://github.com/EderRenato/Embarcatech_Estacao_Enchentes.git
cd sistema-monitoramento-enchentes
```

> Caso tenha esquecido o `--recursive`, execute:
> ```bash
> git submodule update --init --recursive
> ```

---

## 🛠️ Compilação com o SDK do Raspberry Pi Pico

### 1. Instale o SDK e a Extensão da Pico no VS Code

Siga o guia oficial: https://github.com/raspberrypi/pico-setup-windows

### 2. Configure o Caminho do FreeRTOS

No arquivo `CMakeLists.txt`, adicione o caminho para o FreeRTOS:

```cmake
set(FREERTOS_KERNEL_PATH "caminho/para/sua/cópia/do/FreeRTOS-Kernel")
```

Certifique-se de que os headers e `FreeRTOSConfig.h` estejam disponíveis na estrutura correta.

---

### 3. Compile o Projeto

Abra o terminal do VS Code e execute:

```bash
mkdir build
cd build
cmake ..
make -j4
```

O firmware `.uf2` será gerado no diretório `build`.

---

### 4. Grave o Firmware na Pico

1. Pressione e segure o botão **BOOTSEL** da Pico.
2. Conecte o cabo USB.
3. Solte o botão após conexão.
4. Copie o arquivo `.uf2` gerado para o drive que aparecer.

---

## 📁 Estrutura do Projeto

```
📦 sistema-monitoramento-enchentes
 ┣ main.c
 ┣ 📂 lib/
 ┃ ┣ buzzer.c/h
 ┃ ┣ font.h
 ┃ ┣ FreeRTOSConfig.h
 ┃ ┣ led_matrix.c/h
 ┃ ┣ sprites.h
 ┃ ┣ ssd1306.c/h
 ┃ ┗  ws2818b.pio
 ┣ 📜 README.md
 ┣ 📜 CMakeLists.txt
 ┗ 📜 LICENSE
```

---

## 📜 Licença

Este projeto está licenciado sob a **Licença MIT**. Isso significa que você é livre para usar, copiar, modificar e redistribuir este software, desde que preserve os avisos de copyright e a licença.
