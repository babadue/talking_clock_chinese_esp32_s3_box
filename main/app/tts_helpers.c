/**
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "string.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include "esp_rom_sys.h"
#include "bsp_board.h"
#include "es7210.h"
#include "es8311.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "bsp_i2c.h"
#include "bsp_storage.h"
#include "rom/gpio.h"

#include "esp32_s3_box_board.h"

#if ((SOC_SDMMC_HOST_SUPPORTED) && (FUNC_SDMMC_EN))
#include "driver/sdmmc_host.h"
#endif /* ((SOC_SDMMC_HOST_SUPPORTED) && (FUNC_SDMMC_EN)) */

#define GPIO_MUTE_NUM   GPIO_NUM_1
#define GPIO_MUTE_LEVEL 1
#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ADC_I2S_CHANNEL 4
static sdmmc_card_t *card;
static i2c_bus_handle_t i2c_bus_handle = NULL;
static const char *TAG = "tts_helpers";
static int s_play_sample_rate = 16000;
static int s_play_channel_format = 1;
static int s_bits_per_chan = 16;
typedef enum {
    CODEC_TYPE_ES7210 = 0,
    CODEC_TYPE_ES8311,
    CODEC_TYPE_ES8388,
    CODEC_TYPE_MAX,
    CODEC_TYPE_NONE = -1,
} codec_type_t;

typedef struct {
    uint8_t dev_addr;
    char *dev_name;
    codec_type_t dev_type;
} codec_dev_t;

static codec_dev_t codec_dev_list[] = {
    { 0x40, "ES7210", CODEC_TYPE_ES7210 },
    { 0x18, "ES8311", CODEC_TYPE_ES8311 },
    { 0x20, "ES8388", CODEC_TYPE_ES8388 },
};

//unique
esp_err_t bsp_codec_adc_init(audio_hal_iface_samples_t sample_rate)
{
    esp_err_t ret_val = ESP_OK;
    audio_hal_codec_config_t cfg = {
        .codec_mode = AUDIO_HAL_CODEC_MODE_ENCODE,
        .adc_input = AUDIO_HAL_ADC_INPUT_ALL,
        .i2s_iface = {
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,
            .fmt = AUDIO_HAL_I2S_NORMAL,
            .mode = AUDIO_HAL_MODE_SLAVE,
            .samples = sample_rate,
        },
    };

    ret_val |= es7210_adc_init(&cfg);
    ret_val |= es7210_adc_config_i2s(cfg.codec_mode, &cfg.i2s_iface);
    ret_val |= es7210_adc_set_gain(ES7210_INPUT_MIC1, GAIN_37_5DB);
    ret_val |= es7210_adc_set_gain(ES7210_INPUT_MIC2, GAIN_37_5DB);
    ret_val |= es7210_adc_set_gain(ES7210_INPUT_MIC3, GAIN_33DB);
    ret_val |= es7210_adc_set_gain(ES7210_INPUT_MIC4, GAIN_37_5DB);
    ret_val |= es7210_adc_ctrl_state(cfg.codec_mode, AUDIO_HAL_CTRL_START);

    if (ESP_OK != ret_val) {
        ESP_LOGE(TAG, "Failed initialize codec");
    }

    return ret_val;
}

//unique
esp_err_t bsp_codec_dac_init(audio_hal_iface_samples_t sample_rate)
{
    esp_err_t ret_val = ESP_OK;
    audio_hal_codec_config_t cfg = {
        .codec_mode = AUDIO_HAL_CODEC_MODE_DECODE,
        .dac_output = AUDIO_HAL_DAC_OUTPUT_LINE1,
        .i2s_iface = {
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,
            .fmt = AUDIO_HAL_I2S_NORMAL,
            .mode = AUDIO_HAL_MODE_SLAVE,
            .samples = sample_rate,
        },
    };

    ret_val |= es8311_codec_init(&cfg);
    ret_val |= es8311_set_bits_per_sample(cfg.i2s_iface.bits);
    ret_val |= es8311_config_fmt(cfg.i2s_iface.fmt);
    ret_val |= es8311_codec_set_voice_volume(74);
    ret_val |= es8311_codec_ctrl_state(cfg.codec_mode, AUDIO_HAL_CTRL_START);
    // ret_val |= es8311_codec_set_clk();

    if (ESP_OK != ret_val) {
        ESP_LOGE(TAG, "Failed initialize codec");
    }

    return ret_val;
}

//unqiue
static esp_err_t bsp_i2s_init(i2s_port_t i2s_num, uint32_t sample_rate, i2s_channel_fmt_t channel_format, i2s_bits_per_chan_t bits_per_chan)
{
    esp_err_t ret_val = ESP_OK;

    i2s_config_t i2s_config = I2S_CONFIG_DEFAULT();

    i2s_pin_config_t pin_config = {
        .bck_io_num = GPIO_I2S_SCLK,
        .ws_io_num = GPIO_I2S_LRCK,
        .data_out_num = GPIO_I2S_DOUT,
        .data_in_num = GPIO_I2S_SDIN,
        .mck_io_num = GPIO_I2S_MCLK,
    };

    ret_val |= i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    ret_val |= i2s_set_pin(i2s_num, &pin_config);

    return ret_val;
}

//unique
static esp_err_t bsp_i2s_deinit(i2s_port_t i2s_num)
{
    esp_err_t ret_val = ESP_OK;

    ret_val |= i2s_stop(I2S_NUM_0);
    ret_val |= i2s_driver_uninstall(i2s_num);

    return ret_val;
}

//unique
static esp_err_t bsp_codec_prob(int *codec_type)
{
    for (size_t i = 0; i < sizeof(codec_dev_list) / sizeof(codec_dev_list[0]); i++) {
        if (ESP_OK == bsp_i2c_probe_addr(codec_dev_list[i].dev_addr)) {
            *codec_type |= 1 << i;
            ESP_LOGI(TAG, "Detected codec at 0x%02X. Name : %s",
                     codec_dev_list[i].dev_addr, codec_dev_list[i].dev_name);
        }
    }

    if (0 == *codec_type) {
        *codec_type = CODEC_TYPE_NONE;
        ESP_LOGW(TAG, "Codec not detected");
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

//unique
static esp_err_t bsp_codec_init(audio_hal_iface_samples_t sample_rate)
{
    esp_err_t ret_val = ESP_OK;

    ret_val |= bsp_codec_adc_init(sample_rate);
    ret_val |= bsp_codec_dac_init(sample_rate);
    return ret_val;
}

//unique
esp_err_t bsp_audio_play(const int16_t* data, int length, TickType_t ticks_to_wait)
{
    size_t bytes_write = 0;
    esp_err_t ret = ESP_OK;
    int out_length= length;
    int audio_time = 1;
    audio_time *= (16000 / s_play_sample_rate);
    audio_time *= (2 / s_play_channel_format);

    int *data_out = NULL;
    if (s_bits_per_chan != 32) {
        out_length = length * 2;
        data_out = malloc(out_length);
        for (int i = 0; i < length / sizeof(int16_t); i++) {
            int ret = data[i];
            data_out[i] = ret << 16;
        }
    }

    int *data_out_1 = NULL;
    if (s_play_channel_format != 2 || s_play_sample_rate != 16000) {
        out_length *= audio_time;
        data_out_1 = malloc(out_length);
        int *tmp_data = NULL;
        if (data_out != NULL) {
            tmp_data = data_out;
        } else {
            tmp_data = data;
        }

        for (int i = 0; i < out_length / (audio_time * sizeof(int)); i++) {
            for (int j = 0; j < audio_time; j++) {
                data_out_1[audio_time * i + j] = tmp_data[i];
            }
        }
        if (data_out != NULL) {
            free(data_out);
            data_out = NULL;
        }
    }

    if (data_out != NULL) {
        ret = i2s_write(I2S_NUM_1, (const char*) data_out, out_length, &bytes_write, ticks_to_wait);
        free(data_out);
    } else if (data_out_1 != NULL) {
        ret = i2s_write(I2S_NUM_1, (const char*) data_out_1, out_length, &bytes_write, ticks_to_wait);
        free(data_out_1);
    } else {
        ret = i2s_write(I2S_NUM_1, (const char*) data_out, length, &bytes_write, ticks_to_wait);
    }

    return ret;
}

//unique
esp_err_t bsp_get_feed_data(int16_t *buffer, int buffer_len)
{
    esp_err_t ret = ESP_OK;
    size_t bytes_read;
    int audio_chunksize = buffer_len / (sizeof(int16_t) * ADC_I2S_CHANNEL);
    ret = i2s_read(I2S_NUM_1, buffer, buffer_len, &bytes_read, portMAX_DELAY);

    for (int i = 0; i < audio_chunksize; i++) {
        int16_t ref = buffer[4 * i + 0];
        buffer[3 * i + 0] = buffer[4 * i + 1];
        buffer[3 * i + 1] = buffer[4 * i + 3];
        buffer[3 * i + 2] = ref;
    }

    return ret;
}

//unique
int bsp_get_feed_channel(void)
{
    return ADC_I2S_CHANNEL;
}

//unique
esp_err_t esp_board_init(audio_hal_iface_samples_t sample_rate, int channel_format, int bits_per_chan)
{
    /*!< Initialize I2C bus, used for audio codec*/
    bsp_i2c_init(I2C_NUM_0, 400 * 1000, GPIO_I2C_SCL, GPIO_I2C_SDA);

    int sample_fre = 16000;
    switch (sample_rate) {
    case AUDIO_HAL_08K_SAMPLES:
        sample_fre = 8000;
        break;
    case AUDIO_HAL_16K_SAMPLES:
        sample_fre = 16000;
        break;
    case AUDIO_HAL_44K_SAMPLES:
        sample_fre = 44100;
        break;
    default:
        ESP_LOGE(TAG, "Unable to configure sample rate %dHz", sample_fre);
        break;
    }
    s_play_sample_rate = sample_fre;

    if (channel_format != 2 && channel_format != 1) {
        ESP_LOGE(TAG, "Unable to configure channel_format");
        channel_format = 2;
    }
    s_play_channel_format = channel_format;

    if (bits_per_chan != 32 && bits_per_chan != 16) {
        ESP_LOGE(TAG, "Unable to configure bits_per_chan");
        bits_per_chan = 32;
    }
    s_bits_per_chan = bits_per_chan;

    bsp_i2s_init(I2S_NUM_1, 16000, I2S_CHANNEL_FMT_RIGHT_LEFT, I2S_BITS_PER_CHAN_32BIT);
    bsp_codec_init(AUDIO_HAL_16K_SAMPLES);
    /* Initialize PA */
    gpio_config_t  io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << GPIO_PWR_CTRL));
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_PWR_CTRL, 1);
    return ESP_OK;
}


// wrappers section
esp_err_t esp_audio_play(const int16_t* data, int length, TickType_t ticks_to_wait)
{
    return bsp_audio_play(data, length, ticks_to_wait);
}

int esp_get_feed_channel(void)
{
    return bsp_get_feed_channel();
}

esp_err_t FatfsComboWrite(const void* buffer, int size, int count, FILE* stream)
{
    esp_err_t res = ESP_OK;
    res = fwrite(buffer, size, count, stream);
    res |= fflush(stream);
    res |= fsync(fileno(stream));

    return res;
}

esp_err_t esp_get_feed_data(int16_t *buffer, int buffer_len)
{
    return bsp_get_feed_data(buffer, buffer_len);
}

esp_err_t esp_sdcard_init(char *mount_point, size_t max_files)
{
    return bsp_sdcard_init(mount_point, max_files);
}

esp_err_t esp_sdcard_deinit(char *mount_point)
{
    return bsp_sdcard_deinit(mount_point);
}

void set_voice_volume(int volume)
{
    esp_err_t err = es8311_codec_set_voice_volume(volume);

}

void mute_codec(bool enable)
{
    esp_err_t err = es8311_set_voice_mute(enable);

}

esp_err_t esp_bsp_codec_init(audio_hal_iface_samples_t sample_rate)
{
    esp_err_t ret = bsp_codec_init(sample_rate);
    return ret;
}