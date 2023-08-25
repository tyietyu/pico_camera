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