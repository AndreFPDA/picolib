/**
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#ifndef SSD1306I2CPICO_h
#define SSD1306I2CPICO_h

#include "hardware/i2c.h"
#include "string.h"
#include "stdint.h"

#include "OLEDDisplay.h"

class SSD1306I2CPICO : public OLEDDisplay {
	protected:
    uint16_t DevAddr;
		i2c_inst_t * i2c;
		uint8_t width;
		uint8_t height;
		OLEDDISPLAY_GEOMETRY GEOMETRY;
		
		unsigned char * buffer;

		void sendData(uint8_t* buffer, size_t buff_size);
		void sendCommand(uint8_t command);

	public:
		SSD1306I2CPICO(uint16_t const DevAddr, OLEDDISPLAY_GEOMETRY GEOMETRY, i2c_inst_t * i2c);
		~SSD1306I2CPICO();

		void displayON(uint8_t On);
		void invertColors(uint8_t Invert);
		void rotateDisplay(uint8_t Rotate);
		void setContrast(uint8_t Contrast);

		//void drawPixel(int16_t x, int16_t y, OLEDDISPLAY_COLOR Color = OLEDDISPLAY_COLOR::WHITE);
		//void clear(OLEDDISPLAY_COLOR Color = OLEDDISPLAY_COLOR::BLACK);
		//void display(unsigned char *data = nullptr);
		uint8_t getHeight();
		uint8_t getWidth();
};

#endif