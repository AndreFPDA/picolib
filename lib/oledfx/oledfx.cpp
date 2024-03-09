#include "oledfx.hpp"
#include "OLED_font.hpp"

namespace {

	inline static void swap(int &a, int &b) 
	{
	    int tmp = a;
	    a = b;
	    b = tmp;
	}
	
};


// === Font class implementation ===
/*!
	@brief init the OLED  font class object constructor
 */
SSD1306_OLEDFonts::SSD1306_OLEDFonts(){};

/*!
	@brief SSD1306_SetFont
	@param  SelectedFontName Select this font, pass the font pointer name 
	@return	Will return
		-# 0. Success
		-# 2. Not a valid pointer object.
 */
uint8_t SSD1306_OLEDFonts::setFont(const uint8_t * SelectedFontName) {
	if (SelectedFontName == nullptr)
	{
		printf("SSD1306_OLEDFonts::setFont ERROR 2: Invalid pointer object\r\n");
		return 2;
	}
	_FontSelect   = SelectedFontName;
	_Font_X_Size  = *(SelectedFontName + 0);
	_Font_Y_Size  = *(SelectedFontName + 1);
	_FontOffset   = *(SelectedFontName + 2);
	_FontNumChars = *(SelectedFontName + 3);

	return 0;
}

/**
 * Create oledfx instantion 
 *
 * @param DevAddr device i2c address.
 * @param Size screen size (W128xH64 or W128xH32)
 * @param i2c i2c instance
 */
oledfx::oledfx(uint16_t const DevAddr, size_display Size, i2c_inst_t * i2c) : SSD1306(DevAddr, Size, i2c) {};


/**
 * @brief Draw one char.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param chr char to be written
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */


void oledfx::drawChar(int x, int y, char chr, colors color) {
    if (chr < _FontOffset || chr >= _FontOffset + _FontNumChars) {
        // Caractere não disponível na fonte
        return;
    }

    uint16_t fontIndex = (chr - _FontOffset) * ((_Font_X_Size * _Font_Y_Size) / 8) + 4;

    for (int16_t row = 0; row < _Font_Y_Size; ++row) {
        for (int16_t col = 0; col < _Font_X_Size; ++col) {
            uint8_t bits = *(_FontSelect + fontIndex + col + (row / 8) * _Font_X_Size);

            if (bits & (1 << (row % 8))) {
                this->drawPixel(x + col, y + row, color);
            }
        }
    }
}

/**
 * @brief Draw string.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param str string to be written
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */

void oledfx::drawString(int x, int y, std::string str, colors color){ //(int16_t x, int16_t y, char * pText) {
	for (size_t i = 0; i < str.length(); ++i)
	{
		drawChar(x + (i * (_Font_X_Size)), y, str[i], color);
	}
}

/**
 * @brief Draw empty rectangle.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawRectangle(int x, int y, uint16_t w, uint16_t h, colors color)
{
    this->drawHorizontalLine(x, y, w, color);
    this->drawHorizontalLine(x, y+h-1, w, color);
    this->drawVerticalLine(x, y, h, color);
    this->drawVerticalLine(x+w-1, y, h, color);
}


/**
 * @brief Draw filled rectangle.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawFillRectangle(int x, int y, uint16_t w, uint16_t h, colors color)
{
    for (int i=x; i<x+w; i++) {
    	this->drawVerticalLine(i, y, h, color);
    }
}

/**
 * @brief Draw progress bar.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param w width of the rectangle
 * @param h height of the rectangle
 * @param progress progress (0, 100)
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawProgressBar(int x, int y, uint16_t w, uint16_t h, uint8_t progress, colors color)
{
    this->drawRectangle(x, y, w, h, color);
    this->drawFillRectangle(x, y, (uint8_t)((w*progress)/100), h, color);
}


/**
 * @brief Draw vertical line.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param h height of the line
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawVerticalLine(int x, int y, int h, colors color)
{
	this->drawLine(x, y, x, y+h-1, color);
}


/**
 * @brief Draw horizontal line.
 *
 * @param x position from the left edge (0, MAX WIDTH)
 * @param y position from the top edge (0, MAX HEIGHT)
 * @param w width of the line
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawHorizontalLine(int x, int y, int w, colors color)
{
	this->drawLine(x, y, x+w-1, y, color);
}


/**
 * @brief Draw straight line.
 *
 * @param x_start position of the first point from the left edge (0, MAX WIDTH)
 * @param y_start position of the first point from the top edge  (0, MAX HEIGHT)
 * @param x_end position of the second point from the left edge (0, MAX WIDTH)
 * @param y_end position of the second point from the top edge  (0, MAX HEIGHT)
 * @param color colors::BLACK, colors::WHITE or colors::INVERSE
 */
void oledfx::drawLine(int x_start, int y_start, int x_end, int y_end, colors color)
{
	int16_t steep = abs(y_end - y_start) > abs(x_end - x_start);

	if (steep) 
	{
		swap(x_start, y_start);
		swap(x_end, y_end);
	}

	if (x_start > x_end) 
	{
		swap(x_start, x_end);
		swap(y_start, y_end);
	}

	int16_t dx = x_end - x_start;
	int16_t dy = abs(y_end - y_start);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y_start < y_end) 
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	while (x_start <= x_end) 
	{
		if (steep) 
		{
			this->drawPixel(y_start, x_start, color);
		}
		else
		{
			this->drawPixel(x_start, y_start, color);
		}

		x_start++;
		err -= dy;

		if (err < 0) 
		{
			y_start += ystep;
			err += dx;
		}
	}
}
