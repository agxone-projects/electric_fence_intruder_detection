#include "Arduino.h"
#include "sendSMS.h"
#include <driver/adc.h>
#include "ringbuffer.h"
#include "FixedQuene.h"

using namespace std;

static int16_t analog;
static int16_t threshold = 4000;

static int32_t bufferSizeInBytes = 60000;
static int32_t bufferSizeInSamples = bufferSizeInBytes / sizeof(int16_t);
int16_t *buffer1 = (int16_t *)malloc(bufferSizeInBytes);
int16_t *buffer2 = (int16_t *)malloc(bufferSizeInBytes);
static int num_buffer_swaps = 0;

const int16_t maxBufferSizeInSamples = 10;

FixedQueue<int16_t, maxBufferSizeInSamples> maxBuffer;

TaskHandle_t maxCheckerTaskHandle;
void IRAM_ATTR maxChecker(void *param)
{
    SMSModule *smsModule = static_cast<SMSModule *>(param);
    // smsModule->getRecievers();
    while (1)
    {
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));
        if (ulNotificationValue == 1)
        {
            int16_t max = 0;
            for (int a = 0; a < bufferSizeInSamples; a++)
            {
                if (buffer2[a] > max)
                {
                    max = buffer2[a];
                }
                maxBuffer.push(max);
            }
        }
        else if (ulNotificationValue == 2)
        {
            long int total = 0;
            FixedQueue<int16_t, maxBufferSizeInSamples> maxBufferCopy = maxBuffer;
            while (maxBufferCopy.size() > 0)
            {
                total += maxBufferCopy.front();
                maxBufferCopy.pop();
            }
            float mean = total / (float)maxBufferSizeInSamples;
            if (mean < threshold)
            {
                Serial.printf("Something is wrong! Mean is : %.2f \n", mean);
                smsModule->sendSMS("Something is wrong!");
            }
            else
            {
                Serial.printf("Electric fence is fine. Mean is : %.2f \n", mean);
            }
        }
        else
        {
            Serial.printf("ulNotificationValue : %d \n", ulNotificationValue);
        }
    }
}

TaskHandle_t readVoltageTaskHandle;
void IRAM_ATTR readVoltage(void *param)
{
    while (1)
    {
        for (int x = 0; x < bufferSizeInSamples; x++)
        {
            analog = analogRead(34);
            buffer1[x] = analog;
        }
        num_buffer_swaps++;
        swap(buffer1, buffer2);
        if (num_buffer_swaps % 10 == 0)
        {
            xTaskNotify(maxCheckerTaskHandle, 2, eSetValueWithOverwrite);
        }
        else
        {
            xTaskNotify(maxCheckerTaskHandle, 1, eSetValueWithOverwrite);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    SMSModule *smsModule = new SMSModule();
    smsModule->getRecievers();
    smsModule->sendSMS("SMS Works!");
    xTaskCreatePinnedToCore(readVoltage, "Voltage Reader", 4096, NULL, 1, &readVoltageTaskHandle, 1);
    xTaskCreatePinnedToCore(maxChecker, "Max Checker", 4096, smsModule, 1, &maxCheckerTaskHandle, 0);
}

void loop()
{
    vTaskDelay(1);
}