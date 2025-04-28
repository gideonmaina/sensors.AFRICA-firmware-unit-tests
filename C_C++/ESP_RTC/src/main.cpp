#include <Arduino.h>
#include <TimeLib.h>
#include <ESP32Time.h>

#define TIMEZONE_IN_QUARTERS // Uncomment this line to use timezone in quarters of an hour

ESP32Time rtc; // Create an instance of the ESP32Time class

struct datetimetz
{
  tmElements_t datetime;
  time_t timestamp;
  char timezone[6] = {}; // e.g. +0300 // +03
} esp_datetime_tz;

void setESP32Time();
datetimetz extractDateTime(String datetimeStr);

void setup()
{
  Serial.begin(9600);
  delay(5000);
  setESP32Time();
  String datetimeStr = "25/02/24,07:55:53+48"; // Example datetime string // ! expressed in quarters of an hour, between the local time and GMT
  esp_datetime_tz = extractDateTime(datetimeStr);
  rtc.setTime(esp_datetime_tz.timestamp); // Set the RTC time
  Serial.print("RTC time: ");
  Serial.println(rtc.getTime("%Y-%m-%dT%H:%M:%S") + esp_datetime_tz.timezone); // Print the RTC time
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(10000);
  String time = rtc.getTime("%Y-%m-%dT%H:%M:%S"); // YYYY-MM-DDThh:mm:ss+HH:MM
  Serial.println(time + esp_datetime_tz.timezone);
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

  // rtc.offset = 3600 * 3; // Set timezone offset in seconds (3 hours ahead of UTC)
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

datetimetz extractDateTime(String datetimeStr)
{
  datetimetz dtz;
  dtz.datetime = {0, 0, 0, 0, 0, 0, 0};
  dtz.timestamp = 0;

  Serial.println("Received date string: " + datetimeStr); //! format looks like "25/02/24,05:55:53+00" and may include the quotes!

  // check if received string is empty
  if (datetimeStr == "")
  {
    Serial.println("Datetime string is empty");

    return dtz;
  }

  // check if the datetime string has leading or trailing quotes
  if (datetimeStr[0] == '"', datetimeStr[datetimeStr.length() - 1] == '"')
  {
    // remove the first and last character of the string (")
    datetimeStr = datetimeStr.substring(1, datetimeStr.length() - 1);
  }

  // Parse the datetime string

  int _year = datetimeStr.substring(0, 2).toInt();
  int _month = datetimeStr.substring(3, 5).toInt();
  int _day = datetimeStr.substring(6, 8).toInt();
  int _hour = datetimeStr.substring(9, 11).toInt();
  int _minute = datetimeStr.substring(12, 14).toInt();
  int _second = datetimeStr.substring(15, 17).toInt();

  // perform sanity check on the parsed values
  if (_year < 0 || _year > 99 || _month < 1 || _month > 12 || _day < 1 || _day > 31 ||
      _hour < 0 || _hour > 23 || _minute < 0 || _minute > 59 || _second < 0 || _second > 59)
  {
    Serial.println("Invalid date/time values");
    return dtz;
  }

#if defined(TIMEZONE_IN_QUARTERS)

  // time zone = indicates the difference, expressed in quarters of an hour, between the local time and GMT; range: -48 to +56)
  int tz = datetimeStr.substring(18).toInt() / 4;
  String timezone = datetimeStr.substring(17, 18); // extract timezone sign
  if (tz < 10)
  {
    timezone += "0" + String(tz);
  }
  else
  {
    timezone += String(tz);
  }
#else
  String timezone = datetimeStr.substring(17); // +00

#endif
  strncpy(dtz.timezone, timezone.c_str(), 6); // copy timezone to the provided buffer
  Serial.println("Timezone: " + String(dtz.timezone));

  // Serial.println("Day: " + String(day));
  // Serial.println("Month: " + String(month));
  // Serial.println("Year: " + String(year));
  // Serial.println("Hour: " + String(hour));
  // Serial.println("Minute: " + String(minute));
  // Serial.println("Second: " + String(second));

  // Adjust year for TimeLib (TimeLib expects years since 1970)
  _year += 2000; // Assuming 24 is 2024
  _year -= 1970;

  dtz.datetime.Second = _second;
  dtz.datetime.Minute = _minute;
  dtz.datetime.Hour = _hour;
  dtz.datetime.Day = _day;
  dtz.datetime.Month = _month;
  dtz.datetime.Year = _year;

  // Create a time_t value
  dtz.timestamp = makeTime(dtz.datetime);
  Serial.print("Parsed timestamp: ");
  Serial.println(dtz.timestamp); // Print the time_t value

  return dtz;
}