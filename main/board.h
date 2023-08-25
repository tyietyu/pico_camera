#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define I2C_ID i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

#define ov7725_D0 2
#define ov7725_D1 3
#define ov7725_D2 4
#define ov7725_D3 5
#define ov7725_D4 6
#define ov7725_D5 7
#define ov7725_D6 8
#define ov7725_D7 9

#define ov7725_RCLK 18
#define ov7725_WEN 19
#define ov7725_VSYNC 20

#define ov7725_WRST 10
#define ov7725_RRST 11
#define ov7725_OE 12

#define LED_PIN 25

void pico_gpio_init();
void pico_uart_init();
void pico_i2c_init();
