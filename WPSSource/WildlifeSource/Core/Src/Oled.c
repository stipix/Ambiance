/*
 * File:   Oled.c
 * Author: Adam Korycki
 * 
 * Created on November 6, 2023
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <OledDriver.h>
#include <Oled.h>
#include <Ascii.h>

void OledInit(void)
{
    // First initialize the STM32 to be able to talk over I2C to the OLED.
    OledHostInit();
    HAL_Delay(10);

    // Now send initialization commands to the OLED.
    OledDriverInitDisplay();
    HAL_Delay(200);//allow time for OLED screen to initialize

    // Clear the frame buffer by filling it with black pixels.
    OledClear(OLED_COLOR_BLACK);

    // Finally update the screen, triggering a write of all black pixels to the screen.
    OledUpdate();
}

void OledSetPixel(int x, int y, OledColor color)
{
    // Check for valid inputs before doing anything.
    if (x >= OLED_DRIVER_PIXEL_COLUMNS || y >= OLED_DRIVER_PIXEL_ROWS || x < 0 || y < 0) {
        return;
    }

    // Map the x/y coordinates into a byte/bit index.
    unsigned int index = ((y & 0xFFF8) << 4) + x;
    unsigned int shift = y & 0x0007;

    // Now set the pixel to the proper color, doing nothing if an invalid color was specified.
    if (color == OLED_COLOR_WHITE) {
        rgbOledBmp[index] = rgbOledBmp[index] | (1 << shift);
    } else if (color == OLED_COLOR_BLACK) {
        rgbOledBmp[index] = rgbOledBmp[index] & ~(1 << shift);
    } else {
        return;
    }
}

int OledGetPixel(int x, int y)
{
    // Check for valid inputs before doing anything.
    if (x >= OLED_DRIVER_PIXEL_COLUMNS || y >= OLED_DRIVER_PIXEL_ROWS || x < 0 || y < 0) {
        return OLED_COLOR_BLACK;
    }

    // Map the x/y coordinates into a byte/bit index.
    unsigned int index = ((y & 0xFFF8) << 4) + x;
    unsigned int shift = y & 0x0007;

    // Now return the desired bit.
    return (rgbOledBmp[index] >> shift) & 0x01;
}

uint8_t OledDrawChar(int x, int y, char c)
{
    if (x <= OLED_DRIVER_PIXEL_COLUMNS - ASCII_FONT_WIDTH && y <= OLED_DRIVER_PIXEL_ROWS - ASCII_FONT_HEIGHT) {
        // We need to convert our signed char into an unsigned value to index into the ascii[] array.
        int charIndex = (int) (unsigned char) c;

        // Now first determine the columns and rows of the OLED bits that need to be modified
        int rowMin, rowMax, colMin, colMax;
        rowMin = y / ASCII_FONT_HEIGHT;
        int rowY = y % ASCII_FONT_HEIGHT;
        rowMax = (y + ASCII_FONT_HEIGHT) / OLED_DRIVER_BUFFER_LINE_HEIGHT;
        colMin = x;
        colMax = x + ASCII_FONT_WIDTH;
        {
            // Generate a positive mask for where in the column the new symbol will be drawn.
            int colMask = ((1 << ASCII_FONT_HEIGHT) - 1) << rowY;
            int j;
            for (j = 0; j < colMax - colMin; ++j) {
                int oledCol = colMin + j;
                uint8_t newCharCol = rgbOledBmp[rowMin * OLED_DRIVER_PIXEL_COLUMNS + oledCol] & ~colMask;
                // Make sure we always grab from the top part of the character.
                newCharCol |= (ascii[charIndex][j] & (colMask >> rowY)) << rowY;
                rgbOledBmp[rowMin * OLED_DRIVER_PIXEL_COLUMNS + oledCol] = newCharCol;
            }
        }
        if (rowMax > rowMin) {
            // Generate a positive mask for where in the column the new symbol will be drawn.
            // Since we need the lower portion of the symbol, we recalculate its height.
            int colMask = ((1 << ASCII_FONT_HEIGHT) - 1) >> (OLED_DRIVER_BUFFER_LINE_HEIGHT - rowY);
            int j;
            for (j = 0; j < colMax - colMin; ++j) {
                int oledCol = colMin + j;
                uint8_t newCharCol = rgbOledBmp[rowMax * OLED_DRIVER_PIXEL_COLUMNS + ASCII_FONT_WIDTH - 1 - oledCol] & ~colMask;
                // Make sure we grab the proper part of the character from the font.
                newCharCol |= (ascii[charIndex][j] & (colMask << (OLED_DRIVER_BUFFER_LINE_HEIGHT - rowY))) >>
                        (OLED_DRIVER_BUFFER_LINE_HEIGHT - rowY);
                rgbOledBmp[rowMax * OLED_DRIVER_PIXEL_COLUMNS + ASCII_FONT_WIDTH - 1 - oledCol] = newCharCol;
            }
        }
    }

    return FALSE;
}

void OledDrawString(const char *string)
{
    if (string == NULL) {
        return;
    }

    // Track the current line number we're in on the OLED. Valid values are [0, OLED_NUM_LINES).
    int line = 0;

    // Track the current character we're writing to the OLED. Valid values are
    // [0, OLED_CHARS_PER_LINE).
    int column = 0;

    // Run through all characters. The maximum length can be the number of lines times the number
    // of characters per line + three newlines.
    int i;
    for (i = 0; string[i] != '\0' && i < (OLED_NUM_LINES * OLED_CHARS_PER_LINE + 3); ++i) {
        // Move the cursor to the next line if a newline character is encountered. This allows for
        // early line ending.
        if (string[i] == '\n') {
            ++line;
            column = 0;
            continue;
        } else {
            // Reset to the start of the next line if we've hit the character limit of this line
            // without seeing a newline.
            if (column == OLED_CHARS_PER_LINE) {
                ++line;
                column = 0;
            }

            // Now if we're < OLED_NUM_LINES and < OLED_CHARS_PER_LINE we can proceed. The second
            // check is implicitly handled by the above if-statement that forces a newline after
            // encountering a full line of characters.
            if (line == OLED_NUM_LINES) {
                break;
            }

            // Finally at this point, we can write characters to the OLED.
            OledDrawChar(column * ASCII_FONT_WIDTH, line * ASCII_FONT_HEIGHT, string[i]);
            ++column;
        }
    }
}

void OledClear(OledColor p)
{
    int i;
    for (i = 0; i < OLED_DRIVER_BUFFER_SIZE; i++) {
        if (p == OLED_COLOR_WHITE) {
            rgbOledBmp[i] = 0xFF;
        } else {
            rgbOledBmp[i] = 0;
        }
    }
}

void OledSetDisplayInverted(void)
{
    OledDriverSetDisplayInverted();
}

void OledSetDisplayNormal(void)
{
    OledDriverSetDisplayNormal();
}

void OledOn(void)
{
    OledDriverInitDisplay();
}

void OledOff(void)
{
    OledDriverDisableDisplay();
}

void OledUpdate(void)
{
    OledDriverUpdateDisplay();
}



//#define OLED_TEST
#ifdef OLED_TEST
#include <stdio.h>
#include <Board.h>
#include <Oled.h>

int main(void) {
    BOARD_Init();
    OledInit();
    OledDrawString("Hello world!\nOLED TEST\ngo slugs");
    OledUpdate();
    /*
    char SecondLine[40];
    OledDrawString(__DATE__ "\n "__TIME__);
    sprintf(SecondLine,"\n\nRandom: %d",rand());
    OledDrawString(SecondLine);
    OledUpdate();
    printf("\r\nTesting at " __TIME__);
    */
    while(TRUE){}
}

#endif
