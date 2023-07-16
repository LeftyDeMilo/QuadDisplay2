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

#include "QuadDisplayII.h"

const static uint8_t numerals[] =
    { QD_0, QD_1, QD_2, QD_3, QD_4, QD_5, QD_6, QD_7, QD_8, QD_9 };

// Private ************************************************************

void QuadDisplay::beginWrite() {
    digitalWrite(_pinCLK, LOW);
    digitalWrite(_pinSTR, LOW);
}

void QuadDisplay::pulse(uint8_t pin) {
    digitalWrite(pin, HIGH);
    digitalWrite(pin, LOW);
}

void QuadDisplay::setDataInput(uint8_t LEDState) {
    digitalWrite(_pinDI, (LEDState + _commonTerminal) & 0b01);
}

void QuadDisplay::fillDisplay(uint32_t data) {
    if (_useSPI) {
        if (_commonTerminal == COMMON_ANODE) data = ~ data;
        for (uint8_t quad = 0; quad < 4; quad++) {
            SPI.transfer((uint8_t)(data >> (3 - quad) * 8));
        }
    } else {
        for (uint32_t bitmask = 0x80000000; bitmask > 0; bitmask >>= 1)
        {
            data & bitmask ? setDataInput(HIGH)
                           : setDataInput(LOW);
            pulse(_pinCLK);
        }
    }
}

void QuadDisplay::updateDisplay() {
    beginWrite();
    fillDisplay(_displayBuffer);
    endWrite();
}

void QuadDisplay::endWrite() {
    pulse(_pinSTR);
}

// Public *************************************************************

QuadDisplay::QuadDisplay(uint8_t pinSTR
                       , uint8_t commonTerminal)
{
    _commonTerminal = commonTerminal;
    _pinSTR = pinSTR;
    _useSPI = true;
}

QuadDisplay::QuadDisplay(uint8_t pinSTR
                       , bool useSPI
                       , uint8_t commonTerminal)
{
    _commonTerminal = commonTerminal;
    _pinSTR = pinSTR;
    if (useSPI) {
        _useSPI = true;
    } else {
        _pinCLK = SCK;
        _pinDI = MOSI;
        _useSPI = false;
    }
}

QuadDisplay::QuadDisplay(uint8_t pinSTR
                       , uint8_t pinDI
                       , uint8_t pinCLK
                       , uint8_t commonTerminal)
{
    _commonTerminal = commonTerminal;
    _pinSTR = pinSTR;
    _pinDI = pinDI;
    _pinCLK = pinCLK;
}

void QuadDisplay::begin()
{
    if (_useSPI) {
        SPI.begin();
    } else {
        pinMode(_pinDI, OUTPUT);
        pinMode(_pinCLK, OUTPUT);
        digitalWrite(_pinDI, LOW);
        digitalWrite(_pinCLK, LOW);
    }
    pinMode(_pinSTR, OUTPUT);
    digitalWrite(_pinSTR, LOW);
}

void QuadDisplay::end()
{
    digitalWrite(_pinSTR, LOW);
    digitalWrite(_pinDI, LOW);
    digitalWrite(_pinCLK, LOW);
    pinMode(_pinSTR, INPUT);
    pinMode(_pinDI, INPUT);
    pinMode(_pinCLK, INPUT);
}

uint8_t QuadDisplay::getPixelState(uint8_t segmentNo)
{
    return (uint8_t)(_displayBuffer & (0b01 << segmentNo));
}

void QuadDisplay::pushSegment(uint8_t segmentState)
{
    _displayBuffer = (_displayBuffer << 1) + segmentState;
    updateDisplay();
}

void QuadDisplay::displayDigits(uint8_t digit1
                              , uint8_t digit2
                              , uint8_t digit3
                              , uint8_t digit4)
{
    _displayBuffer = (uint32_t)digit1 << 24;
    _displayBuffer += (uint32_t)digit2 << 16;
    _displayBuffer += (uint32_t)digit3 << 8;
    _displayBuffer += (uint32_t)digit4;
    updateDisplay();
}

void QuadDisplay::displaySegments(uint32_t displayBuffer)
{
    _displayBuffer = displayBuffer;
    updateDisplay();
}

void QuadDisplay::displayClear()
{
    displayDigits(QD_NONE, QD_NONE, QD_NONE, QD_NONE);
}

void QuadDisplay::displayInt(int val
                           , bool padZeros
                           , uint8_t dots)
{
    uint8_t fill = padZeros ? QD_0 : QD_NONE;
    uint8_t digits[4] = { fill, fill, fill, fill };
    if (!val) {
        digits[3] = QD_0;
    } else {
        bool negative = val < 0;
        val = abs(val);

        int8_t i;
        for (i = 4; i--;) {
            uint8_t digit = val % 10;
            if (val) {
                digits[i] = numerals[digit];
                val /= 10;
            } else break;
        }
        if (negative) {
            digits[i] = QD_MINUS;
        }
    }
    uint8_t bitmask = 0b1000;
    for (uint8_t i = 0; i < 4; i++) {
        if (dots & bitmask) digits[i] |= QD_DOT;
        bitmask >>= 1;
    }
    displayDigits(digits[0], digits[1], digits[2], digits[3]);
}

void QuadDisplay::displayFloat(float val
                             , uint8_t precision
                             , bool padZeros)
{
    uint8_t dot = 0b0001;
    while (precision) {
        val *= 10;
        --precision;
        dot <<= 1;
    }
    displayInt((int)val, padZeros, dot);
}

void QuadDisplay::displayTemperatureC(int val
                                    , bool padZeros)
{
    uint8_t fill = padZeros ? QD_0 : QD_NONE;
    uint8_t digits[4] = { fill, fill, QD_DEGREE, QD_C };
    if (!val) {
        digits[1] = QD_0;
    } else {
        bool negative = val < 0;
        val = abs(val);

        int8_t i;
        for (i = 2; i--;) {
            uint8_t digit = val % 10;
            if (val) {
                digits[i] = numerals[digit];
                val /= 10;
            } else break;
        }
        if (negative) {
            digits[max(0, (int)i)] = QD_MINUS;
        }
    }
    displayDigits(digits[0], digits[1], digits[2], digits[3]);
}

void QuadDisplay::displayHumidity(int val
                                , bool padZeros)
{
    uint8_t fill = padZeros ? QD_0 : QD_NONE;
    uint8_t digits[4] = { fill, fill, QD_DEGREE, QD_UNDER_DEGREE };
    if (!val) {
        digits[1] = QD_0;
    } else {
        bool negative = val < 0;
        val = abs(val);

        int8_t i;
        for (i = 2; i--;) {
            uint8_t digit = val % 10;
            if (val) {
                digits[i] = numerals[digit];
                val /= 10;
            } else break;
        }
        if (negative) {
            digits[max(0, (int)i)] = QD_MINUS;
        }
    }
    displayDigits(digits[0], digits[1], digits[2], digits[3]);
}

void QuadDisplay::displayScore(int hour
                             , int minute
                             , bool blink)
{
    uint8_t digits[4] = { QD_0, QD_0, QD_0, QD_0 };

    int8_t i;
    for (i = 2; i--;) {
        uint8_t digit = hour % 10;
        digits[i] = numerals[digit];
        hour /= 10;
        if (!hour) {
            break;
        }
    }

    for (i = 4; i--;) {
        uint8_t digit = minute % 10;
        digits[i] = numerals[digit];
        minute /= 10;
        if (!minute) {
            break;
        }
    }

    if (blink) {
        uint32_t chipTime = millis();
        ((chipTime / 500) & 0b01) ? digits[1] &= ~QD_DOT
                                  : digits[1] |= QD_DOT;
    } else {
        digits[1] |= QD_DOT;
    }
    displayDigits(digits[0], digits[1], digits[2], digits[3]);
}

void QuadDisplay::displayIP(uint8_t val)
{
    uint8_t digits[4] = { QD_UNDERSCORE | QD_DOT, QD_0, QD_0, QD_0 };
    int8_t i;
    for (i = 4; i--;) {
        uint8_t digit = val % 10;
        if (val) {
            digits[i] = numerals[digit];
            val /= 10;
        } else break;
    }
    //digits[i] |= QD_DOT;
    displayDigits(digits[0], digits[1], digits[2], digits[3]);
}
