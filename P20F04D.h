#ifndef P20F04D_H
#define P20F04D_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class P20F04D {
public:
    /**
     * constructor: sets the values of the control pins for the screen
     */
    P20F04D(uint8_t red1pin, uint8_t red2pin, uint8_t green1pin, uint8_t green2pin, 
        uint8_t blue1pin, uint8_t blue2pin, uint8_t latchpin, uint8_t clockpin, uint8_t oepin);
    /**
     * Initializes the internal structures of the screen and clears it
     */
    void begin();
    void clearScreen();
    /**
     * sets the screen to a new array, redrawing the screen.
     *  Colors are the first dimension (r=0,g=1,b=2)
     *  Rows are the second dimension, from top to bottom
     *  Pixels are the bit in the int16 with the MSB on the far left edge
     */
    void setScreen(int new_screen[3][8]);
    /**
     * sets the screen to a new array without controlling the latch.
     *  The user must set the latch.  Doing this allows to control multiple screens on the same circuit
     *  Colors are the first dimension (r=0,g=1,b=2)
     *  Rows are the second dimension, from top to bottom
     *  Pixels are the bit in the int16 with the MSB on the far left edge
     */
    void setScreenNL(int new_screen[3][8]);
    void appendColNR(uint8_t col[3]);
    void appendCol(uint8_t col[3]);
    void redraw();
    void redrawNL();
    void enable();
    void disable();
    uint8_t getBrightness();
    void setBrightness(uint8_t b);
    void shiftBlock(int red1, int red2, int green1, int green2, int blue1, int blue2);
    void blinkAll_2Bytes(int n, int d);
    void testSwapArray(int d);
    void testShiftArray(int d);
    void testScanLines(int d);
    void testDiagonalLines(int d);

private:
    uint8_t red1pin;
    uint8_t red2pin;
    uint8_t green1pin;
    uint8_t green2pin;
    uint8_t blue1pin;
    uint8_t blue2pin;
    uint8_t latchpin;
    uint8_t clockpin;
    uint8_t oepin;

    uint8_t brightness;

    int screen[3][8];
};

#endif