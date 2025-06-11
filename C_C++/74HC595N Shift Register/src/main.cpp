#include <Arduino.h>
#include <map>

#define latchPin D8
#define clockPin D5
#define dataPin D7

class SHIFT_REG
{
public:
  int _data_pin, _clk_pin, _latch_pin = 0;

  enum PIN
  {
    Q0 = 0,
    Q1 = 1,
    Q2 = 2,
    Q3 = 3,
    Q4 = 4,
    Q5 = 5,
    Q6 = 6,
    Q7 = 7,

  };

  SHIFT_REG(int data_pin, int clock_pin, int latch_pin)
  {

    _data_pin = data_pin;
    _clk_pin = clock_pin;
    _latch_pin = latch_pin;
    pinMode(_data_pin, OUTPUT);
    pinMode(_clk_pin, OUTPUT);
    pinMode(_latch_pin, OUTPUT);
    updateOutputState();
  }

  void updateOutputState()
  {
    unsigned char _byte = 0;
    for (size_t i = 0; i < 8; ++i)
    {
      if (_pins_state[i] == 1)
      {
        _byte |= (1 << (8 - 1 - i)); // Set the bit if it's 1
      }
    }
    Serial.print("CONVERTED BYTE:\t");
    Serial.println(_byte, BIN); // Add a newline for readability

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    shiftOut(dataPin, clockPin, MSBFIRST, _byte);
    digitalWrite(latchPin, HIGH);
  }

  void mapPin(String pin_name, SHIFT_REG::PIN pin_num)
  {
    Serial.print("PIN NUM: ");
    Serial.println(pin_num);
    pin_map[pin_name] = pin_num;
  }
  void setPinState(String pinName, bool state)
  {
    Serial.print("Set ");
    Serial.print(pinName);
    Serial.print(" pin state: ");
    Serial.println(state);
    int _pin = pin_map[pinName];
    _pins_state[_pin] = state;
    for (int i = 0; i < 8; i++)
    {
      Serial.print(_pins_state[i]);
    }
    Serial.println();
  }

private:
  int _pins_state[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  std::map<String, int> pin_map;
};

SHIFT_REG LEDS(dataPin, clockPin, latchPin);
void setup()
{
  Serial.begin(9600);
  LEDS.mapPin("PMS_LED", SHIFT_REG::PIN::Q0);
  LEDS.mapPin("DHT_LED", SHIFT_REG::PIN::Q3);
  LEDS.setPinState("PMS_LED", HIGH);
  LEDS.setPinState("DHT_LED", HIGH);
  delay(5000);
  LEDS.updateOutputState();
}

void loop()
{

  delay(5000);
  LEDS.setPinState("PMS_LED", LOW);
  LEDS.updateOutputState();
  delay(3000);
  LEDS.setPinState("DHT_LED", LOW);
  LEDS.updateOutputState();
  delay(3000);
  LEDS.setPinState("PMS_LED", HIGH);
  LEDS.updateOutputState();
  delay(3000);
  LEDS.setPinState("DHT_LED", HIGH);
  LEDS.updateOutputState(); // ! 10010000 doesn't get outputed anymore 🤯
  delay(3000);
}
