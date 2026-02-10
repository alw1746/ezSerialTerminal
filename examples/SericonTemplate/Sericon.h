#pragma once

#include <ezSerialTerminal.h>
#include <Preferences.h>

typedef void (*Callback)(bool);

class Sericon {
public:
    Sericon(Stream &output,unsigned long &interval);     //constructor injection
    void begin(unsigned long *intervalPtr);              //pointer injection
    void setUserCallback(Callback cb);                   //callback function
    void readSerial();

private:
    // Member functions for commands
    void cmdHelp();
    void cmdPeriodUp();
    void cmdPeriodDn();
    void cmdPeriodPtrUp();
    void cmdPeriodPtrDn();
    void cmdLoadPrefs();
    void cmdSavePrefs();
    void cmdReset();
    void cmdMute();
    void unknownCommand(const char *command);
    void printConsoleChar();
    Callback _userCallback = nullptr;

    // Static wrapper functions for callbacks
    static void cmdHelpWrapper();
    static void cmdPeriodUpWrapper();
    static void cmdPeriodDnWrapper();
    static void cmdPeriodPtrUpWrapper();
    static void cmdPeriodPtrDnWrapper();
    static void cmdLoadPrefsWrapper();
    static void cmdSavePrefsWrapper();
    static void cmdResetWrapper();
    static void cmdMuteWrapper();
    static void unknownCommandWrapper(const char *command);
    static void printConsoleCharWrapper();

    // Member variables
    SerialTerminal term;
    Stream &serport;

    Preferences preferences;
    unsigned long *periodPtr=nullptr;      //pointer injection
    unsigned long &period;                 //constructor injection
    bool tmrState=true;                    //callback function parameter

    static Sericon* _instance;
};