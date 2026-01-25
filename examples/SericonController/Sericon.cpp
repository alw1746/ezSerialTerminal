/*
3 ways to handle external dependencies:
- constructor injection - reference member initialization.
- method injection - pointer parameter
- callback function
*/
#include "Sericon.h"

// Define the static instance pointer
Sericon* Sericon::_instance = nullptr;

// Constructor injection of dependency.
Sericon::Sericon(Stream &output,unsigned long &interval) :
    serport(output),
    term('\r', ' '),
    period(interval)
{
    _instance = this;
}

//pointer injection
void Sericon::begin(bool *timerflag,int *wavetype) {
    timerEnabled=timerflag;
    waveform=wavetype;
    start();
}

void Sericon::begin() {
    start();
}

void Sericon::start()
{
    preferences.begin("Sericon", false);
    cmdLoadPrefs(); // load initial preferences

    term.addCommand("\\load", cmdLoadPrefsWrapper);
    term.addCommand("\\save", cmdSavePrefsWrapper);
    term.addCommand("\\period", cmdPeriodStepWrapper);
    term.addCommand("\\freq", cmdFreqStepWrapper);
    term.addCommand("\\amp", cmdAmpStepWrapper);
    term.addCommand("\\reset", cmdResetWrapper);
    term.addCommand("w", cmdPeriodUpWrapper);
    term.addCommand("s", cmdPeriodDnWrapper);
    term.addCommand("e", cmdFreqUpWrapper);
    term.addCommand("d", cmdFreqDnWrapper);
    term.addCommand("r", cmdAmpUpWrapper);
    term.addCommand("f", cmdAmpDnWrapper);
    term.addCommand("m", cmdMuteWrapper);
    term.addCommand(" ", cmdWaveWrapper);
    term.addCommand("?", cmdHelpWrapper);
    term.setDefaultHandler(unknownCommandWrapper);
    term.setSerialEcho(true);
    term.setPostCommandHandler(printConsoleCharWrapper);
    neopixelWrite(RGB_BUILTIN, 0, 0, 0);
    cmdHelp();
    printConsoleChar();
}

void Sericon::readSerial() {
    term.readSerial();
}

void Sericon::setFreqCallback(Callback cb) {
    _freqCallback = cb;
}

void Sericon::setAmpCallback(Callback cb) {
    _ampCallback = cb;
}

// --- Private member command handlers ---

void Sericon::cmdHelp() {
    serport.println();
    serport.println("\\load  load prefs");
    serport.println("\\save  save prefs");
    serport.println("\\period n   period step");
    serport.println("\\freq n   freq step");
    serport.println("\\amp n  amp step");
    serport.println("\\reset  ESP32 reset");
    serport.println("spacebar  waveform");
    serport.println("m  mute LED");
    serport.println("w  period+");
    serport.println("s  period-");
    serport.println("e  freq+");
    serport.println("d  freq-");
    serport.println("r  amp+");
    serport.println("f  amp-");
    serport.println("?  print usage");
    serport.flush();
}

void Sericon::printConsoleChar() {
    serport.print(": ");
}

void Sericon::cmdLoadPrefs() {
    period = preferences.getInt("period", 1000);
    freq = preferences.getFloat("freq", 60.0);
    amp = preferences.getFloat("amp", 30.0);
    serport.print("period:"); serport.print(period);
    serport.print(" freq:"); serport.print(freq);
    serport.print(" amp:"); serport.print(amp);
    serport.print(" wave:"); serport.print(*waveform);
    _freqCallback(freq);
    _ampCallback(amp);
    serport.println();
}

void Sericon::cmdSavePrefs() {
    preferences.putInt("period", period);
    preferences.putFloat("freq", freq);
    preferences.putFloat("amp", amp);
    serport.println("Preferences saved.");
}

void Sericon::unknownCommand(const char *command) {
    serport.print("Unknown command: ");
    serport.println(command);
}

void Sericon::cmdPeriodStep() {
    char* arg = term.getNext();
    if (arg != NULL) {
        if (sscanf(arg, "%d", &periodstep) != 1) {
            serport.println("Invalid step");
        }
    }
}

void Sericon::cmdPeriodUp() {
    period += periodstep;
    if (period > 2000) period = 2000;    //max period
    serport.print(" period:");
    serport.println(period);
}

void Sericon::cmdPeriodDn() {
    period -= periodstep;
    if (period < 0) period = 0;         //min period
    serport.print(" period:");
    serport.println(period);
}

void Sericon::cmdFreqStep() {
    char* arg = term.getNext();
    if (arg != NULL) {
        if (sscanf(arg, "%f", &freqstep) != 1) {
            serport.println("Invalid step");
        }
    }
}

void Sericon::cmdFreqUp() {
    freq += freqstep;
    if (freq > 100.0) freq = 100.0;
    serport.print(" freq:");
    serport.println(freq);
    if (_freqCallback != nullptr) {
        _freqCallback(freq);
    }            
}

void Sericon::cmdFreqDn() {
    freq -= freqstep;
    if (freq < -0.0) freq = 0.0;
    serport.print(" freq:");
    serport.println(freq);
    if (_freqCallback != nullptr) {
        _freqCallback(freq);
    }            
}

void Sericon::cmdAmpStep() {
    char* arg = term.getNext();
    if (arg != NULL) {
        if (sscanf(arg, "%f", &ampstep) != 1) {
            serport.println("Invalid step");
        }
    }
}

void Sericon::cmdAmpUp() {
    amp += ampstep;
    if (amp > 30.0) amp = 30.0;
    serport.print(" amp:");
    serport.println(amp, 4);
    if (_ampCallback != nullptr) {
        _ampCallback(amp);
    }            
}

void Sericon::cmdAmpDn() {
    amp -= ampstep;
    if (amp < 0.0) amp = 0.0;
    serport.print(" amp:");
    serport.println(amp, 4);
    if (_ampCallback != nullptr) {
        _ampCallback(amp);
    }            
}

void Sericon::cmdReset() {
    cmdSavePrefs();
    preferences.end();
    ESP.restart();
}

void Sericon::cmdMute() {
    isMuted = !isMuted;
    if (isMuted) {
        serport.println(" mute on");
        if (timerEnabled != nullptr)
          *timerEnabled=false;
        //neopixelWrite(RGB_BUILTIN, 0, 0, 0);
        digitalWrite(RED, LOW);
        digitalWrite(YLW, LOW);
        digitalWrite(GRN, LOW);
    } else {
        serport.println(" mute off");
        if (timerEnabled != nullptr)
          *timerEnabled=true;
        //neopixelWrite(RGB_BUILTIN, 0, 0, 0);
    }
}

void Sericon::cmdWave() {
    if (waveform != nullptr) {
      (*waveform)++;
      if (*waveform > 2)
        *waveform=0;
      switch (*waveform) {
        case 0:
          serport.println("Sine");
          break;
        case 1:
          serport.println("Square");
          break;
        case 2:
          serport.println("Sawtooth");
          break;
        default:
          break;
      }
    }
}

// --- Static wrapper functions ---

void Sericon::cmdHelpWrapper() { if (_instance) _instance->cmdHelp(); }
void Sericon::cmdMuteWrapper() { if (_instance) _instance->cmdMute(); }
void Sericon::cmdPeriodStepWrapper() { if (_instance) _instance->cmdPeriodStep(); }
void Sericon::cmdPeriodUpWrapper() { if (_instance) _instance->cmdPeriodUp(); }
void Sericon::cmdPeriodDnWrapper() { if (_instance) _instance->cmdPeriodDn(); }
void Sericon::cmdFreqStepWrapper() { if (_instance) _instance->cmdFreqStep(); }
void Sericon::cmdFreqUpWrapper() { if (_instance) _instance->cmdFreqUp(); }
void Sericon::cmdFreqDnWrapper() { if (_instance) _instance->cmdFreqDn(); }
void Sericon::cmdAmpStepWrapper() { if (_instance) _instance->cmdAmpStep(); }
void Sericon::cmdAmpUpWrapper() { if (_instance) _instance->cmdAmpUp(); }
void Sericon::cmdAmpDnWrapper() { if (_instance) _instance->cmdAmpDn(); }
void Sericon::cmdLoadPrefsWrapper() { if (_instance) _instance->cmdLoadPrefs(); }
void Sericon::cmdSavePrefsWrapper() { if (_instance) _instance->cmdSavePrefs(); }
void Sericon::cmdResetWrapper() { if (_instance) _instance->cmdReset(); }
void Sericon::cmdWaveWrapper() { if (_instance) _instance->cmdWave(); }
void Sericon::unknownCommandWrapper(const char *command) { if (_instance) _instance->unknownCommand(command); }
void Sericon::printConsoleCharWrapper() { if (_instance) _instance->printConsoleChar(); }
