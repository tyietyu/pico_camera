#include "lcd.h"
#include "font.h"
#include "board.h"

lcdDevice tftDevice = {
    .width = X_MAX_PIXEL,
    .height = Y_MAX_PIXEL,
};

// 设置lcd显示区域
void lcd_set_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    lcd_write_cmd(0x2a);
    lcd_write_data(0x00);
    lcd_write_data(x_start + 2);
    lcd_write_data(0x00);
    lcd_write_data(x_end + 2);

    lcd_write_cmd(0x2b);
    lcd_write_data(0x00);
    lcd_write_data(y_start + 1);
    lcd_write_data(0x00);
    lcd_write_data(y_end + 1);

    lcd_write_cmd(0x2c);
}

void lcd_setXY(uint16_t x, uint16_t y)
{
    lcd_set_area(x, y, x, y);
}

void draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_setXY(x, y);
    lcd_write_data(color);
}

void lcd_clear(uint16_t color)
{
    uint16_t i, j;
    lcd_set_area(0, 0, X_MAX_PIXEL - 1, Y_MAX_PIXEL - 1);
    for (i = 0; i < X_MAX_PIXEL; i++)
    {
        for (j = 0; j < Y_MAX_PIXEL; j++)
        {
            lcd_write_data_16bit(color);
        }
    }
}

uint16_t lcd_bgr2rgb(uint16_t color)
{
    uint16_t r, g, b, rgb;
    b = (color >> 0) & 0x1f;
    g = (color >> 5) & 0x3f;
    r = (color >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return rgb;
}

void lcdCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc)
{
    uint16_t a, b;
    int di;
    a = 0;
    b = R;
    di = 3 - 2 * R; // 判断下个点位置的标志
    while (a < b)
    {
        draw_point(X + a, Y + b, fc); //        7
        draw_point(X - a, Y + b, fc); //        6
        draw_point(X + a, Y - b, fc); //        2
        draw_point(X - a, Y - b, fc); //        3
        draw_point(X + b, Y + a, fc); //        8
        draw_point(X - b, Y + a, fc); //        5
        draw_point(X + b, Y - a, fc); //        1
        draw_point(X - b, Y - a, fc); //        4

        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b -= 1;
        }
        a += 1;
    }
    if (a = b)
    {
        draw_point(X + a, Y + b, fc);
        draw_point(X + a, Y + b, fc);
        draw_point(X + a, Y - b, fc);
        draw_point(X - a, Y - b, fc);
        draw_point(X + b, Y + a, fc);
        draw_point(X - b, Y + a, fc);
        draw_point(X + b, Y - a, fc);
        draw_point(X - b, Y - a, fc);
    }
}

void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int dx,  // difference in x's
        dy,  // difference in y's
        dx2, // dx,dy * 2
        dy2,
        x_inc, // amount in pixel space to move during drawing
        y_inc, // amount in pixel space to move during drawing
        error, // the discriminant i.e. error i.e. decision variable
        index; // used for looping
    lcd_setXY(x0, y0);
    dx = x1 - x0;
    dy = y1 - y0;

    if (dx >= 0)
    {
        x_inc = 1;
    }
    else
    {
        x_inc = -1;
        dx = -dx;
    }
    if (dy >= 0)
    {
        y_inc = 1;
    }
    else
    {
        y_inc = -1;
        dy = -dy;
    }

    dx2 = dx << 1;
    dy2 = dy << 1;

    if (dx > dy)
    {
        error = dy2 - dx;
        for (index = 0; index <= dx; index++)
        {
            draw_point(x0, y0, color);
            if (error >= 0)
            {
                error -= dx2;
                y0 += y_inc;
            }
            error += dy2;
            x0 += x_inc;
        }
    }
    else
    {
        error = dx2 - dy;
        for (index = 0; index <= dy; index++)
        {
            draw_point(x0, y0, color);
            if (error >= 0)
            {
                error -= dy2;
                x0 += x_inc;
            }
            error += dx2;
            y0 += y_inc;
        }
    }
}

void drawFont_GBK16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s)
{
    unsigned char i, j;
    unsigned short k, x0;
    x0 = x;
    while (*s)
    {
        if ((*s) < 128)
        {
            k = *s;
            if (k == 13)
            {
                x = x0;
                y += 16;
            }
            else
            {
                if (k > 32)
                    k -= 32;
                else
                    k = 0;
                for (i = 0; i < 16; i++)
                    for (j = 0; j < 8; j++)
                    {
                        if (asc16[k + i] & (0x80 >> j))draw_point(x, y, fc);
                        else
                        {
                            if (fc != bc)draw_point(x, y, bc);                             
                        }
                    }
                    x += 8;
            }
                s++;
        }
    }    
}