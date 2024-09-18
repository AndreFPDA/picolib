// #pragma once

#ifndef _oledfx_H
#define _oledfx_H

#include "SSD1306.hpp"
#include "OLED_font.hpp"
#include <string>


class SSD1306_OLEDFonts
{
	public:
		
		SSD1306_OLEDFonts();
		~SSD1306_OLEDFonts(){};

		uint8_t setFont(const uint8_t * font);

		const uint8_t * _FontSelect = pFontDefault; /**< Pointer to the active font,  Fonts Stored are Const */
	
	protected:
		uint8_t _Font_X_Size = 0x06; /**< Width Size of a Font character */
		uint8_t _Font_Y_Size = 0x08; /**< Height Size of a Font character */
		uint8_t _FontOffset = 0x00; /**< Offset in the ASCII table 0x00 to 0xFF, where font begins */
		uint8_t _FontNumChars = 0xFE; /**< Number of characters in font 0x00 to 0xFE */

};

class oledfx : public SSD1306, public SSD1306_OLEDFonts {
    public:
        oledfx(uint16_t const DevAddr, size_display Size, i2c_inst_t * i2c);

        void drawChar(int x, int y, char chr, colors color = colors::WHITE);
        void drawString(int x, int y, std::string str, colors color = colors::WHITE);
        void drawProgressBar(int x, int y, uint16_t w, uint16_t h, uint8_t progress, colors color = colors::WHITE);
        void drawFillRectangle(int x, int y, uint16_t w, uint16_t h, colors color = colors::WHITE);
        void drawRectangle(int x, int y, uint16_t w, uint16_t h, colors color = colors::WHITE);
        void drawHorizontalLine(int x, int y, int w, colors color = colors::WHITE);
        void drawVerticalLine(int x, int y, int w, colors color = colors::WHITE);
        void drawLine(int x_start, int y_start, int x_end, int y_end, colors color = colors::WHITE);
};

#endif
