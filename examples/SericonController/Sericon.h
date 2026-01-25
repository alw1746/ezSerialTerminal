#pragma once

#include <ezSerialTerminal.h>
#include <Preferences.h>

typedef void (*Callback)(float);

#define RGB_BUILTIN 21   //Waveshare S3 Zero=21,tenstar robot S3 Supermini=48,WEMOS S3 Mini=47
#define RED 6   //Red led pin
#define YLW 4
#define GRN 5

class Sericon {
public:
    Sericon(Stream &output,unsigned long &interval);
    void begin();
    void begin(bool *timerflag,int *wavetype);
    void readSerial();
    void setFreqCallback(Callback cb);
    void setAmpCallback(Callback cb);

private:
    // Member functions for commands
    void start();
    void cmdHelp();
    void cmdPeriodStep();
    void cmdPeriodUp();
    void cmdPeriodDn();
    void cmdFreqStep();
    void cmdFreqUp();
    void cmdFreqDn();
    void cmdAmpStep();
    void cmdAmpUp();
    void cmdAmpDn();
    void cmdLoadPrefs();
    void cmdSavePrefs();
    void cmdReset();
    void cmdMute();
    void cmdWave();
    void unknownCommand(const char *command);
    void printConsoleChar();
    Callback _freqCallback = nullptr;
    Callback _ampCallback = nullptr;

    // Static wrapper functions for callbacks
    static void cmdHelpWrapper();
    static void cmdPeriodStepWrapper();
    static void cmdPeriodUpWrapper();
    static void cmdPeriodDnWrapper();
    static void cmdFreqStepWrapper();
    static void cmdFreqUpWrapper();
    static void cmdFreqDnWrapper();
    static void cmdAmpStepWrapper();
    static void cmdAmpUpWrapper();
    static void cmdAmpDnWrapper();
    static void cmdLoadPrefsWrapper();
    static void cmdSavePrefsWrapper();
    static void cmdResetWrapper();
    static void cmdMuteWrapper();
    static void cmdWaveWrapper();
    static void unknownCommandWrapper(const char *command);
    static void printConsoleCharWrapper();

    // Member variables
    SerialTerminal term;
    Stream &serport;

    Preferences preferences;
    bool *timerEnabled=nullptr;
    int *waveform=nullptr;
    unsigned long &period;
    int periodstep=50;
    float freqstep=2.0;
    float freq;
    float ampstep=2.0;
    float amp;
    bool isMuted=false;

    static Sericon* _instance;
};