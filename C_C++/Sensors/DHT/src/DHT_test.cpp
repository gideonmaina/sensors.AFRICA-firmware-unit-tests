
#include "DHT.h"

//  pin assignments for NodeMCU V2 board
#if defined(ESP8266)
// define pin for one wire sensors
#define ONEWIRE_PIN D7
#endif
#define DHT_TYPE DHT22
DHT dht(ONEWIRE_PIN, DHT_TYPE);

#define DHT_TYPE DHT22

bool sample_now = false;
unsigned long starttime;
unsigned sampling_intervall_ms = 20000;
unsigned long act_milli;
float last_value_DHT_T = -128.0;
float last_value_DHT_H = -1.0;

#define msSince(timestamp_before) (act_milli - (timestamp_before))

static void fetchSensorDHT()
{

    // Check if valid number if non NaN (not a number) will be send.
    last_value_DHT_T = -128;
    last_value_DHT_H = -1;

    int count = 0;
    const int MAX_ATTEMPTS = 5;
    while ((count++ < MAX_ATTEMPTS))
    {
        auto t = dht.readTemperature();
        auto h = dht.readHumidity();
        if (isnan(t) || isnan(h))
        {
            delay(100);
            t = dht.readTemperature(false);
            h = dht.readHumidity();
        }
        if (isnan(t) || isnan(h))
        {
            Serial.println("DHT11/DHT22 read failed");
        }
        else
        {
            last_value_DHT_T = t;
            last_value_DHT_H = h;
            Serial.print("Temperature: ");
            Serial.print(last_value_DHT_T);
            Serial.print("\t");
            Serial.print("Humidity: ");
            Serial.println(last_value_DHT_H);

            break;
        }
    }
}

void setup()
{
    Serial.begin(9600);
    dht.begin();
    starttime = millis();
}

void loop()
{
    act_milli = millis();
    sample_now = msSince(starttime) > sampling_intervall_ms;
    if (sample_now)
    {
        fetchSensorDHT();
        starttime = millis();
    }
}