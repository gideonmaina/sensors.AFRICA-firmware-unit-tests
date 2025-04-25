#include <Arduino.h>
#include <TimeLib.h>
#include <ESP32Time.h>

ESP32Time rtc; // Create an instance of the ESP32Time class

void setESP32Time();
struct tm timeinfo;
void setup()
{
  Serial.begin(9600);
  delay(5000);
  setESP32Time();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(10000);

  Serial.println(rtc.getTime("%Y-%m-%dT%H:%M:%S")); // YYYY-MM-DDThh:mm:ss+HH:MM
}

void setESP32Time()
{
  // Set the time to a fixed value
  tmElements_t tm;
  tm.Hour = 23; // 11 PM
  tm.Minute = 59;
  tm.Second = 0;
  tm.Day = 1;   // 1st day of the month
  tm.Month = 1; // January
  tm.Year = 53; // Year 2023 (2023 - 1970 = 53)

  time_t t = makeTime(tm);
  Serial.print("Time: ");
  Serial.println(t);
  rtc.setTime(t); // Set time to 23:59:00 on January 1, 2023
  Serial.print("Current hour: ");
  Serial.print("\t");
  Serial.print(hour(t));
  Serial.print("\t");
  Serial.println(rtc.getHour());
}