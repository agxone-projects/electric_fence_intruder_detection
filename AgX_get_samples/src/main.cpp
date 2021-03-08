
#include <Arduino.h>
#include <driver/adc.h>
#include "esp_task_wdt.h"
#include "esp_timer.h"

static int16_t analog;

//static long long int utime1;
//static long long int utime2;
//static int freq;

static int32_t bufferSizeInBytes = 60000;
static int32_t bufferSizeInSamples = bufferSizeInBytes / sizeof(int16_t);
static int32_t bufferPos = 0;

static int buffer1Full = false;

static long long int utime1;
static long long int utime2;
static int freq;

// float_t *buffer1 = (float_t *)malloc(bufferSizeInBytes);
// float_t *buffer2 = (float_t *)malloc(bufferSizeInBytes);
int16_t *buffer1 = (int16_t *)malloc(bufferSizeInBytes);
int16_t *buffer2 = (int16_t *)malloc(bufferSizeInBytes);

TaskHandle_t writerTaskHandle;
//
//static void periodic_timer_callback(void* arg){
//  Serial.println("-----------Time elapsed--------------: ");
//  Serial.println(freq);
//  };

// Task to write samples from ADC to our server
void IRAM_ATTR adcWriterTask(void *param)
{

  // wait for some samples to save
  uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));
  if (ulNotificationValue > 0)
  {

    // wait for user prompt to start sending data
    for (;;)
    {
      if (Serial.available())
      {
        String input = Serial.readString();
        if (input == "start")
        {
          break;
        }
      }
    }

    // print both buffers
    for (int a = 0; a < bufferSizeInSamples; a++)
    {
      Serial.println(buffer2[a]);
    }

    for (int b = 0; b < bufferSizeInSamples; b++)
    {
      Serial.println(buffer1[b]);
    }
    Serial.println("done");

    // delete the task
    vTaskDelete(writerTaskHandle);
  }
}

void setup()
{
  // baud rate of 57600 is needed since pyserial read loop is slow
  Serial.begin(57600);

  psramInit();

  // -------------------- FIX THE WATCHDOG TIMER ISSUE!!!!!!!! ---------------------------
  esp_task_wdt_init(600, false);


  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, NULL, 1, &writerTaskHandle, 0);

  //    const esp_timer_create_args_t periodic_timer_args = {
  //            callback : &periodic_timer_callback
  //
  //    };
  //
  //  esp_timer_handle_t periodic_timer;
  //  esp_timer_create(&periodic_timer_args, &periodic_timer);
  //    /* The timer has been created but is not running yet */
  //  esp_timer_start_periodic(periodic_timer, 500000);

  // Serial.print("Available heap: ");
  // Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  // Serial.print("PSRAM size: ");
  // Serial.println(ESP.getPsramSize());
  // Serial.print("Available largest single allocation: ");
  // Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  // Serial.print("Free PSRAM: ");
  // Serial.println(ESP.getFreePsram());
}

void loop()
{
  //
  analog = analogRead(34);

  // add the sample to the current audio buffer
  buffer1[bufferPos] = analog;
  bufferPos++;

  // have we filled the buffer with data?
  if (bufferPos == bufferSizeInSamples)
  {
    // utime1 = esp_timer_get_time();

    if (!buffer1Full)
    {
      std::swap(buffer1, buffer2);
      bufferPos = 0;
      buffer1Full = true;

      // Timer code
      // utime2 = esp_timer_get_time();
      // freq = utime2 - utime1;
      // Serial.println("Time taken for swap: ");
      // Serial.println(freq);
    }
    else
    {
      xTaskNotify(writerTaskHandle, 1, eIncrement);
      vTaskDelete(NULL);
    }

  }
}
