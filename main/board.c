#include "board.h"

extern void irq_vsync_cb(uint gpio, uint32_t events);

void pico_gpio_init()
{
    gpio_init(ov7725_WEN);
    gpio_set_dir(ov7725_WEN, GPIO_OUT);
    gpio_put(ov7725_WEN, 1);

    gpio_init(ov7725_WRST);
    gpio_set_dir(ov7725_WRST, GPIO_OUT);
    gpio_put(ov7725_WRST, 1);

    gpio_init(ov7725_RRST);
    gpio_set_dir(ov7725_RRST, GPIO_OUT);
    gpio_put(ov7725_RRST, 1);

    gpio_init(ov7725_OE);
    gpio_set_dir(ov7725_OE, GPIO_OUT);
    gpio_put(ov7725_OE, 1);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_set_irq_enabled_with_callback(ov7725_VSYNC, GPIO_IRQ_EDGE_RISE, true, &irq_vsync_cb);
}

void lcd_gpio_init()
{
    gpio_init(lcd_res);
    gpio_set_dir(lcd_res, GPIO_OUT);

    gpio_init(lcd_dc);
    gpio_set_dir(lcd_dc, GPIO_OUT);

    gpio_init(lcd_cs);
    gpio_set_dir(lcd_cs, GPIO_OUT);
    gpio_put(lcd_cs, true);

    gpio_init(lcd_bl);
    gpio_set_dir(lcd_bl, GPIO_OUT);
}
void pico_uart_init()
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void pico_i2c_init()
{
    i2c_init(I2C_ID, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

void pico_spi_init()
{
    spi_init(SPI_ID, SPI_BAUD_RATE);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
}

void SCCB_Init()
{
    pico_i2c_init();
}
uint8_t SCCB_WR_Reg(uint8_t reg, uint8_t data)
{
    i2c_write_blocking(I2C_ID, SCCB_ID, (uint8_t[]){reg, data}, 2, false);
    return 0;
}
uint8_t SCCB_RD_Reg(uint8_t reg)
{
    uint8_t data;
    i2c_write_blocking(I2C_ID, SCCB_ID, &reg, 1, false);
    i2c_read_blocking(I2C_ID, SCCB_ID, &data, 1, false);
    return data;
}

