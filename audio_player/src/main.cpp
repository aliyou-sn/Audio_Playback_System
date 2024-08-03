#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <config.h>
#include <I2SOutput.h>
#include <DACOutput.h>
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#include "minimp3.h"

#include "w25qxx.h"

extern "C"
{
  void app_main();
}

static const char TAG[] = "main";

w25qxx_t dev;


void wait_for_button_push()
{
  while (gpio_get_level(GPIO_BUTTON) == 1)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

const int BUFFER_SIZE = 1024;


void play_task(void *param)
{

  Output *output = new I2SOutput(I2S_NUM_0, i2s_speaker_pins);


  gpio_set_direction(GPIO_BUTTON, GPIO_MODE_INPUT);
  
  short *pcm = (short *)malloc(sizeof(short) * MINIMP3_MAX_SAMPLES_PER_FRAME);
  uint8_t *input_buf = (uint8_t *)malloc(BUFFER_SIZE);
  if (!pcm)
  {
    ESP_LOGE("main", "Failed to allocate pcm memory");
  }
  if (!input_buf)
  {
    ESP_LOGE("main", "Failed to allocate input_buf memory");
  }
  while (true)
  {
    // mp3 decoder state
    mp3dec_t mp3d = {};
    mp3dec_init(&mp3d);
    mp3dec_frame_info_t info = {};
    // keep track of how much data we have buffered, need to read and decoded
    int to_read = BUFFER_SIZE;
    int buffered = 0;
    int decoded = 0;
    bool is_output_started = false;
    uint32_t current_addr = 0x000000;

    uint32_t SONG_1_SIZE = 4280993;

    while (1)
    {

      // if (gpio_get_level(GPIO_BUTTON) == 0) {
            // esp_restart();
        // }

      size_t n =  w25qxx_read(&dev, current_addr, input_buf + buffered, to_read);




      // feed the watchdog
      vTaskDelay(pdMS_TO_TICKS(1));
      ESP_LOGI("main", "Read address %" PRIu32 "\n", current_addr);

      current_addr += n;
      buffered += n;
      if (current_addr == SONG_1_SIZE || buffered == 0)
      {
        // we've reached the end of the file and processed all the buffered data
        output->stop();
        is_output_started = false;
        break;
      }
      // decode the next frame
      int samples = mp3dec_decode_frame(&mp3d, input_buf, buffered, pcm, &info);
      // we've processed this may bytes from teh buffered data
      buffered -= info.frame_bytes;
      // shift the remaining data to the front of the buffer
      memmove(input_buf, input_buf + info.frame_bytes, buffered);
      // we need to top up the buffer from the file
      to_read = info.frame_bytes;
      if (samples > 0)
      {
        // if we haven't started the output yet we can do it now as we now know the sample rate and number of channels
        if (!is_output_started)
        {
          output->start(info.hz);
          is_output_started = true;
        }
        // if we've decoded a frame of mono samples convert it to stereo by duplicating the left channel
        // we can do this in place as our samples buffer has enough space
        if (info.channels == 1)
        {
          for (int i = samples - 1; i >= 0; i--)
          {
            pcm[i * 2] = pcm[i];
            pcm[i * 2 - 1] = pcm[i];
          }
        }
        // write the decoded samples to the I2S output
        output->write(pcm, samples);
        // keep track of how many samples we've decoded
        decoded += samples;
      }
      // ESP_LOGI("main", "decoded %d samples\n", decoded);
    }
    ESP_LOGI("main", "Finished\n");
  }
  }

void app_main()
{

	w25qxx_init(&dev);
  xTaskCreatePinnedToCore(play_task, "task", 40000, NULL, 1, NULL, 1);
}