#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include <stdint.h>
#include <stddef.h>

#define UART_ID         uart0
#define BAUD_RATE       115200
#define UART_TX_PIN     0
#define UART_RX_PIN     1

// camera_i2c
#define I2C_ID          i2c1
#define I2C_SDA_PIN     14
#define I2C_SCL_PIN     15

// lcd_spi
#define SPI_ID          spi1
#define SPI_BAUD_RATE   65 * 1000 * 1000
#define SPI_SCK_PIN     26
#define SPI_TX_PIN      27
#define lcd_res         17
#define lcd_dc          16
#define lcd_cs          22
#define lcd_bl          21

// camera
#define ov7725_D0       2
#define ov7725_D1       3
#define ov7725_D2       4
#define ov7725_D3       5
#define ov7725_D4       6
#define ov7725_D5       7
#define ov7725_D6       8
#define ov7725_D7       9

#define ov7725_RCLK     18
#define ov7725_WEN      19
#define ov7725_VSYNC    20

#define ov7725_WRST     10
#define ov7725_RRST     11
#define ov7725_OE       12

// led
#define LED_PIN         25

extern void uart_printf(const char *format, ...);
extern void usb_printf(const char *format, ...);
extern void web_printf(const char *format, ...);

void pico_gpio_init();
void pico_uart_init();
void pico_i2c_init();
void pico_spi_init();
void lcd_gpio_init();


//camera
#define SCCB_ID 0X21
void SCCB_Init(void);
uint8_t SCCB_WR_Reg(uint8_t reg, uint8_t data);
uint8_t SCCB_RD_Reg(uint8_t reg);