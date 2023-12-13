#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "board.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "rp2040_clock.h"
#include "pico/binary_info.h"
#include "ov7725.h"
#include "lcd.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define APP_TX_DATA_SIZE 2048
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
#define  OV7725 1
#define OV7725_WINDOW_WIDTH 128
#define OV7725_WINDOW_HEIGHT 160
uint8_t camera_buffer[128 * 160 * 2] = {0};

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,

    BLINK_ALWAYS_OFF = UINT32_MAX,
    BLINK_ALWAYS_ON = 0
};

#define URL "candas1.github.io/Hoverboard-Web-Serial-Control"

const tusb_desc_webusb_url_t desc_url =
    {
        .bLength = 3 + sizeof(URL) - 1,
        .bDescriptorType = 3, // WEBUSB URL type
        .bScheme = 1,         // 0: http, 1: https
        .url = URL};

static volatile bool web_serial_connected = false;
static volatile bool usb_connected = false;
static volatile uint32_t ov_sta = 0;
volatile uint8_t ov_frame = 0;

bool repeating_timer_callback(struct repeating_timer *t);
void irq_vsync_cb(uint gpio, uint32_t events);
void OV7725_camera_refresh(void);
void core1_entry();
void web_printf(const char *format, ...);
void usb_printf(const char *format, ...);
void uart_printf(const char *format, ...);
void i2c_detect(i2c_inst_t *i2c);

void setup_timer(uint32_t interval_ms)
{
    struct repeating_timer timer;
    // Setup a repeating alarm to go off every interval_ms
    // The handler function will be called on core 1
    add_repeating_timer_ms(interval_ms, repeating_timer_callback, NULL, NULL);
}
// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr)
{
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main()
{
    uint8_t i;
    uint8_t sensor = 0;
    board_init();
    rp2040_clock_133Mhz();

    pico_gpio_init();
    pico_uart_init();
    pico_i2c_init();
    lcd_init();
    ov7725_Init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    // struct repeating_timer timer;
    // add_repeating_timer_ms(5, repeating_timer_callback, NULL, &timer);
    multicore_launch_core1(core1_entry);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(14, 15, GPIO_FUNC_I2C));

    // ov7725 camera init
    sleep_ms(3000);
    i2c_detect(I2C_ID);
    if (!ov7725_Init())
        usb_printf("OV7725 Init OK\r\n");
    OV7725_Configure();
    lcd_clear(BLUE);
    sleep_ms(1000);
    lcd_clear(RED);
    sleep_ms(1000);
    while (1)
    {
        if (ov7725_Init() == 0)
        {
            OV7725_Configure();
            gpio_put(ov7725_OE, 0);
            break;
        }
        else
        {
            drawFont_GBK16(10, 10, RED, GRAY0, "0v7725 error\r\n");
            sleep_ms(200);
            lcd_clear(WHITE);
            sleep_ms(200);
        }
    }
    setup_timer(1000);
    while (1)
    {
        if(sensor == OV7725)OV7725_camera_refresh();
        if(i!=ov_frame)
        {
            i = ov_frame;
            lcd_clear(WHITE);
            drawFont_GBK16(10, 10, RED, GRAY0, "camera error\r\n");
        }
    }
}

void OV7725_camera_refresh(void)
{
    uint32_t i, j;
    uint16_t color;
    if (ov_sta == 2)
    {
        lcd_scan_dir();
        lcd_set_area(0, 0, OV7725_WINDOW_WIDTH, OV7725_WINDOW_HEIGHT);
        lcd_write_data(0x2c);
        gpio_put(ov7725_OE, 0);
        gpio_put(ov7725_RRST, 0);
        gpio_put(ov7725_RCLK, 0);
        gpio_put(ov7725_RCLK, 1);
        gpio_put(ov7725_RCLK, 0);
        gpio_put(ov7725_RRST, 1);
        gpio_put(ov7725_RCLK, 1);
        for (i = 0; i < OV7725_WINDOW_WIDTH; i++)
        {
            for (j = 0; j < OV7725_WINDOW_HEIGHT; j++)
            {
                gpio_put(ov7725_RCLK, 0);
                color = OV7725_DATA;
                gpio_put(ov7725_RCLK, 1);
                color <<= 8;
                gpio_put(ov7725_RCLK, 0);
                color |= OV7725_DATA;
                gpio_put(ov7725_RCLK, 1);
                lcd_write_data(color);
            }
        }
        gpio_put(ov7725_OE, 1);
        gpio_put(ov7725_RCLK, 0);
        gpio_put(ov7725_RCLK, 1);
        gpio_set_irq_enabled_with_callback(ov7725_VSYNC, GPIO_IRQ_EDGE_RISE, true, &irq_vsync_cb);
        ov_sta = 0;
        ov_frame++;
        lcd_scan_dir();
    }
}
void i2c_detect(i2c_inst_t *i2c)
{
    usb_printf("\nI2C Bus Scan\r\n");
    usb_printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
    for (int addr = 0; addr < (1 << 7); ++addr)
    {
        if (addr % 16 == 0)
        {
            usb_printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c, addr, &rxdata, 1, false);

        usb_printf(ret < 0 ? "." : "@");
        usb_printf(addr % 16 == 15 ? "\r\n" : "  ");
    }
    usb_printf("Done.\r\n");
    usb_printf("\r\n\r\n");
}

bool repeating_timer_callback(struct repeating_timer *t)
{
    // tud_task(); // tinyusb device task
    usb_printf("frame: %d\r\n", ov_frame);
    ov_frame = 0;
    return true;
}

void core1_entry()
{
    while (1)
    {
        sleep_ms(1);
        tud_task();
    }
}

void irq_vsync_cb(uint gpio, uint32_t events)
{
    usb_printf("irq_vsync_cb\r\n");
    if (ov_sta < 2)
    {
        if (ov_sta == 0)
        {
            gpio_put(ov7725_WRST, 0);
            gpio_put(ov7725_WRST, 1);
            gpio_put(ov7725_WEN, 1);
        }
        else
        {
            gpio_put(ov7725_WEN, 0);
            ov_sta++;
        }
    }
}

void web_printf(const char *format, ...)
{
    if (web_serial_connected)
    {
        va_list args;
        uint32_t length;

        va_start(args, format);
        length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
        va_end(args);

        tud_vendor_write(UserTxBufferFS, length);
        tud_vendor_flush();
    }
}

void usb_printf(const char *format, ...)
{
    va_list args;
    uint32_t length;

    va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
    va_end(args);
    tud_cdc_write(UserTxBufferFS, length);
}

void uart_printf(const char *format, ...)
{
    va_list args;
    uint32_t length;

    va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
    va_end(args);
    uart_write_blocking(UART_ID, UserTxBufferFS, length);
}

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    // nothing to with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP)
        return true;

    switch (request->bmRequestType_bit.type)
    {
    case TUSB_REQ_TYPE_VENDOR:
        switch (request->bRequest)
        {
        case VENDOR_REQUEST_WEBUSB:
            // match vendor request in BOS descriptor
            // Get landing page url
            return tud_control_xfer(rhport, request, (void *)(uintptr_t)&desc_url, desc_url.bLength);

        case VENDOR_REQUEST_MICROSOFT:
            if (request->wIndex == 7)
            {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len;
                memcpy(&total_len, desc_ms_os_20 + 8, 2);

                return tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_ms_os_20, total_len);
            }
            else
            {
                return false;
            }

        default:
            break;
        }
        break;

    case TUSB_REQ_TYPE_CLASS:
        if (request->bRequest == 0x22)
        {
            // Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to connect and disconnect.
            web_serial_connected = (request->wValue != 0);

            // Always lit LED if connected
            if (web_serial_connected)
            {
                // board_led_write(true);
                // led0_blink_interval_ms = BLINK_ALWAYS_ON;

                tud_vendor_write_str("\r\nWebUSB interface connected\r\r\n");
                tud_vendor_flush();
            }
            else
            {
                // led0_blink_interval_ms = BLINK_MOUNTED;
            }

            // response with status OK
            return tud_control_status(rhport, request);
        }
        break;

    default:
        break;
    }

    // stall unknown request
    return false;
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // led1_blink_interval_ms = BLINK_MOUNTED;
    usb_connected = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    // led1_blink_interval_ms = BLINK_NOT_MOUNTED;
    usb_connected = false;
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)itf;

    // connected
    if (dtr && rts)
    {
        // led1_blink_interval_ms = BLINK_SUSPENDED;
        // print initial message when connected
        // tud_cdc_write_str("\r\nTinyUSB WebUSB device example\r\r\n");
    }
}
