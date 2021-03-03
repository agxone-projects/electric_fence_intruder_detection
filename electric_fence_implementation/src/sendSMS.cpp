#include "sendSMS.h"

using namespace std;
typedef unsigned int uint32;

#define SIM800L_AXP192_VERSION_20200327

// #define DUMP_AT_COMMANDS
#define TINY_GSM_DEBUG SerialMon

#include "utilities.h"

#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

SMSModule::SMSModule()
{
    SerialMon.begin(115200);
    delay(10);
    if (setupPMU() == false)
    {
        Serial.println("Setting power error");
    }
    setupModem();
    SerialAT.begin(230400, SERIAL_8N1, 26, 27);
    delay(6000);
    Serial.println("Initializing modem...");
    modem.restart();
    delay(10000);
    String imei = modem.getIMEI();
    DBG("IMEI:", imei);
}

// gets the reciever's phone number list
void SMSModule::getRecievers()
{
    receivers[0] = "+94719102569";
    receivers[1] = "+94771309225";
    n_recievers = 2;
}

// send SMS to every
void SMSModule::sendSMS(string message)
{
    for (int i = 0; i < n_recievers; i++)
    {
        string receiver = receivers[i];
        Serial.println(receiver.c_str());
        bool res = modem.sendSMS(receiver.c_str(), message.c_str());
        DBG("SMS:", res ? "OK" : "fail");
    }
}
