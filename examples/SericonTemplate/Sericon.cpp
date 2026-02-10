/*
Universal handler for commands entered on serial terminal keyboard. 
Note: commands must not take up too much CPU as it will impact
Arduino loop() performance. To send results back to caller use:
- constructor injection - reference member initialization.
- pointer injection - function parameter
- callback function
*/
#include "Sericon.h"

// Define the static instance pointer
Sericon* Sericon::_instance = nullptr;

Sericon::Sericon(Stream &output,unsigned long &interval) :
    serport(output),
    term('\r', ' '),
    period(interval)          //constructor injection
{
    _instance = this;
}

//Initialise vars and term object. Called in Arduino setup().
//Each keyboard command is handled by a static function wrapper.
void Sericon::begin(unsigned long *intervalPtr) {
    periodPtr=intervalPtr;    //pointer injection
    preferences.begin("Sericon", false);
    cmdLoadPrefs();
    term.addCommand("\\load", cmdLoadPrefsWrapper);
    term.addCommand("\\save", cmdSavePrefsWrapper);
    term.addCommand("\\reset", cmdResetWrapper);
    term.addCommand("w", cmdPeriodUpWrapper);
    term.addCommand("s", cmdPeriodDnWrapper);
    term.addCommand("e", cmdPeriodPtrUpWrapper);
    term.addCommand("d", cmdPeriodPtrDnWrapper);
    term.addCommand("m", cmdMuteWrapper);
    term.addCommand("?", cmdHelpWrapper);
    term.setDefaultHandler(unknownCommandWrapper);
    term.setSerialEcho(true);
    term.setPostCommandHandler(printConsoleCharWrapper);
    cmdHelp();
    printConsoleChar();
}

void Sericon::setUserCallback(Callback cb) {
    _userCallback = cb;       //callback function
}

//read serial terminal input. Called in Arduino loop().
void Sericon::readSerial() {
    term.readSerial();
}

void Sericon::cmdHelp() {
    serport.println();
    serport.println("\\load  load prefs");
    serport.println("\\save  save prefs");
    serport.println("\\reset  ESP32 reset");
    serport.println("m  mute");
    serport.println("w  period+");
    serport.println("s  period-");
    serport.println("e  periodPtr+");
    serport.println("d  periodPtr-");
    serport.println("?  print usage");
    serport.flush();
}

void Sericon::printConsoleChar() {
    serport.print(": ");
}

void Sericon::cmdLoadPrefs() {
    period = preferences.getLong("period", 1000);
    serport.print("period:"); serport.print(period);
    serport.println();
}

void Sericon::cmdSavePrefs() {
    preferences.putLong("period", period);
    serport.println("Preferences saved.");
}

void Sericon::unknownCommand(const char *command) {
    serport.print("Unknown command: ");
    serport.println(command);
}
    
// --- Serial command handlers ---
//invoked by static wrapper function.

//constructor injection
void Sericon::cmdPeriodUp() {
    period += 50;
    if (period > 2000) period = 2000;
    serport.print(" period:");
    serport.println(period);
}

//constructor injection
void Sericon::cmdPeriodDn() {
    period -= 50;
    if (period < 0) period = 0;
    serport.print(" period:");
    serport.println(period);
}

//pointer injection
void Sericon::cmdPeriodPtrUp() {
    if (periodPtr != nullptr) {
        *periodPtr += 50;
        if (*periodPtr > 2000) *periodPtr = 2000;
        serport.print(" period:");
        serport.println(*periodPtr);
    }
}

//pointer injection
void Sericon::cmdPeriodPtrDn() {
    if (periodPtr != nullptr) {
        *periodPtr -= 50;
        if (*periodPtr < 0) *periodPtr = 0;
        serport.print(" period:");
        serport.println(*periodPtr);
    }
}

//callback function
void Sericon::cmdMute() {
  tmrState = !tmrState;
  (tmrState) ? serport.println(" timer on") : serport.println(" timer off");
  if (_userCallback != nullptr) {
    _userCallback(tmrState);
  }
}

void Sericon::cmdReset() {
    cmdSavePrefs();
    preferences.end();
    ESP.restart();
}

// --- Static wrapper functions ---

void Sericon::cmdHelpWrapper() { if (_instance) _instance->cmdHelp(); }
void Sericon::cmdMuteWrapper() { if (_instance) _instance->cmdMute(); }
void Sericon::cmdPeriodUpWrapper() { if (_instance) _instance->cmdPeriodUp(); }
void Sericon::cmdPeriodDnWrapper() { if (_instance) _instance->cmdPeriodDn(); }
void Sericon::cmdPeriodPtrUpWrapper() { if (_instance) _instance->cmdPeriodPtrUp(); }
void Sericon::cmdPeriodPtrDnWrapper() { if (_instance) _instance->cmdPeriodPtrDn(); }
void Sericon::cmdLoadPrefsWrapper() { if (_instance) _instance->cmdLoadPrefs(); }
void Sericon::cmdSavePrefsWrapper() { if (_instance) _instance->cmdSavePrefs(); }
void Sericon::cmdResetWrapper() { if (_instance) _instance->cmdReset(); }
void Sericon::unknownCommandWrapper(const char *command) { if (_instance) _instance->unknownCommand(command); }
void Sericon::printConsoleCharWrapper() { if (_instance) _instance->printConsoleChar(); }
