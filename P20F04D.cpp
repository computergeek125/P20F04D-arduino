#include "P20F04D.h"

P20F04D::P20F04D(uint8_t red1pin, uint8_t red2pin, uint8_t green1pin, uint8_t green2pin, 
    uint8_t blue1pin, uint8_t blue2pin, uint8_t latchpin, uint8_t clockpin, uint8_t oepin) {
    this->red1pin = red1pin;
    this->red2pin = red2pin;
    this->green1pin = green1pin;
    this->green2pin = green2pin;
    this->blue1pin = blue1pin;
    this->blue2pin = blue2pin;
    this->latchpin = latchpin;
    this->clockpin = clockpin;
    this->oepin = oepin;
}
void P20F04D::begin() {
    //set pins to output so you can control the shift register
    pinMode(latchpin, OUTPUT);
    pinMode(clockpin, OUTPUT);
    pinMode(oepin, OUTPUT);
    
    pinMode(red1pin, OUTPUT);
    pinMode(blue1pin, OUTPUT);
    pinMode(green1pin, OUTPUT);
    pinMode(red2pin, OUTPUT);
    pinMode(blue2pin, OUTPUT);
    pinMode(green2pin, OUTPUT);
    digitalWrite(red1pin, LOW);
    digitalWrite(blue1pin, LOW);
    digitalWrite(green1pin, LOW);
    digitalWrite(red2pin, LOW);
    digitalWrite(blue2pin, LOW);
    digitalWrite(green2pin, LOW);
    
    //digitalWrite(OEpin, LOW);
    analogWrite(oepin, brightness);
    //function that blinks all the LEDs
    //gets passed the number of blinks and the pause time
    P20F04D::clearScreen();
}

void P20F04D::clearScreen() {
    int data[3][8];
    for (int c=0; c<3; c++) {
        for (int r=0; r<8; r++) {
            data[c][r] = 0;
        }
    }
    setScreen(data);
}

void P20F04D::setScreen(int new_screen[3][8]) {
    digitalWrite(latchpin, 0);
    setScreenNoLatch(new_screen);
    digitalWrite(latchpin, 1);
}

void P20F04D::setScreenNoLatch(int new_screen[3][8]) {
    for (uint8_t c=0; c<3; c++) {
        for (uint8_t i=0; i<8; i++) {
            screen[c][i] = new_screen[c][i];
        }
        P20F04D::recalc(c);
    }
    P20F04D::redrawNoLatch();
}

void P20F04D::appendCol(uint8_t col[3]) {
    for (uint8_t c=0; c<3; c++) {
        for (uint8_t r=0; r<8; r++) {
            screen[c][r] = screen[c][r] << 1;
            screen[c][r] = screen[c][r] | ((col[c] >> r) & 0x01);
        }
        P20F04D::recalc(c);
    }
    P20F04D::redraw();
}

void P20F04D::recalc(uint8_t c) {
    for (uint8_t i=0; i<2; i++) {
        for (uint8_t j=0; j<4; j++) {
            screen_internal[c][i][j]=0;
        }
        screen_internal[c][i][0] |= (screen[c][3+4*i] & 0x000F);
        screen_internal[c][i][0] |= (screen[c][2+4*i] & 0x000F) << 4;
        screen_internal[c][i][0] |= (screen[c][1+4*i] & 0x000F) << 8;
        screen_internal[c][i][0] |= (screen[c][0+4*i] & 0x000F) << 12;

        screen_internal[c][i][1] |= (screen[c][3+4*i] & 0x00F0) >> 4;
        screen_internal[c][i][1] |= (screen[c][2+4*i] & 0x00F0);
        screen_internal[c][i][1] |= (screen[c][1+4*i] & 0x00F0) << 4;
        screen_internal[c][i][1] |= (screen[c][0+4*i] & 0x00F0) << 8;

        screen_internal[c][i][2] |= (screen[c][3+4*i] & 0x0F00) >> 8;
        screen_internal[c][i][2] |= (screen[c][2+4*i] & 0x0F00) >> 4;
        screen_internal[c][i][2] |= (screen[c][1+4*i] & 0x0F00);
        screen_internal[c][i][2] |= (screen[c][0+4*i] & 0x0F00) << 4;

        screen_internal[c][i][3] |= (screen[c][3+4*i] & 0xF000) >> 12;
        screen_internal[c][i][3] |= (screen[c][2+4*i] & 0xF000) >> 8;
        screen_internal[c][i][3] |= (screen[c][1+4*i] & 0xF000) >> 4;
        screen_internal[c][i][3] |= (screen[c][0+4*i] & 0xF000);
    }
}

void P20F04D::redraw() {
    digitalWrite(latchpin, 0);
    P20F04D::redrawNoLatch();
    digitalWrite(latchpin, 1);
}
void P20F04D::redrawNoLatch() {
    for (int j=3; j>=0; j--) {
        P20F04D::shiftBlock(
            screen_internal[0][0][j],screen_internal[0][1][j],
            screen_internal[1][0][j],screen_internal[1][1][j],
            screen_internal[2][0][j],screen_internal[2][1][j]);
    }
}

uint8_t P20F04D::getBrightness() {
    return brightness;
}

void P20F04D::setBrightness(uint8_t b) {
    brightness = b;
    P20F04D::enable();
}
void P20F04D::disable() {
    analogWrite(oepin,255);
}
void P20F04D::enable() {
    analogWrite(oepin,256-brightness);
}

void P20F04D::shiftBlock(int red1, int red2, int green1, int green2, int blue1, int blue2) {
    // This shifts 16 bits out MSB first, 
    //on the rising edge of the clock,
    //clock idles low

    //internal function setup
    uint8_t red1State;
    uint8_t red2State;
    uint8_t green1State;
    uint8_t green2State;
    uint8_t blue1State;
    uint8_t blue2State;

    //clear everything out just in case to
    //prepare shift register for bit shifting
    digitalWrite(red1pin, 0);
    digitalWrite(red2pin, 0);
    digitalWrite(green1pin, 0);
    digitalWrite(green2pin, 0);
    digitalWrite(blue1pin, 0);
    digitalWrite(blue2pin, 0);
    digitalWrite(clockpin, 0);

    for (int i=15; i>=0; i--)    {
        digitalWrite(clockpin, 0);

        //if the value passed to myDataOut and a bitmask result 
        // true then... so if we are at i=6 and our value is
        // %11010100 it would the code compares it to %01000000 
        // and proceeds to set pinState to 1.
        if ( red1 & (1<<i) ) {
            red1State= 1;
        }
        else {    
            red1State= 0;
        }
        if ( red2 & (1<<i) ) {
            red2State= 1;
        }
        else {    
            red2State= 0;
        }
        
        if ( green1 & (1<<i) ) {
            green1State= 1;
        }
        else {    
            green1State= 0;
        }
        if ( green2  & (1<<i) ) {
            green2State= 1;
        }
        else {    
            green2State= 0;
        }
        
        if ( blue1  & (1<<i) ) {
            blue1State= 1;
        }
        else {    
            blue1State= 0;
        }
        if ( blue2 & (1<<i) ) {
            blue2State= 1;
        }
        else {    
            blue2State= 0;
        }

        //Sets the pin to HIGH or LOW depending on pinState
        digitalWrite(red1pin, red1State);
        digitalWrite(red2pin, red2State);
        digitalWrite(green1pin, green1State);
        digitalWrite(green2pin, green2State);
        digitalWrite(blue1pin, blue1State);
        digitalWrite(blue2pin, blue2State);
        
        //register shifts bits on upstroke of clock pin    
        digitalWrite(clockpin, 1);
        //zero the data pin after shift to prevent bleed through
        digitalWrite(red1pin, 0);
        digitalWrite(red2pin, 0);
        digitalWrite(green1pin, 0);
        digitalWrite(green2pin, 0);
        digitalWrite(blue1pin, 0);
        digitalWrite(blue2pin, 0);
    }
    //stop shifting
    digitalWrite(clockpin, 0);
}


//blinks the whole register based on the number of times you want to 
//blink "n" and the pause between them "d"
//starts with a moment of darkness to make sure the first blink
//has its full visual effect.
void P20F04D::blinkAll_2Bytes(int n, int d) {
    digitalWrite(latchpin, 0);
    shiftBlock(0,0,0,0,0,0);
    digitalWrite(latchpin, 1);
    delay(200);
    for (int x = 0; x < n; x++) {
        digitalWrite(latchpin, 0);
        shiftBlock(0xffff,0xffff,0xffff,0xffff,0xffff,0xffff);
        digitalWrite(latchpin, 1);
        delay(d);
        digitalWrite(latchpin, 0);
        shiftBlock(0,0,0,0,0,0);
        digitalWrite(latchpin, 1);
        delay(d);
    }
}

void P20F04D::testSwapArray(int d) {
    int swap[2][4];
    swap[0][0] = 0xef; //0b11101111
    swap[0][1] = 0xa2; //0b10100010
    swap[0][2] = 0xe4; //0b11100100
    swap[0][3] = 0xaf; //0b10101111
    swap[1][0] = 0x81; //0b10000001
    swap[1][1] = 0x42; //0b01000010
    swap[1][2] = 0x24; //0b00100100
    swap[1][3] = 0x18; //0b00011000
    int i;
    int j;
    int dataL;
    int dataR;
    for (i=0; i<2; i++) {
        dataR=0;
        dataL=0;
        dataR |= (swap[i][3] & 0x000F);
        dataR |= (swap[i][2] & 0x000F) << 4;
        dataR |= (swap[i][1] & 0x000F) << 8;
        dataR |= (swap[i][0] & 0x000F) << 12;
        dataL |= (swap[i][3] & 0x00F0) >> 4;
        dataL |= (swap[i][2] & 0x00F0);
        dataL |= (swap[i][1] & 0x00F0) << 4;
        dataL |= (swap[i][0] & 0x00F0) << 8;
        digitalWrite(latchpin, 0);
        P20F04D::shiftBlock(dataL,dataL,0,0,0,0);
        P20F04D::shiftBlock(dataR,dataR,0,0,0,0);
        digitalWrite(latchpin, 1);
        P20F04D::enable();
        delay(d);
    }
}

void P20F04D::testShiftArray(int d) {
    int dataArray[16];
    //Binary notation as comment
    dataArray[0] = 0xffff; //0b1111111111111111
    dataArray[1] = 0xfffe; //0b1111111111111110
    dataArray[2] = 0xfffc; //0b1111111111111100
    dataArray[3] = 0xfff8; //0b1111111111111000
    dataArray[4] = 0xfff0; //0b1111111111110000
    dataArray[5] = 0xffe0; //0b1111111111100000
    dataArray[6] = 0xffc0; //0b1111111111000000
    dataArray[7] = 0xff80; //0b1111111110000000
    dataArray[8] = 0xff00; //0b1111111100000000
    dataArray[9] = 0xfe00; //0b1111111000000000
    dataArray[10] = 0xfc00; //0b1111110000000000
    dataArray[11] = 0xf800; //0b1111100000000000
    dataArray[12] = 0xf000; //0b1111000000000000
    dataArray[13] = 0xe000; //0b1110000000000000
    dataArray[14] = 0xc000; //0b1100000000000000
    dataArray[15] = 0x8000; //0b1000000000000000
    for (uint8_t j = 0; j < 16; j++) {
        //load the light sequence you want from array
        int data = dataArray[j];
        //ground latchpin and hold low for as long as you are transmitting
        digitalWrite(latchpin, 0);
        //move 'em out
        //P20F04D::disable();
        P20F04D::shiftBlock(data, ~data, ~data, data, 0, 0);
        //P20F04D::enable();
        //return the latch pin high to signal chip that it 
        //no longer needs to listen for information
        digitalWrite(latchpin, 1);
        delay(d);
    }
}

void P20F04D::testScanLines(int d) {
    int line = 0xFFFF;
    int data[3][8];
    for (int i=0; i<3; i++) {
        for (int j=0; j<8; j++) {
            data[i][j] = line;
            P20F04D::setScreen(data);
            data[i][j] = 0;
            delay(d);
        }
    }
}

void P20F04D::testDiagonalLines(int d) {
    for (int c=0; c<3; c++) {
        uint8_t col[3] = {0,0,0};
        for (uint8_t i=0; i<8; i++) {
            col[c] = 0x01 << i;
            P20F04D::appendCol(col);
            delay(d);
        }
    }
}