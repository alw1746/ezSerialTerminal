#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- SineWaveDisplay Class ---

// Waveform Types
#define WAVE_SINE     0
#define WAVE_SQUARE   1
#define WAVE_SAW      2

class WaveDisplay {
  private:
    Adafruit_SSD1306* _display;
    float _frequency;
    float _amplitude;
    float _timeWindow; 
    int   _waveform; // Stores 0, 1, or 2

  public:
    WaveDisplay(Adafruit_SSD1306* displayHandler) {
      _display = displayHandler;
      _frequency = 100.0;
      _amplitude = (SCREEN_HEIGHT / 2) - 1;
      _timeWindow = 0.05; // 50ms window
      _waveform = WAVE_SINE; // Default
    }

    // Draw a dotted grid (8 horizontal divs, 4 vertical divs)
void drawGrid() {
      int step = 16; 

      // 1. Draw a solid border around the screen edge
      // This guarantees the Top, Left, Right (127), and Bottom (63) lines exist.
      _display->drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
/*
      // 2. Draw Internal Dots
      // Start x at 'step' (16) to avoid drawing over the left border.
      // Stop x before 'SCREEN_WIDTH' to avoid drawing over the right border.
      for (int x = step; x < SCREEN_WIDTH; x += step) {
        for (int y = step; y < SCREEN_HEIGHT; y += step) {
          _display->drawPixel(x, y, WHITE);
        }
      }
*/
      // 3. (Optional) Draw Center Axis Ticks
      int centerX = SCREEN_WIDTH / 2;
      int centerY = SCREEN_HEIGHT / 2;
      int tickSize = 3;

      // Draw ticks on the 4 edges of the box
      _display->drawFastVLine(centerX, 0, tickSize, WHITE); // Top Edge
      _display->drawFastVLine(centerX, SCREEN_HEIGHT - tickSize, tickSize, WHITE); // Bottom Edge
      _display->drawFastHLine(0, centerY, tickSize, WHITE); // Left Edge
      _display->drawFastHLine(SCREEN_WIDTH - tickSize, centerY, tickSize, WHITE); // Right Edge
      
      // Center Dot
      //_display->drawPixel(centerX, centerY, WHITE);

      // Draw Center Axes (Solid lines, but dashed logic can be applied)

      // Draw horizontal center ticks (dashed)
      for (int x = 0; x < SCREEN_WIDTH; x += 4) {
        _display->drawPixel(x, centerY, WHITE); 
      }
      
      // Draw vertical center ticks (dashed)
      for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
        _display->drawPixel(centerX, y, WHITE);
      }

    }

    //Only marks the edges and center with ticks.
    void drawTicks() {
       int centerX = SCREEN_WIDTH / 2;
       int centerY = SCREEN_HEIGHT / 2;
       int tickSize = 3;

       // Center Cross
       _display->drawFastHLine(centerX - tickSize, centerY, tickSize * 2, WHITE);
       _display->drawFastVLine(centerX, centerY - tickSize, tickSize * 2, WHITE);

       // Edge Ticks (every 16 pixels)
       for (int x = 0; x <= SCREEN_WIDTH; x += 16) {
         _display->drawFastVLine(x, SCREEN_HEIGHT - tickSize, tickSize, WHITE); // Bottom ticks
         _display->drawFastVLine(x, 0, tickSize, WHITE); // Top ticks
       }
       for (int y = 0; y <= SCREEN_HEIGHT; y += 16) {
         _display->drawFastHLine(0, y, tickSize, WHITE); // Left ticks
         _display->drawFastHLine(SCREEN_WIDTH - tickSize, y, tickSize, WHITE); // Right ticks
       }
    }
    
    void setFrequency(float freq) {
      if (freq < 0) freq = 0; 
      _frequency = freq;
    }

    void setAmplitude(float amp) {
      float maxAmp = (SCREEN_HEIGHT / 2.0) - 1.0;
      if (amp > maxAmp) amp = maxAmp;
      if (amp < 0) amp = 0;
      _amplitude = amp;
    }

    // 0=Sine, 1=Square, 2=Sawtooth
    void setWaveform(int type) {
      if (type < 0 || type > 2) type = 0; // Default to Sine if invalid
      _waveform = type;
    }

    void draw(bool updateScreen = true) {
      int centerY = SCREEN_HEIGHT / 2;
      int oldY = centerY; 
      
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        // 1. Calculate time 't' based on screen position
        float progress = (float)x / (float)SCREEN_WIDTH;
        float t = progress * _timeWindow;
        
        float signalY = 0;

        // 2. Calculate Y based on Waveform Type
        if (_waveform == WAVE_SINE) {
            signalY = _amplitude * sin(2.0 * PI * _frequency * t);
            
        } else if (_waveform == WAVE_SQUARE) {
            // Determine phase (0.0 to 1.0) of the current cycle
            float phase = (_frequency * t);
            phase = phase - (int)phase; // Get fractional part
            
            // First half of cycle is High, second half is Low
            if (phase < 0.5) signalY = _amplitude;
            else signalY = -_amplitude;

        } else if (_waveform == WAVE_SAW) {
            // Sawtooth: Linear ramp from -A to +A
            float phase = (_frequency * t);
            phase = phase - (int)phase; // Get fractional part (0.0 to 1.0)
            
            // Map 0.0->1.0 to -1.0->+1.0, then multiply by Amplitude
            signalY = _amplitude * (2.0 * phase - 1.0);
            
            // Note: Because OLED Y is inverted (0 is top), this draws a 
            // falling sawtooth. To reverse slope, use: (1.0 - 2.0 * phase)
        }

        // 3. Convert to Screen Coordinates (Inverted Y axis)
        int y = centerY - (int)signalY;

        // 4. Draw continuous lines
        // For Square/Saw, we don't want to draw the vertical "jump" line
        // if it spans a large distance (like top to bottom of square wave).
        // Check if the jump is too big (e.g., > half amplitude) and skip line if so.
        bool isJump = abs(y - oldY) > (_amplitude);

        if (x > 0) {
           //if (!isJump || _waveform == WAVE_SINE) {
               _display->drawLine(x - 1, oldY, x, y, WHITE);
           //}
        } else {
           _display->drawPixel(x, y, WHITE);
        }
        
        oldY = y;
      }

      if (updateScreen) {
        _display->display();
      }
    }
};
