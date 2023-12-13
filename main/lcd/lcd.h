#ifndef _LCD_H
#define _LCD_H

#include <stdint.h>
#include <stdbool.h>

#define X_MAX_PIXEL     128
#define Y_MAX_PIXEL     160
#define LCD_SIZE        X_MAX_PIXEL *Y_MAX_PIXEL
// lcd_color
#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define WHITE 0xffff
#define BLACK 0x0000
#define YELLOW 0xFFE0
#define GRAY0 0xEF7D
#define GRAY1 0x8410
#define GRAY2 0x4208

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t id;
    uint8_t dir;
    uint16_t wramcmd;
    uint16_t setxcmd;
    uint16_t setycmd;
} lcdDevice;

void lcd_set_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void lcd_setXY(uint16_t x, uint16_t y);
void draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_clear(uint16_t color);
uint16_t lcd_bgr2rgb(uint16_t color);
void lcdCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc);
void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void drawFont_GBK16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s);
void lcd_read_point(uint16_t x, uint16_t y);
void lcd_scan();
//lcd
void lcd_init();
void lcd_rest();
void lcd_write_Reg(uint8_t cmd, uint8_t data);
void lcdSpi_write_data(uint8_t data);
void lcd_write_data_16bit(uint16_t data);
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void lcdSetDCAndCS(bool dc, bool cs);

#endif
