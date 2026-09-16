#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"

// Packed BMP headers (Windows BITMAPFILEHEADER / BITMAPINFOHEADER), used only
// to parse 1-bit monochrome .bmp files exported from image tools.
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} bmp_file_header_t;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} bmp_info_header_t;
#pragma pack(pop)

// Holds a bitmap already converted to the SSD1306 GDDRAM layout (pages of 8
// vertical pixels per byte, LSB = topmost pixel), ready to be handed to any
// display driver later, e.g. oledfx::display(image.buffer).
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *buffer;      // size == buffer_size, caller owns it (free() when done)
    size_t   buffer_size; // width * ceil(height / 8)
} bitmap_image_t;

// Reads a 1-bit monochrome BMP file from the SD card and converts it into
// bitmap_image_t.buffer, packed the same way the SSD1306 driver expects.
static bool load_bitmap_from_sd(const char *filename, bitmap_image_t *out) {
    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open bitmap '%s' (%d)\r\n", filename, fr);
        return false;
    }

    bmp_file_header_t fileHeader;
    bmp_info_header_t infoHeader;
    UINT br;

    fr = f_read(&fil, &fileHeader, sizeof(fileHeader), &br);
    if (fr != FR_OK || br != sizeof(fileHeader) || fileHeader.bfType != 0x4D42) {
        printf("ERROR: Invalid BMP file '%s'\r\n", filename);
        f_close(&fil);
        return false;
    }

    fr = f_read(&fil, &infoHeader, sizeof(infoHeader), &br);
    if (fr != FR_OK || br != sizeof(infoHeader) || infoHeader.biBitCount != 1) {
        printf("ERROR: '%s' is not a 1-bit monochrome BMP\r\n", filename);
        f_close(&fil);
        return false;
    }

    const int32_t width = infoHeader.biWidth;
    const bool topDown = infoHeader.biHeight < 0;
    const int32_t height = topDown ? -infoHeader.biHeight : infoHeader.biHeight;
    const uint32_t rowBytes = ((width + 31) / 32) * 4; // BMP rows are padded to 4 bytes
    const size_t pages = (height + 7) / 8;

    uint8_t *buffer = (uint8_t *)calloc((size_t)width * pages, 1);
    uint8_t *row = (uint8_t *)malloc(rowBytes);
    if (buffer == NULL || row == NULL) {
        printf("ERROR: Not enough memory to load '%s'\r\n", filename);
        free(buffer);
        free(row);
        f_close(&fil);
        return false;
    }

    for (int32_t y = 0; y < height; y++) {
        // BMP rows are stored bottom-up unless biHeight is negative
        const uint32_t fileRow = topDown ? (uint32_t)y : (uint32_t)(height - 1 - y);
        fr = f_lseek(&fil, fileHeader.bfOffBits + (FSIZE_t)fileRow * rowBytes);
        if (fr != FR_OK || (fr = f_read(&fil, row, rowBytes, &br)) != FR_OK || br != rowBytes) {
            printf("ERROR: Could not read bitmap row %ld (%d)\r\n", (long)y, fr);
            free(row);
            free(buffer);
            f_close(&fil);
            return false;
        }

        const uint32_t page = (uint32_t)y / 8;
        const uint8_t bitInPage = (uint8_t)(y % 8);
        for (int32_t x = 0; x < width; x++) {
            const uint8_t bit = (row[x / 8] >> (7 - (x % 8))) & 0x01;
            if (bit) {
                buffer[page * width + x] |= (uint8_t)(1 << bitInPage);
            }
        }
    }

    free(row);
    f_close(&fil);

    out->width = (uint16_t)width;
    out->height = (uint16_t)height;
    out->buffer = buffer;
    out->buffer_size = (size_t)width * pages;
    return true;
}

int main() {

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test02.txt";

    // Initialize chosen serial port
    stdio_init_all();

    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");
    while (true) {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n')) {
            break;
        }
    }

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    // Open file for writing ()
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Write something to file
    ret = f_printf(&fil, "This is another test\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }
    ret = f_printf(&fil, "of writing to an SD card.\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // Open file for reading
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Print every line in file over serial
    printf("Reading from file '%s':\r\n", filename);
    printf("---\r\n");
    while (f_gets(buf, sizeof(buf), &fil)) {
        printf(buf);
    }
    printf("\r\n---\r\n");

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // Load a bitmap file and keep it ready in memory for a future display,
    // the same way 'buf' above holds the text already read from the SD card.
    bitmap_image_t logo_image = {};
    if (load_bitmap_from_sd("logo.bmp", &logo_image)) {
        printf("Bitmap 'logo.bmp' loaded: %ux%u, %u bytes ready in logo_image.buffer\r\n",
               logo_image.width, logo_image.height, (unsigned)logo_image.buffer_size);
        // logo_image.buffer is packed in the SSD1306 GDDRAM layout, so it can
        // later be passed directly to a display driver, e.g. oled.display(logo_image.buffer);
    } else {
        printf("WARNING: Bitmap not loaded, continuing without it\r\n");
    }

    // Unmount drive
    f_unmount("0:");

    // Loop forever doing nothing
    while (true) {
        sleep_ms(1000);
    }
}