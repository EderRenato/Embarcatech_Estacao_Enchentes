#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/buzzer.h"
#include "lib/led_matrix.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "lib/sprites.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ADDRESS 0x3C

#define X_AXIS_JOYSTICK 27
#define Y_AXIS_JOYSTICK 26

#define RED_LED 13
#define BLUE_LED 12
#define GREEN_LED 11

#define MAX_WATER_LEVEL 7.0f
#define MIN_WATER_LEVEL 2.0f
#define MAX_RAIN_VOLUME 64.0f

typedef struct
{
    uint16_t water_level;
    uint16_t rain_volume;
} sensors_data_t;

QueueHandle_t xQueueSensorsData;
volatile sensors_data_t g_sensors_data;

int is_rain(void)
{
    float rainVolume = (g_sensors_data.rain_volume/4095.0) * 80.0; // Simula de 0 a 80mm chuva/dia
    if (rainVolume < 15)
    {
        return 0;  // Sem chuva
    } else if (rainVolume < 30)
    {
        return 1;  // Chuva fraca
    } else if (rainVolume < 55)
    {
        return 2;  // Chuva moderada
    } else {
        return 3;  // Chuva forte
    }
}

void vDistributorTask(void *params)
{
    sensors_data_t sensors_data_local;
    while (true)
    {
        if (xQueueReceive(xQueueSensorsData, &sensors_data_local, portMAX_DELAY) == pdTRUE)
        {
            // Atualiza a variável global
            g_sensors_data = sensors_data_local;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

uint16_t read_adc_avg(uint input) {
    adc_select_input(input);
    uint32_t sum = 0;
    for (int i = 0; i < 16; ++i)
        sum += adc_read();
    return sum / 16;
}

static bool check_critical_conditions(const volatile sensors_data_t *g_sensors_data)
{
    float waterLevel = (g_sensors_data->water_level/4095.0) * 10.0; // Simula de 0 a 10m
    if (waterLevel > MAX_WATER_LEVEL || waterLevel < MIN_WATER_LEVEL)
    {
        return true;
    }
    return false;
}


void vSensorsTask(void *params)
{
    adc_gpio_init(Y_AXIS_JOYSTICK);
    adc_gpio_init(X_AXIS_JOYSTICK);
    adc_init();

    sensors_data_t sensors_data;

    while (true)
    {
        
        sensors_data.rain_volume = read_adc_avg(1); // GPIO 26 = ADC0

        sensors_data.water_level = read_adc_avg(0); // GPIO 27 = ADC1

        xQueueSend(xQueueSensorsData, &sensors_data, 0); // Envia o valor do joystick para a fila
        vTaskDelay(pdMS_TO_TICKS(100));              // 10 Hz de leitura
    }
}

void vDisplayTask(void *params)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADDRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    bool cor = true;
    while (true)
    {
        float rainVolume = (g_sensors_data.rain_volume/4095.0) * 80.0; // Simula de 0 a 80mm chuva/dia
        float waterLevel = (g_sensors_data.water_level/4095.0) * 10.0; // Simula de 0 a 10m
        char buffer1[16];
        char buffer2[16];
        snprintf(buffer1, sizeof(buffer1), "%.2f", rainVolume);
        snprintf(buffer2, sizeof(buffer2), "%.2f", waterLevel);
        strncat(buffer1, "mm3", sizeof(buffer1) - strlen(buffer1) - 1);
        strncat(buffer2, "m", sizeof(buffer2) - strlen(buffer2) - 1);
        ssd1306_fill(&ssd, !cor);     // Limpa a tela
        ssd1306_draw_string(&ssd, "Volume Chuva", 5, 10);
        ssd1306_draw_string(&ssd, buffer1, 5, 25);
        ssd1306_draw_string(&ssd, "Nivel Agua", 5, 40);
        ssd1306_draw_string(&ssd, buffer2, 5, 55);
        ssd1306_send_data(&ssd);
    }
}

void vAlarmTask(void *params)
{
    init_buzzer_pwm(BUZZER_A);
    init_buzzer_pwm(BUZZER_B);
    while (true)
    {
        float rainVolume = (g_sensors_data.rain_volume/4095.0) * 80.0; // Simula de 0 a 80mm chuva/dia
        if (check_critical_conditions(&g_sensors_data))
        {
            play_alarm_critic();
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        else if (rainVolume > MAX_RAIN_VOLUME)
        {
            play_alarm_rain();
            vTaskDelay(pdMS_TO_TICKS(600));
        }
        else
        {
            stop_buzzer(BUZZER_A);
            stop_buzzer(BUZZER_B);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vLedsTask(void *params)
{
    // Configura os LEDs como saída digital
    gpio_init(GREEN_LED);
    gpio_init(RED_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_set_dir(RED_LED, GPIO_OUT);
    
    // Desliga ambos LEDs inicialmente
    gpio_put(GREEN_LED, 0);
    gpio_put(RED_LED, 0);

    while (true)
    {
        float waterLevel = (g_sensors_data.water_level / 4095.0f) * 10.0f; // Converte para 0-10m
        
        if (waterLevel >= 3.0f && waterLevel <= 6.0f) {
            // Nível normal - Verde
            gpio_put(GREEN_LED, 1);  // Verde ligado
            gpio_put(RED_LED, 0);    // Vermelho desligado
        }
        else if ((waterLevel > 6.0f && waterLevel <= 7.0f) || (waterLevel >= 2.0f && waterLevel < 3.0f)) {
            // Nível de atenção - Amarelo (ambos ligados)
            gpio_put(GREEN_LED, 1);  // Verde ligado
            gpio_put(RED_LED, 1);    // Vermelho ligado
        }
        else {
            // Nível crítico - Vermelho
            gpio_put(GREEN_LED, 0);  // Verde desligado
            gpio_put(RED_LED, 1);    // Vermelho ligado
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Atualiza a cada 100ms
    }
}

void vLedMatrixTask(void *params)
{
    // Inicializa o PIO para controle dos LEDs
    npInit(MATRIX_PIN);
    
    while (true)
    {
        npClear();
        
        // Obter intensidade da chuva
        int rain_intensity = is_rain();
        
        // Debug - imprimir informações úteis
        printf("Rain intensity: %d, Rain volume: %d\n", rain_intensity, g_sensors_data.rain_volume);
        
        // Certifique-se de que o valor está dentro da faixa válida
        if (rain_intensity >= 0 && rain_intensity < 4) {
            // Atualizar a matriz de LED
            for(int linha = 0; linha < 5; linha++) {
                for(int coluna = 0; coluna < 5; coluna++) {
                    // Transformação para corrigir a rotação de 90° anti-horário
                    // Aqui aplicamos a rotação nos índices usados para acessar o sprite,
                    // em vez de modificar a função getIndex
                    int sprite_linha = coluna;
                    int sprite_coluna = 4 - linha;
                    
                    // Usar a função getIndex original
                    int posicao = getIndex(linha, coluna);
                    
                    // Verificar se a posição calculada é válida
                    if (posicao >= 0 && posicao < LED_COUNT) {
                        npSetLED(posicao, 
                                RAIN_VOLUME[rain_intensity][sprite_linha][sprite_coluna][0], 
                                RAIN_VOLUME[rain_intensity][sprite_linha][sprite_coluna][1], 
                                RAIN_VOLUME[rain_intensity][sprite_linha][sprite_coluna][2]);
                    }
                }
            }
        }
        
        // Escrever para a matriz de LED
        npWrite();
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main()
{
    stdio_init_all();

    // Cria a fila para compartilhamento de valor do joystick
    xQueueSensorsData = xQueueCreate(5, sizeof(sensors_data_t));

    // Criação das tasks
    xTaskCreate(vDistributorTask, "Distributor Task", 256, NULL, 3, NULL);
    xTaskCreate(vSensorsTask, "Joystick Task", 256, NULL, 3, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 512, NULL, 1, NULL);
    xTaskCreate(vAlarmTask, "Alarm Task", 256, NULL, 1, NULL);
    xTaskCreate(vLedsTask, "LEDs Task", 256, NULL, 1, NULL);
    xTaskCreate(vLedMatrixTask, "LED Matrix Task", 512, NULL, 2, NULL);
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}
