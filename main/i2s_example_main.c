/* I2S Example

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include <math.h>


#define SAMPLE_RATE     (44000)
#define WAVE_FREQ_HZ    (1000)

#define I2S_NUM         (0)
#define PI 3.14159265

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

int *samples_data;

static void setup_triangle_sine_waves()
{
    const int bits = 16;
    unsigned int i, sample_val;
    double sin_float;

    samples_data = malloc(((bits+8)/16)*SAMPLE_PER_CYCLE*4);

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits+8)/16)*SAMPLE_PER_CYCLE*4);
    printf("SAMPLE_PER_CYCLE=%d\n", SAMPLE_PER_CYCLE);

    for(i = 0; i < SAMPLE_PER_CYCLE; i++)
    {
        sin_float = sin(i * 2 * PI * WAVE_FREQ_HZ / SAMPLE_RATE ); /// 180.0

        sin_float *= 0x7fff;
        //sin_float += 0x7fff;

        ushort sample = (ushort)sin_float;
        //sample = sample << 8;

        printf("sin_float=%f, sample=0x%04x (%d)\n", sin_float, sample, (short)sample);


        sample_val = (unsigned short)sample;
        sample_val = sample_val << 16;
        sample_val |= (unsigned short)sample;
        samples_data[i] = sample_val;
    }

    i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);
    //Using push
    // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
    //     if (bits == 16)
    //         i2s_push_sample(0, &samples_data[i], 100);
    //     else
    //         i2s_push_sample(0, &samples_data[i*2], 100);
    // }
    // or write
    //i2s_write_bytes(I2S_NUM, (const char *)samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, 100);

    //free(samples_data);
}

void app_main()
{

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,                                  // Only TX
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .dma_buf_count = 6,
        .dma_buf_len = 60,                                                      //
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

#if 0
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 22,
        .data_in_num = -1                                                       //Not used
    };

    i2s_set_pin(I2S_NUM, &pin_config);
#else
    i2s_set_pin(I2S_NUM, NULL);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
#endif

    int test_bits = 16;
    setup_triangle_sine_waves();
    while (1) {

        //vTaskDelay(5000/portTICK_RATE_MS);
        //test_bits += 8;
        //if(test_bits > 32)
        //    test_bits = 16;
        i2s_write_bytes(I2S_NUM, (const char *)samples_data, ((test_bits+8)/16)*SAMPLE_PER_CYCLE*4, 100);
    }

}
