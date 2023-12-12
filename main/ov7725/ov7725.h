#ifndef _OV7725_H
#define _OV7725_H
#include "board.h"

#define OV7725_MID 0X7FA2
#define OV7725_PID 0X7721

#define OV7725_WRST PAout(0)     //写指针复位
#define OV7725_RCK PAout(1)      //读数据时钟
#define OV7725_RRST PAout(4)    //读指针复位
#define OV7725_CS PAout(11)     //片选信号 （OE）
#define OV7725_WREN PAout(12)       //写FIFO使能
#define OV7725_VSYNC PAin(15)       //同步信号检测

#define OV7725_DATA  (gpio_get(2) | gpio_get(3) << 1 | gpio_get(4) << 2 | gpio_get(5) << 3 | gpio_get(6) << 4 | gpio_get(7) << 5 | gpio_get(8) << 6 | gpio_get(9) << 7)   //数据输入端口

uint8_t ov7725_Init(void);
void OV7725_Light_Mode(uint8_t mode);
void OV7725_Color_Saturation(int8_t sat);
void OV7725_Brightness(int8_t bright);
void OV7725_Contrast(int8_t contrast);
void OV7725_Special_Effects(uint8_t eft);
void OV7725_Window_Set(uint16_t width, uint16_t height, uint8_t mode);
void OV7725_Configure(void);

void setData2To9_Low();
void setData2To9_High();
#endif //  _OV7725_H
