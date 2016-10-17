#include <P20F04D.h>
//**************************************************************//
//    Name        : P20F04D Driver
//    Author      : Ryan Smith
//    Date        : 09 Oct 2016        
//    Modified    :                                                                    
//    Version     : 0.0                                                                                         
//    Notes       : Code for running the shift register sign
//                : Inspired by https://github.com/tehniq3/HUB40A-2R1G1B-display/
//                : and https://www.arduino.cc/en/Tutorial/ShiftOut
//****************************************************************
#define DEBUG
#define DEBUG_COMMAND
//#define DEBUG_ECHO

int brightness = 30;

int latchPin = 10;    // latch 
int clockPin = 9;     // clock

// upper row
int redTopPin = 2;     // 1R1 colour
int greenTopPin = 3;     // 1G colour
int blueTopPin = 4;     // 1B colour
// lower row
int redBotPin = 5;     // 2R1 colour
int greenBotPin = 6;     // 2G colour
int blueBotPin = 7;     // 2B colour

int oepin = 11;    //    OE = Output enable terminal
//When OE (active) low, the output drivers are enabled;
//when OE high, all output drivers are turned OFF (blanked). 
// MBI5026 datasheet:

int dataArray[16];
int screenSend[3][8];

P20F04D disp(redTopPin, redBotPin, greenTopPin, greenBotPin, blueTopPin, blueBotPin, latchPin, clockPin, oepin);

bool reading = false;
int command = 0;

void setup() {
    Serial.begin(115200);
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
    disp.begin();
    disp.setBrightness(brightness);
    disp.clearScreen();
    pinMode(13,OUTPUT);
}

void loop() {
    digitalWrite(13,LOW);
    delay(2);
    digitalWrite(13,HIGH);
    delay(2);
    //disp.testSwapArray(400);
    //disp.testShiftArray(200);
    //disp.testScanLines(20);
    //disp.testDiagonalLines(50);
    byteReceive();
}

void byteReceive() {
    Serial.write(0x01);
    while (Serial.available() < 1);
        Serial.write(0x01);
        delay(100);
    reading = false;
    int inbyte = Serial.read();
    byte control = (inbyte & 0xc0) >> 6;
    switch(control) {
        case 0b00: {// control byte
            int command = inbyte & 0x3f;
            byte b2s[2] = {0x02, command};
            #ifdef DEBUG_COMMAND
                Serial.write(b2s,2);
            #endif
            switch(command) {
                case 0x0: { // clear screen
                    disp.clearScreen();
                    break;
                }
                case 0x1: { // redraw
                    disp.redraw();
                    break;
                }
                case 0x2: { // append column
                    while (Serial.available() < 4);
                    uint8_t d2a[3] = {0,0,0};
                    unpack(d2a);
                    disp.appendCol(d2a);
                    break;
                }
                case 0x3: { // append column no redraw
                    uint8_t d2a[3] = {0,0,0};
                    unpack(d2a);
                    disp.appendColNR(d2a);
                    break;
                }
                case 0x4: { // send screen
                    while (Serial.available() < 48);
                    int screen[3][8];
                    for (int c=0; c<3; c++) {
                        for (int r=0; r<8; r++) {
                            byte a = Serial.read();
                            byte b = Serial.read();
                            screen[c][r] = (a << 8) | (b);
                        }
                    }
                    disp.setScreen(screen);
                    break;
                }
                
                default:
                    break;
            }
            break;
        }
        default:
            byte def[4] = {0x03, control};
            Serial.write(def, 2);
    }
}

void unpack(uint8_t* d2a) {
    while (Serial.available() < 4);
    byte bb[4];
    Serial.readBytes(bb,4);
    d2a[0] = ((bb[0] & 0x3f) << 2) | ((bb[1] & 0x30) >> 4);
    d2a[1] = ((bb[1] & 0x0f) << 4) | ((bb[2] & 0x3c) >> 2);
    d2a[2] = ((bb[2] & 0x03) << 6) | (bb[3] & 0x3f);
    byte echo[4] = {0x04, d2a[0], d2a[1], d2a[2]};
    #ifdef DEBUG_ECHO
        Serial.write(echo,4);
    #endif
}
        
