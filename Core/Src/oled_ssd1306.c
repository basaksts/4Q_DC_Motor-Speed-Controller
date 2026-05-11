/*
 * oled_ssd1306.c
 *
 *  Created on: May 11, 2026
 *      Author: 90546
 */


#include "oled_ssd1306.h"
#include <string.h>

#define SSD1306_I2C_ADDR   (0x3C << 1)

static I2C_HandleTypeDef *oled_hi2c = NULL;
static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static void OLED_WriteCommand(uint8_t command)
{
    uint8_t data[2];
    data[0] = 0x00;
    data[1] = command;

    if (oled_hi2c != NULL)
    {
        HAL_I2C_Master_Transmit(oled_hi2c, SSD1306_I2C_ADDR, data, 2, 10);
    }
}

static void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    if (color)
    {
        oled_buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
    }
    else
    {
        oled_buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

/*
 * Minimal 5x7 font.
 * Bu font proje ekranında kullanacağımız karakterleri kapsar:
 * A-Z, 0-9, boşluk, :, %, ., -, /
 */
static const uint8_t *OLED_GetFont5x7(char ch)
{
    static const uint8_t space[5] = {0x00,0x00,0x00,0x00,0x00};
    static const uint8_t unknown[5] = {0x7C,0x12,0x12,0x12,0x7C};

    static const uint8_t n0[5] = {0x3E,0x51,0x49,0x45,0x3E};
    static const uint8_t n1[5] = {0x00,0x42,0x7F,0x40,0x00};
    static const uint8_t n2[5] = {0x42,0x61,0x51,0x49,0x46};
    static const uint8_t n3[5] = {0x21,0x41,0x45,0x4B,0x31};
    static const uint8_t n4[5] = {0x18,0x14,0x12,0x7F,0x10};
    static const uint8_t n5[5] = {0x27,0x45,0x45,0x45,0x39};
    static const uint8_t n6[5] = {0x3C,0x4A,0x49,0x49,0x30};
    static const uint8_t n7[5] = {0x01,0x71,0x09,0x05,0x03};
    static const uint8_t n8[5] = {0x36,0x49,0x49,0x49,0x36};
    static const uint8_t n9[5] = {0x06,0x49,0x49,0x29,0x1E};

    static const uint8_t A[5] = {0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t B[5] = {0x7F,0x49,0x49,0x49,0x36};
    static const uint8_t C[5] = {0x3E,0x41,0x41,0x41,0x22};
    static const uint8_t D[5] = {0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t E[5] = {0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t F[5] = {0x7F,0x09,0x09,0x09,0x01};
    static const uint8_t G[5] = {0x3E,0x41,0x49,0x49,0x7A};
    static const uint8_t H[5] = {0x7F,0x08,0x08,0x08,0x7F};
    static const uint8_t I[5] = {0x00,0x41,0x7F,0x41,0x00};
    static const uint8_t J[5] = {0x20,0x40,0x41,0x3F,0x01};
    static const uint8_t K[5] = {0x7F,0x08,0x14,0x22,0x41};
    static const uint8_t L[5] = {0x7F,0x40,0x40,0x40,0x40};
    static const uint8_t M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t N[5] = {0x7F,0x04,0x08,0x10,0x7F};
    static const uint8_t O[5] = {0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t P[5] = {0x7F,0x09,0x09,0x09,0x06};
    static const uint8_t Q[5] = {0x3E,0x41,0x51,0x21,0x5E};
    static const uint8_t R[5] = {0x7F,0x09,0x19,0x29,0x46};
    static const uint8_t S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t U[5] = {0x3F,0x40,0x40,0x40,0x3F};
    static const uint8_t V[5] = {0x1F,0x20,0x40,0x20,0x1F};
    static const uint8_t W[5] = {0x7F,0x20,0x18,0x20,0x7F};
    static const uint8_t X[5] = {0x63,0x14,0x08,0x14,0x63};
    static const uint8_t Y[5] = {0x07,0x08,0x70,0x08,0x07};
    static const uint8_t Z[5] = {0x61,0x51,0x49,0x45,0x43};

    static const uint8_t colon[5] = {0x00,0x36,0x36,0x00,0x00};
    static const uint8_t percent[5] = {0x63,0x13,0x08,0x64,0x63};
    static const uint8_t dot[5] = {0x00,0x60,0x60,0x00,0x00};
    static const uint8_t minus[5] = {0x08,0x08,0x08,0x08,0x08};
    static const uint8_t slash[5] = {0x20,0x10,0x08,0x04,0x02};

    if (ch >= 'a' && ch <= 'z')
    {
        ch = ch - 32;
    }

    switch (ch)
    {
        case ' ': return space;
        case '0': return n0;
        case '1': return n1;
        case '2': return n2;
        case '3': return n3;
        case '4': return n4;
        case '5': return n5;
        case '6': return n6;
        case '7': return n7;
        case '8': return n8;
        case '9': return n9;

        case 'A': return A;
        case 'B': return B;
        case 'C': return C;
        case 'D': return D;
        case 'E': return E;
        case 'F': return F;
        case 'G': return G;
        case 'H': return H;
        case 'I': return I;
        case 'J': return J;
        case 'K': return K;
        case 'L': return L;
        case 'M': return M;
        case 'N': return N;
        case 'O': return O;
        case 'P': return P;
        case 'Q': return Q;
        case 'R': return R;
        case 'S': return S;
        case 'T': return T;
        case 'U': return U;
        case 'V': return V;
        case 'W': return W;
        case 'X': return X;
        case 'Y': return Y;
        case 'Z': return Z;

        case ':': return colon;
        case '%': return percent;
        case '.': return dot;
        case '-': return minus;
        case '/': return slash;

        default: return unknown;
    }
}

void OLED_SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    oled_hi2c = hi2c;

    HAL_Delay(100);

    OLED_WriteCommand(0xAE); // display off
    OLED_WriteCommand(0x20); // memory addressing mode
    OLED_WriteCommand(0x00); // horizontal addressing
    OLED_WriteCommand(0xB0);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x10);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0x7F);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // display on

    OLED_SSD1306_Fill(0);
    OLED_SSD1306_UpdateScreen();
}

void OLED_SSD1306_Fill(uint8_t color)
{
    memset(oled_buffer, color ? 0xFF : 0x00, sizeof(oled_buffer));
}

void OLED_SSD1306_UpdateScreen(void)
{
    if (oled_hi2c == NULL)
    {
        return;
    }

    for (uint8_t page = 0; page < 8; page++)
    {
        OLED_WriteCommand(0xB0 + page);
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);

        uint8_t data[OLED_WIDTH + 1];
        data[0] = 0x40;

        memcpy(&data[1], &oled_buffer[OLED_WIDTH * page], OLED_WIDTH);

        HAL_I2C_Master_Transmit(oled_hi2c,
                                SSD1306_I2C_ADDR,
                                data,
                                OLED_WIDTH + 1,
                                20);
    }
}

void OLED_SSD1306_SetCursor(uint8_t x, uint8_t y)
{
    cursor_x = x;
    cursor_y = y;
}

void OLED_SSD1306_WriteChar(char ch)
{
    const uint8_t *font = OLED_GetFont5x7(ch);

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = font[col];

        for (uint8_t row = 0; row < 7; row++)
        {
            if (line & (1 << row))
            {
                OLED_DrawPixel(cursor_x + col, cursor_y + row, 1);
            }
            else
            {
                OLED_DrawPixel(cursor_x + col, cursor_y + row, 0);
            }
        }
    }

    for (uint8_t row = 0; row < 7; row++)
    {
        OLED_DrawPixel(cursor_x + 5, cursor_y + row, 0);
    }

    cursor_x += 6;

    if (cursor_x > OLED_WIDTH - 6)
    {
        cursor_x = 0;
        cursor_y += 8;
    }
}

void OLED_SSD1306_WriteString(const char *str)
{
    while (*str)
    {
        OLED_SSD1306_WriteChar(*str);
        str++;
    }
}
