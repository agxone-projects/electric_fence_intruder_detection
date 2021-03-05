#include "Arduino.h"
#include "sendSMS.h"
#include <driver/adc.h>
#include "ringbuffer.h"
#include "FixedQuene.h"

using namespace std;

static int16_t analog;
const int16_t threshold = 4000;
const int16_t num_samples = 30000;

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
    }
}

TaskHandle_t readVoltageTaskHandle;
void IRAM_ATTR readVoltage(void *param)
{
    int num_max = 0;
    while (1)
    {
        int16_t max = 0;
        for (int x = 0; x < num_samples; x++)
        {
            analog = analogRead(34);
            if (max < analog)
            {
                max = analog;
            }
        }
        maxBuffer.push(max);
        num_max++;
        if (num_max == maxBufferSizeInSamples)
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
    smsModule->sendSMS("GSM Module Initiated!");
    xTaskCreatePinnedToCore(readVoltage, "Voltage Reader", 4096, NULL, 1, &readVoltageTaskHandle, 1);
    xTaskCreatePinnedToCore(maxChecker, "Max Checker", 4096, smsModule, 1, &maxCheckerTaskHandle, 0);
}

void loop()
{
    vTaskDelay(1);
}