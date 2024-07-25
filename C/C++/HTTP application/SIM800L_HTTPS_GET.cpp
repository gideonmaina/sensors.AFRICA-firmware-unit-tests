/**
 * @file SIM800L_HTTPS_GET.cpp
 * @author Gideon Maina
 * @brief Test HTTPS application with SIMCOM's SIM800L GSM module
 * @version 0.1
 * @date 2024-07-25
 *
 * @copyright Copyright (c) 2024
 *
 * This test was performed using a NodeMCU Lolin v3 with a CH340G driver
 *
 */

#include <SoftwareSerial.h>
#define MCU_RXD D5 // GSM TX
#define MCU_TXD D6 // GSM RX
SoftwareSerial GSM_Serial(MCU_RXD, MCU_TXD);

#define MESSAGE_BUFFER_SIZE 513
String url = "https://api.sensors.africa/v1/data";

// Function declarations
void handle_AT_CMD(String cmd);
void read_serial(char *buffer);

void setup()
{

    Serial.begin(9600);
    GSM_Serial.begin(9600);
    delay(5000); // Allow some delay for you to open the serial monitor

    // !!!! GSM reset may be required to clear any configurations
    // Deactivate GPRS PDP context
    handle_AT_CMD("AT+CIPSHUT");
    handle_AT_CMD("AT+SAPBR=0,1");
    handle_AT_CMD("AT+CGATT=0");

    // Restart the module
    handle_AT_CMD("AT+CFUN=1,1");
    // !!!!!!!!!!!

    // Wait for GSM module to warm up and register to network. Increase delay time if necessary
    delay(20000);
    Serial.println("Initializing..");

    // Check if newtwork is registered
    handle_AT_CMD("AT+CREG?");

    // Attach GPRS Service
    handle_AT_CMD("AT+CGATT=1");

    // Bearer settings
    handle_AT_CMD("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
    handle_AT_CMD("AT+SAPBR=1,1");
    // ! If this unit test fails, attempt to configure APN settings

    // HTTP GET
    handle_AT_CMD("AT+HTTPINIT");
    handle_AT_CMD("AT+HTTPSSL=1");
    handle_AT_CMD("AT+HTTPPARA=CID,\"1\"");
    handle_AT_CMD("AT+HTTPPARA=URL,\"" + url + "\"");
    handle_AT_CMD("AT+HTTPACTION=0");
    delay(5000); // * delay to get response from the query
    handle_AT_CMD("AT+HTTPREAD");
    handle_AT_CMD("AT+HTTPTERM");
}

void loop()
{
    // In case you stll want to interact with module after the test
    if (Serial.available() > 0)
    {
        handle_AT_CMD(Serial.readString());
    }
}

/**
 * @brief - send commands to GSM serial and wait for response
 * @param cmd: AT command string
 */
void handle_AT_CMD(String cmd)
{

    char gsm_stream[MESSAGE_BUFFER_SIZE];
    unsigned int timeout = 5000;
    int send_time = millis();
    GSM_Serial.println(cmd);

    if (!(GSM_Serial.available() > 0))
    {
        while (millis() - send_time < timeout)
        {

            if (GSM_Serial.available() > 0)
            {
                // Serial.print("Serial available in timeout loop");
                read_serial(gsm_stream);

                break;
            }
        }
    }

    else
    {
        read_serial(gsm_stream);
    }

    Serial.println();
    Serial.print("GSM RESPONSE TO: ");
    Serial.println(cmd);
    Serial.println("-------");
    Serial.print(gsm_stream);
    Serial.println("-----");
}

/**
 * @brief - read GSM serial data and put it in a buffer
 * @param buffer: pointer char array
 */
void read_serial(char *buffer)
{
    bool bufferfull = false;
    int buff_pos = 0;

    while (GSM_Serial.available() > 0 && !bufferfull)
    {
        unsigned char c = GSM_Serial.read();
        // Serial.write(c);
        if (buff_pos == MESSAGE_BUFFER_SIZE - 1)
        {
            bufferfull = true;
            Serial.println("\nBuffer full");
            break;
        }
        buffer[buff_pos] = c;
        buff_pos++;
        delay(1); // ? This seems to be the trick to get all serial data if it comes in chunks
    }

    buffer[buff_pos] = '\0';
}
