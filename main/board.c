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

static inline void lcdSetDCAndCS(bool dc, bool cs)
{
    gpio_put_masked((1u << lcd_dc) | (1u << lcd_cs), (dc << lcd_dc) | (cs << lcd_cs));
}

void lcd_write_data(uint8_t data)
{
    lcdSetDCAndCS(1, 0);

    uint8_t dataBuffer[2];
    dataBuffer[0] = (data >> 8) & 0xff;
    dataBuffer[1] = data & 0xff;
    spi_write_blocking(SPI_ID, dataBuffer, 2);

    lcdSetDCAndCS(1, 1);
}

void lcd_write_cmd(uint8_t cmd)
{
    lcdSetDCAndCS(0, 0);
    spi_write_blocking(SPI_ID, &cmd, 1);
    lcdSetDCAndCS(0, 1);
}

// 向lcd写一个16位数据
void lcd_write_data_16bit(uint16_t data)
{
    lcdSetDCAndCS(1, 0);
    spi_write_blocking(SPI_ID, &data, 2);
    gpio_put(lcd_cs, 1);
}

void lcd_write_Reg(uint8_t cmd, uint8_t data)
{
    lcd_write_cmd(cmd);
    lcd_write_data(data);
}

void lcd_rest()
{
    gpio_put(lcd_res, 0);
    sleep_ms(100);
    gpio_put(lcd_res, 1);
    sleep_ms(50);
}

void lcd_init()
{
    lcd_gpio_init();
    pico_spi_init();
    lcd_rest();
    lcd_write_cmd(0x11);
    sleep_ms(120);

    lcd_write_cmd(0xB1);
    lcd_write_data(0x01);
    lcd_write_data(0x2C);
    lcd_write_data(0x2D);

    lcd_write_cmd(0xB2);
    lcd_write_data(0x01);
    lcd_write_data(0x2C);
    lcd_write_data(0x2D);
    lcd_write_data(0x01);
    lcd_write_data(0x2C);
    lcd_write_data(0x2D);

    lcd_write_cmd(0xB4);
    lcd_write_data(0x07);

    lcd_write_cmd(0xC0);
    lcd_write_data(0xA2);
    lcd_write_data(0x02);
    lcd_write_data(0x84);
    lcd_write_cmd(0xC1);
    lcd_write_data(0xC5);

    lcd_write_cmd(0xC2);
    lcd_write_data(0x0A);
    lcd_write_data(0x00);

    lcd_write_cmd(0xC3);
    lcd_write_data(0x8A);
    lcd_write_data(0x2A);
    lcd_write_cmd(0xC4);
    lcd_write_data(0x8A);
    lcd_write_data(0xEE);

    lcd_write_cmd(0xC5);
    lcd_write_data(0x0E);

    lcd_write_cmd(0x36);
    lcd_write_data(0xC8);

    lcd_write_cmd(0xE0);
    lcd_write_data(0x0f);
    lcd_write_data(0x1a);
    lcd_write_data(0x0f);
    lcd_write_data(0x18);
    lcd_write_data(0x2f);
    lcd_write_data(0x28);
    lcd_write_data(0x20);
    lcd_write_data(0x22);
    lcd_write_data(0x1f);
    lcd_write_data(0x1b);
    lcd_write_data(0x23);
    lcd_write_data(0x37);
    lcd_write_data(0x00);
    lcd_write_data(0x07);
    lcd_write_data(0x02);
    lcd_write_data(0x10);

    lcd_write_cmd(0xE1);
    lcd_write_data(0x0f);
    lcd_write_data(0x1b);
    lcd_write_data(0x0f);
    lcd_write_data(0x17);
    lcd_write_data(0x33);
    lcd_write_data(0x2c);
    lcd_write_data(0x29);
    lcd_write_data(0x2e);
    lcd_write_data(0x30);
    lcd_write_data(0x30);
    lcd_write_data(0x39);
    lcd_write_data(0x3f);
    lcd_write_data(0x00);
    lcd_write_data(0x07);
    lcd_write_data(0x03);
    lcd_write_data(0x10);

    lcd_write_cmd(0x2a);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x7f);

    lcd_write_cmd(0x2b);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x9f);

    lcd_write_cmd(0xF0);
    lcd_write_data(0x01);
    lcd_write_cmd(0xF6);
    lcd_write_data(0x00);

    lcd_write_cmd(0x3A);
    lcd_write_data(0x05);

    lcd_write_cmd(0x29);

}