/**
 * This file is based on the one from QuadDisplay2 library.
 * The goal of forking was to invert a bit logic so the ones now turns
 * LEDs on, zeroes — turns off. Also bit order was changed to place the
 * decimal pointer to match up it hardware implementation in Troyka
 * QuadDisplay Module.
 * Fork Author: Ivan Savochenko <isavoch@gmail.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Product page: https://https://amperka.ru/product/troyka-quad-display
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Igor Dementiev <igor@amperka.ru>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __QUAD_DISPLAY__
#define __QUAD_DISPLAY__

#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

#define COMMON_ANODE HIGH
#define COMMON_CATHODE LOW

/**
 *            a
 *         -----
 *     f /     / b
 *      /  g  /
 *      -----
 *  e /     / c
 *   /  d  /
 *   -----    • dp
 *
 *  Let's code the PI number
 *
 *  Bit order: g f e d c b a dp     0b     0x
 *      3.     * . . * * * * *   10011111  9F
 *      1      . . . . * * . .   00001100  0C
 *      4      * * . . * * . .   11001100  CC
 *      2      * . * * . * * .   10110110  B6
 *
 *  "3.142" = 0x9F0CCCB6
 */

#define QD_NONE  0b00000000
#define QD_DOT   0b00000001
#define QD_MINUS 0b10000000

#define QD_UNDERSCORE   0b00010000
#define QD_DEGREE       0b11000110
#define QD_UNDER_DEGREE 0b10111000

#define QD_0 0b01111110
#define QD_1 0b00001100
#define QD_2 0b10110110
#define QD_3 0b10011110
#define QD_4 0b11001100
#define QD_5 0b11011010
#define QD_6 0b11111010
#define QD_7 0b00001110
#define QD_8 0b11111110
#define QD_9 0b11011110

#define QD_A 0b11101110
#define QD_a 0b10111110
#define QD_b 0b11111000
#define QD_C 0b01110010
#define QD_c 0b10110000
#define QD_d 0b10111100
#define QD_E 0b11110010
#define QD_f 0b11100010
#define QD_F QD_f
#define QD_H 0b11101100
#define QD_h 0b11101000
#define QD_I QD_1
#define QD_J 0b00011100
#define QD_K QD_H
#define QD_L 0b01110000
#define QD_M 0b00101010
#define QD_n 0b10101000
#define QD_o 0b10111000
#define QD_O QD_0
#define QD_P 0b11100110
#define QD_r 0b10100000
#define QD_S QD_5
#define QD_t 0b11110000
#define QD_u 0b00111000
#define QD_U 0b01111100
#define QD_Y 0b11011100

class QuadDisplay {
private:
    uint32_t _displayBuffer;
    uint8_t _commonTerminal;
    uint8_t _pinCLK;
    uint8_t _pinDI;
    uint8_t _pinSTR;
    bool _useSPI;
    void beginWrite();
    void pulse(uint8_t pin);
    void setDataInput(uint8_t ledBit);
    void fillDisplay(uint32_t data);
    void updateDisplay();
    void endWrite();

public:
    QuadDisplay(uint8_t pinSTR, uint8_t commonTerminal = COMMON_ANODE);
    QuadDisplay(uint8_t pinSTR, bool useSPI, uint8_t commonTerminal = COMMON_ANODE);
    QuadDisplay(uint8_t pinSTR, uint8_t pinDI, uint8_t pinCLK, uint8_t commonTerminal = COMMON_ANODE);
    void begin();
    void end();
    uint8_t getPixelState(uint8_t segmentNo); // from 0 to 31
    void pushSegment(uint8_t segmentState);
    void displayDigits(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
    void displaySegments(uint32_t displayBuffer);
    void displayClear();
    void displayInt(int val, bool padZeros = false, uint8_t dots = 0x00);
    void displayFloat(float val, uint8_t precision, bool padZeros = false);
    void displayTemperatureC(int val, bool padZeros = false);
    void displayHumidity(int val, bool padZeros = false);
    void displayScore(int hour, int minute, bool blink = false);
    void displayIP(uint8_t val); // to show the last IP number e.g. [_.042]
};

#endif
