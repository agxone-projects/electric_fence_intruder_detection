#include "Arduino.h"
#include "sendSMS.h"
#include <driver/adc.h>
#include "ringbuffer.h"
#include "FixedQuene.h"

using namespace std;

static int16_t analog;

const int16_t threshold = 4000;
const int16_t disconnect_threshold = 1000;

const int32_t bufferSizeInBytes = 60000;
const int32_t bufferSizeInSamples = bufferSizeInBytes / sizeof(int16_t);

int16_t *buffer1 = (int16_t *)malloc(bufferSizeInBytes);
int16_t *buffer2 = (int16_t *)malloc(bufferSizeInBytes);

static int buffers_processed = 0;

// buffer that contains the Max of each buffer
const int maxBufferSize = 100;
FixedQueue<int16_t, maxBufferSize> maxBuffer;

TaskHandle_t maxCheckerTaskHandle;
void IRAM_ATTR maxChecker(void *param)
{
    SMSModule *smsModule = static_cast<SMSModule *>(param);
    while (1)
    {
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));
        Serial.printf("ulNotificationValue : %d \n", ulNotificationValue);

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
            while (maxBuffer.size() > 0)
            {
                total += maxBuffer.front();
                maxBuffer.pop();
            }
            float mean = total / (float)maxBufferSize;

            if (mean < disconnect_threshold)
            {
                Serial.printf("Electric Fence is Seems to Disconnected! Mean is : %.2f \n", mean);
                // smsModule->sendSMS("ALERT! Electric Fence Disconnected!");
            }
            else if (mean < threshold)
            {
                Serial.printf("Intruder Detected! Mean is : %.2f \n", mean);
                // smsModule->sendSMS("ALERT! Intruder Detected!");
            }
            else
            {
                Serial.printf("Electric fence is fine. Mean is : %.2f \n", mean);
            }
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
        swap(buffer1, buffer2);
        if (buffers_processed == maxBufferSize)
        {
            xTaskNotify(maxCheckerTaskHandle, 2, eSetValueWithOverwrite);
            buffers_processed = 0;
        }
        else
        {
            xTaskNotify(maxCheckerTaskHandle, 1, eSetValueWithOverwrite);
            buffers_processed++;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    SMSModule *smsModule = new SMSModule();
    smsModule->getRecievers();
    smsModule->sendSMS("GSM Modem Initialized!");
    xTaskCreatePinnedToCore(readVoltage, "Voltage Reader", 4096, NULL, 1, &readVoltageTaskHandle, 1);
    xTaskCreatePinnedToCore(maxChecker, "Max Checker", 4096, smsModule, 1, &maxCheckerTaskHandle, 0);
}

void loop()
{
    vTaskDelete(NULL);
}