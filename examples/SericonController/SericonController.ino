/*
  Test sketch for Sericon class. Sericon uses a PC keyboard to emulate switches,
  potentiometers and rotary encoders. It also provides a CLI facility for the mcu.
  - control RGB led sequence period.
  - control waveform,frequency,amplitude on OLED screen.
  - use Preferences API to remember period,freq,ampl settings.

  Dependency libraries:
    ezSerialTerminal
    Adafruit libs for OLED
  Hardware peripherals:
    RGB led
    128x64 I2C OLED display
*/
#include <Arduino.h>
#include <Sericon.h>
#include <WaveDisplay.h>

unsigned long timer1=0;               //loop timer
unsigned long period1;                //interval msec
bool timer1Enabled=true;              //toggle on/off

uint8_t r=0,g=0,b=0,pin=0;
WaveDisplay wave(&display);          //OLED display declared in WaveDisplay.h
int wavetype=0;
float freq,amp;

Sericon sericon(Serial,period1);      //constructor injection to control LED period

//callback function to receive frequency changes.
void frequency(float data) {
  freq=data;
}

//callback function to receive amplitude changes.
void amplitude(float data) {
  amp=data;
}

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println("SericonController v1.0");
  pinMode(RED,OUTPUT);
  pinMode(YLW,OUTPUT);
  pinMode(GRN,OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(YLW, LOW);
  digitalWrite(GRN, LOW);
  sericon.setFreqCallback(frequency);         //change frequency callback
  sericon.setAmpCallback(amplitude);          //change amplitude callback
  sericon.begin(&timer1Enabled,&wavetype);    //pointer injection for led toggle,waveform type
  pin=RED;
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setRotation(0);
  display.setTextColor(WHITE);
}

void loop() {
  sericon.readSerial();

  if ((millis() - timer1 > period1) && timer1Enabled) {
    timer1 = millis();
    digitalWrite(pin, LOW);
    if (pin==RED) {
      pin=YLW;
    }
    else if (pin==YLW) {
      pin=GRN;
    }
    else if (pin==GRN) {
      pin=RED;
    }
    digitalWrite(pin, HIGH);
  }
  display.clearDisplay();
  wave.setWaveform(wavetype);
  wave.setFrequency(freq);
  wave.setAmplitude(amp); 
  wave.drawGrid();
  wave.drawTicks();
  wave.draw(false);
  display.display();
}
