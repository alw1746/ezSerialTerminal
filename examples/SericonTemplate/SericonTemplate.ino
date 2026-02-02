/*
  Template sketch for Sericon class. Sericon uses a PC keyboard to emulate switches,
  potentiometers and rotary encoders. It also provides a CLI facility for the mcu.
  Using constructor injection,pointer injection and callback functions, Sericon can
  control external events and update data. This template sketch features:
  - change led blink interval by constructor injection.
  - change led blink interval by pointer injection.
  - enable/disable led blink by callback.

  Dependency libraries:
    ezSerialTerminal
*/
#include <Arduino.h>
#include <Sericon.h>

#define LED_BUILTIN 48            //change to suit

unsigned long timer1=0;          //loop timer
unsigned long period1=1000;      //interval msec
bool timer1Enabled=true;         //toggle on/off
int state=0;

Sericon sericon(Serial,period1);      //constructor injection to control period

//callback function to receive data changes.
void mycallback(bool data) {
  timer1Enabled = !data;             //if data=true, disable timer
}

void setup(void) {
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  delay(1000);
  Serial.println("SericonTemplate v1.0");
  sericon.setUserCallback(mycallback);         //callback function stop/start blink
  sericon.begin(&period1);                     //pointer injection to control period
}

void loop() {
  sericon.readSerial();
  if ((millis() - timer1 > period1) && timer1Enabled) {
    timer1 = millis();
    state = !state;
    digitalWrite(LED_BUILTIN,state);      //blink led
  }
}
